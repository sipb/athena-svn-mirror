/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

/*
 * $Id: os-sunos5.h,v 1.1.1.1 1996-10-07 20:16:55 ghudson Exp $
 */

#ifndef __os_sunos5_h__
#define __os_sunos5_h__

/*
 * SunOS 5.x
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/cpu.h>
#include <sys/systeminfo.h>
#include <sys/fbio.h>
#include <sys/mtio.h>
#include <sys/dkio.h>
#include <sys/vtoc.h>
#include <sys/mnttab.h>
#include <sys/vfstab.h>

#if	!defined(ELF)
#define ELF 		1
#endif	/* ELF */

#define IS_POSIX_SOURCE
#define HAVE_ANONINFO
#define HAVE_DEVICE_SUPPORT
#define HAVE_DLPI
#define HAVE_GETHOSTID
#define HAVE_KVM
#define HAVE_NLIST
#define HAVE_SYSINFO
#define HAVE_DDI_PROP
#define NEED_SOCKIO

/*
 * SPARC Specifics
 */
#if 	defined(sparc)
#define HAVE_SUNROMVEC
#define HAVE_OPENPROM
#define HAVE_HDIO
/*
 * The IDPROM interface was removed as of SunOS 5.6.
 */
#if	defined(OSVER) && OSVER < 56
#define HAVE_IDPROM
#endif	/* OSVER */
#endif	/* sparc */

/*
 * We're done define HAVE_* features
 */
#include "os-sunos.h"

/*
 * regcomp(3c) first appeared in SunOS 5.4
 */
#if	defined(OSVER) && OSVER < 54
#define RE_TYPE			RE_REGCMP
#else
#define RE_TYPE			RE_REGCOMP
#endif	/* OSVER */

/*
 * Paths
 */
#define _PATH_DEV		"/dev"
#define _PATH_DEV_DSK		"/dev/dsk"
#define _PATH_DEV_FBS		"/dev/fbs"
#define _PATH_DEV_RDSK		"/dev/rdsk"
#define _PATH_DEV_RMT		"/dev/rmt"

#if defined(HAVE_IDPROM)
#	include <sys/idprom.h>
#endif
#if defined(HAVE_OPENPROM)
#	include <sys/openpromio.h>
#endif
#if defined(HAVE_HDIO)
#	include <sys/hdio.h>
#endif

/*
 * Types
 */
typedef struct vtoc		DKvtoc;
typedef struct dk_cinfo		DKcinfo;
typedef struct dk_geom		DKgeom;
#if defined(HAVE_HDIO)
typedef struct hdk_type		DKtype;
#else
typedef void			DKtype;
#endif	/* HAVE_HDIO */

#endif	/* __os_sunos5_h__ */
