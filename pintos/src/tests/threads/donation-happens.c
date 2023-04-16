
#include "tests/lib.h"
#include <threads/synch.h>
#include <threads/thread.h>

struct lock lock;
struct semaphore semaphore;

bool t1_before_t2 = false;

void t1_func ();
void t2_func ();
void t3_func ();

void
test_donation_happens (void)
{
  thread_set_priority (0);

  lock_init (&lock);
  sema_init (&semaphore, 0);

  struct thread t1, t2, t3;

  thread_create ("t1", 2, t1_func, NULL);
  thread_yield ();
  thread_create ("t2", 31, t2_func, NULL);
  thread_yield ();
  thread_create ("t3", 63, t3_func, NULL);
  thread_yield ();
  sema_up (&semaphore);
}

void
t1_func ()
{
  lock_acquire (&lock);
  sema_down (&semaphore);
  t1_before_t2 = true;
  sema_up (&semaphore);
  lock_release (&lock);
}

void
t2_func ()
{
  sema_down (&semaphore);
  CHECK (t1_before_t2, "t1_before_t2 should be true, actually %s",
         t1_before_t2);
  sema_up (&semaphore);
}

void
t3_func ()
{
  lock_acquire (&lock);
  lock_release (&lock);
}