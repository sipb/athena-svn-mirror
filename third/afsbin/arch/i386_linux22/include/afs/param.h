/* Copyright (C) 1998 by Transarc Corporation */


#ifndef _PARAM_I386_LINUX22_H_
#define _PARAM_I386_LINUX22_H_

/* In user space the AFS_LINUX20_ENV should be sufficient. In the kernel,
 * it's a judgment call. If something is obviously i386 specific, use that
 * #define instead. Note that "20" refers to the linux 2.0 kernel. The "2"
 * in the sysname is the current version of the client. This takes into
 * account the perferred OS user space configuration as well as the kernel.
 */

#define AFS_LINUX20_ENV	1
#define AFS_LINUX22_ENV	1
#define AFS_I386_LINUX20_ENV	1
#define AFS_I386_LINUX22_ENV	1
#define AFS_NONFSTRANS 1

#define AFS_MOUNT_AFS "afs"	/* The name of the filesystem type. */
#define AFS_SYSCALL 137
#include <afs/afs_sysnames.h>

#define AFS_USERSPACE_IP_ADDR 1
#define RXK_LISTENER_ENV 1

/* Machine / Operating system information */
#define SYS_NAME	"i386_linux22"
#define SYS_NAME_ID	SYS_NAME_ID_i386_linux22
#define AFSLITTLE_ENDIAN    1
#define AFS_HAVE_FFS            1       /* Use system's ffs. */
#define AFS_VM_RDWR_ENV	1	/* read/write implemented via VM */

#if defined(__KERNEL__) && !defined(KDUMP_KERNEL)
#include <linux/autoconf.h>
#if defined(CONFIG_MODVERSIONS) && !defined(MODVERSIONS)
#define MODVERSIONS 1
#endif
#ifdef MODVERSIONS
#include "./afs_modversions.h"
#endif
#endif /* __KERNEL__ */

#endif /* _PARAM_I386_LINUX20_H_ */
