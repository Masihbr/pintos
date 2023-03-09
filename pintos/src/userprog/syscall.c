#include "userprog/syscall.h"
#include "filesys/filesys.h"
#include "lib/stdio.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
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
      if (!is_block_valid (args + 1, sizeof (uint32_t)))
        return false;

    case SYS_CREATE:
    case SYS_SEEK:
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
  struct thread *cur = thread_current ();
  f->eax = exit_code;
  cur->return_value = exit_code;
  printf ("%s: exit(%d)\n", cur->name, cur->return_value);
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
      if (fd == STDOUT_FILENO)
        putbuf (buffer, (f->eax = size));
      else if (fd == STDIN_FILENO)
        exit (f, -1);
      else
        {
          struct file_t *file = find_file (fd);
          f->eax = file ? file_write (file->f, buffer, size) : -1;
        }
    }

  else if (args[0] == SYS_READ)
    {
      int fd = (int) args[1];
      char *buffer = (char *) args[2];
      unsigned size = (unsigned) args[3];
      if (fd == STDIN_FILENO)
        {
          for (f->eax = 0; f->eax < size; f->eax++)
            {
              *(buffer + f->eax) = input_getc ();
              if (*(buffer + f->eax) == '\n')
                break;
            }
        }
      else if (fd == STDOUT_FILENO)
        exit (f, -1);
      else
        {
          struct file_t *file = find_file (fd);
          f->eax = file ? file_read (file->f, buffer, size) : -1;
        }
    }

  else if (args[0] == SYS_CREATE)
    {
      char *file_name = (char *) args[1];
      int32_t initial_size = (int32_t) args[2];

      if (!is_block_valid (file_name, strlen (file_name) + 1))
        exit (f, -1);
      else
        f->eax = filesys_create (file_name, initial_size);
    }

  else if (args[0] == SYS_REMOVE)
    {
      char *file_name = (char *) args[1];

      if (!is_block_valid (file_name, strlen (file_name) + 1))
        exit (f, -1);
      else
        f->eax = filesys_remove (file_name);
    }

  else if (args[0] == SYS_OPEN)
    {
      if (!is_block_valid (args[1], sizeof (args[1])))
        exit (f, -1);
      else
        {
          char *file_name = (char *) args[1];
          size_t len = strlen (file_name) + 1;

          if (len == 1)
            f->eax = -1;
          else if (!is_block_valid (file_name, len))
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
    }

  else if (args[0] == SYS_CLOSE)
    {
      int fd = args[1];
      struct file_t *file = find_file (fd);
      if (file == NULL)
        exit (f, -1);
      else
        {
          f->eax = file_close (file->f);
          list_remove(&file->elem);
        }
    }

  else if (args[0] == SYS_FILESIZE)
    {
      int fd = args[1];
      struct file_t *file = find_file (fd);
      f->eax = file ? file_length (file->f) : -1;
    }

  else if (args[0] == SYS_SEEK)
    {
      int fd = args[1];
      off_t pos = args[2];
      struct file_t *file = find_file (fd);
      if (file != NULL)
        file_seek (file->f, pos);
    }

  else if (args[0] == SYS_TELL)
    {
      int fd = args[1];
      struct file_t *file = find_file (fd);
      f->eax = file ? file_tell (file->f) : -1;
    }
  else {
    printf("UNIMPLEMENTED SYSCALL: %d\n", args[0]);
  }
}
