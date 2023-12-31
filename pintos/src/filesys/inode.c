#include "filesys/inode.h"
#include "filesys/filesys.h"
#include "filesys/free-map.h"
#include "threads/malloc.h"
#include "filesys/cache.h"
#include "filesys/directory.h"
#include "filesys/file.h"
#include <debug.h>
#include <list.h>
#include <round.h>
#include <string.h>
#include <stdio.h>

/* Identifies an inode. */
#define INODE_MAGIC 0x494e4f44
#define DIRECT_BLOCKS_COUNT 123
#define INDIRECT_BLOCKS_COUNT 128

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

struct inode_disk *get_inode_disk (const struct inode *);
static bool inode_allocate (struct inode_disk *disk_inode, off_t length);
static bool allocate_sector (block_sector_t *sector_idx);
static bool inode_allocate_indirect (block_sector_t sector_idx, size_t num_sectors_to_allocate);
static bool inode_disk_deallocate (struct inode *inode);
static bool inode_deallocate_indirect (block_sector_t sector_num, size_t num_sectors_to_allocate);

/* On-disk inode.
   Must be exactly BLOCK_SECTOR_SIZE bytes long. */
struct inode_disk
{
  off_t length;   /* File size in bytes. */
  unsigned magic; /* Magic number. */

  /* Unix FFS (Fast File System) */
  block_sector_t direct_blocks[DIRECT_BLOCKS_COUNT]; /* Direct pointers */
  block_sector_t indirect_blocks;                    /* Indirect pointers */
  block_sector_t doubly_indirect_blocks; /* Doubly-indirect pointers */

  int type_is_dir;
};

/* Returns the number of sectors to allocate for an inode SIZE
   bytes long. */
static inline size_t
bytes_to_sectors (off_t size)
{
  return DIV_ROUND_UP (size, BLOCK_SECTOR_SIZE);
}

/* In-memory inode. */
struct inode
  {
    struct list_elem elem;              /* Element in inode list. */
    block_sector_t sector;              /* Sector number of disk location. */
    int open_cnt;                       /* Number of openers. */
    bool removed;                       /* True if deleted, false otherwise. */
    int deny_write_cnt;                 /* 0: writes ok, >0: deny writes. */
    struct lock ilock;                  /* Inode lock. */
  };

struct inode_disk *
get_inode_disk (const struct inode *inode)
{
  ASSERT (inode != NULL);
  struct inode_disk *disk_inode = malloc (sizeof (struct inode_disk));
  read_cache_block (inode->sector, 0, (void *) disk_inode, 0, BLOCK_SECTOR_SIZE);
  return disk_inode;
}

/* Returns the block device sector that contains byte offset POS
   within INODE.
   Returns -1 if INODE does not contain data for a byte at offset
   POS. */
static block_sector_t
byte_to_sector (const struct inode *inode, off_t pos)
{
  ASSERT (inode != NULL);
  block_sector_t sector;
  struct inode_disk *disk_inode = get_inode_disk (inode);

  if (pos >= disk_inode->length)
    {
      free (disk_inode);
      return -1;
    }

  off_t block_index = pos / BLOCK_SECTOR_SIZE;

  /* direct block */
  if (block_index < DIRECT_BLOCKS_COUNT)
    {
      sector = disk_inode->direct_blocks[block_index];
      free (disk_inode);
      return sector;
    }

  /* indirect block */
  else if (block_index < DIRECT_BLOCKS_COUNT + INDIRECT_BLOCKS_COUNT)
    {
      block_sector_t blocks[INDIRECT_BLOCKS_COUNT];
      read_cache_block (disk_inode->indirect_blocks, 0, &blocks, 0, BLOCK_SECTOR_SIZE);
      sector = blocks[block_index - DIRECT_BLOCKS_COUNT];
      free (disk_inode);
      return sector;
    }

  /* double indirect block */
  else
    {
      block_index -= (DIRECT_BLOCKS_COUNT + INDIRECT_BLOCKS_COUNT);
      block_sector_t blocks[INDIRECT_BLOCKS_COUNT];
      int double_indirect_block_index = block_index / INDIRECT_BLOCKS_COUNT;
      int indirect_block_index = block_index % INDIRECT_BLOCKS_COUNT;
      read_cache_block (disk_inode->doubly_indirect_blocks, 0, &blocks, 0, BLOCK_SECTOR_SIZE);
      read_cache_block (blocks[double_indirect_block_index], 0, &blocks, 0, BLOCK_SECTOR_SIZE);
      sector = blocks[indirect_block_index];
      free (disk_inode);
      return sector;
    }
}

/* List of open inodes, so that opening a single inode twice
   returns the same `struct inode'. */
static struct list open_inodes;

/* Initializes the inode module. */
void
inode_init (void)
{
  list_init (&open_inodes);
}

/* Initializes an inode with LENGTH bytes of data and
   writes the new inode to sector SECTOR on the file system
   device.
   Returns true if successful.
   Returns false if memory or disk allocation fails. */
bool
inode_create (block_sector_t sector, off_t length, bool type_is_dir)
{
  struct inode_disk *disk_inode = NULL;
  bool success = false;

  ASSERT (length >= 0);

  /* If this assertion fails, the inode structure is not exactly
     one sector in size, and you should fix that. */
  ASSERT (sizeof *disk_inode == BLOCK_SECTOR_SIZE);

  disk_inode = calloc (1, sizeof *disk_inode);
  if (disk_inode != NULL)
    {
      size_t sectors = bytes_to_sectors (length);
      disk_inode->length = length;
      disk_inode->magic = INODE_MAGIC;
      disk_inode->type_is_dir = type_is_dir ? 1 : 0;
      // printf("inode_create: disk_inode=%p disk_inode->type_is_dir=%d\n", disk_inode, disk_inode->type_is_dir);
      if (inode_allocate (disk_inode, length))
        {
          // printf("inode_create: before write_cache_block (sector, 0, disk_inode, 0, BLOCK_SECTOR_SIZE) %p %d\n", sector, disk_inode);
          write_cache_block (sector, 0, disk_inode, 0, BLOCK_SECTOR_SIZE);
          success = true;
        }
      free (disk_inode);
    }
  return success;
}

/* Reads an inode from SECTOR
   and returns a `struct inode' that contains it.
   Returns a null pointer if memory allocation fails. */
struct inode *
inode_open (block_sector_t sector)
{
  struct list_elem *e;
  struct inode *inode;

  /* Check whether this inode is already open. */
  for (e = list_begin (&open_inodes); e != list_end (&open_inodes);
       e = list_next (e))
    {
      inode = list_entry (e, struct inode, elem);
      if (inode->sector == sector)
        {
          inode_reopen (inode);
          return inode;
        }
    }

  /* Allocate memory. */
  inode = malloc (sizeof *inode);
  if (inode == NULL)
    return NULL;

  /* Initialize. */
  list_push_front (&open_inodes, &inode->elem);
  inode->sector = sector;
  inode->open_cnt = 1;
  inode->deny_write_cnt = 0;
  inode->removed = false;
  lock_init (&inode->ilock);
  return inode;
}

int
inode_open_count (struct inode *inode)
{
  return inode->open_cnt;
}

/* Reopens and returns INODE. */
struct inode *
inode_reopen (struct inode *inode)
{
  if (inode != NULL)
    inode->open_cnt++;
  return inode;
}

/* Returns INODE's inode number. */
block_sector_t
inode_get_inumber (const struct inode *inode)
{
  return inode->sector;
}

/* Closes INODE and writes it to disk.
   If this was the last reference to INODE, frees its memory.
   If INODE was also a removed inode, frees its blocks. */
void
inode_close (struct inode *inode)
{
  /* Ignore null pointer. */
  if (inode == NULL)
    return;

  /* Release resources if this was the last opener. */
  if (--inode->open_cnt == 0)
    {
      /* Remove from inode list and release lock. */
      list_remove (&inode->elem);

      /* Deallocate blocks if removed. */
      if (inode->removed)
        {
          free_map_release (inode->sector, 1);
          inode_disk_deallocate (inode);
        }

      free (inode);
    }
}

/* Marks INODE to be deleted when it is closed by the last caller who
   has it open. */
void
inode_remove (struct inode *inode)
{
  ASSERT (inode != NULL);
  inode->removed = true;
  if (inode_is_dir (inode))
    {
      char name[NAME_MAX + 1];
      struct dir *dir = dir_open (inode);
      while (dir_readdir (dir, name))
        {
          struct file *file = filesys_open (name);
          inode_remove (file_get_inode (file));
          file_close(file);
        }
    }
}

/* Reads SIZE bytes from INODE into BUFFER, starting at position OFFSET.
   Returns the number of bytes actually read, which may be less
   than SIZE if an error occurs or end of file is reached. */
off_t
inode_read_at (struct inode *inode, void *buffer_, off_t size, off_t offset)
{
  uint8_t *buffer = buffer_;
  off_t bytes_read = 0;

  // lock_acquire (&inode->ilock);

  while (size > 0)
    {
      /* Disk sector to read, starting byte offset within sector. */
      block_sector_t sector_idx = byte_to_sector (inode, offset);
      int sector_ofs = offset % BLOCK_SECTOR_SIZE;

      /* Bytes left in inode, bytes left in sector, lesser of the two. */
      off_t inode_left = inode_length (inode) - offset;
      int sector_left = BLOCK_SECTOR_SIZE - sector_ofs;
      int min_left = inode_left < sector_left ? inode_left : sector_left;

      /* Number of bytes to actually copy out of this sector. */
      int chunk_size = size < min_left ? size : min_left;
      if (chunk_size <= 0)
        break;

      read_cache_block (sector_idx, sector_ofs, buffer, bytes_read,
                        chunk_size);

      /* Advance. */
      size -= chunk_size;
      offset += chunk_size;
      bytes_read += chunk_size;
    }
  
  // lock_release (&inode->ilock);

  return bytes_read;
}

/* Writes SIZE bytes from BUFFER into INODE, starting at OFFSET.
   Returns the number of bytes actually written, which may be
   less than SIZE if end of file is reached or an error occurs.
   (Normally a write at end of file would extend the inode, but
   growth is not yet implemented.) */
off_t
inode_write_at (struct inode *inode, const void *buffer_, off_t size,
                off_t offset)
{
  const uint8_t *buffer = buffer_;
  off_t bytes_written = 0;

  if (inode->deny_write_cnt)
    return 0;

  // lock_acquire (&inode->ilock);
  
  /* extend file if needed */
  if (byte_to_sector (inode, offset + size - 1) == (size_t) -1)
    {
      struct inode_disk *disk_inode = get_inode_disk (inode);

      if (!inode_allocate (disk_inode, offset + size))
        {
          free (disk_inode);
          return 0;
        }

      disk_inode->length = offset + size;
      write_cache_block (inode->sector, 0, (void *) disk_inode, 0,
                         BLOCK_SECTOR_SIZE);
      free (disk_inode);
    }

  while (size > 0)
    {
      /* Sector to write, starting byte offset within sector. */
      block_sector_t sector_idx = byte_to_sector (inode, offset);
      int sector_ofs = offset % BLOCK_SECTOR_SIZE;

      /* Bytes left in inode, bytes left in sector, lesser of the two. */
      off_t inode_left = inode_length (inode) - offset;
      int sector_left = BLOCK_SECTOR_SIZE - sector_ofs;
      int min_left = inode_left < sector_left ? inode_left : sector_left;

      /* Number of bytes to actually write into this sector. */
      int chunk_size = size < min_left ? size : min_left;
      if (chunk_size <= 0)
        break;

      write_cache_block (sector_idx, sector_ofs, buffer,
                         bytes_written, chunk_size);

      /* Advance. */
      size -= chunk_size;
      offset += chunk_size;
      bytes_written += chunk_size;
    }
  
  // lock_release (&inode->ilock);

  return bytes_written;
}

/* Disables writes to INODE.
   May be called at most once per inode opener. */
void
inode_deny_write (struct inode *inode)
{
  inode->deny_write_cnt++;
  ASSERT (inode->deny_write_cnt <= inode->open_cnt);
}

/* Re-enables writes to INODE.
   Must be called once by each inode opener who has called
   inode_deny_write() on the inode, before closing the inode. */
void
inode_allow_write (struct inode *inode)
{
  ASSERT (inode->deny_write_cnt > 0);
  ASSERT (inode->deny_write_cnt <= inode->open_cnt);
  inode->deny_write_cnt--;
}

/* Returns the length, in bytes, of INODE's data. */
off_t
inode_length (const struct inode *inode)
{
  ASSERT (inode != NULL);
  struct inode_disk *disk_inode = get_inode_disk (inode);
  off_t len = disk_inode->length;
  free (disk_inode);
  return len;
}

static bool
inode_allocate (struct inode_disk *disk_inode, off_t length)
{
  if (length < 0)
    return false;

  size_t number_of_sectors = bytes_to_sectors (length);
  size_t i;
  for (i = 0; i < MIN (number_of_sectors, DIRECT_BLOCKS_COUNT); i++)
    if (!disk_inode->direct_blocks[i] && !allocate_sector (&disk_inode->direct_blocks[i]))
      return false;

  if (number_of_sectors == i)
    return true;
  
  if (!disk_inode->indirect_blocks && !allocate_sector (&disk_inode->indirect_blocks))
    return false;

  number_of_sectors -= i;
  if (!inode_allocate_indirect (disk_inode->indirect_blocks, number_of_sectors))
    return false;

  if (number_of_sectors > INDIRECT_BLOCKS_COUNT)
    number_of_sectors -= INDIRECT_BLOCKS_COUNT;
  else
    return true;

  number_of_sectors = MIN (number_of_sectors, INDIRECT_BLOCKS_COUNT * INDIRECT_BLOCKS_COUNT);

  if (!disk_inode->doubly_indirect_blocks && !allocate_sector(&disk_inode->doubly_indirect_blocks))
    return false;
  block_sector_t di_blocks[INDIRECT_BLOCKS_COUNT];
  read_cache_block (disk_inode->doubly_indirect_blocks, 0, &di_blocks, 0, BLOCK_SECTOR_SIZE);

  size_t chunk_size;
  size_t required_indirect_sectors = DIV_ROUND_UP (number_of_sectors, INDIRECT_BLOCKS_COUNT);
  for (i = 0; i < required_indirect_sectors; i++)
    {
      chunk_size = MIN (number_of_sectors, INDIRECT_BLOCKS_COUNT);
      if (!di_blocks[i] && !allocate_sector (&di_blocks[i]))
        return false;
      if (!inode_allocate_indirect (di_blocks[i], chunk_size))
        return false;
      number_of_sectors -= chunk_size;
    }

  write_cache_block (disk_inode->doubly_indirect_blocks, 0, &di_blocks, 0, BLOCK_SECTOR_SIZE);

  return number_of_sectors <= INDIRECT_BLOCKS_COUNT * INDIRECT_BLOCKS_COUNT;
}

bool 
inode_allocate_indirect (block_sector_t sector_idx, size_t number_of_sectors)
{
  block_sector_t indirect_blocks[INDIRECT_BLOCKS_COUNT];
  read_cache_block (sector_idx, 0, &indirect_blocks, 0, BLOCK_SECTOR_SIZE);
  for (size_t i = 0; i < MIN (number_of_sectors, INDIRECT_BLOCKS_COUNT); i++)
    if (!indirect_blocks[i] && !allocate_sector (&indirect_blocks[i]))
      return false;
  write_cache_block (sector_idx, 0, &indirect_blocks, 0, BLOCK_SECTOR_SIZE);
  return true;
}

static bool
allocate_sector (block_sector_t *sector_idx)
{
  static char zeros[BLOCK_SECTOR_SIZE];
  if (free_map_allocate(1, sector_idx))
    {
      write_cache_block (*sector_idx, 0, zeros, 0, BLOCK_SECTOR_SIZE);
      return true;
    }
  return false;
}

static bool
inode_disk_deallocate (struct inode *inode)
{
  struct inode_disk *disk_inode = get_inode_disk (inode);

  if (disk_inode == NULL)
    return true;

  size_t number_of_sectors = bytes_to_sectors (disk_inode->length);
  size_t i;
  for (i = 0; i < MIN (number_of_sectors, DIRECT_BLOCKS_COUNT); i++)
    free_map_release (disk_inode->direct_blocks[i],1);

  if (number_of_sectors == i)
    {
      free (disk_inode);
      return true;
    }

  number_of_sectors -= i;
  if (!inode_deallocate_indirect (disk_inode->indirect_blocks, number_of_sectors)) {
    free (disk_inode);
    return false;
  }
  number_of_sectors -= MIN (number_of_sectors, INDIRECT_BLOCKS_COUNT);
  if (number_of_sectors == 0)
    {
      free(disk_inode);
      return true;
    }

  block_sector_t blocks[INDIRECT_BLOCKS_COUNT];
  read_cache_block (disk_inode->doubly_indirect_blocks, 0, &blocks, 0, BLOCK_SECTOR_SIZE);

  if (number_of_sectors > INDIRECT_BLOCKS_COUNT * INDIRECT_BLOCKS_COUNT) {
    free(disk_inode);
    return false;
  }

  size_t no_blocks = DIV_ROUND_UP (number_of_sectors, INDIRECT_BLOCKS_COUNT);
  for (i = 0; i < no_blocks; i++)
    {
      block_sector_t double_indirect_blocks[INDIRECT_BLOCKS_COUNT];
      read_cache_block (blocks[i], 0, &double_indirect_blocks, 0, BLOCK_SECTOR_SIZE);
      size_t j;
      for (j = 0; j < MIN (number_of_sectors, INDIRECT_BLOCKS_COUNT); j++)
        free_map_release (double_indirect_blocks[j], 1); 
      free_map_release (blocks[i], 1);
      number_of_sectors -= j;
    }
  free_map_release (disk_inode->doubly_indirect_blocks, 1);
  free (disk_inode);
  return true;
}

static bool inode_deallocate_indirect (block_sector_t sector_num, size_t number_of_sectors)
{
  block_sector_t indirect_blocks[INDIRECT_BLOCKS_COUNT];
  read_cache_block (sector_num, 0, &indirect_blocks, 0, BLOCK_SECTOR_SIZE);
  for (size_t i = 0; i < MIN (number_of_sectors, INDIRECT_BLOCKS_COUNT); i++)
    free_map_release (indirect_blocks[i], 1);
  free_map_release (sector_num, 1);
  return true;
}

int
inode_is_dir (struct inode *inode)
{
  // printf ("inode_is_dir: inode=%p get_inode_disk (inode)=%p get_inode_disk(inode)->type_is_dir=%d\n", inode, get_inode_disk (inode),
  //         get_inode_disk (inode)->type_is_dir);
  struct inode_disk *inode_disk = get_inode_disk (inode);
  bool is_dir = inode_disk->type_is_dir;
  free (inode_disk);
  return is_dir;
}

bool
inode_is_removed (struct inode *inode)
{
  return inode->removed;
}

void
inode_aquire_lock (struct inode *inode)
{
  lock_acquire (&inode->ilock);
}

void
inode_release_lock (struct inode *inode)
{
  lock_release (&inode->ilock);
}
