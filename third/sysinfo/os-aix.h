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

#ifndef __os_aix_h__
#define __os_aix_h__

/*
 * IBM AIX
 */

#include <sys/file.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>

#define MAN_SHORT		"IBM"
#define MAN_LONG		"International Business Machines Corporation"
#define ARCH_TYPE		"power"
#define KARCH_TYPE		ARCH_TYPE
#define IS_POSIX_SOURCE		1
#define HAVE_DEVICE_SUPPORT
#define HAVE_GETHOSTID
#define HAVE_KNLIST
#define HAVE_NLIST
#define HAVE_STR_DECLARE
#define HAVE_UNAME
#define HAVE_VARARGS
#define HAVE_WAITPID
#define HAVE_IN_IFADDR
#define NAMELIST		"/unix"
#define NEED_KVM
#define UNAME_REL_VER_COMB
#define WAITARG_T		int
#define _PATH_ODM		"/etc/objrepos"
#define RE_TYPE			RE_COMP

/*
 * We don't need Root Access
 */
#define RA_LEVEL		RA_NONE

/*
 * Directories where the catalog files of devices reside.
 * The file is usually called "devices.cat" and can be
 * found in "/usr/lib/methods" as well as "/usr/lib/nls/msg/%L"
 * The former usually contains a later version than the latter.
 */
#define ENV_NLSPATH \
"NLSPATH=/usr/lib/methods/%N:/usr/lib/nls/msg/%L/%N:/usr/lib/nls/msg/prime/%N"

/*
 * Names of files that can contain our OS version.
 */
#define OSV_MAINT	"/usr/lib/instl/maintopts"
#define OSV_OPP		"/usr/lib/instl/newopps"

/*
 * Name of default national language ($LANG)
 */
#define DEFAULT_LANG	"En_US"

/*
 * Node Name
 */
#define NN_SYS0		"sys0"

/*
 * Attribute strings
 */
#define AT_SIZE		"size"
#define AT_MODELCODE	"modelcode"
#define AT_REALMEM	"realmem"

/*
 * Vital Product Data Information
 */
typedef struct {
    char	       *Code;			/* VPD ID code */
    char	       *Value;			/* Cleaned up value */
} VPDinfo_t;

#endif	/* __aix_h__ */
