/*
 * Copyright (c) 1992-1998 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not
 * sold for profit or used in part or in whole for commercial gain
 * without prior written agreement, and the author is credited
 * appropriately.
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
#include <sys/sockio.h>

/*
 * There's no such thing as a single manufacturer for BSDOS systems.
 */
#undef MAN_SHORT
#undef MAN_LONG

/*
 * What features we have
 */
#define HAVE_GETHOSTID		/* gethostid() */
#define HAVE_UNAME		/* uname() */
#define HAVE_KVM		/* kvm(3) interface */
#define HAVE_NLIST		/* nlist(3) interface */
#define HAVE_DEVICE_SUPPORT	/* Sysinfo Device support */
#if	OSMVER <= 3
/* Starting in BSD/OS 4, in_ifaddr is still present, but not what we need */
#define HAVE_IN_IFADDR		/* struct in_ifaddr */
#endif	/* OSMVER <= 3 */

/*
 * What we are
 */
#define _BIT_FIELDS_LTOH	/* Little Indian byte order */
#define IS_POSIX_SOURCE		1
#define RE_TYPE			RE_REGCOMP
#define GETNETIF_TYPE		GETNETIF_IFCONF
#define SETMACINFO_FUNC		SetMacInfoSysCtl

/*
 * Pathnames
 */
#define _PATH_DEV		"/dev"

#endif	/* __os_bsdos_h__ */
