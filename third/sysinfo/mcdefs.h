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

/*
 * MagniComp common definetions
 * 
 * You should almost always include "mconfig.h" directly instead of this file.
 * "mconfig.h" will in turn make sure things like IS_POSIX_SOURCE are defined.
 */
#include "options.h"		/* Need to include options.h before errno.h */
#include "strutil.h"
#include <sys/errno.h>

/*
 * How to get system error string
 */
#if	defined(SYSERR)
#	undef SYSERR
#endif	/* SYSERR */
#if	defined(IS_POSIX_SOURCE)
extern		int errno;
#define		SYSERR			strerror(errno)
#else	/* !IS_POSIX_SOURCE */
extern		int 			errno;
extern		int 			sys_nerr;
extern		char			*sys_errlist[];
#define		SYSERR \
((errno>0 && errno<sys_nerr) ? sys_errlist[errno] : "(unknown system error)")
#endif	/* IS_POSIX_SOURCE */

/*
 * NULL
 */
#define CNULL			'\0'
#define C_NULL			CNULL
