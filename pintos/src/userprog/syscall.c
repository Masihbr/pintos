#include "userprog/syscall.h"
#include "lib/stdio.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
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
  if (!block_is_valid (args, sizeof(uint32_t)))
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
      if (!block_is_valid (args + 1, sizeof(uint32_t)))
        return false;

    case SYS_CREATE:
    case SYS_SEEK:
      if (!block_is_valid (args + 2, sizeof(uint32_t)))
        return false;

    case SYS_READ:
    case SYS_WRITE:
      if (!block_is_valid (args + 3, sizeof(uint32_t)))
        return false;
      break;
    default:
      return false;
    }
  return true;
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

  /* printf("System call number: %d\n", args[0]); */

  bool args_valid = args_are_valid(args);
  if (!args_valid || args[0] == SYS_EXIT)
    {
      int exit_code = args_valid ? args[1] : -1;
      f->eax = exit_code;
      printf ("%s: exit(%d)\n", &thread_current ()->name, exit_code);
      thread_exit ();
    }

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
      f->eax = process_execute(cmd);
    }

  else if (args[0] == SYS_WAIT)
    {
      int pid = args[1];
      f->eax = process_wait(pid);
    }

  else if (args[0] == SYS_WRITE)
    {
      int fd = (int) args[1];
      char *buffer = (char *) args[2];
      unsigned size = (unsigned) args[3];
      if (fd == STDOUT_FILENO)
        {
          putbuf (buffer, size);
          f->eax = size; /* retrun val */
        }
    }
}
