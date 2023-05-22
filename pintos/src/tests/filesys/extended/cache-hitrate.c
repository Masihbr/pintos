/* Checks Cache hitrate */

#include "tests/filesys/extended/cache-hitrate.h"
#include "tests/lib.h"
#include "tests/main.h"
#include <random.h>
#include <stdlib.h>
#include <syscall.h>

#define READ_FILE_COUNT 10

const char *test_name = "cache-hitrate";

static char buf[BUF_SIZE];

void
read_file (void)
{
  int fd;
  CHECK ((fd = open (file_name)) > 1, "open \"%s\"", file_name);
  for (size_t ofs = 0; ofs < sizeof buf; ofs += CHUNK_SIZE)
    {
      char read_buffer[CHUNK_SIZE];
      CHECK (read (fd, read_buffer, CHUNK_SIZE) > 0, "read \"%s\"", file_name);
      compare_bytes (&read_buffer, buf + ofs, CHUNK_SIZE, ofs, file_name);
    }
  close (fd);
}

void
test_main (void)
{
  int fd;

  CHECK (create (file_name, sizeof buf), "create \"%s\"", file_name);
  CHECK ((fd = open (file_name)) > 1, "open \"%s\"", file_name);

  random_init (0);
  random_bytes (buf, sizeof buf);
  buf[BUF_SIZE - 1] = NULL;

  msg ("Going to write data in file");

  // write data in file.
  quiet = true;
  for (size_t ofs = 0; ofs < BUF_SIZE; ofs += CHUNK_SIZE)
    CHECK (write (fd, buf + ofs, CHUNK_SIZE) > 0,
           "write %d bytes at offset %zu in \"%s\"", (int) CHUNK_SIZE, ofs,
           file_name);
  quiet = false;
  close (fd);

  // empty cache
  msg ("Going to empty cache and reset stats");
  flush_cache ();
  reset_cache_stats ();

  // read first time
  msg ("Going to read file 1st time");
  quiet = true;
  for (int i = 0; i < READ_FILE_COUNT; i++)
    read_file ();
  quiet = false;

  unsigned cache_hit_empty = count_cache_hit ();
  unsigned cache_miss_empty = count_cache_miss ();

  // msg ("Empty cache stats: hit:%d miss:%d cnt:%d", cache_hit_empty,
  //      cache_miss_empty, cache_hit_empty + cache_miss_empty);

  // read second time
  msg ("Going to read file 2nd time");
  quiet = true;
  for (int i = 0; i < READ_FILE_COUNT; i++)
    read_file ();
  quiet = false;

  unsigned cache_hit_filled = count_cache_hit () - cache_hit_empty;
  unsigned cache_miss_filled = count_cache_miss () - cache_miss_empty;

  // msg ("Filled cache stats: hit:%d miss:%d cnt:%d", cache_hit_filled,
  //      cache_miss_filled, cache_hit_filled + cache_miss_filled);

  CHECK (cache_hit_filled > cache_hit_empty, "Better hitrate after filling cache.");
  CHECK (!(count_cache_write ()), "No write should have been made.");
  remove (file_name);
}
