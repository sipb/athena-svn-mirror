/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

/*
 * $Revision: 1.1.1.1 $
 *
 * Types header file for SysInfo
 */

#ifndef __mctypes_h__
#define __mctypes_h__ 

/*
 * Find the largest type we can
 */
#if	!defined(LARGE_T) && SIZEOF_UNSIGNED_LONG_LONG > SIZEOF_LONG_LONG && SIZEOF_UNSIGNED_LONG_LONG > SIZEOF_LONG_INT && SIZEOF_UNSIGNED_LONG_LONG > SIZEOF_UINT64_T && SIZEOF_UNSIGNED_LONG_LONG > SIZEOF_LONGLONG
#define LARGE_T		unsigned long long
#endif
#if	!defined(LARGE_T) && SIZEOF_LONG_LONG > SIZEOF_LONG_INT && SIZEOF_LONG_LONG > SIZEOF_UINT64_T && SIZEOF_LONG_LONG > SIZEOF_LONGLONG
#define LARGE_T		long long
#endif
#if	!defined(LARGE_T) && SIZEOF_LONGLONG > SIZEOF_LONG_INT && SIZEOF_LONGLONG > SIZEOF_UINT64_T
#define LARGE_T		longlong
#endif
#if	!defined(LARGE_T) && SIZEOF_LONG_INT > SIZEOF_LONG_LONG && SIZEOF_LONG_INT > SIZEOF_UINT64_T
#define LARGE_T		long int
#endif
#if	!defined(LARGE_T) && SIZEOF_UINT64_T > 0
#define LARGE_T		uint64_t
#endif

/*
 * Internal type for large values
 */
#if 	!defined(LARGE_T)
#	define LARGE_T		u_long
#endif
typedef LARGE_T			Large_t;
typedef LARGE_T			Offset_t;

/*
 * Hide type
 */
typedef void *			Opaque_t;

#endif	/* __mctypes_h__ */
