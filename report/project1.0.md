تمرین گروهی ۱/۰ - آشنایی با pintos
======================

شماره گروه:
-----
> نام و آدرس پست الکترونیکی اعضای گروه را در این قسمت بنویسید.

نام و نام خانوادگی <example@example.com>

نام و نام خانوادگی <example@example.com> 

نام و نام خانوادگی <example@example.com> 

نام و نام خانوادگی <example@example.com> 

مقدمات
----------
> اگر نکات اضافه‌ای در مورد تمرین یا برای دستیاران آموزشی دارید در این قسمت بنویسید.


> لطفا در این قسمت تمامی منابعی (غیر از مستندات Pintos، اسلاید‌ها و دیگر منابع  درس) را که برای تمرین از آن‌ها استفاده کرده‌اید در این قسمت بنویسید.

آشنایی با pintos
============
>  در مستند تمرین گروهی ۱۹ سوال مطرح شده است. پاسخ آن ها را در زیر بنویسید.


## یافتن دستور معیوب

Page fault at 0xc0000008: rights violation error reading page in user context.
do-nothing: dying due to interrupt 0x0e (#PF Page-Fault Exception).
Interrupt 0x0e (#PF Page-Fault Exception) at eip=0x8048757
 cr2=c0000008 error=00000005
 eax=00000000 ebx=00000000 ecx=00000000 edx=00000000
 esi=00000000 edi=00000000 esp=bfffffe4 ebp=00000000
 cs=001b ds=0023 es=0023 ss=0023

۱.<br/>
Page fualt at 0xc0000008: which means address 0xc0000008 was accessed unauthorized.

۲.<br/>
Register eip holds the address 0x8048757 which is the address of instruction responsible for the error.

۳.<br/>
Function: 08048754 <_start>
<br/>
Instruction: 8048757:       8b 44 24 24             mov    0x24(%esp),%eax

۴.<br/>
Pintos/src/lib/user/entry.c
<br/>
Before calling the function main in line 9 of entry.c - argc and argv will be pushed into stack to be used after call of function.

۵.<br/>
To put argv in stack, value stored in %esp is moved to 0x24(%esp) and due to insufficient memory allocation for this program 0x24(%esp) has the address to another memory area.

## به سوی crash

۶.

۷.

۸.

۹.

۱۰.

۱۱.

۱۲.

۱۳.


## دیباگ

۱۴.

۱۵.

۱۶.

۱۷.

۱۸.

۱۹.