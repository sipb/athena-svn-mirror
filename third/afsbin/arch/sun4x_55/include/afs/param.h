#ifndef	_PARAM_SUN4C_51_H_
#define	_PARAM_SUN4C_51_H_

/*
XCFLAGS=-I/usr/ucbinclude
XLIBS=/lib/libsocket.a /usr/ucblib/libucb.a
*/

#include <afs/afs_sysnames.h>

#define AFS_VFS_ENV	1
/* Used only in vfsck/* code; is it needed any more???? */

#define AFS_VFSINCL_ENV	1	/* NOBODY uses this.... */
#define AFS_GREEDY43_ENV	1	/* Used only in rx/rx_user.c */
#define AFS_ENV			1
#define AFS_SUN_ENV		1
#define AFS_SUN5_ENV		1
#define	AFS_SUN52_ENV		1
#define	AFS_SUN53_ENV		1
#define	AFS_SUN54_ENV		1
#define	AFS_SUN55_ENV		1
#define AFS_BSD_ENV		1
#define AFS_GLOBAL_SUNLOCK	1	/* For fine mp lock granularity comment this out */
/*#define AFS_FINEGR_SUNLOCK	1	/* For global locking comment this out */

#define	AFS_3DISPARES		1	/* Utilize the 3 available disk inode 'spares' */
#define	AFS_SYSCALL		105

/* File system entry (used if mount.h doesn't define MOUNT_AFS */
#define AFS_MOUNT_AFS	 "afs"

/* Machine / Operating system information */
#define sys_sun4x_55	1
#define SYS_NAME	"sun4x_55"
#define SYS_NAME_ID	SYS_NAME_ID_sun4x_55
#define AFSBIG_ENDIAN	1
#define AFS_HAVE_FFS            1       /* Use system's ffs. */

/* Extra kernel definitions (from kdefs file) */
#ifdef KERNEL
/* sun definitions here */
#define	AFS_UIOFMODE		1	/* Only in afs/afs_vnodeops.c (afs_ustrategy) */
#define	AFS_SYSVLOCK		1	/* sys v locking supported */
/*#define	AFS_USEBUFFERS	1*/
#define	afsio_iov		uio_iov
#define	afsio_iovcnt	uio_iovcnt
#define	afsio_offset	uio_offset
#define	afsio_seg		uio_segflg
#define	afsio_fmode	uio_fmode
#define	afsio_resid	uio_resid
#define	AFS_UIOSYS	UIO_SYSSPACE
#define	AFS_UIOUSER	UIO_USERSPACE
#define	AFS_CLBYTES	MCLBYTES
#define	AFS_MINCHANGE	2
#define	osi_GetTime(x)	uniqtime(x)
#define	AFS_KALLOC(n)	kmem_alloc(n, KM_SLEEP)
#define	AFS_KFREE	kmem_free
#define	VATTR_NULL	vattr_null
#endif KERNEL
#define	AFS_DIRENT	
#ifndef CMSERVERPREF
#define CMSERVERPREF
#endif
#define	ROOTINO		UFSROOTINO


#endif	_PARAM_SUN4C_51_H_