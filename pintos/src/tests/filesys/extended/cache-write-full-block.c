/* In this scenario, we test the ability of our buffer cache to write an entire
block without having to read that block. To For example, if you write 100
kilobytes (200 blocks) in a file, your buffer cache must execute write_block
200 times. call but never request to read from memory (of course request to
read information about the files themselves It is acceptable.) */

#include "tests/filesys/extended/cache-write-full-block.h"
#include "tests/lib.h"
#include "tests/main.h"
#include <random.h>
#include <stdlib.h>
#include <syscall.h>

const char *test_name = "cache-write-full-block";

static char buf[BUF_SIZE];

void
test_main (void)
{
  int fd;

  CHECK (create (file_name, sizeof buf), "create \"%s\"", file_name);
  CHECK ((fd = open (file_name)) > 1, "open \"%s\"", file_name);

  // empty cache
  msg ("Going to empty cache and reset stats.");
  reset_cache ();

  random_init (0);
  random_bytes (buf, sizeof buf);
  buf[BUF_SIZE - 1] = NULL;

  // write data in file.
  msg ("Going to write data in file.");
  quiet = true;
  CHECK ((write (fd, buf, BUF_SIZE) == BUF_SIZE),
         "write %d bytes at offset %zu in \"%s\"", (int) CHUNK_SIZE, 0,
         file_name);
  quiet = false;
  close (fd);

  CHECK (CHUNK_CNT - 2 < (count_cache_write ()) < CHUNK_CNT + 2, "About 200 block writes.");
  CHECK (!(count_cache_read ()), "No read should have been made.");
  remove (file_name);
}
