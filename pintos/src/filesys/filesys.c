#include "filesys/filesys.h"
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include "filesys/file.h"
#include "filesys/free-map.h"
#include "filesys/inode.h"
#include "filesys/cache.h"
#include "filesys/directory.h"

/* Partition that contains the file system. */
struct block *fs_device;

static void do_format (void);

/* Initializes the file system module.
   If FORMAT is true, reformats the file system. */
void
filesys_init (bool format)
{
  fs_device = block_get_role (BLOCK_FILESYS);
  if (fs_device == NULL)
    PANIC ("No file system device found, can't initialize file system.");

  cache_init ();
  inode_init ();
  free_map_init ();

  if (format)
    do_format ();

  free_map_open ();
}

/* Shuts down the file system module, writing any unwritten data
   to disk. */
void
filesys_done (void)
{
  free_map_close ();
  cache_flush ();
}

/* Creates a file named NAME with the given INITIAL_SIZE.
   Returns true if successful, false otherwise.
   Fails if a file named NAME already exists,
   or if internal memory allocation fails. */
bool
filesys_create (const char *name, off_t initial_size, bool type_is_dir)
{
  block_sector_t inode_sector = 0;
  char parent_name[strlen (name) + 1], file_name[strlen (name) + 1];
  parent_name[0] = file_name[0] = NULL;
  bool seperate_result = seperate_path_parent (name, parent_name, file_name);
  struct dir *dir = dir_open_path (parent_name);
  bool success = (seperate_result
                  && dir
                  && free_map_allocate (1, &inode_sector)
                  && inode_create (inode_sector, initial_size, type_is_dir)
                  && dir_add (dir, file_name, inode_sector, type_is_dir));
  if (!success && inode_sector != 0)
    free_map_release (inode_sector, 1);
  dir_close (dir);

  return success;
}

/* Opens the file with the given NAME.
   Returns the new file if successful or a null pointer
   otherwise.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
struct file *
filesys_open (const char *name)
{
  char parent_name[strlen (name) + 1], file_name[NAME_MAX + 1];
  parent_name[0] = file_name[0] = NULL;
  bool seperate_result = seperate_path_parent (name, parent_name, file_name);
  struct dir *dir = dir_open_path (parent_name);
  struct inode *inode = NULL;

  if (!dir || !seperate_result)
    return NULL;

  if (strlen (file_name) == 0)
    inode = dir_get_inode (dir);
  else
    {
      dir_lookup (dir, file_name, &inode);
      dir_close (dir);
    }

  return file_open (inode);
}

/* Deletes the file named NAME.
   Returns true if successful, false on failure.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
bool
filesys_remove (const char *name)
{
  char parent_name[strlen (name) + 1], file_name[NAME_MAX + 1];
  parent_name[0] = file_name[0] = NULL;
  seperate_path_parent (name, parent_name, file_name);
  struct dir *dir = dir_open_path (parent_name);
  bool success = !dir && dir_remove (dir, file_name);
  dir_close (dir);

  return success;
}

bool
filesys_is_dir (struct file *file)
{
  return file_is_dir(file);
}

/* Formats the file system. */
static void
do_format (void)
{
  printf ("Formatting file system...");
  free_map_create ();
  if (!dir_create (ROOT_DIR_SECTOR, 16))
    PANIC ("root directory creation failed");
  free_map_close ();
  printf ("done.\n");
}

bool
next_dir_entry (struct file *file, char *buffer)
{
  return dir_readdir(dir_open(file_get_inode(file)), buffer);
}