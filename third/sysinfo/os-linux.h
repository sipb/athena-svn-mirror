/*
 * Copyright (c) 1992-1998 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not
 * sold for profit or used in part or in whole for commercial gain
 * without prior written agreement, and the author is credited
 * appropriately.
 */

/*
 * $Revision: 1.1.1.1 $
 */

#ifndef __os_linux_h__
#define __os_linux_h__

/*
 * Linux
 */

#include <fcntl.h>
#include <sys/param.h>

/*
 * There's no such thing as a single manufacturer for Linux systems.
 */
#undef MAN_SHORT
#undef MAN_LONG

/*
 * What features we have
 */
#define HAVE_GETHOSTID		/* gethostid() */
#define HAVE_UNAME		/* uname() */

/*
 * What we are
 */
#define IS_POSIX_SOURCE		1
#define RE_TYPE			RE_REGCOMP

/*
 * Pathnames and such.
 */
#define PROC_FILE_UPTIME	"/proc/uptime"
#define PROC_FILE_CPUINFO	"/proc/cpuinfo"
#define PROC_FILE_MEMINFO	"/proc/meminfo"
#define PROC_FILE_VERSION	"/proc/version"

#endif	/* __os_linux_h__ */
