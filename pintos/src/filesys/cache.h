#ifndef FILESYS_CACHE_H
#define FILESYS_CACHE_H

#include "devices/block.h"
#include "filesys/off_t.h"
#include "threads/synch.h"
#include <stdbool.h>

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

typedef struct cache_stats
{
  unsigned hit;
  unsigned miss;
  unsigned read;
  unsigned write;
} cache_stats_t;

cache_block_t
    cache_blocks[CACHE_BLOCKS_COUNT]; /* cache blocks with size of 64. */
struct list lru_cache_list;      /* cache list with lru replacement policy.*/
struct lock lru_cache_list_lock; /* lock for Synchornization (multiple threads
                                    accessing lru_cache_list).*/

void cache_init (void);
void cache_flush (void);
void reset_cache_stats (void);
void read_cache_block (block_sector_t sector_idx, off_t sector_ofs,
                       void *buffer, off_t bytes_read, int chunk_size);
void write_cache_block (block_sector_t sector_idx, off_t sector_ofs,
                        void *buffer, off_t bytes_written, int chunk_size);
cache_stats_t *get_cache_stats_instance (void);

#endif /* filesys/cache.h */
