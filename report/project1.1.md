# گزارش تمرین ۱.۱ - برنامه‌های کاربر

> نام و آدرس پست الکترونیکی اعضای گروه

درین ستوده <dorrinsotoudeh123@gmail.com> 

مسیح بیگی ریزی <masihbr@gmail.com>

علی مورکیان <moroukianali@gmail.com> 


> تغییراتی که نسبت به سند طراحی اولیه داشتید و دلیلی که این تغییر را انجام دادید را بیان کنید 

اضافه شدن داده ساختار ```status_t```. از این داده ساختار در process.c استفاده شده است.

```c
struct status_t
  {
    tid_t tid;                          /* Thread identifier. */

    int return_value;                   /* The return value of this thread. */
    bool waited;                        /* Wether the thread is being waited on. */
    bool finished;                      /* Wether the thread is finished. */
    
    struct list_elem elem;              /* List element. */
  };
```
---

عدم پیاده‌سازی توابع
```c
int get_argc(char *cmd_line);
char** get_argv(char *cmd_line, int argc);
``` 
و پیاده‌سازی آن‌ها در تابع 
```c
process_args *get_process_args (const char *cmd_line);
```
برای بهبود پرفورمنس

---

تغییر جزئی در تابع is_ptr_valid و اضافه‌شدن توابع is_block_valid و is_cmd_valid در pagedir.c

```c
bool
is_ptr_valid (void *ptr)
{
  return ptr != NULL && is_user_vaddr (ptr)
         && pagedir_get_page (thread_current ()->pagedir, ptr) != NULL;
}

bool
is_block_valid (void *ptr, size_t size)
{
  return is_ptr_valid (ptr) && is_ptr_valid (ptr + size);
}

bool
is_cmd_valid (char *cmd)
{
  void *cmd_page = pagedir_get_page (thread_current ()->pagedir, cmd);
  return cmd_page != NULL && is_block_valid (cmd, strlen (cmd_page) + 1);
}
```



> بیان کنید که هر فرد گروه دقیقا چه بخشی را انجام داد؟ آیا این کار را به صورت مناسب انجام دادید و چه کارهایی برای بهبود
عملکردتان می توانید انجام دهید.

علی مورکیان: پیاده سازی پاس دادن آرگومان‌های خط فرمان و نوشتن داک
