#ifndef FILESYS_INODE_H
#define FILESYS_INODE_H

#include "devices/block.h"
#include "threads/synch.h"
#include "filesys/off_t.h"
#include <stdbool.h>

struct bitmap;

#define CACHE_BLOCKS_COUNT 64

typedef struct cache_block
{
  // Element in lru_cache_list.
  struct list_elem elem;
  // Sector number (or NULL when invalid) of disk location.
  block_sector_t sector;
  // cache block is dirty and should be written back on remove from
  // lru_cache_list.
  bool dirty;
  // cached data with size of each block sector.
  char data[BLOCK_SECTOR_SIZE];
  // lock for Synchronization (multiple threads accessing cache_block).
  struct lock lock;
} cache_block_t;

cache_block_t
    cache_blocks[CACHE_BLOCKS_COUNT]; /* cache blocks with size of 64. */
struct list lru_cache_list;      /* cache list with lru replacement policy.*/
struct lock lru_cache_list_lock; /* lock for Synchornization (multiple threads
                                    accessing lru_cache_list).*/

void inode_init (void);
void cache_init (void);
bool inode_create (block_sector_t, off_t);
struct inode *inode_open (block_sector_t);
struct inode *inode_reopen (struct inode *);
block_sector_t inode_get_inumber (const struct inode *);
void inode_close (struct inode *);
void inode_remove (struct inode *);
off_t inode_read_at (struct inode *, void *, off_t size, off_t offset);
off_t inode_write_at (struct inode *, const void *, off_t size, off_t offset);
void inode_deny_write (struct inode *);
void inode_allow_write (struct inode *);
off_t inode_length (const struct inode *);
void read_cache_block (block_sector_t sector_idx, off_t sector_ofs,
                       void *buffer, off_t bytes_read, int chunk_size);
void write_cache_block (block_sector_t sector_idx, off_t sector_ofs,
                        void *buffer, off_t bytes_written, int chunk_size);
cache_block_t *find_cache_block (block_sector_t sector_idx);

#endif /* filesys/inode.h */
