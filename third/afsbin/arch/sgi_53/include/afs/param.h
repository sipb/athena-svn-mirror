#ifndef	_PARAM_SGI_50_H
#define	_PARAM_SGI_50_H

#include <afs/afs_sysnames.h>

#define AFS_VFS_ENV		1
#define AFS_VFSINCL_ENV		1
#define AFS_ENV			1	/* NOBODY uses this.... */
#define CMUSTD_ENV		1	/* NOBODY uses this.... */
#define AFS_SGI_ENV		1
#define AFS_SGI51_ENV		1	/* Dist from 5.0.1 */
#define	AFS_SGI52_ENV		1
#define	AFS_SGI53_ENV		1
#define AFS_SGI_EXMAG		1	/* use magic fields in extents for AFS extra fields */
#define AFS_SGI_SHORTSTACK	1	/* only have a 1 page kernel stack */

#define _ANSI_C_SOURCE		1	/* rx_user.h */

#define AFS_PIOCTL	64+1000
#define AFS_SETPAG	65+1000
#define AFS_IOPEN	66+1000
#define AFS_ICREATE	67+1000
#define AFS_IREAD	68+1000
#define AFS_IWRITE	69+1000
#define AFS_IINC	70+1000
#define AFS_IDEC	71+1000
#define AFS_SYSCALL	73+1000

/* File system entry (used if mount.h doesn't define MOUNT_AFS */
#define AFS_MOUNT_AFS	 "afs"

/* Machine / Operating system information */
#define sys_sgi_50	1
#define SYS_NAME	"sgi_53"
#define SYS_NAME_ID	SYS_NAME_ID_sgi_53
#define AFSBIG_ENDIAN	1

/* Extra kernel definitions (from kdefs file) */
#ifdef KERNEL
/* definitions here */
#define	AFS_VFS34		1	/* afs/afs_vfsops.c (afs_vget), afs/afs_vnodeops.c (afs_lockctl, afs_noop) */
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
#define	osi_GetTime(x)	microtime(x)
#define	AFS_KALLOC(n)	kmem_alloc(n, KM_SLEEP)
#define	AFS_KFREE	kmem_free
#define	VATTR_NULL	vattr_null
#define	DEBUG		1
#endif /* KERNEL */

#ifndef CMSERVERPREF
#define CMSERVERPREF
#endif
#endif	/* _PARAM_SGI_50_H_ */
