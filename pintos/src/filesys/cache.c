#include "filesys/cache.h"
#include "filesys/filesys.h"
#include "threads/synch.h"
#include <debug.h>
#include <list.h>
#include <stdio.h>
#include <string.h>

static cache_stats_t cache_stats = { 0, 0, 0, 0 };

cache_stats_t *
get_cache_stats_instance (void)
{
  return &cache_stats;
}

/* Initializes the cache module. */
void
cache_init (void)
{
  lock_init (&lru_cache_list_lock);
  list_init (&lru_cache_list);
  for (int i = 0; i < CACHE_BLOCKS_COUNT; i++)
    {
      cache_blocks[i].dirty = false;
      cache_blocks[i].sector = NULL;
      lock_init (&(cache_blocks[i].lock));
      list_push_back (&lru_cache_list, &(cache_blocks[i].elem));
    }
}

/* empty cache */
void
cache_flush (void)
{
  struct list_elem *e;
  cache_block_t *cache_block;
  /* empty cache */
  lock_acquire (&lru_cache_list_lock);
  for (e = list_begin (&lru_cache_list); e != list_end (&lru_cache_list);
       e = list_next (e))
    {
      cache_block = list_entry (e, cache_block_t, elem);
      if (cache_block->dirty)
        {
          get_cache_stats_instance ()->write++;
          block_write (fs_device, cache_block->sector, cache_block->data);
        }
      cache_block->dirty = false;
      cache_block->sector = NULL;
    }
  lock_release (&lru_cache_list_lock);
}

/* reset cache stats */
void 
reset_cache_stats (void) {
  get_cache_stats_instance ()->hit = 0;
  get_cache_stats_instance ()->miss = 0;
  get_cache_stats_instance ()->read = 0;
  get_cache_stats_instance ()->write = 0;
}

/* find, get, replace cache block */
static cache_block_t *
find_cache_block (block_sector_t sector_idx, bool do_read)
{
  struct list_elem *e;
  cache_block_t *cache_block;
  lock_acquire (&lru_cache_list_lock);
  /* cache hit */
  for (e = list_begin (&lru_cache_list); e != list_end (&lru_cache_list);
       e = list_next (e))
    {
      cache_block = list_entry (e, cache_block_t, elem);
      if (cache_block->sector == sector_idx)
        {
          get_cache_stats_instance ()->hit++;
          list_remove (&cache_block->elem);
          list_push_front (&lru_cache_list, &(cache_block->elem));
          lock_release (&lru_cache_list_lock);
          return cache_block;
        }
    }
  /* cache miss */
  get_cache_stats_instance ()->miss++;
  cache_block = list_entry (list_back (&lru_cache_list), cache_block_t, elem);
  if (cache_block->dirty)
    {
      get_cache_stats_instance ()->write++;
      block_write (fs_device, cache_block->sector, cache_block->data);
    }
  if (do_read)
    {
      get_cache_stats_instance ()->read++;
      // printf("block_read (fs_device, sector_idx, cache_block->data);\n");
      block_read (fs_device, sector_idx, cache_block->data);
    }
  cache_block->sector = sector_idx;
  cache_block->dirty = false;
  list_remove (&(cache_block->elem));
  list_push_front (&lru_cache_list, &(cache_block->elem));
  lock_release (&lru_cache_list_lock);
  return cache_block;
}

/* Read buffer cache of disk sector to memory buffer */
void
read_cache_block (block_sector_t sector_idx, off_t sector_ofs, void *buffer,
                  off_t bytes_read, int chunk_size)
{
  // printf("read_cache_block (sector_idx=%d, sector_ofs=%d)\n", sector_idx, sector_ofs);
  cache_block_t *cache_block = find_cache_block (sector_idx, true);
  // printf("read_cache_block: cache_block=%p\n", cache_block);
  ASSERT (cache_block != NULL);
  lock_acquire (&(cache_block->lock));
  memcpy (buffer + bytes_read, cache_block->data + sector_ofs, chunk_size);
  // printf("read_cache_block: buffer + bytes_read=%d\n", buffer + bytes_read);
  lock_release (&(cache_block->lock));
}

/* write memory buffer to buffer cache of disk sector */
void
write_cache_block (block_sector_t sector_idx, off_t sector_ofs, void *buffer,
                   off_t bytes_written, int chunk_size)
{
  // printf("write_cache_block (sector_idx=%d, sector_ofs=%d)\n", sector_idx, sector_ofs);
  bool do_read = !(sector_ofs == 0 && chunk_size == BLOCK_SECTOR_SIZE);
  // printf("write_cache_block: do_read=%d\n", do_read);
  cache_block_t *cache_block = find_cache_block (sector_idx, do_read);
  // printf("write_cache_block: cache_block=%p\n", cache_block);
  ASSERT (cache_block != NULL);
  lock_acquire (&(cache_block->lock));
  memcpy (cache_block->data + sector_ofs, buffer + bytes_written, chunk_size);
  // printf("read_cache_block: cache_block->data + sector_ofs=%d\n", cache_block->data + sector_ofs);
  cache_block->dirty = true;
  lock_release (&(cache_block->lock));
}
