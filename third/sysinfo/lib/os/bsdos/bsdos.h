/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

/*
 * $Revision: 1.1.1.1 $
 */

#ifndef __os_bsdos_h__
#define __os_bsdos_h__

/*
 * BSD/OS from BSDI
 */

#include <fcntl.h>
#include <sys/param.h>
#if	defined(HAVE_SYS_SOCKIO_H)
#include <sys/sockio.h>
#endif

/*
 * There's no such thing as a single manufacturer for BSDOS systems.
 */
#undef MAN_SHORT
#undef MAN_LONG

/*
 * What features we have
 */
#define HAVE_DEVICE_SUPPORT	/* Sysinfo Device support */

/*
 * What we are
 */
#define SETMACINFO_FUNC		SetMacInfoSysCtl

/*
 * Pathnames
 */
#define _PATH_DEV		"/dev"

#endif	/* __os_bsdos_h__ */
