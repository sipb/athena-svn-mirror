/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

/*
 * $Revision: 1.1.1.1 $
 */

#ifndef __os_sunos5_h__
#define __os_sunos5_h__

/*
 * SunOS 5.x
 */

#include <stdlib.h>		/* Need this if we're a 64-bit app */
#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/cpu.h>
#include <sys/systeminfo.h>
#include <sys/mtio.h>
#include <sys/dkio.h>
#include <sys/vtoc.h>
#include <sys/mnttab.h>
#include <sys/vfstab.h>
#include <sys/fbio.h>

#if	!defined(ELF)
#define ELF 		1
#endif	/* ELF */

#define HAVE_DEVICE_SUPPORT
#define HAVE_SOFTINFO_SUPPORT
#define HAVE_DEVICEDB
/*
 * The ATA code doesn't appear to work in any version of Solaris through
 * Solaris 8 and causes Bus Errors on 32-bit Solaris 7-8 SPARC systems.
 */
#undef HAVE_ATA

/*
 * The DADKIO support exists in 5.6, but is broken.
 */
#if	defined(OSVER) && OSVER <= 56
#undef HAVE_DADKIO
#endif	/* OSVER <= 56 */

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
#include "sunos.h"

/*
 * regcomp(3c) first appeared in SunOS 5.4, but is broken in 5.4.
 */
#if	defined(OSVER) && OSVER < 55
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
#define _PATH_PKG_DIR		"/var/sadm/pkg"
#define _PATH_PKG_CONTENTS	"/var/sadm/install/contents"
#define OS_RELEASE_FILE		"/etc/release"

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
