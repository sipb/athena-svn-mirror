/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1987, 1988
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
/* Copyright (C) 1995 Transarc Corporation - All rights reserved. */
/* $Header: /afs/transarc.com/project/fs/dev/afs/rcs/afs/RCS/afs.h,v 2.431 1996/12/23 22:05:40 zumach Exp $ */

/*
 * AFS system call opcodes
 */

#ifndef	__AFS_CONFIG_INCL_ENV_
#define	__AFS_CONFIG_INCL_ENV_	1

#define	AFSOP_START_RXCALLBACK	  0	/* no aux parms */
#define	AFSOP_START_AFS		  1	/* no aux parms */
#define	AFSOP_START_BKG		  2	/* no aux parms */
#define	AFSOP_START_TRUNCDAEMON	  3	/* no aux parms */

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
#define AFSOP_AFS_VFSMOUNT	 16     /* vfsmount cover for hpux */
#define AFSOP_ADVISEADDR	 17	/* to init rx cid generator */
#define AFSOP_CLOSEWAIT 	 18	/* make all closes synchronous */
#define	AFSOP_RXEVENT_DAEMON	 19	/* rxevent daemon */
#define	AFSOP_GETMTU		 20	/* stand-in for SIOCGIFMTU, for now */
#define	AFSOP_GETIFADDRS	 21	/* get machine's ethernet interfaces */
#define	AFSOP_ADDCELL2		 29	/* 2nd cell interface, newer than 5 */
/* 30 is reserved by AFSCALL_ICL for SGI */
#ifdef	AFS_SGI53_ENV
#define AFSOP_NFSSTATICADDR	 32	/* to contents addr of nfs kernel addr */
#define AFSOP_NFSSTATICADDRPTR	 33	/* pass address in as ptr - obs. */
#define AFSOP_NFSSTATICADDR2	 34	/* pass address in as hyper. */
#define AFSOP_SBLOCKSTATICADDR2  35	/* for sblock and sbunlock */
#endif
#define	AFSOP_GETMASK		 42	/* stand-in for SIOCGIFNETMASK */

/* The range 20-30 is reserved for AFS system offsets in the afs_syscall */
/* The range 30-40 is reserved for auxcalls */
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

#define	AFSOP_GO		100	/* whether settime is being done */
/* not for initialization: debugging assist */
#define	AFSOP_CHECKLOCKS	200	/* dump lock state */
#define	AFSOP_SHUTDOWN		201	/* Totally shutdown afs (deallocate all) */

/* The following aren't used by afs_initState but by afs_termState! */
#define	AFSOP_STOP_RXCALLBACK	210	/* Stop CALLBACK process */
#define	AFSOP_STOP_AFS		211	/* Stop AFS process */
#define	AFSOP_STOP_BKG		212	/* Stop BKG process */
#define	AFSOP_STOP_TRUNCDAEMON	213	/* Stop cache truncate daemon */

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
 * Macros to uniquely identify the AFS vfs struct
 */
#define	AFS_VFSMAGIC		0x1234
#if    defined(AFS_SUN_ENV) || defined(AFS_NEXT_ENV) || defined(AFS_HPUX90_ENV) || defined(AFS_NCR_ENV)
#define	AFS_VFSFSID		99
#else
#if defined(AFS_SGI_ENV)
#define AFS_VFSFSID		afs_fstype
#else
#define	AFS_VFSFSID		AFS_MOUNT_AFS
#endif
#endif

/* The following are various levels of afs debugging */
#define	AFSDEB_GENERAL		1	/* Standard debugging */
#define	AFSDEB_NETWORK		2	/* low level afs networking */
#define	AFSDEB_RX		4	/* RX debugging */
#define	AFSDEB_VNLAYER		8	/* interface layer to AFS (aixops, gfsops, etc) */

/* generic undefined vice id */
#define	UNDEFVID	    (-1)

/* The basic defines for the Andrew file system
    better keep things powers of two so "& (foo-1)" hack works for masking bits */
#define	MAXHOSTS	13	    /* max hosts per single volume */
#define	OMAXHOSTS	 8	    /* backwards compatibility */
#define	NBRS		15	    /* max number of queued daemon requests */
#define	NUSERS		16	    /* hash table size for unixuser table */
#define	NSERVERS	16	    /* hash table size for server table */
#define	NVOLS		64	    /* hash table size for volume table */
#define	NFENTRIES	256	    /* hash table size for disk volume table */
#define	VCSIZE	       1024	    /* stat cache hash table size */
#define	DCSIZE		512	    /* disk cache hash table size */
#define	PIGGYSIZE	1350	    /* max piggyback size */
#define	MAXVOLS		128	    /* max vols we can store */
#define	MAXSYSNAME	128	    /* max sysname (i.e. @sys) size */
#define	NOTOKTIMEOUT	(2*3600)    /* time after which to timeout conns sans tokens */
#define	NOPAG		0xffffffff
#define AFS_NCBRS	300	    /* max # of call back return entries */
#define AFS_MAXCBRSCALL	16	    /* max to return in a given call */
#if	defined(AFS_AIX32_ENV) || defined(AFS_HPUX_ENV)  /* XXX Because of rxkad_cprivate... XXX */
#define	AFS_MDALLOCSIZ 	428	    /* "Medium" allocated size */
#define	AFS_MALLOC_LOW_WATER	50 /* Min free blocks before allocating more */
#define	AFS_SMALLOCSIZ 	152	    /* "Small" allocated size */
#else
#ifdef	AFS_ALPHA_ENV
#define	AFS_SMALLOCSIZ 	192	    /*  "Small" allocated size */
#else
#define	AFS_SMALLOCSIZ 	128	    /*  was 256 - "Small" allocated size */
#endif
#endif
#define	AFS_SALLOC_LOW_WATER	250 /* Min free blocks before allocating more */
#ifdef	AFS_NEXT_ENV
#define	AFS_LRALLOCSIZ	8192
#else
#define	AFS_LRALLOCSIZ 	4096	    /* "Large" allocated size */
#endif
#define	VCACHE_FREE	5
#define	AFS_NRXPACKETS	80
#define	AFS_RXDEADTIME	50

/* flags to use with AFSOP_CACHEINIT */
#define AFSCALL_INIT_MEMCACHE        0x1         /* use a memory-based cache */

/* below here used only for kernel procedures */
#ifdef KERNEL
/* Store synchrony flags - SYNC means that data should be forced to server's
 * disk immediately upon completion. */
#define AFS_ASYNC 	0
#define AFS_SYNC  	1
#define AFS_DONTVMSYNC  2
#define AFS_LASTSTORE   4


/* background request structure */
#define	BPARMS		4

#define	BOP_NOOP	0	    /* leave 0 unused */
#define	BOP_FETCH	1	    /* parm1 is chunk to get */
#define	BOP_STORE	2	    /* parm1 is chunk to store */
#define	BOP_PATH	3	    /* parm1 is path, parm2 is chunk to fetch */

#define	B_DONTWAIT	1	    /* On failure return; don't wait */

/* protocol is: refCount is incremented by user to take block out of free pool.
    Next, BSTARTED is set when daemon finds request.  This prevents
    other daemons from picking up the same request.  Finally, when
    request is done, refCount is zeroed.  BDONE and BWAIT are used by
    dudes waiting for operation to proceed to a certain point before returning.
*/
#define	BSTARTED	1	    /* request picked up by a daemon */
#define	BUVALID		2	    /* code is valid (store) */
#define	BUWAIT		4	    /* someone is waiting for BUVALID */
struct brequest {
    struct vcache *vnode;	    /* vnode to use, with vrefcount bumped */
#ifdef	AFS_FINEGR_SUNLOCK
    kmutex_t lock;
    kcondvar_t cv;
#endif
    struct AFS_UCRED *cred;	    /* credentials to use for operation */
    long parm[BPARMS];		    /* random parameters - long's work for
				     * both 32 and 64 bit platforms.
				     */
    int32 code;			    /* return code */
    short refCount;		    /* use counter for this structure */
    char opcode;		    /* what to do (store, fetch, etc) */
    char flags;			    /* free, etc */
};

struct SecretToken {
    char data[56];
};

struct ClearToken {
	int32 AuthHandle;
	char HandShakeKey[8];
	int32 ViceId;
	int32 BeginTimestamp;
	int32 EndTimestamp;
};

struct VenusFid {
    int32 Cell;			    /* better sun packing if at end of structure */
    struct AFSFid Fid;
};

/* Temporary struct to be passed between afs_fid and afs_vget; in SunOS4.x we can only pass a maximum of 10 bytes for a handle (we ideally need 16!) */
struct SmallFid {
    int32 Volume;
    int32 CellAndUnique;
    u_short Vnode;
};
/* The actual number of bytes in the SmallFid, not the sizeof struct. */
#define SIZEOF_SMALLFID 10


/*
  * Queues implemented with both pointers and short offsets into a disk file.
  */
struct afs_q {
    struct afs_q *next;
    struct afs_q *prev;
};

struct vrequest {
    int32 uid;			/* user id making the request */
    int32 busyCount;		/* how many busies we've seen so far */
    int32 flags;                 /* things like O_SYNC, O_NONBLOCK go here */
    char initd;			/* if non-zero, non-uid fields meaningful */
    char accessError;		/* flags for overriding error return code */
    char volumeError;		/* encountered a missing or busy volume */
    char networkError;		/* encountered network problems */
};
#define VOLMISSING 1
#define VOLBUSY 2

/* structure linked off of a server to keep track of queued returned
 * callbacks.  Sent asynchronously when we run a little low on free dudes.
 */
struct afs_cbr {
    struct afs_cbr *next;
    struct AFSFid fid;
};


#define	LOCALCELL	    1	/* local cell's number is always 1 */

/* cell flags */
#define	CPrimary	    1	    /* on if this is the primary cell */
#define	CNoSUID		    2	    /* 1 if no suid progs can run from this cell */
#define CHasVolRef	   16	    /* Volumes were referenced in this cell*/
#define CLinkedCell	   32

struct cell {
    struct afs_q lruq;			     /* lru q next and prev */
#ifdef	AFS_FINEGR_SUNLOCK
    afs_rwlock_t lock;
#endif
    int32 cell;				    /* unique id assigned by venus */
    char *cellName;			    /* char string name of cell */
    struct server *cellHosts[MAXHOSTS];    /* volume *location* hosts for this cell */
    struct cell *lcellp;		    /* Associated linked cell */
    u_short fsport;			    /* file server port */
    u_short vlport;			    /* volume server port */
    short states;			    /* state flags */
    short cellIndex;			    /* relative index number per cell */
};

#ifdef	AFS_FINEGR_SUNLOCK
#define	afs_PutCell(cellp, locktype)	ReleaseLock(&(cellp)->lock, (locktype))
#else
#define	afs_PutCell(cellp, locktype)
#endif

/* the unixuser flag bit definitions */
#define	UHasTokens	1	    /* are the st and ct fields valid (ever set)? */
#define	UTokensBad	2	    /* are tokens bad? */
#define UPrimary        4           /* on iff primary identity */
#define UNeedsReset	8	    /* needs afs_ResetAccessCache call done */
#define UPAGCounted    16	    /* entry seen during PAG search (for stats) */

struct unixuser {
    struct unixuser *next;	    /* next hash pointer */
    int32 uid;			    /* search based on uid and cell */
    int32 cell;
    int32 vid;			    /* corresponding vice id in specified cell */
    short refCount;		    /* reference count for allocation */
    char states;		    /* flag info */
    int32 tokenTime;		    /* last time tokens were set, used for timing out conn data */
    int32 stLen;			    /* ticket length (if kerberos, includes kvno at head) */
    char *stp;			    /* pointer to ticket itself */
#ifdef	AFS_FINEGR_SUNLOCK
    afs_rwlock_t lock;
    kmutex_t mlock;
    kcondvar_t cv;
#endif
    struct ClearToken ct;
    struct afs_exporter	*exporter;  /* more info about the exporter for the remote user */
};

struct conn {
    /* Per-connection block. */
    struct conn	*next;		    /* Next dude same server. */
#ifdef	AFS_FINEGR_SUNLOCK
    afs_rwlock_t lock;	
#endif
    struct unixuser *user;	    /* user validated with respect to. */
    struct rx_connection *id;	    /* RPC connid. */
    struct srvAddr *srvr;	    /* server associated with this conn */
    short refCount;		    /* reference count for allocation */
    unsigned short port;	    /* port associated with this connection */
    char forceConnectFS;	    /* Should we try again with these tokens? */
};


#define SQNULL -1

/* Fid comparison routines */
#define	FidCmp(a,b) ((a)->Fid.Unique != (b)->Fid.Unique \
    || (a)->Fid.Vnode != (b)->Fid.Vnode \
    || (a)->Fid.Volume != (b)->Fid.Volume \
    || (a)->Cell != (b)->Cell)

#define	FidMatches(afid,tvc) ((tvc)->fid.Fid.Vnode == (afid)->Fid.Vnode && \
	(tvc)->fid.Fid.Volume == (afid)->Fid.Volume && \
	(tvc)->fid.Cell == (afid)->Cell && \
	( (tvc)->fid.Fid.Unique == (afid)->Fid.Unique || \
	 (!(afid)->Fid.Unique && ((tvc)->states & CUnique))))


/*
  * Operations on circular queues implemented with pointers.  Note: these queue
  * objects are always located at the beginning of the structures they are linking.
  */
#define	QInit(q)    ((q)->prev = (q)->next = (q))
#define	QAdd(q,e)   ((e)->next = (q)->next, (e)->prev = (q), \
			(q)->next->prev = (e), (q)->next = (e))
#define	QRemove(e)  ((e)->next->prev = (e)->prev, (e)->prev->next = (e)->next)
#define	QNext(e)    ((e)->next)
#define QPrev(e)    ((e)->prev)
#define QEmpty(q)   ((q)->prev == (q))
/* this one takes q1 and sticks it on the end of q2 - that is, the other end, not the end
 * that things are added onto.  q1 shouldn't be empty, it's silly */
#define QCat(q1,q2) ((q2)->prev->next = (q1)->next, (q1)->next->prev=(q2)->prev, (q1)->prev->next=(q2), (q2)->prev=(q1)->prev, (q1)->prev=(q1)->next=(q1))
/*
 * Do lots of address arithmetic to go from vlruq to the base of the vcache
 * structure.  Don't move struct vnode, since we think of a struct vcache as
 * a specialization of a struct vnode
 */
#define	QTOV(e)	    ((struct vcache *)(((char *) (e)) - (((char *)(&(((struct vcache *)(e))->vlruq))) - ((char *)(e)))))
#define	QTOC(e)	    ((struct cell *)((char *) (e)))

#define	SRVADDR_MH	1
#define	SRVADDR_ISDOWN	0x20	/* same as SRVR_ISDOWN */
struct srvAddr {
    struct srvAddr *next_bkt;   /* next item in hash bucket */
    struct srvAddr *next_sa;    /* another interface on same host */
    struct server *server;	/* back to parent */
    struct conn	*conns;		/* All user connections to this server */
    int32 sa_ip;	        /* Host addr in network byte order */
    u_short sa_iprank;		/* indiv ip address priority */
    u_short sa_portal;		/* port addr in network byte order */
    u_char sa_flags;
};

/*
 * Values used in the flags field of the server structure below.
 *
 *	AFS_SERVER_FLAG_ACTIVATED Has the server ever had a user connection
 *				  associated with it?
 */
#define AFS_SERVER_FLAG_ACTIVATED	0x01
#define	SNO_LHOSTS			0x04
#define	SYES_LHOSTS			0x08
#define	SVLSRV_UUID			0x10
#define	SRVR_ISDOWN			0x20
#define	SRVR_MULTIHOMED			0x40

struct server {
    union {
	struct {
	    afsUUID suuid;
	    int32 addr_uniquifier;
	    int32 spares[2];
	} _srvUuid;
        struct {
	    struct srvAddr haddr;
	} _srvId;	
    } _suid;
#define sr_uuid		_suid._srvUuid.suuid
#define sr_addr_uniquifier	_suid._srvUuid.addr_uniquifier
#define sr_host		_suid._srvId.haddr.ip
#define sr_portal	_suid._srvId.haddr.portal
#define sr_rank		_suid._srvId.haddr.ip_rank
#define sr_flags	_suid._srvId.haddr.flags
#define sr_conns	_suid._srvId.haddr.conns
    struct server *next;	/* Ptr to next server in hash chain */
#ifdef	AFS_FINEGR_SUNLOCK
    afs_rwlock_t lock;	
#endif
    struct cell	*cell;		/* Cell in which this host resides */
    struct afs_cbr *cbrs;	/* Return list of callbacks */
    int32 activationTime;	/* Time when this record was first activated */
    int32 lastDowntimeStart;	/* Time when last downtime incident began */
    int32 numDowntimeIncidents;	/* # (completed) downtime incidents */
    int32 sumOfDowntimes;	/* Total downtime experienced, in seconds */
    struct srvAddr *addr;
    char flags;			/* Misc flags*/
#ifdef	AFS_FINEGR_SUNLOCK
    kmutex_t lockcv;
    kcondvar_t cv;
#endif
};

#ifdef	AFS_FINEGR_SUNLOCK
#define	afs_PutServer(servp, locktype)	if (locktype) ReleaseLock(&(servp)->lock, (locktype))
#else
#define	afs_PutServer(servp, locktype)	
#endif

/* structs for some pioctls  - these are (or should be) 
 * also in venus.h
 */
struct spref {
	struct in_addr host;
	unsigned short rank;
};

struct sprefrequest_33 {
	unsigned short offset;
	unsigned short num_servers;
};


struct sprefrequest {             /* new struct for 3.4 */
	unsigned short offset;
	unsigned short num_servers;
	unsigned short flags;
};
#define DBservers 1

struct sprefinfo {
	unsigned short next_offset;
	unsigned short num_servers;
	struct spref servers[1];		/* we overrun this array intentionally...*/
};

struct setspref {
	unsigned short flags;
	unsigned short num_servers;
	struct spref servers[1];		/* we overrun this array intentionally...*/
};
/* struct for GAG pioctl
 */
struct gaginfo {
	u_int32 showflags, logflags, logwritethruflag, spare[3];
	unsigned char spare2[128];
};
#define GAGUSER    1
#define GAGCONSOLE 2
#define logwritethruON	1

struct rxparams {
	int32 rx_initReceiveWindow, rx_maxReceiveWindow,
	      rx_initSendWindow, rx_maxSendWindow, rxi_nSendFrags,
	      rxi_nRecvFrags, rxi_OrphanFragSize;
	int32 rx_maxReceiveSize, rx_MyMaxSendSize;
	u_int32 spare[21];
      };

/* struct for checkservers */

struct chservinfo
{
        int magic;
	char tbuffer[128];
	int tsize;
	int32 tinterval;
	int32 tflags;
}
;


/* state bits for volume */
#define VRO			1		/* volume is readonly */
#define VRecheck		2		/* recheck volume info with server */
#define	VBackup			4		/* is this a backup volume? */
#define	VForeign		8		/* this is a non-afs volume */
#define VResort         16   /* server order was rearranged, sort when able */
#define VMoreReps       32   /* This volume has more replicas than we are   */
                             /* keeping track of now -- check with VLDB     */

 enum repstate { not_busy, end_not_busy = 6, rd_busy, rdwr_busy, offline };

struct volume {
    /* One structure per volume, describing where the volume is located
	  and where its mount points are. */
    struct volume *next;	/* Next volume in hash list. */
    int32 cell;			/* the cell in which the volume resides */
    afs_rwlock_t lock;		/* the lock for this structure */
    int32 volume;		/* This volume's ID number. */
    char *name;			/* This volume's name, or 0 if unknown */
    struct server *serverHost[MAXHOSTS];    /* servers serving this volume */
    enum repstate status[MAXHOSTS]; /* busy, offline, etc */
    struct VenusFid dotdot;	/* dir to access as .. */
    struct VenusFid mtpoint;	/* The mount point for this volume. */
    int32 rootVnode, rootUnique;	/* Volume's root fid */
    int32 roVol;
    int32 backVol;
    int32 rwVol;			/* For r/o vols, original read/write volume. */
    int32 accessTime;		/* last time we used it */
    int32 vtix;			/* volume table index */
    int32 copyDate;		/* copyDate field, for tracking vol releases */
    int32 expireTime;            /* for per-volume callbacks... */
    short refCount;		/* reference count for allocation */
    char states;		/* here for alignment reasons */
};

#ifndef	AFS_FINEGR_SUNLOCK
#define afs_PutVolume(av, locktype) ((av)->refCount--)
#else
#define afs_PutVolume(av, locktype) 	(av)->refCount --; \
                                        if (locktype) ReleaseLock(&(av)->lock, (locktype))
#endif

/* format of an entry in volume info file */
struct fvolume {
    int32 cell;			/* cell for this entry */
    int32 volume;		/* volume */
    int32 next;			/* has index */
    struct VenusFid dotdot;	/* .. value */
    struct VenusFid mtpoint;	/* mt point's fid */
    int32 rootVnode, rootUnique;	/* Volume's root fid */
};

struct SimpleLocks {
    struct SimpleLocks *next;
    int type;
    int32 boff, eoff;
    int32 pid;
#if	defined(AFS_AIX32_ENV) || defined(AFS_SUN5_ENV) || defined(AFS_SGI_ENV)
    int32 sysid;
#endif
};

/* vcache state bits */
#define	CStatd	    1		/* has this file ever been stat'd? */
#define CBackup	    2		/* file is on a backup volume */
#define	CRO	    4		/* is it on a read-only volume */
#define	CMValid	    8		/* is the mount point info valid? */
#define	CCore	    0x10	/* storing a core file, needed since we don't get an open */
#define CDirty      0x20        /* file has been modified since first open (... O_RDWR) */
#define	CSafeStore  0x40	/* close must wait for store to finish (should be in fd) */
#define	CMAPPED	    0x80	/* Mapped files; primarily used by SunOS 4.0.x */
#define	CNSHARE	    0x100	/* support O_NSHARE semantics */
#define	CLied	    0x200
#define	CTruth	    0x400
#ifdef	AFS_OSF_ENV
#define	CWired	    0x800	/* OSF hack only */
#else
#define	CWRITE_IGN  0x800	/* Next OS hack only */
#endif
#define	CUnique	    0x1000	/* vc's uniquifier - latest unifiquier for fid */
#define	CForeign    0x2000	/* this is a non-afs vcache */
#define CHasPages   0x4000
#define CRevokeWait 0x8000
#define CUnlinked   0x10000
#define CBulkStat   0x20000	/* loaded by a bulk stat, and not ref'd since */
#define CUnlinkedDel  0x40000
#define CVFlushed   0x80000
#define	CCore1	    0x100000	/* osf1 core file; not same as CCore above */
#define CWritingUFS 0x200000	/* to detect vm deadlock - used by sgi */
#define CCreating   0x400000	/* avoid needless store after open truncate */
#define CPageHog    0x800000	/* AIX - dumping large cores is a page hog. */

#define	CPSIZE	    2
#define	vrefCount   v.v_count

#define	AFS_MAXDV   0x7fffffff	    /* largest dataversion number */
#define	AFS_NOTRUNC 0x7fffffff	    /* largest dataversion number */

extern int32 vmPageHog; /* counter for # of vnodes which are page hogs. */

/*
 * Fast map from vcache to dcache courtesy mts@umich.edu
 */
struct	vtodc
	{
	struct dcache * dc;
	u_int32		stamp;
	struct osi_file * f;
	u_int32		minLoc;	/* smallest offset into dc. */
	u_int32		len;	/* largest offset into dc. */
	};

extern u_int32 afs_stampValue;		/* stamp for pair's usage */
#define	MakeStamp()	(++afs_stampValue)

/* INVARIANTs: (vlruq.next != NULL) == (vlruq.prev != NULL)
 *             nextfree => !vlruq.next && ! vlruq.prev
 * !(avc->nextfree) && !avc->vlruq.next => (FreeVCList == avc->nextfree)
 */
struct vcache {
    struct vnode v;			/* Has reference count in v.v_count */
    struct afs_q vlruq;			/* lru q next and prev */
    struct vcache *nextfree;		/* next on free list (if free) */
    struct vcache *hnext;		/* Hash next */
    struct VenusFid fid;
    struct mstat {
        u_int32 Length;
        hyper DataVersion;
        u_int32 Date;
        u_int32 Owner;
	u_int32 Group;
        ushort Mode;			/* XXXX Should be int32 XXXX */
        unsigned char LinkCount;
	/* vnode type is in v.v_type */
    } m;
    afs_rwlock_t lock;			/* The lock on the vcache contents. */
#if defined(AFS_SUN_ENV) || defined(AFS_ALPHA_ENV) || defined(AFS_NCR_ENV)
#if	defined(AFS_SUN5_ENV) && !defined(AFS_NCR_ENV)
    krwlock_t rwlock;
    struct cred *credp;
#endif
#ifdef	AFS_FINEGR_SUNLOCK
    afs_lock_t vm_lock;
    kcondvar_t vm_cv;    
#endif
    afs_bozoLock_t pvnLock;	/* see locks.x */
#endif
#ifdef	AFS_AIX32_ENV
    afs_lock_t pvmlock;
    vmhandle_t vmh;
    int  segid;
    struct ucred *credp;
#endif
#ifdef AFS_AIX_ENV
    int ownslock;	/* pid of owner of excl lock, else 0 - defect 3083 */
#endif
    int32 parentVnode;			/* Parent dir, if a file. */
    int32 parentUnique;
    struct VenusFid *mvid;		/* Either parent dir (if root) or root (if mt pt) */
    char *linkData;			/* Link data if a symlink. */
    hyper flushDV;			/* data version last flushed from text */
    hyper mapDV;			/* data version last flushed from map */
    u_int32 truncPos;			/* truncate file to this position at next store */
    struct server *callback;		/* The callback host, if any */
    u_int32 cbExpires;			/* time the callback expires */
    struct afs_q callsort;              /* queue in expiry order, sort of */
    struct axscache *Access;            /* a list of cached access bits */
    int32 anyAccess;			/* System:AnyUser's access to this. */
    int32 last_looker;                  /* pag/uid from last lookup here */
    int32 activeV;
    struct SimpleLocks *slocks;
    short opens;		    /* The numbers of opens, read or write, on this file. */
    short execsOrWriters;	    /* The number of execs (if < 0) or writers (if > 0) of
				       this file. */
    short flockCount;		    /* count of flock readers, or -1 if writer */
    char mvstat;			/* 0->normal, 1->mt pt, 2->root. */
    u_int32 states;			/* state bits */
    struct vtodc quick;
    u_int32 symhintstamp;
    union {
      struct vcache *symhint;
      struct dcache *dchint;
    } h1;
#if defined(AFS_SGI_ENV)
    daddr_t lastr;			/* for read-ahead */
    short rwlockpid;			/* pid of process owning rwlock */
    short locktrips;			/* # of rwlock reacquisitions */
    sema_t rwlock;			/* vop_rwlock for afs */
    pgno_t mapcnt;			/* # of pages mapped */
    struct cred *cred;			/* last writer's cred */
#endif
    int32 vc_error;			/* stash write error for this vnode. */
    int xlatordv;			/* Used by nfs xlator */
    struct AFS_UCRED *uncred;
    int asynchrony;                     /* num kbytes to store behind */
#ifdef AFS_NEXT_ENV
    short mfsCount;		    /* count of mfs_fsync-type ops in progress */
#endif
};

#define	DONT_CHECK_MODE_BITS	0
#define	CHECK_MODE_BITS		1

#if defined(AFS_SGI_ENV)
#define AVCRWLOCK(avc)		(valusema(&(avc)->rwlock) <= 0)

/* Shouldn't we try this for all platforms and move it to osi.h? */
#ifdef AFS_SGI62_ENV
#define AFS_USER_CRED curprocp->p_cred
#else
#define AFS_USER_CRED u.u_cred
#endif

/* SGI vnode rwlock macros and flags. */
#ifndef AFS_SGI62_ENV
/* The following are defined here. SGI 6.2 declares them in vnode.h */
#define VRWLOCK_READ		0
#define VRWLOCK_WRITE		1
#define VRWLOCK_WRITE_DIRECT	2
#endif

#ifdef AFS_SGI53_ENV
#ifdef AFS_SGI62_ENV
#define AFS_RWLOCK_T vrwlock_t
#else 
#define AFS_RWLOCK_T int
#endif /* AFS_SGI62_ENV */
#define AFS_RWLOCK(V,F) afs_rwlock((V), (F) )
#define AFS_RWUNLOCK(V,F) afs_rwunlock((V), (F) )
#else /* AFS_SGI53_ENV */
#define AFS_RWLOCK(V,F) afs_rwlock((V))
#define AFS_RWUNLOCK(V,F) afs_rwunlock((V))
#endif /* AFS_SGI53_ENV */
#endif /* AFS_SGI_ENV */

struct vcxstat {
    struct VenusFid fid;
    hyper DataVersion;
    afs_rwlock_t lock;
    int32 parentVnode;	
    int32 parentUnique;
    hyper flushDV;
    hyper mapDV;
    int32 truncPos;
    int32 randomUid[CPSIZE];
    int32 callback;		/* Now a pointer to 'server' struct */
    int32 cbExpires;
    int32 randomAccess[CPSIZE];
    int32 anyAccess;
    short opens;
    short execsOrWriters;
    short flockCount;
    char mvstat;
    u_int32 states;
};

struct sbstruct {
  int sb_thisfile;
  int sb_default;
};


/*----------------------------------------------------------------------
 * AFS Data cache definitions
 *
 * Each entry describes a Unix file on the local disk that is
 * is serving as a cached copy of all or part of a Vice file.
 * Entries live in circular queues for each hash table slot
 *
 * Which queue is this thing in?  Good question.
 * A struct dcache entry is in the freeDSlot queue when not associated with a cache slot (file).
 * Otherwise, it is in the DLRU queue.  The freeDSlot queue uses the lruq.next field as
 * its "next" pointer.
 *
 * Cache entries in the DLRU queue are either associated with vice files, in which case
 * they are hashed by hvNextp and hcNextp pointers, or they are in the freeDCList
 * and are not associated with any vice file.  This last list uses the hvNextp pointer for
 * its "next" pointer.
 *
 * Note that the DLRU and freeDSlot queues are *memory* queues, while the hvNextp and
 * hcNextp hash lists and the freeDCList all go through the disk elements.  Think of the
 * memory queues as lower-level queues caching a number of the disk entries.
 *----------------------------------------------------------------------*/

#define	NULLIDX	    (-1)	/* null index definition */
/* struct dcache states bits */
#define	DWriting    8		/* file being written (used for cache validation) */

/* dcache flags */
#define	DFNextStarted	1	/* next chunk has been prefetched already */
#define	DFEntryMod	2	/* has entry itself been modified? */
#define	DFFetching	4	/* file is currently being fetched */
#ifndef	AFS_FINEGR_SUNLOCK
#define	DFWaiting	8	/* someone waiting for file */
#endif
#define	DFFetchReq	0x10	/* someone is waiting for DFFetching to go on */


/* flags in afs_indexFlags array */
#define	IFEverUsed	1		/* index entry has >= 1 byte of data */
#define	IFFree		2		/* index entry in freeDCList */
#define	IFDataMod	4		/* file needs to be written out */
#define	IFFlag		8		/* utility flag */
#define	IFDirtyPages	16
#define	IFAnyPages	32

struct afs_ioctl {
	caddr_t in, out;	/* Data to be transferred in, or out */
	short in_size;		/* Size of input buffer <= 2K */
	short out_size;		/* Maximum size of output buffer, <= 2K */
};

#ifdef AFS_SGI61_ENV
/* This version of afs_ioctl is required to convert pointers in 32 bit apps. */
struct irix5_afs_ioctl {
	app32_ptr_t in, out;	/* Data to be transferred in, or out */
	short in_size;		/* Size of input buffer <= 2K */
	short out_size;		/* Maximum size of output buffer, <= 2K */
};
#endif /* AFS_SGI61_ENV */

/* CacheItems file has a header of type struct afs_fheader (keep aligned properly) */
struct afs_fheader {
    int32 magic;
    int32 firstCSize;
    int32 otherCSize;
    int32 spare;
};
#define	AFS_FHMAGIC	    0x7635faba

/* kept on disk and in dcache entries */
struct fcache {
    int32 hvNextp;		/* Next in vnode hash table, or freeDCList */
    int32 hcNextp;		/* Next index in [fid, chunk] hash table */
    struct VenusFid fid;	/* Fid for this file */
    int32 modTime;		/* last time this entry was modified */
    hyper versionNo;		/* Associated data version number */
    int32 chunk;			/* Relative chunk number */
    int32 inode;			/* Unix inode for this chunk */
    int32 chunkBytes;		/* Num bytes in this chunk */
    char states;		/* Has this chunk been modified? */
};

/* magic numbers to specify the cache type */

#define AFS_FCACHE_TYPE_UFS 0x0
#define AFS_FCACHE_TYPE_MEM 0x1
#define AFS_FCACHE_TYPE_NFS 0x2
#define AFS_FCACHE_TYPE_EPI 0x3

/* kept in memory */
struct dcache {
    struct afs_q lruq;		/* Free queue for in-memory images */
#ifdef	AFS_FINEGR_SUNLOCK
    kmutex_t lock;
    kmutex_t lockw;
    kcondvar_t cv_waiting;
    kmutex_t lockf;
    kcondvar_t cv_fetchreq;
#else
    afs_rwlock_t lock;		/* XXX */
#endif
    short refCount;		/* Associated reference count. */
    int32 index;			/* The index in the CacheInfo file*/
    short flags;		/* more flags bits */
    int32 validPos;		/* number of valid bytes during fetch */
    struct fcache f;		/* disk image */
    int32 stamp; 		/* used with vtodc struct for hints */
};
/* this is obsolete and should be removed */
#define ihint stamp 

/* macro to mark a dcache entry as bad */
#define ZapDCE(x)	((x)->f.fid.Fid.Unique = 0, (x)->flags |= DFEntryMod)

/* FakeOpen and Fake Close used to be real subroutines.  They're only used in
 * sun_subr and afs_vnodeops, and they're very frequently called, so I made 
 * them into macros.  They do:
 * FakeOpen:  fake the file being open for writing.  avc->lock must be held
 * in write mode.  Having the file open for writing is like having a DFS
 * write-token: you're known to have the best version of the data around, 
 * and so the CM won't let it be overwritten by random server info.
 * FakeClose:  undo the effects of FakeOpen, noting that we want to ensure
 * that a real close eventually gets done.  We use CCore to achieve this if
 * we would end up closing the file.  avc->lock must be held in write mode */

#define afs_FakeOpen(avc) { avc->opens++; avc->execsOrWriters++; }
#define afs_FakeClose(avc, acred) \
{ if (avc->execsOrWriters == 1) {  \
	/* we're the last writer, just use CCore flag */   \
	avc->states |= CCore;	/* causes close to be called later */ \
                                                                      \
	/* The cred and vnode holds will be released in afs_FlushActiveVcaches */  \
	crhold(acred);                                                        \
	VN_HOLD((struct vnode *)avc);	/* So it won't disappear */           \
	avc->linkData =	(char *) acred;	/* Should use a better place for the creds */ \
    }                                                                         \
    else {                                                                    \
	/* we're not the last writer, let the last one do the store-back for us */    \
	avc->opens--;                                                         \
	avc->execsOrWriters--;                                                \
    }                                                                         \
}                                                                             

#define	AFS_ZEROS   64	    /* zero buffer */

/*#define afs_DirtyPages(avc)	(((avc)->states & CDirty) || osi_VMDirty_p((avc)))*/
#define	afs_DirtyPages(avc)	((avc)->states & CDirty)

/* The PFlush algorithm makes use of the fact that Fid.Unique is not used in
  below hash algorithms.  Change it if need be so that flushing algorithm
  doesn't move things from one hash chain to another
*/
/* extern int afs_dhashsize; */
#define	DCHash(v, c)	((((v)->Fid.Vnode + (v)->Fid.Volume + (c))) & (afs_dhashsize-1))
	/*Vnode, Chunk -> Hash table index*/
#define	DVHash(v)	((((v)->Fid.Vnode + (v)->Fid.Volume )) & (afs_dhashsize-1))
	/*Vnode -> Other hash table index*/
/* don't hash on the cell, our callback-breaking code sometimes fails to compute
    the cell correctly, and only scans one hash bucket */
#define	VCHash(fid)	(((fid)->Fid.Volume + (fid)->Fid.Vnode) & (VCSIZE-1))

extern struct dcache **afs_indexTable;		/*Pointers to in-memory dcache entries*/
extern int32 afs_cacheFiles;			/*Size of afs_indexTable*/
extern int32 afs_cacheBlocks;			/*1K blocks in cache*/
extern int32 afs_cacheStats;			/*Stat entries in cache*/
extern struct vcache *afs_vhashT[VCSIZE];	/*Stat cache hash table*/
extern int32 afs_initState;			/*Initialization state*/
extern int32 afs_termState;			/* Termination state */
extern struct VenusFid afs_rootFid;		/*Root for whole file system*/
extern int32 afs_allCBs;				/* Count of callbacks*/
extern int32 afs_oddCBs;				/* Count of odd callbacks*/
extern int32 afs_evenCBs;			/* Count of even callbacks*/
extern int32 afs_allZaps;			/* Count of fid deletes */
extern int32 afs_oddZaps;			/* Count of odd fid deletes */
extern int32 afs_evenZaps;			/* Count of even fid deletes */
extern struct brequest afs_brs[NBRS];		/* request structures */

#define	UHash(auid)	((auid) & (NUSERS-1))
#define	VHash(avol)	((avol)&(NVOLS-1))
#define	SHash(aserv)	((ntohl(aserv)) & (NSERVERS-1))
#define	FVHash(acell,avol)  (((avol)+(acell)) & (NFENTRIES-1))

extern struct cell	    *afs_GetCell();
extern struct cell	    *afs_GetCellByName();
extern struct unixuser	    *afs_GetUser();
extern struct volume	    *afs_GetVolume();
extern struct volume	    *afs_GetVolumeByName();
extern struct conn	    *afs_Conn();
extern struct conn	    *afs_ConnByHost();
extern struct conn	    *afs_ConnByMHosts();
extern struct server	    *afs_GetServer();
extern int32		    afs_NewCell();
extern struct dcache	    *afs_GetDCache();
extern struct dcache	    *afs_FindDCache();
extern struct dcache	    *afs_NewDCache();
extern struct dcache	    *afs_GetDSlot();
extern struct vcache	    *afs_GetVCache();
extern struct brequest	    *afs_BQueue();

/* afs_cache.c */
extern int afs_CacheInit();
extern int32 afs_FlushVCBs();
extern void afs_StoreWarn();
extern void afs_AdjustSize();
extern void afs_ComputeCacheParms();
extern void afs_FlushDCache();
extern void afs_FlushActiveVcaches();
extern void afs_StuffVcache();
extern void afs_PutVCache();
extern void afs_TryToSmush();
extern void afs_ProcessFS();
extern void afs_WriteThroughDSlots();
extern void afs_CheckVolSync();
extern void shutdown_cache();
/* afs_call.c */
extern void afs_shutdown();
/* afs_osifile.c */
extern void shutdown_osifile();

#ifdef	AFS_HPUX_ENV
extern sema_t afs_global_sema;
#endif

/* Performance hack - we could replace VerifyVCache2 with the appropriate
 * GetVCache incantation, and could eliminate even this code from afs_UFSRead 
 * by making intentionally invalidating quick.stamp in the various callbacks
 * expiration/breaking code */
#define afs_VerifyVCache(avc, areq)  \
  (((avc)->states & CStatd) ? 0 : afs_VerifyVCache2((avc),areq))

#define DO_STATS 1  /* bits used by FindVCache */
#define DO_VLRU 2

/* values for flag param of afs_CheckVolumeNames */
#define AFS_VOLCHECK_EXPIRED	0x1	/* volumes whose callbacks have expired */
#define AFS_VOLCHECK_BUSY	0x2	/* volumes which were marked busy */
#define AFS_VOLCHECK_MTPTS	0x4	/* mount point invalidation also */
#define AFS_VOLCHECK_FORCE	0x8	/* do all forcibly */

#endif /* KERNEL */

#define	AFS_FSPORT	    ((unsigned short) htons(7000))
#define	AFS_VLPORT	    ((unsigned short) htons(7003))

#define	afs_read(avc, uio, acred, albn, abpp, nolock) \
        (*(afs_cacheType->vread))(avc, uio, acred, albn, abpp, nolock)
#define	afs_write(avc, uio, aio, acred, nolock) \
        (*(afs_cacheType->vwrite))(avc, uio, aio, acred, nolock)

#define	afs_rdwr(avc, uio, rw, io, cred) \
    (((rw) == UIO_WRITE) ? afs_write(avc, uio, io, cred, 0) : afs_read(avc, uio, cred, 0, 0, 0))
#define	afs_nlrdwr(avc, uio, rw, io, cred) \
    (((rw) == UIO_WRITE) ? afs_write(avc, uio, io, cred, 1) : afs_read(avc, uio, cred, 0, 0, 1))

extern int32 afs_blocksUsed, afs_discardedChunks, freeDCCount;
extern int32 afs_bulkStatsDone, afs_bulkStatsLost;
/* Cache size truncation uses the following low and high water marks:
 * If the cache is more than 95% full (CM_DCACHECOUNTFREEPCT), the cache
 * truncation daemon is awakened and will free up space until the cache is 85%
 * (CM_DCACHESPACEFREEPCT - CM_DCACHEEXTRAPCT) full.
 * afs_UFSWrite and afs_GetDCache (when it needs to fetch data) will wait on
 * afs_WaitForCacheDrain if the cache is 98% (CM_WAITFORDRAINPCT) full.
 * afs_GetDownD wakes those processes once the cache is 95% full
 * (CM_CACHESIZEDRAINEDPCT).
 */
extern int afs_CacheTruncateDaemon();
extern int afs_WaitForCacheDrain;
#define CM_MAXDISCARDEDCHUNKS	16      /* # of chunks */
#define CM_DCACHECOUNTFREEPCT	95      /* max pct of chunks in use */
#define CM_DCACHESPACEFREEPCT	90      /* max pct of space in use */
#define CM_DCACHEEXTRAPCT    	 5      /* extra to get when freeing */
#define CM_CACHESIZEDRAINEDPCT	95      /* wakeup processes when down to here.*/
#define CM_WAITFORDRAINPCT	98      /* sleep if cache is this full. */

#define	afs_MaybeWakeupTruncateDaemon()	\
    do { \
       if (afs_discardedChunks > CM_MAXDISCARDEDCHUNKS || \
	   afs_blocksUsed > (CM_DCACHECOUNTFREEPCT*afs_cacheBlocks)/100 || \
	   freeDCCount < ((100-CM_DCACHECOUNTFREEPCT)*afs_cacheFiles)/100) \
	 osi_Wakeup((char *)afs_CacheTruncateDaemon); \
    } while (0)


#endif	/* __AFS_CONFIG_INCL_ENV_ */

