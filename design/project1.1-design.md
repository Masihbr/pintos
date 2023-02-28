# تمرین گروهی ۱.۱ - مستند طراحی

گروه
-----
 > نام و آدرس پست الکترونیکی اعضای گروه را در این قسمت بنویسید.

مسیح بیگی ریزی <masihbr@gmail.com>

درین ستوده <dorrinsotoudeh123@gmail.com>

نام و نام خانوادگی <example@example.com> 

عرفان وهابی <erfanv1@example.com> 

مقدمات
----------
> اگر نکات اضافه‌ای در مورد تمرین یا برای دستیاران آموزشی دارید در این قسمت  بنویسید.

> لطفا در این قسمت تمامی منابعی (غیر از مستندات Pintos، اسلاید‌ها و دیگر منابع درس) را که برای تمرین از آن‌ها استفاده کرده‌اید در این قسمت بنویسید.

پاس‌دادن آرگومان
============
داده‌ساختار‌ها
----------------
> در این قسمت تعریف هر یک از `struct` ها، اعضای `struct` ها، متغیرهای سراسری یا ایستا، `typedef` ها یا `enum` هایی که ایجاد کرده‌اید یا تغییر داده‌اید را بنویسید و دلیل هر کدام را در حداکثر ۲۵ کلمه توضیح دهید.

```c
#define MAX_ARGC 64
#define MAX_ARG_LEN 128
typedef struct process_args
{
  char** argv;
  int argc;
} process_args;
```
از این داده ساختار برای نگهداری از آرایه ای از رشته های ورودی برنامه و تعداد آنها استفاده می‌کنیم. (برگرفته از استراکت process در hw1)

الگوریتم‌ها
------------
> به‌طور خلاصه توضیح دهید چگونه آرگومان‌ها را پردازش کرده‌اید؟ چگونه اعضای `argv[]` را به ترتیب درست در پشته قرار داده‌اید؟ و چگونه از سرریز پشته جلوگیری کرده‌اید؟

```c
int get_argc(char *cmd_line);
char** get_argv(char *cmd_line, int argc);
process_args* get_process_args(char *cmd_line);
bool push_args_in_stack(void **esp, process_args);
```
در تابع start_process با استفاده از تابعی مشابه با getToks در تمرین اول فردی، خط ورودی را به توکن‌ها شکسته و استخراج می‌کنیم. سپس یک استراکت process_args تشکیل داده و با استفاده از آن و در تابع push_args_in_stack (که می‌تواند در تابع load  و یا بعد از آن صدا زده شود) آرگومان‌ها پاس داده‌ خواهد شد.<br/>
برای ایجاد ترتیب درست با توجه به بخش 4.2.6 Program Startup Details و 4.2.5 80x86 Calling Convention
 در منابع داده شده، آرگومان‌ها را از راست به چپ (آخری به اولی) وارد پشته می‌کنیم. بدین صورت که ابتدا مقادیر رشته ها در پشته قرار گرفته، سپس موارد مربوط به align بودن استک انجام ‌می‌شود. سپس باز به صورت آخر به اول آدرس رشته های پوش شده در استک به عنوان pointer های argv در استک قرار می‌گیرد و در نهایت یک pointer به ابتدای argv و argc و return address را در استک قرار ‌می‌دهیم.<br/>
 رشته‌ها با \0 از هم جدا می‌شوند و انتهای آرایه argv نیز یک خانه null است که در پشته زودتر از دیگر خانه‌ها وارد می‌شود.<br/>
 برای جلوگیری از سرریز استک محدودیت‌های لازم روی تعداد و طول هر آرگومان اعمال می‌شود.

منطق طراحی
-----------------
> چرا Pintos به‌جای تابع‌ `strtok()` تابع‌ `strtok_r()` را پیاده‌سازی کرده‌است؟

تابع strtok() از یک بافر ثابت برای پارس کردن رشته استفاده می‌کند. این پیاده سازی هنگامی که دو thread به صورت همزمان از این تابع استفاده کنند باعث race condition شده و مشکل به همراه می‌آورد. این مشکل در خود string.h نیز حل شده و تابع strtok_s به این کتابخانه افزوده شده.
تابع strtok_s مانند strtok_r، یک پوینتر به عنوان ورودی می‌گیرد که همان بافر داینامیک مورد نیاز را تامین می‌کند.

> در Pintos عمل جدا کردن نام فایل از آرگومان‌ها، در داخل کرنل انجام می‌شود. در سیستم عامل‌های برپایه‌ی Unix، این عمل توسط shell انجام می‌شود. حداقل دو مورد از برتری‌های رویکرد Unix را توضیح دهید.

با اینکار بار (overhead) جداسازی رشته‌های ‌آرگومان از نام فایل به شل منتقل می‌شود و پیاده سازی کرنل سبک ‌تر و اجرا سریعتر می‌شود. علاوه بر این هندل کردن این موضوع در کرنل از نظر امنیتی مشکل دارد زیرا پراسس های مختلف به کرنل دسترسی دارند، همچنین نیاز به دست بردن در کد کرنل برای تغییر در چک‌های امنیتی و غیره نیست و صرفا ارتباط شل با کرنل تغییر می‌کند. 

فراخوانی‌های سیستمی
================
داده‌ساختار‌ها
----------------
> در این قسمت تعریف هر یک از `struct` ها، اعضای `struct` ها، متغیرهای سراسری یا ایستا، `typedef` ها یا `enum` هایی که ای.جاد کرده‌اید یا تغییر داده‌اید را بنویسید و دلیل هر کدام را در حداکثر ۲۵ کلمه توضیح دهید.

- ***pintos/src/threads/thread.h***
```c
stuct file_t
  {
    int fd;                             /* File Descriptor id (returned from SYS_OPEN). */
    struct file *f;                     /* The actual file. */
  };
```
```diff
struct thread
  {
    /* Owned by thread.c. */
    tid_t tid;                          /* Thread identifier. */
    enum thread_status status;          /* Thread state. */
    char name[16];                      /* Name (for debugging purposes). */
    uint8_t *stack;                     /* Saved stack pointer. */
    int priority;                       /* Priority. */
    struct list_elem allelem;           /* List element for all threads list. */

    /* Shared between thread.c and synch.c. */
    struct list_elem elem;              /* List element. */

#ifdef USERPROG
+    pid_t pid;                          /* This Thread's Process's identifier. */
+    struct list children;               /* List of this thread's children. */
+    int return_value;                   /* The return value of this thread. */
+    struct semaphore sema;              /* Semaphore for exec and wait. */
+    struct list file_descs;             /* The list of this thread's File Descriptors. */
    /* Owned by userprog/process.c. */
    uint32_t *pagedir;                  /* Page directory. */
#endif

    /* Owned by thread.c. */
    unsigned magic;                     /* Detects stack overflow. */
  };
```
```diff
/* Lock used by allocate_tid(). */
static struct lock tid_lock;

+/* Lock used for File System READ and WRITE. */
+static struct lock fs_lock;
```

> 6. توضیح دهید که توصیف‌کننده‌های فایل چگونه به فایل‌های باز مربوط می‌شوند. آیا این توصیف‌کننده‌ها در کل سیستم‌عامل به‌طور یکتا مشخص می‌شوند یا فقط برای هر پردازه یکتا هستند؟

هر
thread
یک لیستی از
file descriptorهای
مورد استفاده دارد. هر یک از این
file descriptorها
در آن پردازه یکتا هستند اما در کل سیستم‌عامل یکتا نیستند. علاوه بر این، مقادیر 0 و 1 بترتیب برای
`STDIN_FILENO`
و
`STDOUT_FILENO`
رزرو شده است.
همچنین اگر یک فایل توسط یک پردازه چندین بار باز شود، هربار
file descriptor
متفاوتی دریافت میکند. همچنین یک پردازه به
file descriptorهای
پردازه پدر خود دسترسی ندارد و برعکس.

درصورت فراخوانی 
syscall
خواندن، اگر
file descriptor
ما
`STDIN_FILENO`
بود باید تابع
`devices/input.c::input_getc()`
را فراخوانی کنیم.

درصورت فراخوانی 
syscall
نوشتن اگر
file descriptor
ما
`STDOUT_FILENO`
بود باید تابع
`lib/kernel/console.c::putbuf(buffer, size)`
را فراخوانی کنیم.

الگوریتم‌ها
------------
> 7. توضیح دهید خواندن و نوشتن داده‌های کاربر از داخل هسته، در کد شما چگونه انجام شده است.

برای خواندن و نوشتن داده ابتدا بررسی میکنیم که آیا
file descriptor
دریافت شده
`STDOUT_FILENO`
یا
`STDIN_FILENO`
نباشد.
سپس بررسی میکنیم که آیا اجازه خواندن یا نوشتن آنرا داریم یا خیر. اگر پاسخ سوال اول «نه» و سوال دوم «بله» بود، سعی در دریافت 
`fs_lock`
کرده و درصورت موفقیت پردازه را شروع میکنیم و با اجرای
`sema_down(&thread_current()->sema)`
منتظر اتمام کار پردازه میمانیم و درنهایت
`fs_lock`
را آزاد میکنیم.

> 8. فرض کنید یک فراخوانی سیستمی باعث شود یک صفحه‌ی کامل (۴۰۹۶ بایت) از فضای کاربر در فضای هسته کپی شود. بیشترین و کمترین تعداد بررسی‌‌های جدول صفحات (page table) چقدر است؟ (تعداد دفعاتی که `pagedir_get_page()` صدا زده می‌شود.) در‌ یک فراخوانی سیستمی که فقط ۲ بایت کپی می‌شود چطور؟ آیا این عددها می‌توانند بهبود یابند؟ چقدر؟

- **حالت داده‌ی ۴۰۹۶بایتی**

  از آنجا که اندازه 
  page
  ما خود نیز ۴۰۹۶ بایت است، در حالت مینیمم این داده میتواند بصورت یکجا در یک 
  page
  موجود باشد و درنتیجه تنها 
  ***یک***
  بار تابع
  `pagedir_get_page()`
  صدا میشود.
  
  از طرفی این داده میتواند در 
  page
  های مختلف پخش باشد و درنتیجه در حالت ماکسیمم نیاز خواهیم داشت تابع
  `pagedir_get_page()`
  را 
  ***۴۰۹۶***
  بار صدا کنیم تا بایت‌ها را از ۴۰۹۶ صفحه مختلف بیابیم.

- **حالت داده‌ی ۲بایتی**

  اگر هر دو بایت داده در یک صفحه باشند، درحالت مینیمم تنها
  ***یک***
  بار تابع
  `pagedir_get_page()`
  صدا میشود.

  اما اگر دو بایت داده هریک در صفحه متفاوتی باشند، تابع
  `pagedir_get_page()`
  ***دو***
  بار صدا میشود.

<!-- 
- **Data is 4096B**

    Since our page size is also 4096B, this data can be fully available in a single page and so will only need ***one*** call to `pagedir_get_page()`.

    On the other hand, this data can be available in multiple pages and so at maximum we might need to call `pagedir_get_page()` ***4096*** times to retrieve the bytes from different pages.

- **Data is 2B**

    If both bytes of the data available in a single page, there's only ***one*** `pagedir_get_page()` calls needed.

    If the two bytes are available in different pages, there's ***two*** `pagedir_get_page()` calls needed
-->

> 9. پیاده‌سازی فراخوانی سیستمی `wait` را توضیح دهید و بگویید چگونه با پایان یافتن پردازه در ارتباط است.

Add an `if` case for `SYS_WAIT` in `pintos/src/userprog/syscall::syscall_handler(struct intr_frame*)` which looks for the child thread with the specified `pid` in `thread_current()`'s children. Then check if the child has finished already by checking its `status`; If it is finished, return its `return_value`, otherwise `sema_down` on its `semaphore` and return its `return_value`.

> 10. هر دستیابی هسته به حافظه‌ی برنامه‌ی کاربر، که آدرس آن را کاربر مشخص کرده است، ممکن است به دلیل مقدار نامعتبر اشاره‌گر منجر به شکست شود. در این صورت باید پردازه‌ی کاربر خاتمه داده شود. فراخوانی های سیستمی پر از چنین دستیابی‌هایی هستند. برای مثال فراخوانی سیستمی `write‍` نیاز دارد ابتدا شماره‌ی فراخوانی سیستمی را از پشته‌ی کاربر بخواند، سپس باید سه آرگومان ورودی و بعد از آن مقدار دلخواهی از حافظه کاربر را (که آرگومان ها به آن اشاره می کنند) بخواند. هر یک از این دسترسی ها به حافظه ممکن است با شکست مواجه شود. بدین ترتیب با یک مسئله‌ی طراحی و رسیدگی به خطا (error handling) مواجهیم. بهترین روشی که به ذهن شما می‌رسد تا از گم‌شدن مفهوم اصلی کد در بین شروط رسیدگی به خطا جلوگیری کند چیست؟ همچنین چگونه بعد از تشخیص خطا، از آزاد شدن تمامی منابع موقتی‌ای که تخصیص داده‌اید (قفل‌ها، بافر‌ها و...) مطمئن می‌شوید؟ در تعداد کمی پاراگراف، استراتژی خود را برای مدیریت این مسائل با ذکر مثال بیان کنید.

برای اینکار میتوانیم بصورت زیر قابل‌قبول بودن 
pointerها
را بررسی کنیم. به این صورت که شرط اول بررسی میکند آیا چنین
pointerای
در حافظه 
user
(و نه kernel)
موجود است یا خیر و شرط دوم موجود بودن این
pointer
در 
pagedir
ترد فعلی‌ما را بررسی میکند.

```c
bool
ptr_is_valid (void *ptr)
{
  return is_user_vaddr(ptr) &&
         pagedir_get_page (thread_current ()->pagedir, ptr) != NULL;
}
```

علاوه بر این، تابع 
`thread_exit()`
را بگونه‌ای تغییر میدهیم که کلیه فایل‌های باز را ببندد و درنهایت کلیه 
semaphore
و
lockها
را رها کند.
علاوه بر این باید از انجا که ممکن است این تابع توسط سیستم‌عامل بصورت ناگهانی و بدون مقداردهی 
`return_value`
در صورت بروز خطا 
صدا شود، مقدار
`return_value`
برابر
`NULL`
باشد و دراینصورت باید مقدار آن را به -۱ تغییر دهیم.

همگام‌سازی
---------------
> 11. فراخوانی سیستمی `exec` نباید قبل از پایان بارگذاری فایل اجرایی برگردد، چون در صورتی که بارگذاری فایل اجرایی با خطا مواجه شود باید `-۱` برگرداند. کد شما چگونه از این موضوع اطمینان حاصل می‌کند؟ چگونه وضعیت موفقیت یا شکست در اجرا به ریسه‌ای که `exec` را فراخوانی کرده اطلاع داده می‌شود؟

- **تغییرات فایل `pintos/src/threads/thread.h`**
```c
struct thread *find_thread (tid_t tid);
```

- **تغییرات فایل `pintos/src/threads/thread.c`**
```c
struct thread *
find_thread (tid_t tid)
{
  struct list_elem *e;
  for (e = list_begin (&all_list);
       e != list_end (&all_list);
       e = list_next (e)) {
    struct thread *t = list_entry (e, struct thread, allelem);
    if (t->tid == tid) return t;
  }
  return NULL;
}
```

- **تغییرات فایل `pintos/src/userprog/process.c`**
```diff
process_execute (const char *file_name)
{
  ...

  tid = thread_create (file_name, PRI_DEFAULT, start_process, fn_copy);
+  sema_down(&find_thread(tid)->sema);

  ...
}
```
```diff
static void
start_process (void *file_name_)
{
  ...

  success = load (file_name, &if_.eip, &if_.esp);
  
+  /* After file load finished notify the parent thread that was waiting. */
+  if (!success)
+    thread_current()->return_value = -1;
+  sema_up(&thread_current()->sema);

  ...
}
```

> پردازه‌ی والد P و پردازه‌ی فرزند C را درنظر بگیرید. هنگامی که P فراخوانی `wait(C)` را اجرا می‌کند و C  هنوز خارج نشده است، توضیح دهید که چگونه همگام‌سازی مناسب را برای جلوگیری از ایجاد شرایط مسابقه (race condition) پیاده‌سازی کرده‌اید. وقتی که C از قبل خارج شده باشد چطور؟ در هر حالت چگونه از آزاد شدن تمامی منابع اطمینان حاصل می‌کنید؟ اگر P بدون منتظر ماندن، قبل از C خارج شود چطور؟ اگر بدون منتظر ماندن بعد از C خارج شود چطور؟ آیا حالت‌های خاصی وجود دارد؟

منطق طراحی
-----------------
> به چه دلیل روش دسترسی به حافظه سطح کاربر از داخل هسته را این‌گونه پیاده‌سازی کرده‌اید؟

> طراحی شما برای توصیف‌کننده‌های فایل چه نقاط قوت و ضعفی دارد؟

> در حالت پیش‌فرض نگاشت `tid` به `pid` یک نگاشت همانی است. اگر این را تغییر داده‌اید، روی‌کرد شما چه نقاط قوتی دارد؟

سوالات افزون بر طراحی
===========
> تستی را که هنگام اجرای فراخوانی سیستمی از یک اشاره‌گر پشته‌ی(esp) نامعتبر استفاده کرده است بیابید. پاسخ شما باید دقیق بوده و نام تست و چگونگی کارکرد آن را شامل شود.

در [این فایل](../pintos/src/tests/userprog/sc-bad-sp.c) هنگام system call  اشاره‌گر %esp به یک آدرس بد اشاره می‌کند.
```c
/* Invokes a system call with the stack pointer (%esp) set to a
   bad address.  The process must be terminated with -1 exit
   code.

   For Project 3: The bad address lies approximately 64MB below
   the code segment, so there is no ambiguity that this attempt
   must be rejected even after stack growth is implemented.
   Moreover, a good stack growth heuristics should probably not
   grow the stack for the purpose of reading the system call
   number and arguments. */

#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void)
{
  asm volatile ("movl $.-(64*1024*1024), %esp; int $0x30");
  // movl $.-(64*1024*1024), %esp; :: This instructions tells the processor that the stack should be located at the address -(64*1024*1024) from the current position of the instruction pointer (%eip). 
  // int $0x30; :: an interrupt for system call
  fail ("should have called exit(-1)");
}
```
این تست چک می‌کند که آیا یک system call با آدرس آرگومان نامعتبر به درستی کار‌ کرده و با کد 1- خارج می‌شود یا خیر. برای این تست پوینتر استک به حدود 64 مگابایت زیر بخش کد اشاره می‌کند. اگر بتوانیم این آدرس بد (خارج از محدوده)
تشخیص دهیم پراسس با کد 1- خارج می‌شود و در غیر این صورت آن آدرس خواهنده شده و پراسس ادامه پیدا می‌کند که باعث رد شدن تست است.

> تستی را که هنگام اجرای فراخوانی سیستمی از یک اشاره‌گر پشته‌ی معتبر استفاده کرده ولی اشاره‌گر پشته آنقدر به مرز صفحه نزدیک است که برخی از آرگومان‌های فراخوانی سیستمی در جای نامعتبر مموری قرار گرفته اند مشخص کنید. پاسخ شما باید دقیق بوده و نام تست و چگونگی کارکرد آن را شامل شود.


در [این فایل](../pintos/src/tests/userprog/sc-bad-arg.c) هنگام system call  اشاره‌گر %esp به یک آدرس بد اشاره می‌کند.
```c
/* Sticks a system call number (SYS_EXIT) at the very top of the
   stack, then invokes a system call with the stack pointer
   (%esp) set to its address.  The process must be terminated
   with -1 exit code because the argument to the system call
   would be above the top of the user address space. */

#include <syscall-nr.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void)
{
  asm volatile ("movl $0xbffffffc, %%esp; movl %0, (%%esp); int $0x30"
                : : "i" (SYS_EXIT));
  // movl $0xbffffffc, %%esp; :: moves the value 0xbffffffc into the register esp
  // movl %0, (%%esp); :: moves the value of the 0th argument of the function into the memory location pointed to by esp.
  // int $0x30; :: an interrupt for system call
  fail ("should have called exit(-1)");
}
```

با توجه به اینکه PHYS_BASE از آدرس 0xc000 0000 شروع می‌شود و در این برنامه آدرس 0xbfff fffc مورد اشاره استک است. با فراخوانی سیستم کال و با دسترسی یافتن به جایی بیش از 4 بایت از شروع استک  باید با خطا مواجه شویم و با کد 1- از برنامه خارج شویم. این تست این موضوع را چک می‌کند. استفاده از SYS_EXIT چون دو آرگیومنت دارد و حتما منجر به ارور می‌شود.

> یک قسمت از خواسته‌های تمرین را که توسط مجموعه تست موجود تست نشده‌است، نام ببرید. سپس مشخص کنید تستی که این خواسته را پوشش بدهد چگونه باید باشد.

یکی از مسائلی که برای آن تستی موجود نیست بررسی تعداد 
Argumentهای
syscall
هاست.

برای اینکار میتوان تواابع 
`syscallX`
در فایل 
`pintos/src/lib/user/syscall.c`
را با یکی از مقادیر
enum
موجود در فایل
`pintos/src/lib/user/../syscall-nr.h`
صدا کنیم. البته باید حواسم باشد که میخواهیم عدم
crash
کردن سیستم‌مان در صورت دریافت
syscallای
با تعداد ناکافی 
argument
بررسی کنیم و از این رو اگر
syscall
موردنظرمان نیاز به ۲ ورودی دارد، تابع
`syscall1` یا `syscall0`
را صدا کنیم.

سوالات نظرخواهی
==============
پاسخ به این سوالات اختیاری است، ولی پاسخ به آن‌ها می‌تواند به ما در بهبود درس در ترم‌های آینده کمک کند. هر چه در ذهن خود دارید بگویید. این سوالات برای دریافت افکار شما هستند. هم‌چنین می‌توانید پاسخ خود را به صورت ناشناس در انتهای ترم ارائه دهید.

> به نظر شما، این تمرین یا هر یک از سه بخش آن، آسان یا سخت بودند؟ آیا وقت خیلی کم یا وقت خیلی زیادی گرفتند؟

> آیا شما بخشی را در تمرین یافتید که دید عمیق‌تری نسبت به طراحی سیستم عامل به شما بدهد؟

> آیا مسئله یا راهنمایی خاصی وجود دارد که بخواهید برای حل مسائل تمرین به دانشجویان ترم‌های آینده بگویید؟

> آیا توصیه‌ای برای دستیاران آموزشی دارید که چگونه دانشجویان را در ترم‌های آینده یا در ادامه‌ی ترم بهتر یاری کنند؟

> اگر نظر یا بازخورد دیگری دارید در این قسمت بنویسید.
