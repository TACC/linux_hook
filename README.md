# linux_hook
linux_hook is a mini framework to hook / intercept the functions in shared libraries 
under Linux. It only works on x86_64 at this time. I might extend it to support Power 
PC and ARM in future. 

<br>
[udis86](https://github.com/vmt/udis86) was adopted to disasseble binary code on x86_64. 

<br>
Trampoline technique is adopted in the implementation of linux_hook, so the smallest 
size functions we can hook needs 5 bytes. A jmp (+/-2GB) instruction with 5 bytes will 
be placed at the entry of the function we intend to hook. Memory blocks close to the 
shared libraries are allocated dynamically to provide the space for setting up trampoline (
the code to jump to new function and the code to call old function). 
<br>

Get started,<br> 
`git clone https://github.com/TACC/linux_hook` <br>
`cd linux_hook` <br>
`make` <br>

Test

`LD_PRELOAD=./hook_open.so touch aaa bbb ccc` <br>

You should see, 

`DBG> new_open(aaa)` <br>
`DBG> new_open(bbb)` <br>
`DBG> new_open(ccc)` <br>

One more test to monitor open() in run a simple hello world in python. 

`LD_PRELOAD=./hook_open.so python3 ./test/hello.py > 1 ; wc -l 1` <br>
`LD_PRELOAD=./mini_open.so python3 ./test/hello.py > 2 ; wc -l 2` <br>

mini_open.so implements the simplest LD_PRELOAD trick to intercept functions. 
You should observe file 1 and 2 have different number of lines. We can see the 
hook based on trampoline is more reliable. 

<br>
Example of a mini code to use hook_linux to intercept the open() in libc.so. 

``` C
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>

#include "include/hook.h"

typedef int (*org_open_libc)(const char *pathname, int oflags, ...);
org_open_libc real_open_libc=NULL;

// When the open() in libc.so is called, new_open_libc() will be executed.
int new_open_libc(const char *pathname, int oflags, ...)
{
        int mode = 0, two_args=1, ret;
        char msg[512];

        if (oflags & O_CREAT)   {
                va_list arg;
                va_start (arg, oflags);
                mode = va_arg (arg, int);
                va_end (arg);
                two_args=0;
        }

        snprintf(msg, sizeof(msg), "DBG> new_open_libc(%s)\n", pathname);
        write(STDOUT_FILENO, msg, strlen(msg));

        if(two_args)    {
                ret = real_open_libc(pathname, oflags);
        }
        else    {
                ret = real_open_libc(pathname, oflags, mode);
        }
        return ret;
}

static __attribute__((constructor)) void init_myhook()
{
        register_a_hook("libc", "open64", (void*)new_open_libc, (long int *)(&real_open_libc));

        install_hook();
}

static __attribute__((destructor)) void finalize_myhook()
{
        uninstall_hook();
}
```
<br>

Save the code as 1.c. <br>
`gcc -fPIC -c 1.c` <br>
`gcc -fPIC -shared -o 1.so 1.o obj/hook.o obj/decode.o obj/itab.o obj/syn-att.o obj/syn-intel.o obj/syn.o obj/udis86.o` <br>

Now test it. 

`LD_PRELOAD=./1.so touch aaa bbb ccc` <br>

You should see,

`DBG> new_open(aaa)` <br>
`DBG> new_open(bbb)` <br>
`DBG> new_open(ccc)` <br>


