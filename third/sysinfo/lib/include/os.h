/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

/*
 * $Revision: 1.1.1.1 $
 */

#ifndef __sysinfo_system__
#define __sysinfo_system__

#if	defined(HAVE_AUTOCONFIG_H)
#include "autoconfig.h"
#endif	/* HAVE_AUTOCONFIG_H */

/*
 * System dependent information
 */

#if defined(sun)
#	if OSMVER == 5 || defined(__svr4__)
#		include "sunos5.h"
#	else
#		include "sunos4.h"
#	endif
#endif /* sun */

#if defined(_AIX)
#	include "aix.h"
#endif /* _AIX */

#if (defined(hpux) || defined(__hpux)) && !defined(__convex__)
#	include "hpux.h"
#endif /* hpux */

#if defined(__convex__) || defined(__convex_spp)
#	include "sppux.h"
#endif /* __convex__ */

#if defined(sgi)
#      	include "irix.h"
#endif

#if defined(linux)
#	include "linux.h"
#endif

#if defined(__bsdi__)
#	include "bsdos.h"
#endif

#if defined(__FreeBSD__)
#	include "freebsd.h"
#endif

/*
 * Everything depends on HAVE_KVM
 */
#if	defined(NEED_KVM) && !defined(HAVE_KVM)
#define HAVE_KVM
#endif

#endif /* __sysinfo_system__ */
