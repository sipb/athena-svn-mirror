/* $Header: /afs/transarc.com/project/fs/dev/afs/rcs/vol/RCS/partition.h,v 2.11 1994/06/13 21:02:08 vasilis Exp $ */
/* $Source: /afs/transarc.com/project/fs/dev/afs/rcs/vol/RCS/partition.h,v $ */

#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsidpartition = "$Header: /afs/transarc.com/project/fs/dev/afs/rcs/vol/RCS/partition.h,v 2.11 1994/06/13 21:02:08 vasilis Exp $";
#endif

/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1987
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
/*

	System:		VICE-TWO
	Module:		partition.h
	Institution:	The Information Technology Center, Carnegie-Mellon University

 */

#include <afs/param.h>
#include "nfs.h"
#if	defined(AFS_HPUX_ENV) || defined(AFS_NCR_ENV)
#define	AFS_DSKDEV	"/dev/dsk"
#define	AFS_RDSKDEV	"/dev/rdsk/"
#define	AFS_LVOLDEV	"/dev/vg0"
#define	AFS_ACVOLDEV	"/dev/ac"
#define	AFS_RACVOLDEV	"/dev/ac/r"
#else
#define	AFS_DSKDEV	"/dev"
#define	AFS_RDSKDEV	"/dev/r"
#endif

/* All Vice partitions on a server will have the following name prefix */
#define VICE_PARTITION_PREFIX	"/vicep"
#define VICE_PREFIX_SIZE	(sizeof(VICE_PARTITION_PREFIX)-1)

struct DiskPartition {
    struct DiskPartition *next;
    char	name[32];	/* Mounted partition name */
    char	devName[32];	/* Device mounted on */
    Device	device;		/* device number */
    int		lock_fd;	/* File descriptor of this partition if locked; otherwise -1;
    				   Not used by the file server */
    int		free;		/* Total number of blocks (1K) presumed
				   available on this partition (accounting
				   for the minfree parameter for the
				   partition).  This is adjusted
				   approximately by the sizes of files
				   and directories read/written, and
				   periodically the superblock is read and
				   this is recomputed.  This number can
				   be negative, if the partition starts
				   out too full */
    int		totalUsable;	/* Total number of blocks available on this
    				   partition, taking into account the minfree
				   parameter for the partition (see the
				   4.2bsd command tunefs, but note that the
				   bug mentioned there--that the superblock
				   is not reread--does not apply here.  The
				   superblock is re-read periodically by
				   VSetPartitionDiskUsage().) */
    int		minFree;	/* Percentage to be kept free, as last read
    				   from the superblock */
    int		flags;
};
#define	PART_DONTUPDATE	1

struct DiskPartition *DiskPartitionList;
struct DiskPartition *VGetPartition();
