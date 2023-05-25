# گزارش تمرین ۱.۱ - برنامه‌های کاربر

> نام و آدرس پست الکترونیکی اعضای گروه

درین ستوده <dorrinsotoudeh123@gmail.com> 

مسیح بیگی ریزی <masihbr@gmail.com>

علی مورکیان <moroukianali@gmail.com> 


> تغییراتی که نسبت به سند طراحی اولیه داشتید و دلیلی که این تغییر را انجام دادید را بیان کنید 

> گزارش تست های پیاده سازی شده

تست اول و سوم در داک، پیاده سازی شده است.

برای تست اول این مورد بررسی می‌شود که `buffer cache` پیاده سازی شده hitrate را افزایش می‌دهد و تعداد read های مستقیم از دیسک را کاهش ‌می‌دهد.

در تست سوم این مورد بررسی شده که برای نوشتن یک بلوک کامل لازم نیست مقادیر دیسک read شوند و همچنین تعداد write های دیسک نیز در این تست چک می‌شوند تا مطابق انتظار باشد.

تست های این بخش به کمک تعدادی syscall صورت می‌گیرد:

- count_cache_hit
- count_cache_miss
- count_cache_read
- count_cache_write
- flush_cache
- reset_cache_stats

چهار syscall اول برای گرفتن آمار cache و syscall پنجم برای  خالی کردن cache استفاده می‌شود.

با انجام اعمال خواندن و نوشتن روی فایل و گرفتن این امار ها می‌توانیم بررسی کنیم که cache به درستی کار می‌کند یا خیر.

## cache-hitrate:
* output:
```
Copying tests/filesys/extended/cache-hitrate to scratch partition...
Copying tests/filesys/extended/tar to scratch partition...
qemu-system-i386 -device isa-debug-exit -hda /tmp/cXMimIgvj2.dsk -hdb tmp.dsk -m 4 -net none -nographic -monitor null
PiLo hda1
Loading...........
Kernel command line: -q -f extract run cache-hitrate
Pintos booting with 3,968 kB RAM...
367 pages available in kernel pool.
367 pages available in user pool.
Calibrating timer...  419,020,800 loops/s.
hda: 1,008 sectors (504 kB), model "QM00001", serial "QEMU HARDDISK"
hda1: 183 sectors (91 kB), Pintos OS kernel (20)
hda2: 245 sectors (122 kB), Pintos scratch (22)
hdb: 5,040 sectors (2 MB), model "QM00002", serial "QEMU HARDDISK"
hdb1: 4,096 sectors (2 MB), Pintos file system (21)
filesys: using hdb1
scratch: using hda2
Formatting file system...done.
Boot complete.
Extracting ustar archive from scratch device into file system...
Putting 'cache-hitrate' into the file system...
Putting 'tar' into the file system...
Erasing ustar archive...
Executing 'cache-hitrate':
(cache-hitrate) begin
(cache-hitrate) create "data.bin"
(cache-hitrate) open "data.bin"
(cache-hitrate) Going to write data in file
(cache-hitrate) Going to empty cache and reset stats
(cache-hitrate) Going to read file 1st time
(cache-hitrate) Going to read file 2nd time
(cache-hitrate) Better hitrate after filling cache.
(cache-hitrate) No write should have been made.
(cache-hitrate) end
cache-hitrate: exit(0)
Execution of 'cache-hitrate' complete.
Timer: 157 ticks
Thread: 94 idle ticks, 58 kernel ticks, 5 user ticks
hdb1 (filesys): 121 reads, 559 writes
hda2 (scratch): 244 reads, 2 writes
Console: 1375 characters output
Keyboard: 0 keys pressed
Exception: 0 page faults
Powering off...
```
* result:
```
PASS
```

# cache-write:
* output:
```
Copying tests/filesys/extended/cache-write to scratch partition...
Copying tests/filesys/extended/tar to scratch partition...
qemu-system-i386 -device isa-debug-exit -hda /tmp/cbrsWHSHwL.dsk -hdb tmp.dsk -m 4 -net none -nographic -monitor null
PiLo hda1
Loading...........
Kernel command line: -q -f extract run cache-write
Pintos booting with 3,968 kB RAM...
367 pages available in kernel pool.
367 pages available in user pool.
Calibrating timer...  384,204,800 loops/s.
hda: 1,008 sectors (504 kB), model "QM00001", serial "QEMU HARDDISK"
hda1: 183 sectors (91 kB), Pintos OS kernel (20)
hda2: 242 sectors (121 kB), Pintos scratch (22)
hdb: 5,040 sectors (2 MB), model "QM00002", serial "QEMU HARDDISK"
hdb1: 4,096 sectors (2 MB), Pintos file system (21)
filesys: using hdb1
scratch: using hda2
Formatting file system...done.
Boot complete.
Extracting ustar archive from scratch device into file system...
Putting 'cache-write' into the file system...
Putting 'tar' into the file system...
Erasing ustar archive...
Executing 'cache-write':
(cache-write) begin
(cache-write) Going create file.
(cache-write) create "data2.bin"
(cache-write) open "data2.bin"
(cache-write) Going to empty cache and reset stats.
(cache-write) Going to write data in file.
(cache-write) 200 block writes.
(cache-write) No read should have been made.
(cache-write) end
cache-write: exit(0)
Execution of 'cache-write' complete.
Timer: 124 ticks
Thread: 60 idle ticks, 59 kernel ticks, 6 user ticks
hdb1 (filesys): 49 reads, 889 writes
hda2 (scratch): 241 reads, 2 writes
Console: 1278 characters output
Keyboard: 0 keys pressed
Exception: 0 page faults
Powering off...
```
* result:
```
PASS
```

اگر هسته به جای خواندن از کش به طور مستقیم از دیسک داده ها را بخواند چون hitrate تغییری نمی کند تست cache-hitrate پاس نخواهد شد و در خط زیر دچار اشکال می‌شود:

```c
CHECK (cache_hit_filled > cache_hit_empty, "Better hitrate after filling cache.");
```

اگر هسته وقتی فقط عملیات read از کاربر گرفته write انجام دهد توسط خط زیر اشکالش گرفته می‌شود

```c
CHECK (!(count_cache_write ()), "No write should have been made.");
```

به طور مشابه در تست دیگر تعداد read ها باید صفر باشد و همچنین تعداد write ها باید دقیقا به اندازه ۲۰۰ باشد و اگر هسته کار دیگری انجام دهد توسط خط کد زیر در تست چک می‌شود
```c
CHECK (CHUNK_CNT == count_cache_write (),
         "200 block writes.");
CHECK (!(count_cache_read ()), "No read should have been made.");
```

تجربه تست نویسی برای pintos بسیار چالش برانگیز بود، محدودیت روی نام فایل‌ها و اضافه کردن آنها به Make.tests و فهمیدن ساختار چک شدن در این 
[لینک](https://cs162.org/static/proj/proj-filesys/docs/tasks/testing/)
کمک کننده بود.

> بیان کنید که هر فرد گروه دقیقا چه بخشی را انجام داد؟ آیا این کار را به صورت مناسب انجام دادید و چه کارهایی برای بهبود
عملکردتان می توانید انجام دهید.

- علی مورکیان: پیاده سازی پاس دادن آرگومان‌های خط فرمان و نوشتن داک

- درین ستوده و مسیح بیگی: پیاده‌سازی فراخوانی‌های سیستمی

- برای همگام‌سازی، برای هر یک از
inodeها
(چه file و چه dir)
از قفل مختص آن استفاده کردیم که هنگام انجام فرآیند‌های read، write، و ... که بر روی فایل‌های دیگر تاثیری ندارند قفل خود فایل aquire و پس از پایان کار release شود.
از طرفی برای فرایند‌هایی مانند create، mkdir، remove، و ... که مربوط به پوشه خاصی هستند از قفل آن پوشه استفاده میشود.

- برای فرایند سیستمی readdir نیاز پیدا کردیم تا متغیر pos دایرکتوری مورداستفاده را با file هم نام آن بروز کنیم. چرا که از فهرست دایرکتوری‌های باز را در جایی نگه نمیداریم ولی لیست فایل‌های باز را در فاز ۱ پیاده‌سازی کردیم.

- برای جلوگیری از حذف تصادفی cwd یک ترد بجای استفاده از متغیر cwd_use_count،‌ هر بار هنگام حذف، درصورتی که فایل درحال حذف متناظر با یک دایرکتوری بود بررسی میکنیم که مبادا "." که بمعنی cwd ریسه فعلی است، نباشد و همچنین dir_entry آن in_use نباشد.

- برای تحلیل مسیر‌های relative و absolute کاراکتر اول مسیر بررسی شده و اگر / بود مسیر absolute است. حال باتوجه به این مبدا را root یا cwd ریسه فعلی قرار میدهیم. 