/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1987, 1988
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */

/*
 * Auxilliary include file for vice changes to inodes.
 */
#ifndef	__AUXINODEH__
#define	__AUXINODEH__

#ifndef KERNEL
#include <afs/param.h>
#endif
#define VICEMAGIC       0x84fa1cb6
#ifdef	AFS_AIX22_ENV
#define	OLD_STUFF
#endif

/* These are totally ignored by Aix/Aux since no available fields are possible on a 64 byte inode.h structure that most sys-V systems have */
#if	(!defined(AFS_AUX_ENV) && !defined(AFS_AIX22_ENV) && !defined(AFS_SGI_ENV))

#ifdef	AFS_AIX32_ENV
/*
 * lost of spare stuff in the inode, so we'll take the last ones in the
 * hope that this will make us less vulnerable to collisions with other
 * desired uses.
 */
#define	di_vicemagic	di_rsrvd[0]
#define	di_vicep1	di_rsrvd[1]
#define	di_vicep2	di_rsrvd[2]
#define	di_vicep3	di_rsrvd[3]
#define	di_vicep4	di_rsrvd[4]

#define	i_vicemagic	i_dinode.di_vicemagic
#define	i_vicep1	i_dinode.di_vicep1
#define	i_vicep2	i_dinode.di_vicep2
#define	i_vicep3	i_dinode.di_vicep3
#define	i_vicep4	i_dinode.di_vicep4

#else
#ifdef i_gen

#ifdef	AFS_HPUX_ENV
#define i_vicemagic	i_icun.i_ic.ic_gen
#define i_vicep1        i_icun.i_ic.ic_spare[0]
#define i_vicep2        i_icun.i_ic.ic_spare[1]
#define i_vicep4        i_icun.i_ic.ic_flags
#define i_vicep3        i_icun.i_ic.ic_size.val[0]

#define di_vicemagic	di_ic.ic_gen
#define di_vicep1       di_ic.ic_spare[0]
#define di_vicep2       di_ic.ic_spare[1]
#define di_vicep4       di_ic.ic_flags
#define di_vicep3       di_ic.ic_size.val[0]
#else

#ifndef	AFS_DEC_ENV

#ifdef	AFS_OSF_ENV
/*
 * This probably is not correct, but for now at least things will compile.
 * In particular, the di_gen field may not be available for use.
 */
#define i_vicemagic	i_din.di_gen
#define i_vicep1	i_din.di_spare[0]
#define i_vicep2	i_din.di_spare[1]
#define i_vicep3	i_din.di_spare[2]
#define i_vicep4	i_din.di_spare[3]

#define di_vicemagic	di_gen
#define di_vicep1	di_spare[0]
#define di_vicep2	di_spare[1]
#define di_vicep3	di_spare[2]
#define di_vicep4	di_spare[3]

#else	/* AFS_OSF_ENV */

#define i_vicemagic	i_ic.ic_gen
#define i_vicep1        i_ic.ic_spare[0]
#define i_vicep2        i_ic.ic_spare[1]
#define i_vicep3        i_ic.ic_spare[2]
#define i_vicep4        i_ic.ic_spare[3]

#define di_vicemagic	di_ic.ic_gen
#define di_vicep1	di_ic.ic_spare[0]
#define di_vicep2	di_ic.ic_spare[1]
#define di_vicep3	di_ic.ic_spare[2]
#define di_vicep4	di_ic.ic_spare[3]
#endif	/* AFS_OSF_ENV */

#else 

#define	di_vicemagic	di_ic.dg_gennum
#define	di_vicep1	di_ic.dg_spare[0]
#define	di_vicep2	di_ic.dg_spare[1]
#define	di_vicep3	di_ic.dg_spare[2]
#define	di_vicep4 	di_ic.dg_spare[3]

#define	i_vicemagic	di_vicemagic
#define	i_vicep1	di_vicep1
#define	i_vicep2	di_vicep2
#define	i_vicep3	di_vicep3
#define	i_vicep4	di_vicep4
#endif
#endif /* AFS_HPUX_ENV */

#else /* i_gen */

#ifndef	AFS_DEC_ENV
#define i_vicemagic	i_ic.ic_spare[0]
#define i_vicep1        i_ic.ic_spare[1]
#define i_vicep2        i_ic.ic_spare[2]
#define i_vicep3        i_ic.ic_spare[3]
#define i_vicep4        i_ic.ic_spare[4]

#define di_vicemagic	di_ic.ic_spare[0]
#define di_vicep1	di_ic.ic_spare[1]
#define di_vicep2	di_ic.ic_spare[2]
#define di_vicep3	di_ic.ic_spare[3]
#define di_vicep4	di_ic.ic_spare[4]
#else 
/* ultrix 2.2 comes in here */
#define	di_vicemagic	di_ic.dg_gennum
#define	di_vicep1	di_ic.dg_spare[0]
#define	di_vicep2	di_ic.dg_spare[1]
#define	di_vicep3	di_ic.dg_spare[2]
#define	di_vicep4 	di_ic.dg_spare[3]

#define	i_vicemagic	di_vicemagic
#define	i_vicep1	di_vicep1
#define	i_vicep2	di_vicep2
#define	i_vicep3	di_vicep3
#define	i_vicep4	di_vicep4
#endif	
#endif /* i_gen */

#endif /* !AFS_AIX32_ENV	*/
#endif /*(!defined(AFS_AUX_ENV) && !defined(AFS_AIX22_ENV))*/

#if	defined(AFS_AUXVOLFILE) || defined(AFS_SUN_ENV) || defined(AFS_SUN5_ENV)
/* 
 * This is a temporary hack for handling the vol stuff in aix 
 */
#define	AUXVOLFILE  ".VOLFILE"
#define	BACKAUXDIR  "/usr/afs/backup"
#ifdef	AFS_3DISPARES
#undef di_vicemagic
#undef di_vicep1
#undef di_vicep2
#undef di_vicep3
/*#define di_vicemagic	di_vicep2*/
#define di_vicep1	di_ic.ic_gen		/* Volume Id */
#define di_vicep2	di_ic.ic_flags		/* Unique */
#if	defined(AFS_NCR_ENV) || defined(AFS_X86_ENV)
#define di_vicep3	di_ic.ic_size.val[1]	/* DV + vnode */
#else
#define di_vicep3	di_ic.ic_size.val[0]	/* DV + vnode */
#endif

#undef i_vicemagic
#undef i_vicep1
#undef i_vicep2
#undef i_vicep3
/*#define i_vicemagic	i_vicep2*/
#define i_vicep1	i_ic.ic_gen
#define i_vicep2	i_ic.ic_flags
#if	defined(AFS_NCR_ENV) || defined(AFS_X86_ENV)
#define i_vicep3	i_ic.ic_size.val[1]	/* DV + vnode */
#else
#define i_vicep3	i_ic.ic_size.val[0]
#endif
#else /* AFS_3DISPARES */
/*
 * We use the only field that it's still unused (ic_flags) in dinode to handle the magic field
 * Note that is used to distinguish AFS files from non-AFS files and allows us to save some
 * additional I/O (for deletes) to the auxiliary file...
 */
#if	defined(AFS_SUN_ENV) || defined(AFS_SUN5_ENV)
#undef	i_vicemagic
#undef	di_vicemagic
#define i_vicemagic	i_ic.ic_gen			
#define di_vicemagic	di_ic.ic_gen			
#endif

struct afs_auxheader {
    int32	magic;	    
    int32	spare1;
};

/* 
 * Disk image of the auxinode sructute (no spares to cut the size of the file low!)
 */
struct dauxinode {
#ifdef	AFS_AIX22_ENV
    int32	aux_magic;
#else
    int32	aux_inode;
#endif
    int32	aux_param1;
    int32	aux_param2;
    int32	aux_param3;
    int32	aux_param4;
};

struct sauxinode {		/* "In-memory" version of dauxinode */
    struct sauxinode *aux_next;
    int32	aux_alive;	/* Inode is still a valid AFS one */
    int32	aux_inode;
    int32	aux_param1;
    int32	aux_param2;
    int32	aux_param3;
    int32	aux_param4;
};

#ifdef	KERNEL
#define	AUX_MSGSIZE	409
struct aux_msgbuf {
    struct {
	struct afs_lock lockw;
	int32 bufc;
	int32 bufw;
	int32 bufr;
    } hd;
#define	m_lockw	hd.lockw
#define	m_bufc	hd.bufc
#define	m_bufw	hd.bufw
#define	m_bufr	hd.bufr
    struct dauxinode bufd[AUX_MSGSIZE];
};


/*
 * A structure per "/vicep" partition; mainly used to get easy access to its
 * VOLFILE (i.e. tfile field below)
 */
struct afspart {
    struct afspart  *next;
    int32	    fileInode;
    struct osi_dev  cdev;
    int32	    maxIndex;
    struct osi_file *tfile;
    int32	    modTime;
    struct aux_msgbuf *bufp;
};


/* 
 * Virtual memory image of the structure 
 */
struct vauxinode {
    struct vauxinode *aux_next;
    struct afspart  *aux_afspart;
    struct afs_q    aux_lruq;
    int32	    aux_modTime;
    dev_t	    aux_dev;
    ino_t	    aux_ino;
    int32	    aux_dirty;
    int32	    aux_refCount;
    struct dauxinode aux_dimage;
};


/*
 * General auxiliary file stats
 */
struct auxstats {
#ifdef	OLD_STUFF
    u_int32	hits;		/* Cache hits */
    u_int32	misses;		/* Cache misses (also disk reads) */
    u_int32	writes;		/* Number of disk writes */
    u_int32	creates;      	/* Number of newly created anodes (icreates) */

    u_int32	deletes;      	/* Number of zeroed (on disk) entries */
    u_int32	gccnt;		/* Number of calls to standard gc routine */
    u_int32	gcflushold;	/* Number of flushed "old" entries */
    u_int32	gcflushdirty;	/* Number of flushed "dirty" entries */

    u_int32	reuse;		/* Number of reused "dirty" entries */
    u_int32	reusebad;	/* Number of bad (not deleted) reused entries */
    u_int32	overrun;	/* Not free slots; allocate new one! */
#else
    u_int32	writes;		/* Number of disk writes */
    u_int32	flushbuf;	/* Number of flush buffer called */
    u_int32	msgbuf;		/* Number of calls to flush_auxmsgbuf */
    u_int32	msgbuf1;	/* Number of calls to flush_auxmsgbuf1 */
    u_int32	offline;	/* Number of "offline" icreate calls */
#endif
};

#endif /* KERNEL */
#endif /* AFS_3DISPARES */
#endif	/* defined(AFS_AUXVOLFILE) || defined(AFS_SUN_ENV) */


#if defined(AFS_SGI_ENV) && defined(AFS_SGI_EXMAG)
/*
 * We use the 12 8-bit unused ex_magic fields!
 * Plus 2 values of di_version
 * di_version = 0 - current EFS
 *	    1 - AFS INODESPECIAL
 *	    2 - AFS inode
 * AFS inode:
 * magic[0-3] - VOLid
 * magic[4-6] - vnode number (24 bits)
 * magic[7-9] - disk uniqifier
 * magic[10-11]+di_spare - data version
 *
 * INODESPECIAL:
 * magic[0-3] - VOLid
 * magic[4-7] - parent
 * magic[8]   - type
 */
#define SGI_UNIQMASK	0xffffff
#define SGI_DATAMASK	0xffffff
#define SGI_DISKMASK	0xffffff

/* we hang this struct off of the incore inode */
struct afsparms {
	int32 vicep1;
	int32 vicep2;
	int32 vicep3;
	int32 vicep4;
};
#endif /* defined(AFS_SGI_ENV) && defined(AFS_SGI_EXMAG) */

/* KAZAR-RA */
#define	IFILLING		0x1000	/* file currently being filled from network */
#define	IFILLWAIT		0x2000	/* someone is waiting for this data */
#define	IFILLERR		0x4000	/* an error occurred during the fetch --> set EIO */

#ifdef	AFS_3DISPARES
#define	IS_VICEMAGIC(ip)	(((ip)->i_vicep2 || (ip)->i_vicep3) ? 1 : 0)
#define	IS_DVICEMAGIC(dp)	(((dp)->di_vicep2 || (dp)->di_vicep3) ? 1 : 0)
#define	CLEAR_VICEMAGIC(ip)	(ip)->i_vicep2 = (ip)->i_vicep3 = 0
#define	CLEAR_DVICEMAGIC(dp)	(dp)->di_vicep2 = (dp)->di_vicep3 = 0
#else

#if defined(AFS_SGI_EXMAG)
#define dmag(p,n)	((p)->di_u.di_extents[n].ex_magic)

#define	IS_VICEMAGIC(ip)	(((ip)->i_version == EFS_IVER_AFSSPEC || \
				  (ip)->i_version == EFS_IVER_AFSINO) \
				 ?  1 : 0)
#define	IS_DVICEMAGIC(dp)	(((dp)->di_version == EFS_IVER_AFSSPEC || \
				  (dp)->di_version == EFS_IVER_AFSINO) \
				 ?  1 : 0)
#define	CLEAR_VICEMAGIC(ip)	(ip)->i_version = EFS_IVER_EFS
#define	CLEAR_DVICEMAGIC(dp)	dp->di_version = EFS_IVER_EFS

#else
#define	IS_VICEMAGIC(ip)	((ip)->i_vicemagic == VICEMAGIC ?  1 : 0)
#define	IS_DVICEMAGIC(dp)	((dp)->di_vicemagic == VICEMAGIC ?  1 : 0)
#ifdef AFS_OSF_ENV
#define	CLEAR_VICEMAGIC(ip)	(ip)->i_vicemagic  = (ip)->i_vicep2  = (ip)->i_vicep3  = 0
#define	CLEAR_DVICEMAGIC(dp)	(dp)->di_vicemagic = (dp)->di_vicep2 = (dp)->di_vicep3 = 0
#else
#define	CLEAR_VICEMAGIC(ip)	(ip)->i_vicemagic = 0
#define	CLEAR_DVICEMAGIC(dp)	(dp)->di_vicemagic = 0
#endif
#endif /* AFS_SGI_EXMAG */
#endif
#endif /* __AUXINODEH__ */
