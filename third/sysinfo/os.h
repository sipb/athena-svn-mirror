/*
 * Copyright (c) 1992-1998 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not
 * sold for profit or used in part or in whole for commercial gain
 * without prior written agreement, and the author is credited
 * appropriately.
 */

/*
 * $Revision: 1.1.1.3 $
 */

#ifndef __sysinfo_system__
#define __sysinfo_system__

/*
 * System dependent information
 */

#if defined(sun)
#	if OSMVER == 5 || defined(__svr4__)
#		include "os-sunos5.h"
#	else
#		include "os-sunos4.h"
#	endif
#endif /* sun */

#if defined(_AIX)
#	include "os-aix.h"
#endif /* _AIX */

#if (defined(hpux) || defined(__hpux)) && !defined(__convex__)
#	include "os-hpux.h"
#endif /* hpux */

#if defined(__convex__) || defined(__convex_spp)
#	include "os-sppux.h"
#endif /* __convex__ */

#if defined(sgi)
#      	include "os-irix.h"
#endif

#if defined(linux)
#	include "os-linux.h"
#endif

#if defined(__bsdi__)
#	include "os-bsdos.h"
#endif

#if defined(__FreeBSD__)
#	include "os-freebsd.h"
#endif

/*
 * Everything depends on HAVE_KVM
 */
#if	defined(NEED_KVM) && !defined(HAVE_KVM)
#define HAVE_KVM
#endif

#endif /* __sysinfo_system__ */
