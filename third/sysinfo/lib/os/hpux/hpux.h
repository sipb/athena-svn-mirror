/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

/*
 * $Revision: 1.1.1.1 $
 */

#ifndef __os_hpux_h__
#define __os_hpux_h__

/*
 * HP's HP-UX
 */

#include <sys/param.h>
#if	defined(HAVE_NETINET_IN_H)
#include <netinet/in.h>		/* For ntohl() */
#endif

#define MAN_SHORT		"HP"
#define MAN_LONG		"Hewlett-Packard Company"

#if 	OSMVER >= 10
#define NAMELIST		"/stand/vmunix"
#else
#define NAMELIST		"/hp-ux"
#endif	/* OSMVER >= 10 */

#define VERSION_SYM		"_release_version"
#define PHYSMEM_SYM		"physmem"

/*
 * Don't check Serial numbers when adding devices to the device tree.
 * We need to skip this because SCSI device's Serial numbers on HPUX
 * are usually not unique.  They appear to be used for other info.
 */
#define DONT_FIND_ON_SERIAL	1

/*
 * Undocumented file which provides a mapping of model to CPU Type.
 */
#define _PATH_SCHED_MODELS	"/usr/sam/lib/mo/sched.models"

/*
 * Device support depends on ioscan(1m) which appears first in HPUX 10.x
 */
#if	OSMVER >= 10
/*#define HAVE_SOFTINFO_SUPPORT XXX It's broken right now */
#define HAVE_DEVICE_SUPPORT
#define HAVE_SCSI_CTL
#define SETMACINFO_FUNC		HPUXSetMacInfo
#define _PATH_IOSCAN		"/usr/sbin/ioscan"
#define IOSCAN_ARGS		"-k -n -F"
#endif	/* OSMVER >= 10 */

#if	OSMVER == 9
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
