/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
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
#if	defined(HAVE_SYS_SOCKIO_H)
#include <sys/sockio.h>
#endif
/*
 * Need these in FreeBSD 3.0 and later to get netif stuff compiled.
 */
#include <sys/socket.h>
#if	defined(HAVE_NET_IF_H)
#include <net/if.h>
#endif
#if	defined(HAVE_NET_IF_TYPES_H)
#include <net/if_types.h>
#endif
#if	defined(HAVE_NET_IF_VAR_H)
#include <net/if_var.h>
#endif

#if	OSMVER <= 3
/*
 * Headers required for ISA*() functions
 */
#include <sys/types.h>
#include <i386/isa/isa_device.h>	/* ISA*() functions */
#include <sys/buf.h>			/* For fdc.h */
#include <i386/isa/fdc.h>		/* ProbeFloppyCtlr() */
#endif	/* OSMVER <= 3 */

/*
 * There's no such thing as a single manufacturer for FREEBSD systems.
 */
#undef MAN_SHORT
#undef MAN_LONG

/*
 * What features we have
 */
#define HAVE_DEVICE_SUPPORT	/* Sysinfo Device support */

/*
 * What features we need
 */

/*
 * What we are
 */
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
