/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

/*
 * $Id: config.h,v 1.1.1.1 1996-10-07 20:16:55 ghudson Exp $
 */

#ifndef __config_h__
#define __config_h__

#include "config-data.h"

/*
 * Pathname of things
 */
#define _PATH_NULL		"/dev/null"

/*
 * Path to configuration directory.
 * Override in Makefile.
 */
#ifndef CONFIG_DIR
#define CONFIG_DIR		"/usr/lsd/conf/sysinfo"
#endif

/*
 * Name of default config file
 */
#ifndef DEFAULT_CONFIG_FILE
#define DEFAULT_CONFIG_FILE	"Default.cf"
#endif

#if	defined(IS_POSIX_SOURCE)
/*
 * Common include files
 */
#	include <unistd.h>
#	include <string.h>
#	define HAVE_UNAME
#	define HAVE_WAITPID
#	define HAVE_SYSCONF
#else	/* !IS_POSIX_SOURCE */
/*
 * Backwards compatibility with non-POSIX systems.
 */
#	define memcpy(b,a,s)	bcopy(a,b,s)
#	define memset(a,c,s)	bzero(a,s)
#	define strchr		index
#	define strrchr		rindex
#endif	/* IS_POSIX_SOURCE */

/*
 * Arg type
 */
#if	!defined(ARG_TYPE)
# if	defined(__STDC__) && (defined(IS_POSIX_SOURCE) || defined(HAVE_STDARG))
#  define 	ARG_TYPE		ARG_STDARG
#  include 	<stdarg.h>
# endif	/* HAVE_STDARG */
# if	defined(HAVE_VARARGS)
#  define 	ARG_TYPE		ARG_VARARGS
#  include 	<varargs.h>
# endif	/* HAVE_VARARGS */
#endif	/* !ARG_TYPE */

/*
 * Get MAC Info
 */
#if	!defined(GETMAC_TYPE) && defined(HAVE_DLPI)
#define GETMAC_TYPE		GETMAC_DLPI
#endif
#if	!defined(GETMAC_TYPE) && defined(HAVE_NIT)
#define GETMAC_TYPE		GETMAC_NIT
#endif
#if	!defined(GETMAC_TYPE) && defined(HAVE_PACKETFILTER)
#define GETMAC_TYPE		GETMAC_PACKETFILTER
#endif

/*
 * Get Network InterFaces
 */
#if	!defined(GETNETIF_TYPE) && defined(HAVE_IFNET)
#define GETNETIF_TYPE		GETNETIF_IFNET
#endif
#if	!defined(GETNETIF_TYPE) && defined(SIOCGIFCONF)
#define GETNETIF_TYPE		GETNETIF_IFCONF		
#endif

/*
 * Wait type
 */
#if	!defined(WAIT_TYPE) && defined(HAVE_WAITPID)
#include <sys/wait.h>
#define WAIT_TYPE		WAIT_WAITPID
#endif
#if	!defined(WAIT_TYPE) && defined(HAVE_WAIT4)
#include <sys/wait.h>
#define WAIT_TYPE		WAIT_WAIT4
#ifdef WEXITSTATUS
#define WAITEXITSTATUS(s)	WEXITSTATUS(s.w_status)
#else
#define WAITEXITSTATUS(s)	s.w_status
#endif	/* WEXITSTATUS */
#endif
#if	!defined(WAITARG_T) && defined(IS_POSIX_SOURCE)
#define WAITARG_T		int
#endif	/* WAITARG_T */

#if	defined(WEXITSTATUS) && !defined(WAITEXITSTATUS)
#	define WAITEXITSTATUS	WEXITSTATUS
#endif

typedef WAITARG_T		waitarg_t;

#endif /* __config_h__ */
