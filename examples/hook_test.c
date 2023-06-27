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
typedef int (*org_open)(const char *pathname, int oflags, ...);

// define the function pointer that will hold the address of orginal function. 
org_open real_open_ld=NULL;
org_open real_open_libc=NULL;
org_open real_open_pthread=NULL;

//static int (*libc_close_nocancel)(int fd);
static int (*ld_close)(int fd);
static int (*libc_close)(int fd);
static int (*pthread_close)(int fd);

static ssize_t (*libc_read)(int fd, void *buf, size_t count);
static ssize_t (*pthread_read)(int fd, void *buf, size_t count);
static ssize_t (*next_pread)(int fd, void *buf, size_t size, off_t offset);

static ssize_t (*libc_write)(int fd, const void *buf, size_t count);
static ssize_t (*pthread_write)(int fd, const void *buf, size_t count);
static ssize_t (*next_pwrite)(int fd, const void *buf, size_t size, off_t offset);

static int
open_common(int (*org_open)(const char *pathname, int oflags, ...), const char *szCallerName, const char *pathname, int oflags, ...)
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

static int
new_close_common(int (*next_close)(int fd), int fd)
{
	char fd_path[64];
	char fname[1024];
	char path[1024];
	ssize_t bytes_read;

	sprintf(fd_path, "/proc/self/fd/%d", fd);
	bytes_read = readlink(fd_path, path, 1023);
	if (bytes_read > 0)
		printf("DBG> close(%s)\n", path);
	return next_close(fd);
}

static int
new_close_ld(int fd)
{
	return new_close_common(ld_close, fd);
}

static int
new_close_libc(int fd)
{
	return new_close_common(libc_close, fd);
}

static int
new_close_pthread(int fd)
{
	return new_close_common(pthread_close, fd);
}

static ssize_t
read_comm(ssize_t (*next_read)(int fd, void *buf, size_t size), int fd, void *buf, size_t size)
{
	printf("DBG> In read(), %zu\n", size);
	return next_read(fd, buf, size);
}

static ssize_t
new_read_libc(int fd, void *buf, size_t size)
{
	return read_comm(libc_read, fd, buf, size);
}

static ssize_t
new_read_pthread(int fd, void *buf, size_t size)
{
	return read_comm(pthread_read, fd, buf, size);
}

ssize_t
write_comm(ssize_t (*next_write)(int fd, const void *buf, size_t size), int fd, const void *buf,
	   size_t size)
{
	return next_write(fd, buf, size);
}

static ssize_t
new_write_libc(int fd, const void *buf, size_t size)
{
	return write_comm(libc_write, fd, buf, size);
}

static ssize_t
new_write_pthread(int fd, const void *buf, size_t size)
{
	return write_comm(pthread_write, fd, buf, size);
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
	register_a_hook("libpthread", "open64", (void*)new_open_pthread,
			(long int *)(&real_open_pthread));

	register_a_hook("ld", "__close", (void *)new_close_libc, (long int *)(&ld_close));
	register_a_hook("libc", "__close", (void *)new_close_libc, (long int *)(&libc_close));
	register_a_hook("libpthread", "__close", (void *)new_close_pthread,
			(long int *)(&pthread_close));

	register_a_hook("libc", "__read", (void *)new_read_libc, (long int *)(&libc_read));
	register_a_hook("libpthread", "__read", (void *)new_read_pthread,
			(long int *)(&pthread_read));
	register_a_hook("libc", "__write", (void *)new_write_libc, (long int *)(&libc_write));
	register_a_hook("libpthread", "__write", (void *)new_write_pthread,
			(long int *)(&pthread_write));	
	install_hook();
}

static __attribute__((destructor)) void finalize_myhook()
{
	uninstall_hook();
}
