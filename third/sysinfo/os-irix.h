/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

/*
 * $Id: os-irix.h,v 1.1.1.1 1996-10-07 20:16:55 ghudson Exp $
 */

#ifndef __os_irix_h__
#define __os_irix_h__

#include <unistd.h>
#include <fcntl.h>
#include <sys/param.h>
#include <stdlib.h>
#include <nlist.h>

#define IS_POSIX_SOURCE		1
#define KMEMFILE		"/dev/kmem"
#define WAITARG_T		int
#define NAMELIST		"/unix"
#define HAVE_SYSINFO
#define HAVE_NLIST
#define HAVE_IFNET
#define HAVE_GETHOSTID
#define HAVE_PHYSMEM
#define HAVE_DEVICE_SUPPORT
#define PHYSMEM_SYM 		"physmem"
#define GETNETIF_TYPE		GETNETIF_IFCONF
#define GETMAC_TYPE		GETMAC_IFREQ_ENADDR
#define NEED_SOCKIO
#define NEED_KVM
#define RE_TYPE			RE_COMP

#if	OSMVER >= 6
#define NLIST_TYPE		struct nlist64
#define NLIST_FUNC		nlist64
#define OFF_T_TYPE		off64_t
#else	/* OSMVER < 6 */
#define NLIST_TYPE		struct nlist
#define OFF_T_TYPE		off_t
#endif	/* OSMVER >= 6 */

/*
 * Pathnames
 */
#define _PATH_DEV_DSK		"/dev/dsk"
#define _PATH_DEV_RDSK		"/dev/rdsk"
#define _PATH_DEV_GRAPHICS	"/dev/graphics"

/*
 * Compatibility
 */
#define GETPAGESIZE()		sysconf(_SC_PAGESIZE)



#endif	/* __os_irix_h__ */
