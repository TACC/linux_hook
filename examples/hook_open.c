#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>

#include "include/hook.h"

/*
This code demonstrates how to use "Linux hook" to intercept function open() with 
a minimal skeleton. 

The interface of open(), 
int open(const char *pathname, int flags);
int open(const char *pathname, int flags, mode_t mode);

I found three implentations commonly used under Linux, ld.so, libc.so and libpthread.so. 
You may encounter various versions. We intercept all of them in this example. 
*/

// define the interface of the function we want to intercept
typedef int (*org_open_ld)(const char *pathname, int oflags, ...);

// define the function pointer that will hold the address of orginal function. 
org_open_ld real_open_ld=NULL;

typedef int (*org_open_libc)(const char *pathname, int oflags, ...);
org_open_libc real_open_libc=NULL;

typedef int (*org_open_pthread)(const char *pathname, int oflags, ...);
org_open_pthread real_open_pthread=NULL;

int open_common(int (*org_open)(const char *pathname, int oflags, ...), const char *szCallerName, const char *pathname, int oflags, ...)
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
	snprintf(msg, sizeof(msg), "DBG> %s(%s)\n", szCallerName, pathname);
	write(STDOUT_FILENO, msg, strlen(msg));

	if(two_args)    {
		ret = org_open(pathname, oflags);
	} else {
		ret = org_open(pathname, oflags, mode);
	}
	return ret;
}
// When the open() in ld.so is called, new_open_ld() will be executed. 
int new_open_ld(const char *pathname, int oflags, ...)
{
	int mode = 0, two_args=1, ret;

	if (oflags & O_CREAT)	{
		va_list arg;
		va_start (arg, oflags);
		mode = va_arg (arg, int);
		va_end (arg);
		two_args=0;
	}
	
	if(two_args)	{
		ret = open_common(real_open_ld, "new_open_ld", pathname, oflags);
	}
	else	{
		ret = open_common(real_open_ld, "new_open_ld", pathname, oflags, mode);
	}

	return ret;
}

// When the open() in libc.so is called, new_open_libc() will be executed.
int new_open_libc(const char *pathname, int oflags, ...)
{
	int mode = 0, two_args=1, ret;

	if (oflags & O_CREAT)	{
		va_list arg;
		va_start (arg, oflags);
		mode = va_arg (arg, int);
		va_end (arg);
		two_args=0;
	}

	if(two_args)	{
		ret = open_common(real_open_libc, "new_open_libc", pathname, oflags);
	}
	else	{
		ret = open_common(real_open_libc, "new_open_libc", pathname, oflags, mode);
	}
	return ret;
}

// When the open() in libpthread.so is called, new_open_pthread() will be executed.
int new_open_pthread(const char *pathname, int oflags, ...)
{
	int mode = 0, two_args=1, ret;

	if (oflags & O_CREAT)	{
		va_list arg;
		va_start (arg, oflags);
		mode = va_arg (arg, int);
		va_end (arg);
		two_args=0;
	}

	if(two_args)	{
		ret = open_common(real_open_pthread, "new_open_pthread", pathname, oflags);
	}
	else	{
		ret = open_common(real_open_pthread, "new_open_pthread", pathname, oflags, mode);
	}
	return ret;
}

static __attribute__((constructor)) void init_myhook()
{
	// Providing version numbers in shared object names. This should be reliable, but users 
	// need to make sure you are intercepting the correct libraries. 
//	register_a_hook("ld-2.17.so", "open64", (void*)new_open_ld, (long int *)(&real_open_ld));
//	register_a_hook("libc-2.17.so", "open64", (void*)new_open_libc, (long int *)(&real_open_libc));
//	register_a_hook("/lib64/libpthread-2.17.so", "open64", (void*)new_open_pthread, (long int *)(&real_open_pthread));

	register_a_hook("ld", "open64", (void*)new_open_ld, (long int *)(&real_open_ld));
	register_a_hook("libc", "open64", (void*)new_open_libc, (long int *)(&real_open_libc));
	register_a_hook("libpthread", "open64", (void*)new_open_pthread, (long int *)(&real_open_pthread));
	
	install_hook();
}

static __attribute__((destructor)) void finalize_myhook()
{
	uninstall_hook();
}
