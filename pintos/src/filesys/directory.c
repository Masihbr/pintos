#include "filesys/directory.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "filesys/inode.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include <list.h>
#include <stdio.h>
#include <string.h>

/* A directory. */
struct dir
{
  struct inode *inode; /* Backing store. */
  off_t pos;           /* Current position. */
  struct lock lock;    /* Lock for dir modification. */
};

/* A single directory entry. */
struct dir_entry
{
  block_sector_t inode_sector; /* Sector number of header. */
  char name[NAME_MAX + 1];     /* Null terminated file name. */
  bool in_use;                 /* In use or free? */
};

/* Creates a directory with space for ENTRY_CNT entries in the
   given SECTOR.  Returns true if successful, false on failure. */
bool
dir_create (block_sector_t sector, size_t entry_cnt)
{
  bool result;
  if (result
      = inode_create (sector, entry_cnt * sizeof (struct dir_entry), true))
    {
      struct dir *dir = dir_open (inode_open (sector));
      struct dir_entry e;
      e.inode_sector = sector;
      e.in_use = false;

      inode_aquire_lock (dir->inode);
      if (inode_write_at (dir->inode, &e, sizeof (e), 0) != sizeof (e))
        result = false;

      inode_release_lock (dir->inode);
      dir_close (dir);
    }
  return result;
}

bool
is_abosulte_path (char *path)
{
  return path[0] == '/';
}

/* Extracts a file name part from *SRCP into PART, and updates *SRCP so that
   the next call will return the next file name part. Returns 1 if successful,
   0 at end of string, -1 for a too-long file name part. */
static int
get_next_part (char part[NAME_MAX + 1], const char **srcp)
{
  const char *src = *srcp;
  char *dst = part;

  /* Skip leading slashes.  If it's all slashes, we're done. */
  while (*src == '/')
    src++;
  if (*src == '\0')
    return 0;

  /* Copy up to NAME_MAX character from SRC to DST.  Add null terminator. */
  while (*src != '/' && *src != '\0')
    {
      if (dst < part + NAME_MAX)
        *dst++ = *src;
      else
        return -1;
      src++;
    }
  *dst = '\0';

  /* Advance source pointer. */
  *srcp = src;
  return 1;
}

bool
separate_path_parent_from_filename (char *full_path, char *parent_name,
                                    char *file_name)
{
  if (!full_path || !full_path[0])
    return false;

  int i;
  for (i = strlen (full_path) - 1; i >= 0; i--)
    {
      if (full_path[i] == '/')
        {
          break;
        }
    }

  if (i == -1)
    {
      memcpy (file_name, full_path, strlen (full_path) + 1);
      file_name[strlen (full_path) + 1] = '\0';
    }
  else
    {
      memcpy (parent_name, full_path, i + 1);
      parent_name[i + 1] = '\0';
      memcpy (file_name, full_path + i + 1, strlen (full_path) - i - 1);
      file_name[strlen (full_path) - i - 1] = '\0';
    }

  return true;
}

/* Opens and returns the directory for the given INODE, of which
   it takes ownership.  Returns a null pointer on failure. */
struct dir *
dir_open (struct inode *inode)
{
  struct dir *dir = calloc (1, sizeof *dir);
  if (inode != NULL && dir != NULL)
    {
      dir->inode = inode;
      dir->pos = 0;
      return dir;
    }
  else
    {
      inode_close (inode);
      free (dir);
      return NULL;
    }
}

/* Opens and returns the directory for the given INODE, of which
   it takes ownership.  Returns a null pointer on failure. */
struct dir *
dir_open_path (char *path)
{
  // printf("dir_open_path(%s)\n", path);
  struct dir *current;
  if (is_abosulte_path (path) || !thread_current ()->cwd)
    current = dir_open_root ();
  else
    current = dir_reopen(thread_current ()->cwd);

  char token[NAME_MAX + 1];
  *token = '\0';

  for (struct dir *next; get_next_part (token, &path);)
    {
      struct inode *inode;
      if (!dir_lookup (current, token, &inode))
        {
          dir_close (current);
          return NULL;
        }
      next = dir_open (inode);
      // printf("next = %p\n", next);
      if (!next)
        {
          dir_close (current);
          return NULL;
        }
      dir_close (current);
      current = next;
    }
  if (inode_is_removed (current->inode))
    {
      dir_close (current);
      return NULL;
    }
  // printf("current = %p\n", current);
  return current;
}

/* Opens the root directory and returns a directory for it.
   Return true if successful, false on failure. */
struct dir *
dir_open_root (void)
{
  return dir_open (inode_open (ROOT_DIR_SECTOR));
}

/* Opens and returns a new directory for the same inode as DIR.
   Returns a null pointer on failure. */
struct dir *
dir_reopen (struct dir *dir)
{
  return dir_open (inode_reopen (dir->inode));
}

/* Destroys DIR and frees associated resources. */
void
dir_close (struct dir *dir)
{
  if (dir != NULL)
    {
      inode_close (dir->inode);
      free (dir);
    }
}

/* Returns the inode encapsulated by DIR. */
struct inode *
dir_get_inode (struct dir *dir)
{
  return dir->inode;
}

void
dir_set_pos (struct dir *dir, off_t new_pos)
{
  dir->pos = new_pos;
}

off_t
dir_get_pos (struct dir *dir)
{
  return dir->pos;
}

/* Searches DIR for a file with the given NAME.
   If successful, returns true, sets *EP to the directory entry
   if EP is non-null, and sets *OFSP to the byte offset of the
   directory entry if OFSP is non-null.
   otherwise, returns false and ignores EP and OFSP. */
static bool
lookup (const struct dir *dir, const char *name, struct dir_entry *ep,
        off_t *ofsp)
{
  struct dir_entry e;
  size_t ofs;

  ASSERT (dir != NULL);
  ASSERT (name != NULL);

  for (ofs = 0; inode_read_at (dir->inode, &e, sizeof e, ofs) == sizeof e;
       ofs += sizeof e)
    if (e.in_use && !strcmp (name, e.name))
      {
        if (ep != NULL)
          *ep = e;
        if (ofsp != NULL)
          *ofsp = ofs;
        return true;
      }
  return false;
}

/* Searches DIR for a file with the given NAME
   and returns true if one exists, false otherwise.
   On success, sets *INODE to an inode for the file, otherwise to
   a null pointer.  The caller must close *INODE. */
bool
dir_lookup (const struct dir *dir, const char *name, struct inode **inode)
{
  struct dir_entry e;

  ASSERT (dir != NULL);
  ASSERT (name != NULL);
  // printf("dir_lookup(name=%s)", name);

  if (lookup (dir, name, &e, NULL))
    *inode = inode_open (e.inode_sector);
  else if (strcmp (name, ".") == 0 || strlen (name) == 0)
    *inode = inode_reopen (dir->inode);
  else if (strcmp (name, "..") == 0)
    {
      inode_read_at (dir->inode, &e, sizeof (e), 0);
      *inode = inode_open (e.inode_sector);
    }
  else
    *inode = NULL;
  // printf(" inode=%p\n", inode);

  return *inode != NULL;
}

/* Adds a file named NAME to DIR, which must not already contain a
   file by that name.  The file's inode is in sector
   INODE_SECTOR.
   Returns true if successful, false on failure.
   Fails if NAME is invalid (i.e. too long) or a disk or memory
   error occurs. */
bool
dir_add (struct dir *dir, const char *name, block_sector_t inode_sector,
         bool type_is_dir)
{
  struct dir_entry e;
  off_t ofs;
  bool success = false;

  ASSERT (dir != NULL);
  ASSERT (name != NULL);

  inode_aquire_lock (dir_get_inode (dir));

  /* Check NAME for validity. */
  if (*name == '\0' || strlen (name) > NAME_MAX)
    return false;

  /* Check that NAME is not in use. */
  if (lookup (dir, name, NULL, NULL))
    goto done;

  if (type_is_dir)
    {
      struct dir *cur_dir = dir_open (inode_open (inode_sector));

      if (!cur_dir)
        goto done;

      inode_aquire_lock (dir_get_inode (cur_dir));

      struct dir_entry dir_e;
      dir_e.in_use = false;
      dir_e.inode_sector = inode_get_inumber (dir_get_inode (dir));

      bool write = inode_write_at (cur_dir->inode, &dir_e, sizeof (dir_e), 0)
                   == sizeof (dir_e);

      inode_release_lock (dir_get_inode (cur_dir));
      dir_close (cur_dir);
      if (!write)
        goto done;
    }

  /* Set OFS to offset of free slot.
     If there are no free slots, then it will be set to the
     current end-of-file.

     inode_read_at() will only return a short read at end of file.
     Otherwise, we'd need to verify that we didn't get a short
     read due to something intermittent such as low memory. */
  for (ofs = 0; inode_read_at (dir->inode, &e, sizeof e, ofs) == sizeof e;
       ofs += sizeof e)
    if (!e.in_use)
      break;

  /* Write slot. */
  e.in_use = true;
  strlcpy (e.name, name, sizeof e.name);
  e.inode_sector = inode_sector;
  success = inode_write_at (dir->inode, &e, sizeof e, ofs) == sizeof e;

done:
  inode_release_lock (dir_get_inode (dir));
  return success;
}

bool
check_if_subdir_is_cwd (struct inode *inode)
{
  struct dir *dir = dir_open (inode);
  struct dir_entry e;
  off_t ofs;
  bool result = false;

  inode_aquire_lock (dir_get_inode (dir));

  for (ofs = 0; inode_read_at (dir->inode, &e, sizeof e, ofs) == sizeof e;
       ofs += sizeof e)
    if (e.in_use && strcmp (e.name, ".") == 0)
      {
        result = true;
        break;
      }

  inode_release_lock (dir_get_inode (dir));
  dir_close (dir);
  return result;
}

/* Removes any entry for NAME in DIR.
   Returns true if successful, false on failure,
   which occurs only if there is no file with the given NAME. */
bool
dir_remove (struct dir *dir, const char *name)
{
  struct dir_entry e;
  struct inode *inode = NULL;
  bool success = false;
  off_t ofs;

  ASSERT (dir != NULL);
  ASSERT (name != NULL);

  inode_aquire_lock (dir_get_inode (dir));

  // char name2[NAME_MAX + 1];
  // while (dir_readdir (dir, name2))
  //   {
  //     printf ("dir_readdir(%p, %s)\n", dir, name2);
  //   }

  /* Find directory entry. */
  if (!lookup (dir, name, &e, &ofs))
    {
      // printf("lookup(%p, %s) failed\n", dir, name);
      goto done;
    }

  /* Open inode. */
  inode = inode_open (e.inode_sector);
  if (inode == NULL || inode_open_count (inode) > 1)
    goto done;

  /* If the dir to be deleted is or has cwd. */
  if (inode_is_dir (inode) && thread_current ()->cwd)
    {
      if (check_if_subdir_is_cwd (inode))
        {
          success = false;
          goto done;
        }

      struct dir_entry e;
      off_t ofs;

      success = true;
      for (ofs = 1 * sizeof e;
           inode_read_at (dir->inode, &e, sizeof e, ofs) == sizeof e;
           ofs += sizeof e)
        if (e.in_use)
          {
            success = false;
            break;
          }
      if (!success) 
        goto done;
    }

  /* Erase directory entry. */
  e.in_use = false;
  if (inode_write_at (dir->inode, &e, sizeof e, ofs) != sizeof e)
    goto done;

  /* Remove inode. */
  inode_remove (inode);
  success = true;

done:
  inode_release_lock (dir_get_inode (dir));
  inode_close (inode);
  return success;
}

/* Reads the next directory entry in DIR and stores the name in
   NAME.  Returns true if successful, false if the directory
   contains no more entries. */
bool
dir_readdir (struct dir *dir, char name[NAME_MAX + 1])
{
  struct dir_entry e;

  while (inode_read_at (dir->inode, &e, sizeof e, dir->pos) == sizeof e)
    {
      dir->pos += sizeof e;
      if (e.in_use)
        {
          strlcpy (name, e.name, NAME_MAX + 1);
          return true;
        }
    }
  return false;
}
