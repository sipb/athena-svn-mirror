/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

/*
 * $Revision: 1.1.1.1 $
 */

#ifndef __os_aix_h__
#define __os_aix_h__

/*
 * IBM AIX
 */
#include <string.h>
#include <sys/file.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>

#define MAN_SHORT		"IBM"
#define MAN_LONG		"International Business Machines Corporation"
#define ARCH_TYPE		"powerpc"
#define KARCH_TYPE		ARCH_TYPE
#define HAVE_DEVICE_SUPPORT
#define HAVE_PARTINFO_SUPPORT
#define HAVE_SOFTINFO_SUPPORT
#define HAVE_STR_DECLARE
#define NAMELIST		"/unix"
#define UNAME_REL_VER_COMB
#define _PATH_ODM		"/etc/objrepos"

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
