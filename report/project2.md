# گزارش تمرین دو

> نام و آدرس پست الکترونیکی اعضای گروه

درین ستوده <dorrinsotoudeh123@gmail.com> 

مسیح بیگی ریزی <masihbr@gmail.com>

علی مورکیان <moroukianali@gmail.com> 


> تغییراتی که نسبت به سند طراحی اولیه داشتید و دلیلی که این تغییر را انجام دادید را بیان کنید 

## ساعت زنگ‌دار

تنها تغییر داده شده نسبت به طراحی، عدم تعریف `lock sleeping_lock` بود. طبق متن زیر از مستند تمرین

>> Interrupt handlers cannot acquire locks. If you need to access a synchronized variable from
an interrupt handler, consider disabling interrupts.

نیازی به تعریف قفل نبود و شرایط race condition با غیرفعال کردن وقفه‌ها پیش نمی‌آید.

---
