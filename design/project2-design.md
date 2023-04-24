# سیستم‌های عامل - تمرین گروهی دوم

## مشخصات گروه

>> نام، نام خانوادگی و ایمیل خود را در ادامه وارد کنید.

مسیح بیگی ریزی <masihbr@gmail.com>

درین ستوده <dorrinsotoudeh123@gmail.com>

علی مورکیان <moroukianali@gmail.com>

## مقدمه

>> اگر نکته‌ای درباره فایل‌های سابمیت شده یا برای TAها دارید، لطفا اینجا بیان کنید.

>> اگر از هر منبع برخط یا غیر برخطی به غیر از مستندات Pintos، متن درس، اسلایدهای درس یا نکات گفته شده در کلاس در تمرین گروهی استفاده کرده‌اید، لطفا اینجا آن(ها) را ذکر کنید.

## ساعت زنگ‌دار

### داده ساختارها

>> پرسش اول: تعریف `struct`های جدید، `struct`های تغییر داده شده، متغیرهای گلوبال یا استاتیک، `typedef`ها یا `enumeration`ها را در اینجا آورده و برای هریک در 25 کلمه یا کمتر توضیح بنویسید.

```c
// devices/timer.c

// لیستی از تردهایی که در حالت خواب قرار دارند به ترتیب زمان پایان خواب
struct list sleeping_threads;

// lock برای استفاده از sleeping_threads برای جلوگیری از race condition
struct lock sleeping_lock;


// threads/thread.h

struct thread
{
    ...
    // زمان بیدار شدن ریسه
    int64_t waking_time;
    ...
};
```

### الگوریتم

>> پرسش دوم: به اختصار آن‌چه هنگام صدا زدن تابع `timer_sleep()` رخ می‌دهد و همچنین اثر `timer interrupt handler` را توضیح دهید.

ابتدا زمان بیدار شدن ریسه محاسبه می‌شود(`timer_ticks() + ticks`) و در استراکت thread ذخیره می‌شود.
</br>
سپس ریسه به لیست `sleeping_threads` اضافه می‌شود. برای این‌کار لازم است از قفل `sleeping_lock` استفاده شود و همچنین ریسه باید در محل مناسبی از `sleeping_threads` قرار گیرد تا زمان پایان خواب تردها به ترتیب باشد.
</br>
درنهایت با فراخوانی تابع `thread_block` ریسه به حالت خواب می‌رود
</br>
همچنین در ابتدا باید وقفه‌های سیستم‌عامل متوقف شوند و در انتها دوباره فعال شوند.


در timer interrupt handler باید ابتدا قفل `sleeping_lock` دریافت شود و سپس ریسه‌هایی که باید اجرا شود از لیست `sleeping_threads` حذف می‌شوند و تابع `thread_unblock` فراخوانی می‌شود.


>> پرسش سوم: مراحلی که برای کوتاه کردن زمان صرف‌شده در `timer interrupt handler` صرف می‌شود را نام ببرید.

از آن‌جا که زمان بیدار شدن ریسه‌ها در لیست `sleeping_threads` به ترتیب صعودی مرتب شده است؛ تعداد ریسه‌های مورد بررسی به حداقل می‌رسد. 

### همگام‌سازی

>> پرسش چهارم: هنگامی که چند ریسه به طور همزمان `timer_sleep()` را صدا می‌زنند، چگونه از `race condition` جلوگیری می‌شود؟

همان‌طور که گفته شد دسترسی به `sleeping_threads` مشروط به استفاده از قفل `sleeping_lock` می‌باشد.

>> پرسش پنجم: هنگام صدا زدن `timer_sleep()` اگر یک وقفه ایجاد شود چگونه از `race condition` جلوگیری می‌شود؟

در ابتدای `timer_sleep` وقفه‌ها را غیرفعال می‌کنیم در نتیجه تاثیری در اجرای این تابع نمی‌گذارند.

### منطق

>> پرسش ششم: چرا این طراحی را استفاده کردید؟ برتری طراحی فعلی خود را بر طراحی‌های دیگری که مدنظر داشته‌اید بیان کنید.

استفاده از لیست مرتب‌شده‌ی `sleeping_threads` باعث می‌شود که زمان کمتری را برای پیدا کردن ریسه‌ها در timer interrupt handler صرف کنیم. البته برای این کار زمان بیشتری در `timer_sleep` صرف می‌شود تا لیست را مرتب نگه داریم.

## زمان‌بند اولویت‌دار

### داده ساختارها

>> پرسش اول: تعریف `struct`های جدید، `struct`های تغییر داده شده، متغیرهای گلوبال یا استاتیک، `typedef`ها یا `enumeration`ها را در اینجا آورده و برای هریک در ۲۵ کلمه یا کمتر توضیح بنویسید.

>> پرسش دوم: داده‌ساختارهایی که برای اجرای `priority donation` استفاده شده‌است را توضیح دهید. (می‌توانید تصویر نیز قرار دهید)
``` diff
// thread.h:
struct thread
  {
    ...

    int priority;                       /* Priority. */
+    int effective_priority;             /* Effective priority. (init at `priority`. can be donated to) */

+    struct list acquired_locks;
+    struct lock *blocking_lock;

    ...
  };

// synch.h:
struct lock
  {
    struct thread *holder;
    struct semaphore semaphore;

+    int max_priority;      // is thread->priority at init
+    struct list_elem elem; // for thread::locks
  };
```

حال هربار یک ترد تابع
`lock_acquire (lock)`
را روی یک قفل فراخوانی میکند، درصورتی که مقدار ترد 
`holder`
آن قفل
دارای
`effective_priority`
کمتری نسب به ترد فعلی ما باشد، مقدار آن به 
`effective_priority`
ترد فعلی تغییر میکند.
علاوه بر این باید مقدار
`blocking_lock`
ترد
`holder`
این قفل نیز بررسی شود و درصورتی که NULL نبود، باید مقدار
`effective_priority`
ترد
`holder`
این قفل نیز بروز شود.

```
thread ta: p=1
    has lock la
thread tb: p=2
    has lock lb
    aquire la (has to wait for ta)
thread tc: p=3
    aquire lb

// Using blocking_lock and it's holder we can traverse
// through threads until blocking_lock is NULL
// will donate priority if needed on the way
// priority after donation will be like:

thread ta: p=3
    has lock la
thread tb: p=3
    has lock lb
    aquire la (has to wait for ta)
thread tc: p=3
    aquire lb
```
هنگام رهایی یک قفل با فراخوانی تابع 
`lock_release (lock)`
باید قفل را از لیست 
`acquired_locks`
حذف کرده و مقدار
`effective_priority`
ترد قعلی (تردی که درحال release قفل است)
باید باتوجه به ماکسیمم 
`max_priority`
قفل‌های موجود در لیست
`acquired_locks`
بروز میکنیم.

### الگوریتم

>> پرسش سوم: چگونه مطمئن می‌شوید که ریسه با بیشترین اولویت که منتظر یک قفل، سمافور یا `condition variable` است زودتر از همه بیدار می‌شود؟

توابع زیر همگی برای بیدار‌ کردن ترد منتظر، از سمافور استفاده میکنند. 
بنابراین میتوان با تغییر جزیی در تابع 
`sema_up` 
کاری کنیم که همواره ترد با بیشترین
`effective_priority`
بیدار شود.

``` c
// synch.c

void
sema_up (struct semaphore *sema)
{
  enum intr_level old_level;

  ASSERT (sema != NULL);

  old_level = intr_disable ();
  if (!list_empty (&sema->waiters))
    thread_unblock (list_entry (list_pop_front (&sema->waiters), // change this to remove highest priority thread
                                struct thread, elem));
  sema->value++;
  intr_set_level (old_level);
}

void
lock_release (struct lock *lock)
{
  ASSERT (lock != NULL);
  ASSERT (lock_held_by_current_thread (lock));

  lock->holder = NULL;
  sema_up (&lock->semaphore);
}

void
cond_signal (struct condition *cond, struct lock *lock UNUSED)
{
  ASSERT (cond != NULL);
  ASSERT (lock != NULL);
  ASSERT (!intr_context ());
  ASSERT (lock_held_by_current_thread (lock));

  if (!list_empty (&cond->waiters))
    sema_up (&list_entry (list_pop_front (&cond->waiters),
                          struct semaphore_elem, elem)->semaphore);
}
```

>> پرسش چهارم: مراحلی که هنگام صدازدن `lock_acquire()` منجر به `priority donation` می‌شوند را نام ببرید. دونیشن‌های تو در تو چگونه مدیریت می‌شوند؟

ابتدا مقدار
`effective_priority`
ترد
`holder`
آن قفل را بروز میکنیم.
سپس بررسی میکنیم که ایا این ترد صاحب قفل، خود blocking هست یا خیر
(برای اینکار مقدار
`blocking_lock`
آن را بررسی میکنیم).
اگر نال نبود، مقدار
`effective_priority`
ترد 
`holder`
این قفل را نیز آپدیت میکنیم. (این عملیات بصورت iterative ادامه دارد تا جایی که ترد موردنظر blocking نباشد).
هر بار مقدار 
`max_priority`
قفل نیز بروز میشود.

>> پرسش پنجم: مراحلی که هنگام صدا زدن `lock_release()` روی یک قفل که یک ریسه با اولویت بالا منتظر آن است، رخ می‌دهد را نام ببرید.

مقدار 
`max_priority`
این قفل باید باتوجه به 
`effective_priority`
ترد‌های موجود در
`lock->semaphore->waiters`
بروز شود. همچنین مقدار
`effective_priority`
ترد فعلی نیز باید برابر ماکسیمم مقدار
`max_priority`
بین قفل‌های موجود در
`acquired_locks`
ش بشود.
درصورتی که
`acquired_locks`
خالی بود
`effective_priority`
به
`priority`
بازمی‌گردد.

### همگام‌سازی

>> پرسش ششم: یک شرایط احتمالی برای رخداد `race condition` در `thread_set_priority` را بیان کنید و توضیح دهید که چگونه پیاده‌سازی شما از رخداد آن جلوگیری می‌کند. آیا می‌توانید با استفاده از یک قفل از رخداد آن جلوگیری کنید؟

وقتی میخواهیم مقدار priority را بخوانیم، چون توابع لازم این کار در sema_up و sema_down استفاده می‌شوند و به دلیل disable شدن intrupt ها context-switch رخ نمی دهد و مقدار priority تغییر نمیابد. در این حالت با race condition  مواجه نیستیم.


### منطق

>> پرسش هفتم: چرا این طراحی را استفاده کردید؟ برتری طراحی فعلی خود را بر طراحی‌های دیگری که مدنظر داشته‌اید بیان کنید.

برای بیدارکردن ترد دارای بالاترین اولویت، این روش هم میتوان استفاده کرد که هنگام افزودن به لیست ترد‌ها از list_insert_sorted استفاده کنیم و هربار اولویت تغییر کرد لیست را مرتب کنیم اما باتوجه به وجود تعداد لیست‌های زیاد اینکار به‌نظرمان وقتگیر و پیچیده امد.

درصورتی که max_prioty را در قفل قرار نمیدادیم فرایند بروزسازی اولویت هنگام release بسیار پیچیده میشد.

## سوالات افزون بر طراحی

>> پرسش هشتم: در کلاس سه صفت مهم ریسه‌ها که سیستم عامل هنگامی که ریسه درحال اجرا نیست را ذخیره می‌کند، بررسی کردیم:‍‍ `program counter` ، ‍‍‍`stack pointer` و `registers`. بررسی کنید که این سه کجا و چگونه در `Pintos` ذخیره می‌شوند؟ مطالعه ‍`switch.S` و تابع ‍`schedule` در فایل `thread.c` می‌تواند مفید باشد.

در فایل switch.S میبینیم که برای تابع switch_thread که در تابع schedule کال می‌شود کد اسمبلی  قرار  دارد. تابع switch_threads مسئول ذخیره سازی وضعیت ترد در حال اجرا cur است و سوییچ به ترد بعدی next است.

برای اینکار این تابع در ابتدا مقادیر رجیستر های ebx, ebp, esi, edi را پوش می‌کند. این رجیستر ها توسط فراخوانی کننده محافظت شده و در انتهای تابع بازیابی می‌شوند. سپس این تابع esp که همان استک پوینتر فعلی است را در خانه حافظه ای که مربوط به نشانگر استک در ترد قدیمی است ذخیره می‌کند. این مهم با استفاده از movl و رجیستر های esp, eax + offset_address_in_edx صورت می‌گیرد.
در انتها پس از پاپ کردن رجیستر های ذخیره شده کنترل با ret به فراخوانی کننده داده ‌می‌شود. 
وقتی که یک intrupt  رخ می‌دهد رجیستر حاوی program counter به طور خودکار در استک ذخیره می‌شود.

```c
static void
schedule (void)
{
  struct thread *cur = running_thread ();
  struct thread *next = next_thread_to_run ();
  struct thread *prev = NULL;

  ASSERT (intr_get_level () == INTR_OFF);
  ASSERT (cur->status != THREAD_RUNNING);
  ASSERT (is_thread (next));

  if (cur != next)
    prev = switch_threads (cur, next);
  thread_schedule_tail (prev);
}
```
```asm
.globl switch_threads
.func switch_threads
switch_threads:
	# Save caller's register state.
	#
	# Note that the SVR4 ABI allows us to destroy %eax, %ecx, %edx,
	# but requires us to preserve %ebx, %ebp, %esi, %edi.  See
	# [SysV-ABI-386] pages 3-11 and 3-12 for details.
	#
	# This stack frame must match the one set up by thread_create()
	# in size.
	pushl %ebx
	pushl %ebp
	pushl %esi
	pushl %edi

	# Get offsetof (struct thread, stack).
.globl thread_stack_ofs
	mov thread_stack_ofs, %edx

	# Save current stack pointer to old thread's stack, if any.
	movl SWITCH_CUR(%esp), %eax
	movl %esp, (%eax,%edx,1)

	# Restore stack pointer from new thread's stack.
	movl SWITCH_NEXT(%esp), %ecx
	movl (%ecx,%edx,1), %esp

	# Restore caller's register state.
	popl %edi
	popl %esi
	popl %ebp
	popl %ebx
        ret
.endfunc
```

>> پرسش نهم: وقتی یک ریسه‌ی هسته در ‍`Pintos` تابع `thread_exit` را صدا می‌زند، کجا و به چه ترتیبی صفحه شامل پشته و `TCB` یا `struct thread` آزاد می‌شود؟ چرا این حافظه را نمی‌توانیم به کمک صدازدن تابع ‍`palloc_free_page` داخل تابع ‍`thread_exit` آزاد کنیم؟

همونطور که میبینید در آخر تابع
`thread_exit`
تابع 
`schedule`
فراخوانی میشود که باتوجه به سوال قبلی در این تابع مقادیر
stack pointer، program counter و registers
در پشته ذخیره میشود و درنهایت تابع 
`thread_schedule_tail (prev)`
صدا میشود و
`palloc_free_page`
را بر روی این تابع فراخوانی میکند.

بنابراین درصورتی که 
`palloc_free_page`
در 
`thread_exit`
صدا شود، چه قبل از
`schedule`
و چه بعد از آن مشکل‌ساز خواهد بود.

```c
// thread.c

void
thread_exit (void)
{
  ASSERT (!intr_context ());

#ifdef USERPROG
  process_exit ();
#endif

  /* Remove thread from all threads list, set our status to dying,
     and schedule another process.  That process will destroy us
     when it calls thread_schedule_tail(). */
  intr_disable ();

  struct thread *cur = thread_current ();
  struct status_t *status = status_current ();
  printf ("%s: exit(%d)\n", cur->name, status->return_value);
  sema_up (&cur->sema);
  status->finished = true;
  list_remove (&cur->allelem);
  cur->status = THREAD_DYING;
  schedule ();
  NOT_REACHED ();
}

static void
schedule (void)
{
  struct thread *cur = running_thread ();
  struct thread *next = next_thread_to_run ();
  struct thread *prev = NULL;

  ASSERT (intr_get_level () == INTR_OFF);
  ASSERT (cur->status != THREAD_RUNNING);
  ASSERT (is_thread (next));

  if (cur != next)
    prev = switch_threads (cur, next);
  thread_schedule_tail (prev);
}

void
thread_schedule_tail (struct thread *prev)
{
  struct thread *cur = running_thread ();

  ASSERT (intr_get_level () == INTR_OFF);

  /* Mark us as running. */
  cur->status = THREAD_RUNNING;

  /* Start new time slice. */
  thread_ticks = 0;

#ifdef USERPROG
  /* Activate the new address space. */
  process_activate ();
#endif

  /* If the thread we switched from is dying, destroy its struct
     thread.  This must happen late so that thread_exit() doesn't
     pull out the rug under itself.  (We don't free
     initial_thread because its memory was not obtained via
     palloc().) */
  if (prev != NULL && prev->status == THREAD_DYING && prev != initial_thread)
    {
      ASSERT (prev != cur);
      palloc_free_page (prev);
    }
}
```

>> پرسش دهم: زمانی که تابع ‍`thread_tick` توسط `timer interrupt handler` صدا زده می‌شود، در کدام پشته اجرا می‌شود؟

> Called by the timer interrupt handler at each timer tick. Thus, this function runs in an external interrupt context.

از آن‌جا که این تابع در تابع `timer_interrupt` فراخوانی می‌شود، بنابراین در حالت هسته اجرا می‌شود و از پشته‌ی هسته اجرا می‌شود.

>> پرسش یازدهم: یک پیاده‌سازی کاملا کاربردی و درست این پروژه را در نظر بگیرید که فقط یک مشکل درون تابع ‍`sema_up()` دارد. با توجه به نیازمندی‌های پروژه سمافورها(و سایر متغیرهای به‌هنگام‌سازی) باید ریسه‌های با اولویت بالاتر را بر ریسه‌های با اولویت پایین‌تر ترجیح دهند. با این حال پیاده‌سازی ریسه‌های با اولویت بالاتر را براساس اولویت مبنا `Base Priority` به جای اولویت موثر ‍`Effective Priority` انتخاب می‌کند. اساسا اهدای اولویت زمانی که سمافور تصمیم می‌گیرد که کدام ریسه رفع مسدودیت شود، تاثیر داده نمی‌شود. تستی طراحی کنید که وجود این باگ را اثبات کند. تست‌های `Pintos` شامل کد معمولی در سطح هسته (مانند متغیرها، فراخوانی توابع، جملات شرطی و ...) هستند و می‌توانند متن چاپ کنند و می‌توانیم متن چاپ شده را با خروجی مورد انتظار مقایسه کنیم و اگر متفاوت بودند، وجود مشکل در پیاده‌سازی اثبات می‌شود. شما باید توضیحی درباره این که تست چگونه کار می‌کند، خروجی مورد انتظار و خروجی واقعی آن فراهم کنید.

ترد t1 بعد از yeild شدن ترد اصلی به کار می افتد و lock را میگیرد و سپس در sema_down متوقف می‌شود. اجرا به کد اصلی بر می‌گردد ترد t2 ساخته شده و بعد از yeild تابع مربوطه کال شده و در sema_down گیر می‌کند. باز کنترل به ترد اصلی بر میگردد اینبار t3 ساخته شده و سعی میکند lock را بگیرد اما چون لاک دست ترد t1 است که priority پایین تری دارد ما باید effective_priority این ترد را افزایش دهیم تا لاک ازاد شود. در صورتی که این اتفاق بیوفتد t1 افزایش یابد. و این ترد شروع به اجرا کند و مقدار t1_before_t2 به true تغییر یابد و در t2 عبارت CHECK به درستی مقدار دهی شود.
اما چون priority مورد استفاده قرار میگیرد مقدار t1_before_t2 همان False است و t2 به دلیل priority بیشتر زودتر اجرا خواهد شد و تست فیل می‌شود.

[Test file](../pintos/src/tests/threads/donation-happens.c)

## سوالات نظرسنجی

پاسخ به این سوالات دلخواه است، اما به ما برای بهبود این درس در ادامه کمک خواهد کرد. نظرات خود را آزادانه به ما بگوئید—این سوالات فقط برای سنجش افکار شماست. ممکن است شما بخواهید ارزیابی خود از درس را به صورت ناشناس و در انتهای ترم بیان کنید.

>> به نظر شما، این تمرین گروهی، یا هر کدام از سه وظیفه آن، از نظر دشواری در چه سطحی بود؟ خیلی سخت یا خیلی آسان؟

>> چه مدت زمانی را صرف انجام این تمرین کردید؟ نسبتا زیاد یا خیلی کم؟

>> آیا بعد از کار بر روی یک بخش خاص از این تمرین (هر بخشی)، این احساس در شما به وجود آمد که اکنون یک دید بهتر نسبت به برخی جنبه‌های سیستم عامل دارید؟

>> آیا نکته یا راهنمایی خاصی وجود دارد که بهتر است ما آنها را به توضیحات این تمرین اضافه کنیم تا به دانشجویان ترم های آتی در حل مسائل کمک کند؟

>> متقابلا، آیا راهنمایی نادرستی که منجر به گمراهی شما شود وجود داشته است؟

>> آیا پیشنهادی در مورد دستیاران آموزشی درس، برای همکاری موثرتر با دانشجویان دارید؟

این پیشنهادات میتوانند هم برای تمرین‌های گروهی بعدی همین ترم و هم برای ترم‌های آینده باشد.

>> آیا حرف دیگری دارید؟
