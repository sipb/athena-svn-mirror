/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

/*
 * $Revision: 1.1.1.1 $
 */

#ifndef __os_linux_h__
#define __os_linux_h__

/*
 * Linux
 */

#include <fcntl.h>
#include <mntent.h>
#include <sys/param.h>
#include <linux/sockios.h>	/* For netif.c */

/*
 * There's no such thing as a single manufacturer for Linux systems.
 */
#undef MAN_SHORT
#undef MAN_LONG

/*
 * What features we have
 */
#define HAVE_ATA		/* Have ATA/IDE support */
#define HAVE_SOFTINFO_SUPPORT	/* Software Information supported */
#define HAVE_DEVICE_SUPPORT	/* Device Informatin supported */

/*
 * Pathnames and such.
 */
#ifndef _PATH_DEV
#define _PATH_DEV		"/dev"
#endif	/* _PATH_DEV */
#define PROC_FILE_UPTIME	"/proc/uptime"
#define PROC_FILE_CPUINFO	"/proc/cpuinfo"
#define PROC_FILE_MEMINFO	"/proc/meminfo"
#define PROC_FILE_VERSION	"/proc/version"
#define PROC_PATH_BUS_PCI	"/proc/bus/pci"
#define PROC_FILE_PCI_DEV	"/proc/bus/pci/devices"

/*
 * Our local Device Definetions
 */
#ifndef MAX_SCSI_UNITS
#define MAX_SCSI_UNITS		26		/* Max # of devs total */
#endif
#ifndef MAX_ATA_UNITS
#define MAX_ATA_UNITS		4		/* Max # of devs / bus */
#endif

/*
 * Linux HD command type.  This type is used as input to HDIO_DRIVE_CMD.
 * It is undocumented.  It's used by /usr/src/linux/drivers/block/ide.c 
 * (at least as of Linux 2.2.12).  There really should be a defined struct
 * for this in a nice header file in /usr/include, but there isn't.
 *
 * This type must be exactly 4 bytes.
 */
typedef struct {
    u_char		Cmd;		/* Command to issue */
    u_char		NumSects;	/* ??? # of Sectors */
    u_char		Feature;	/* ??? Feature */
    u_char		SectLen;	/* Length of results in 512 byte 
					   units (1 sector=512 bytes) */
} HDcmd_t;
#define HDCMD_OFFSET	(sizeof(HDcmd_t))

#endif	/* __os_linux_h__ */
