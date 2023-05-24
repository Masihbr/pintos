#ifndef FILESYS_FILESYS_H
#define FILESYS_FILESYS_H

#include <stdbool.h>
#include "filesys/off_t.h"

/* Sectors of system file inodes. */
#define FREE_MAP_SECTOR 0       /* Free map file inode sector. */
#define ROOT_DIR_SECTOR 1       /* Root directory file inode sector. */

/* Block device that contains the file system. */
struct block *fs_device;

void filesys_init (bool format);
void filesys_done (void);
bool filesys_create (const char *name, off_t initial_size, bool type_is_dir);
struct file *filesys_open (const char *name);
bool filesys_remove (const char *name);
bool filesys_is_dir (struct file *file);
bool next_dir_entry (struct file *, char *);
int file_get_inumber (struct file *);
void file_aquire_lock(struct file*);
void file_release_lock(struct file*);

#endif /* filesys/filesys.h */
