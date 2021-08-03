
#include <stdio.h>

#define __USE_GNU
#define _GNU_SOURCE

#include <dlfcn.h>
#include <link.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>

typedef int (*org_open)(const char *pathname, int oflags, ...);
org_open real_open=NULL;

int open(const char *pathname, int oflags, ...)
{
	int mode = 0, two_args=1, ret;
	char msg[512];

	if (oflags & O_CREAT)	{
		va_list arg;
		va_start (arg, oflags);
		mode = va_arg (arg, int);
		va_end (arg);
		two_args=0;
	}

	if(real_open == NULL)	{
		real_open = (org_open)dlsym(RTLD_NEXT, "open64");
	}

	snprintf(msg, sizeof(msg), "DBG> new_open(%s)\n", pathname);
	write(STDOUT_FILENO, msg, strlen(msg));
	
	if(two_args)	{
		ret = real_open(pathname, oflags);
	}
	else	{
		ret = real_open(pathname, oflags, mode);
	}

	return ret;
}
extern int __open(const char *pathname, int oflags, ...) __attribute__ ( (alias ("open")) );
extern int open64(const char *pathname, int oflags, ...) __attribute__ ( (alias ("open")) );
extern int __open64(const char *pathname, int oflags, ...) __attribute__ ( (alias ("open")) );
