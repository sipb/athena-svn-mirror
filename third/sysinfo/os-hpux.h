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

#ifndef __os_hpux_h__
#define __os_hpux_h__

/*
 * HP HP-UX
 */

#include <sys/param.h>

#define MAN_SHORT		"HP"
#define MAN_LONG		"Hewlett-Packard Company"

#if 	OSMVER >= 10
#define NAMELIST		"/stand/vmunix"
#else
#define NAMELIST		"/hp-ux"
#endif	/* OSMVER >= 10 */

#define VERSION_SYM		"_release_version"
#define PHYSMEM_SYM		"physmem"
#define IS_POSIX_SOURCE		1
#define NEED_KVM		1
#define HAVE_NLIST		1
#define RE_TYPE			RE_REGCOMP

/*
 * Don't check Serial numbers when adding devices to the device tree.
 * We need to skip this because SCSI device's Serial numbers on HPUX
 * are usually not unique.  They appear to be used for other info.
 */
#define DONT_FIND_ON_SERIAL	1

/*
 * Device support depends on ioscan(1m) which appears first in HPUX 10.x
 */
#if	OSMVER >= 10
/* HPUX cc doesn't appear to define __STDC__ */
#ifndef __STDC__
#define __STDC__
#endif
#define HAVE_STDARG
#define IOSCAN_ARGS		"-k -n -F"
#define HAVE_DEVICE_SUPPORT
#define HAVE_SCSI_CTL
#define SETMACINFO_FUNC		HPUXSetMacInfo
#define _PATH_IOSCAN		"/usr/sbin/ioscan"
#endif	/* OSMVER >= 10 */

#if	OSMVER == 9
#define HAVE_VARARGS
/* For compatibility */
#define setreuid(r,e)		setresuid(r,e,-1)
#endif	/* OSMVER == 9 */

/*
 * General pathnames
 */
#define _PATH_DEV_DSK		"/dev/dsk"
#define _PATH_DEV_RDSK		"/dev/rdsk"
#define _PATH_DEV_RMT		"/dev/rmt"
#define _PATH_DEV_RSCSI		"/dev/rscsi"

#ifndef _BIT_FIELDS_HTOL
#define _BIT_FIELDS_HTOL 1	/* High-to-low bit order */
#endif

/*
 * Run Test Programs using sh(1) since HPUX (9.x)
 * does not put a #! in the header of /bin/hp9000s* programs.
 */
#define RUN_TEST_CMD		{ "/bin/sh", "-c", NULL }

/*
 * Constants
 */
#define HP_KA_HP200		"hp9000s200"
#define HP_KA_HP300		"hp9000s300"
#define HP_KA_HP400		"hp9000s400"
#define HP_KA_HP500		"hp9000s500"
#define HP_KA_HP700		"hp9000s700"
#define HP_KA_HP800		"hp9000s800"
#define HP_KA_HPCOMB		"hp9000"
#define HP_AA_PARISC		"PA-RISC"
#define HP_AA_MC68K		"MC680x0"

/*
 * Default Frame Buffer device file
 */
#define FB_DEFAULT_FILE		"/dev/crt"

/*
 * Compatibility
 */
#define GETPAGESIZE()		sysconf(_SC_PAGE_SIZE)

#endif	/* __os_hpux_h__ */
