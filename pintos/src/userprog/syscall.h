#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

/* Lock used for File System READ and WRITE. */
static struct lock fs_lock;

void syscall_init (void);

#endif /* userprog/syscall.h */
