#include "userprog/syscall.h"
#include "filesys/cache.h"
#include "filesys/directory.h"
#include "filesys/filesys.h"
#include "filesys/inode.h"
#include "filesys/file.h"
#include "lib/stdio.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include <stdio.h>
#include <syscall-nr.h>

static void syscall_handler (struct intr_frame *);

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

bool
args_are_valid (uint32_t *args)
{
  if (!is_block_valid (args, sizeof (uint32_t)))
    return false;

  switch (args[0])
    {
    case SYS_HALT:
    case SYS_CACHE_HIT:
    case SYS_CACHE_MISS:
    case SYS_CACHE_FLUSH:
    case SYS_CACHE_RESET_STATS:
    case SYS_CACHE_READ:
    case SYS_CACHE_WRITE:
      break;
    case SYS_PRACTICE:
    case SYS_EXIT:
    case SYS_EXEC:
    case SYS_WAIT:
    case SYS_REMOVE:
    case SYS_OPEN:
    case SYS_FILESIZE:
    case SYS_TELL:
    case SYS_CLOSE:
    case SYS_CHDIR:
    case SYS_MKDIR:
    case SYS_ISDIR:
    case SYS_INUMBER:
      if (!is_block_valid (args + 1, sizeof (uint32_t)))
        return false;

    case SYS_CREATE:
    case SYS_SEEK:
    case SYS_READDIR:
      if (!is_block_valid (args + 2, sizeof (uint32_t)))
        return false;

    case SYS_READ:
    case SYS_WRITE:
      if (!is_block_valid (args + 3, sizeof (uint32_t)))
        return false;
      break;
    default:
      return false;
    }
  return true;
}

static void
exit (struct intr_frame *f, int exit_code)
{
  f->eax = exit_code;
  status_current ()->return_value = exit_code;
  thread_exit ();
}

static void
syscall_handler (struct intr_frame *f)
{
  uint32_t *args = ((uint32_t *) f->esp);

  /*
   * The following print statement, if uncommented, will print out the syscall
   * number whenever a process enters a system call. You might find it useful
   * when debugging. It will cause tests to fail, however, so you should not
   * include it in your final submission.
   */

  // printf("System call number: %d\n", args[0]);

  if (!args_are_valid (args))
    exit (f, -1);

  if (args[0] == SYS_EXIT)
    exit (f, args[1]);

  else if (args[0] == SYS_PRACTICE)
    {
      int num = args[1];
      f->eax = num + 1;
    }

  else if (args[0] == SYS_HALT)
    {
      shutdown_power_off ();
    }

  else if (args[0] == SYS_EXEC)
    {
      char *cmd = args[1];
      if (!is_cmd_valid (cmd))
        exit (f, -1);
      else
        f->eax = process_execute (cmd);
    }

  else if (args[0] == SYS_WAIT)
    {
      int pid = args[1];
      f->eax = process_wait (pid);
    }

  else if (args[0] == SYS_WRITE)
    {
      int fd = (int) args[1];
      char *buffer = (char *) args[2];
      unsigned size = (unsigned) args[3];
      if (fd == STDIN_FILENO || !is_block_valid (buffer, size))
        exit (f, -1);
      else if (fd == STDOUT_FILENO)
        putbuf (buffer, (f->eax = size));
      else
        {
          struct file_t *file = find_file (fd);
          if (file == NULL)
            exit (f, -1);
          else if (filesys_is_dir (file->f))
            f->eax = -1;
          else
            {
              file_aquire_lock (file->f);
              f->eax = file ? file_write (file->f, buffer, size) : -1;
              file_release_lock (file->f);
            }
        }
    }

  else if (args[0] == SYS_READ)
    {
      int fd = (int) args[1];
      char *buffer = (char *) args[2];
      unsigned size = (unsigned) args[3];

      if (fd == STDOUT_FILENO || !is_block_valid (buffer, size))
        exit (f, -1);
      else if (fd == STDIN_FILENO)
        {
          for (f->eax = 0; f->eax < size; f->eax++)
            {
              *(buffer + f->eax) = input_getc ();
              if (*(buffer + f->eax) == '\n')
                break;
            }
        }
      else
        {
          struct file_t *file = find_file (fd);
          if (file == NULL)
            exit (f, -1);
          else
            {
              file_aquire_lock (file->f);
              f->eax = file ? file_read (file->f, buffer, size) : -1;
              file_release_lock (file->f);
            }
        }
    }

  else if (args[0] == SYS_CREATE)
    {
      char *file_name = (char *) args[1];
      int32_t initial_size = (int32_t) args[2];

      if (!is_ptr_valid (file_name)
          || !is_block_valid (file_name, strlen (file_name) + 1))
        exit (f, -1);
      else
        f->eax = filesys_create (file_name, initial_size, false);
    }

  else if (args[0] == SYS_REMOVE)
    {
      char *file_name = (char *) args[1];

      if (!is_ptr_valid (file_name)
          || !is_block_valid (file_name, strlen (file_name) + 1))
        exit (f, -1);
      else
        f->eax = filesys_remove (file_name);
    }

  else if (args[0] == SYS_OPEN)
    {
      char *file_name = (char *) args[1];
      if (!is_ptr_valid (file_name)
          || !is_block_valid (file_name, strlen (file_name) + 1))
        exit (f, -1);
      else
        {
          struct file *file = filesys_open (file_name);
          if (file == NULL)
            f->eax = -1;
          else
            {
              int fd = thread_current ()->next_fd++;

              struct file_t *ft = malloc (sizeof (struct file_t *));
              ft->fd = fd;
              ft->f = file;

              list_push_back (&thread_current ()->file_descs, &ft->elem);

              f->eax = fd;
            }
        }
    }

  else if (args[0] == SYS_CLOSE)
    {
      int fd = args[1];
      struct file_t *file = find_file (fd);
      if (file == NULL)
        exit (f, -1);
      else
        {
          file_close (file->f);
          list_remove (&file->elem);
        }
    }

  else if (args[0] == SYS_FILESIZE)
    {
      int fd = args[1];
      struct file_t *file = find_file (fd);
      if (file == NULL)
        exit (f, -1);
      else
        {
          file_aquire_lock (file->f);
          f->eax = file ? file_length (file->f) : -1;
          file_release_lock (file->f);
        }
    }

  else if (args[0] == SYS_SEEK)
    {
      int fd = args[1];
      off_t pos = args[2];
      struct file_t *file = find_file (fd);
      if (file == NULL)
        exit (f, -1);
      else
        {
          file_aquire_lock (file->f);
          file_seek (file->f, pos);
          file_release_lock (file->f);
        }
    }

  else if (args[0] == SYS_TELL)
    {
      int fd = args[1];
      struct file_t *file = find_file (fd);
      if (file == NULL)
        exit (f, -1);
      else
        {
          file_aquire_lock (file->f);
          f->eax = file ? file_tell (file->f) : -1;
          file_release_lock (file->f);
        }
    }

  else if (args[0] == SYS_CHDIR)
    {
      char *path = args[1];
      f->eax = thread_chdir (path);
    }

  else if (args[0] == SYS_MKDIR)
    {
      char *path = args[1];
      f->eax = strlen (path) > 0 && filesys_create (path, 0, true);
    }

  else if (args[0] == SYS_READDIR)
    {
      int fd = args[1];
      char *buf = args[2];
      struct file_t *file = find_file (fd);
      if (!file || !filesys_is_dir (file->f))
        exit (f, -1);
      else
        f->eax = next_dir_entry (file->f, buf);
    }

  else if (args[0] == SYS_ISDIR)
    {
      int fd = args[1];
      struct file_t *file = find_file (fd);
      if (file == NULL)
        f->eax = false;
      else
        f->eax = filesys_is_dir (file->f);
    }

  else if (args[0] == SYS_INUMBER)
    {
      int fd = args[1];
      struct file_t *file = find_file (fd);
      if (file == NULL)
        f->eax = 0;
      else
        f->eax = file_get_inumber (file->f);
    }

  else if (args[0] == SYS_CACHE_HIT)
    {
      f->eax = get_cache_stats_instance ()->hit;
    }
  else if (args[0] == SYS_CACHE_MISS)
    {
      f->eax = get_cache_stats_instance ()->miss;
    }
  else if (args[0] == SYS_CACHE_READ)
    {
      f->eax = get_cache_stats_instance ()->read;
    }
  else if (args[0] == SYS_CACHE_WRITE)
    {
      f->eax = get_cache_stats_instance ()->write;
    }
  else if (args[0] == SYS_CACHE_FLUSH)
    {
      cache_flush ();
    }
  else if (args[0] == SYS_CACHE_RESET_STATS)
    {
      reset_cache_stats ();
    }
  else
    {
      printf ("UNIMPLEMENTED SYSCALL: %d\n", args[0]);
    }
}
