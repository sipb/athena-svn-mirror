/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef __mcdefs_h__
#define __mcdefs_h__

/*
 * $Revision: 1.1.1.1 $
 */

/*
 * MagniComp common definetions
 * 
 * You should almost always include "mconfig.h" directly instead of this file.
 */
#include "options.h"		/* Need to include options.h before errno.h */
#include <sys/errno.h>

/*
 * How to get system error string
 */
#if	defined(SYSERR)
#	undef SYSERR
#endif	/* SYSERR */
extern		int 			errno;
#define		SYSERR			strerror(errno)

/*
 * NULL
 */
#define CNULL			'\0'
#define C_NULL			CNULL

#endif	/* __mcdefs_h__ */
