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

#ifndef __os_freebsd_h__
#define __os_freebsd_h__

/*
 * FreeBSD from http://www.freebsd.org
 */

#include <fcntl.h>
#include <sys/param.h>
#include <sys/sockio.h>
/*
 * Need these in FreeBSD 3.0 and later to get netif stuff compiled.
 */
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_types.h>
#if	OSMVER >= 3
#include <net/if_var.h>
#endif	/* OSMVER >= 3 */

/*
 * There's no such thing as a single manufacturer for FREEBSD systems.
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
#define HAVE_ETHER_NTOHOST	/* Have ether_ntohost() */

/*
 * What features we need
 */

/*
 * What we are
 */
#define _BIT_FIELDS_LTOH	/* Little Indian byte order */
#define IS_POSIX_SOURCE		1
#define RE_TYPE			RE_REGCOMP
#define GETNETIF_TYPE		GETNETIF_IFCONF
#define SETMACINFO_FUNC		SetMacInfoSysCtl

/*
 * Constants
 */
#define NSWAP_SIZE		8192	/* Size of "nswap" blocks */
#define MAX_PCI_DEVICES		255
#define PCI_DEV_NAME_MAX	128	/* Max size of a PCI dev name */

/*
 * Pathnames
 */
#define _PATH_DEV		"/dev"
#define _PATH_DEVPCI		"/dev/pci"

#endif	/* __os_freebsd_h__ */
