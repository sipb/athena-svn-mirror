/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

/*
 * $Revision: 1.1.1.1 $
 */

#ifndef __config_h__
#define __config_h__

#include "config-data.h"

/*
 * NOTES:
 * - ARG_TYPE is set in <options.h>
 */

/*
 * Pathname of things
 */
#ifndef _PATH_DEV
#define _PATH_DEV		"/dev"
#endif
#ifndef _PATH_NULL
#define _PATH_NULL		"/dev/null"
#endif

/*
 * Path name of master sysinfo.cf file (optional)
 */
#ifndef MASTER_CONFIG_FILE
#define MASTER_CONFIG_FILE	"/etc/sysinfo.cf"
#endif

/*
 * Name of default config file
 */
#ifndef DEFAULT_CONFIG_FILE
#define DEFAULT_CONFIG_FILE	"Default.cf"
#endif

/*
 * Default Root Access level is RA_ADVISED.
 */
#if	!defined(RA_LEVEL)
#define RA_LEVEL RA_ADVISED
#endif	/* RA_LEVEL */

/*
 * Common include files
 */
#if	defined(HAVE_UNISTD_H)
#	include <unistd.h>
#endif	/* HAVE_UNISTD_H */
#if	defined(HAVE_STRING_H)
#	include <string.h>
#endif	/* HAVE_STRING_H */

#if defined(STDC_HEADERS)
# include <string.h>
#else
# ifndef HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif
char *strchr (), *strrchr ();
# ifndef HAVE_MEMCPY
#  define memcpy(d, s, n) bcopy ((s), (d), (n))
#  define memmove(d, s, n) bcopy ((s), (d), (n))
# endif
#endif

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
#if	!defined(GETMAC_TYPE) && defined(SIOCGIFHWADDR)
#define GETMAC_TYPE		GETMAC_SIOCGIFHWADDR
#endif

/*
 * Get Network InterFaces
 */
/* Give precedense to _IFCONF which works where _IFNET doesn't */
#if	!defined(GETNETIF_TYPE) && defined(SIOCGIFCONF)
#define GETNETIF_TYPE		GETNETIF_IFCONF		
#endif
#if	!defined(GETNETIF_TYPE) && defined(HAVE_IFNET)
#define GETNETIF_TYPE		GETNETIF_IFNET
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
#if	!defined(WAITARG_T)
#define WAITARG_T		int
#endif	/* WAITARG_T */

typedef WAITARG_T		waitarg_t;

#if	defined(WEXITSTATUS) && !defined(WAITEXITSTATUS)
#	define WAITEXITSTATUS	WEXITSTATUS
#endif

/*
 * Reg Expression type
 */
#if	!defined(RE_TYPE) && defined(HAVE_REGCOMP)
#		define RE_TYPE RE_REGCOMP
#endif
#if	!defined(RE_TYPE) && defined(HAVE_RE_COMP)
#		define RE_TYPE RE_COMP
#endif
#if	!defined(RE_TYPE) && defined(HAVE_REGCMP)
#		define RE_TYPE RE_REGCMP
#endif

/*
 * Byte order
 */
#if	!defined(_BIT_FIELDS_LTOH) && !defined(_BIT_FIELDS_HTOL)
#if		defined(WORDS_BIGENDIAN)
#			define _BIT_FIELDS_HTOL
#else
#			define _BIT_FIELDS_LTOH
#endif		/* WORDS_BIGENDIAN */
#endif	/* !_BIT_FIELDS_* */

#endif /* __config_h__ */
