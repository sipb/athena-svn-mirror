#ifndef _AFS_ARGS_H_
#define _AFS_ARGS_H_
/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1987, 1988
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083

 * AFS system call opcodes
 */

#define	AFSOP_START_RXCALLBACK	  0	/* no aux parms */
#define	AFSOP_START_AFS		  1	/* no aux parms */
#define	AFSOP_START_BKG		  2	/* no aux parms */
#define	AFSOP_START_TRUNCDAEMON	  3	/* no aux parms */
#define AFSOP_START_CS		  4	/* no aux parms */

#define	AFSOP_ADDCELL		  5	/* parm 2 = cell str */
#define	AFSOP_CACHEINIT		  6	/* parms 2-4 -> cache sizes */
#define	AFSOP_CACHEINFO		  7	/* the cacheinfo file */
#define	AFSOP_VOLUMEINFO	  8	/* the volumeinfo file */
#define	AFSOP_CACHEFILE		  9	/* a random cache file (V*) */
#define	AFSOP_CACHEINODE	 10	/* random cache file by inode */
#define	AFSOP_AFSLOG		 11	/* output log file */
#define	AFSOP_ROOTVOLUME	 12	/* non-standard root volume name */
#define	AFSOP_STARTLOG		 14	/* temporary: Start afs logging */
#define	AFSOP_ENDLOG		 15	/* temporary: End afs logging */
#define AFSOP_AFS_VFSMOUNT	 16	/* vfsmount cover for hpux */
#define AFSOP_ADVISEADDR	 17	/* to init rx cid generator */
#define AFSOP_CLOSEWAIT 	 18	/* make all closes synchronous */
#define	AFSOP_RXEVENT_DAEMON	 19	/* rxevent daemon */
#define	AFSOP_GETMTU		 20	/* stand-in for SIOCGIFMTU, for now */
#define	AFSOP_GETIFADDRS	 21	/* get machine's ethernet interfaces */

#define	AFSOP_ADDCELL2		 29	/* 2nd add cell protocol interface */

/* The range 20-30 is reserved for AFS system offsets in the afs_syscall */
#define	AFSCALL_PIOCTL		20
#define	AFSCALL_SETPAG		21
#define	AFSCALL_IOPEN		22
#define	AFSCALL_ICREATE		23
#define	AFSCALL_IREAD		24
#define	AFSCALL_IWRITE		25
#define	AFSCALL_IINC		26
#define	AFSCALL_IDEC		27
#define	AFSCALL_CALL		28

#define AFSCALL_ICL             30

/* 64 bit versions of inode system calls. */
#define AFSCALL_IOPEN64		41
#define AFSCALL_ICREATE64	42
#define AFSCALL_IINC64		43
#define AFSCALL_IDEC64		44
#define AFSCALL_ILISTINODE64	45	/* Used by ListViceInodes */
#define AFSCALL_ICREATENAME64	46	/* pass in platform specific pointer
					 * used to create a name in in a
					 * directory.
					 */
#ifdef AFS_SGI_VNODE_GLUE
#define AFSCALL_INIT_KERNEL_CONFIG 47	/* set vnode glue ops. */
#endif

#ifdef	AFS_SGI53_ENV
#define AFSOP_NFSSTATICADDR	 32	/* to contents addr of nfs kernel addr */
#define AFSOP_NFSSTATICADDRPTR	 33	/* pass addr of variable containing 
					   address into kernel. */
#define AFSOP_NFSSTATICADDR2	 34	/* pass address in as hyper. */
#define AFSOP_SBLOCKSTATICADDR2  35	/* for sblock and sbunlock */
#endif
#define	AFSOP_GETMASK		 42	/* stand-in for SIOCGIFNETMASK */
/* For SGI, this can't interfere with any of the 64 bit inode calls. */
#define AFSOP_RXLISTENER_DAEMON  48	/* starts kernel RX listener */

/* these are for initialization flags */

#define AFSCALL_INIT_MEMCACHE 0x1
#define AFSCALL_INIT_MEMCACHE_SLEEP 0x2	/* Use osi_Alloc to allocate memcache
					 * instead of osi_Alloc_NoSleep */

#define	AFSOP_GO		100	/* whether settime is being done */
/* not for initialization: debugging assist */
#define	AFSOP_CHECKLOCKS	200	/* dump lock state */
#define	AFSOP_SHUTDOWN		201	/* Totally shutdown afs (deallocate all) */

/* The following aren't used by afs_initState but by afs_termState! */
#define	AFSOP_STOP_RXCALLBACK	210	/* Stop CALLBACK process */
#define	AFSOP_STOP_AFS		211	/* Stop AFS process */
#define AFSOP_STOP_CS		216	/* Stop CheckServer daemon. */
#define	AFSOP_STOP_BKG		212	/* Stop BKG process */
#define	AFSOP_STOP_TRUNCDAEMON	213	/* Stop cache truncate daemon */
/* #define AFSOP_STOP_RXEVENT   214     defined in osi.h	      */
/* #define AFSOP_STOP_COMPLETE     215  defined in osi.h	      */
/* #define AFSOP_STOP_RXK_LISTENER   217     defined in osi.h	      */

/* Main afs syscall entry; this number may vary per system (i.e. defined in afs/param.h) */
#ifndef	AFS_SYSCALL
#define	AFS_SYSCALL		31
#endif

/* arguments passed by afsd */
struct afs_cacheParams {
    int32 cacheScaches;
    int32 cacheFiles;
    int32 cacheBlocks;
    int32 cacheDcaches;
    int32 cacheVolumes;
    int32 chunkSize;
    int32 setTimeFlag;
    int32 memCacheFlag;
    int32 inodes;
    int32 users;
};

/*
 * Note that the AFS_*ALLOCSIZ values should be multiples of sizeof(void*) to
 * accomodate pointer alignment.
 */
/* Used in rx.c as well as afs directory. */
#if	defined(AFS_AIX32_ENV) || defined(AFS_HPUX_ENV)
/* XXX Because of rxkad_cprivate... XXX */
#define	AFS_MDALLOCSIZ 	(127*sizeof(void *))	    /* "Medium" allocated size */
#define	AFS_MALLOC_LOW_WATER	50 /* Min free blocks before allocating more */
#define	AFS_SMALLOCSIZ 	(38*sizeof(void *))	    /* "Small" allocated size */
#else
#define	AFS_SMALLOCSIZ 	(64*sizeof(void *))         /*  "Small" allocated size */
#endif

#endif /* _AFS_ARGS_H_ */
