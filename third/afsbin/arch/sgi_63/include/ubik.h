/*
 * ubik.h:
 * This file is automatically generated; please do not edit it.
 */
/* Including ubik.p.h at beginning of ubik.h file. */

/* $Header: /afs/dev.mit.edu/source/repository/third/afsbin/arch/sgi_63/include/ubik.h,v 1.1.2.1 1997-11-04 18:46:13 ghudson Exp $ */
/* $Source: /afs/dev.mit.edu/source/repository/third/afsbin/arch/sgi_63/include/ubik.h,v $ */

#ifndef UBIK_H
#define UBIK_H

#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsidlock = "$Header: /afs/dev.mit.edu/source/repository/third/afsbin/arch/sgi_63/include/ubik.h,v 1.1.2.1 1997-11-04 18:46:13 ghudson Exp $";
#endif

/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1987, 1989
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */

/* these are now appended by the error table compiler */
#if 0
/* ubik error codes */
#define	UMINCODE	100000	/* lowest ubik error code */
#define	UNOQUORUM	100000	/* no quorum elected */
#define	UNOTSYNC	100001	/* not synchronization site (should work on sync site) */
#define	UNHOSTS		100002	/* too many hosts */
#define	UIOERROR	100003	/* I/O error writing dbase or log */
#define	UINTERNAL	100004	/* mysterious internal error */
#define	USYNC		100005	/* major synchronization error */
#define	UNOENT		100006	/* file not found when processing dbase */
#define	UBADLOCK	100007	/* bad lock range size (must be 1) */
#define	UBADLOG		100008	/* read error reprocessing log */
#define	UBADHOST	100009	/* problems with host name */
#define	UBADTYPE	100010	/* bad operation for this transaction type */
#define	UTWOENDS	100011	/* two commits or aborts done to transaction */
#define	UDONE		100012	/* operation done after abort (or commmit) */
#define	UNOSERVERS	100013	/* no servers appear to be up */
#define	UEOF		100014	/* premature EOF */
#define	ULOGIO		100015	/* error writing log file */
#define	UMAXCODE	100100	/* largest ubik error code */

#endif

/* ubik_trans types */
#define	UBIK_READTRANS	    0
#define	UBIK_WRITETRANS	    1

/* ubik_lock types */
#define	LOCKREAD	    1
#define	LOCKWRITE	    2

/* ubik client flags */
#define UPUBIKONLY 	    1    /* only check servers presumed functional */

/* RX services types */
#define	VOTE_SERVICE_ID	    50
#define	DISK_SERVICE_ID	    51
#define	USER_SERVICE_ID	    52		/* Since most applications use same port! */

#define	UBIK_MAGIC	0x354545

/* global ubik parameters */
#define	MAXSERVERS	    20		/* max number of servers */

/* version comparison macro */
#define vcmp(a,b) ((a).epoch == (b).epoch? ((a).counter - (b).counter) : ((a).epoch - (b).epoch))

/* ubik_client state bits */
#define	CFLastFailed	    1		/* last call failed to this guy (to detect down hosts) */

/* per-client structure for ubik */
struct ubik_client {
    short initializationState;		/* ubik client init state */
    short states[MAXSERVERS];		/* state bits */
    struct rx_connection *conns[MAXSERVERS];
};

#define	ubik_GetRPCConn(astr,aindex)	((aindex) >= MAXSERVERS? 0 : (astr)->conns[aindex])
#define	ubik_GetRPCHost(astr,aindex)	((aindex) >= MAXSERVERS? 0 : (astr)->hosts[aindex])

/* ubik transaction id representation */
struct ubik_tid {
    int32 epoch;	    /* time this server started */
    int32 counter;   /* counter within epoch of transactions */
};

/* ubik version representation */
struct ubik_version {
    int32 epoch;	    /* time this server started */
    int32 counter;   /* counter within epoch of transactions */
};

/* ubik header file structure */
struct ubik_hdr {
    int32 magic;		    /* magic number */
    short pad1;		    /* some 0-initd padding */
    short size;		    /* header allocation size */
    struct ubik_version	version;    /* the version for this file */
};

/* representation of a ubik transaction */
struct ubik_trans {
    struct ubik_dbase *dbase;	    /* corresponding database */
    struct ubik_trans *next;	    /* in the list */
    struct ubik_lock *activeLocks;
    struct ubik_trunc *activeTruncs;/* queued truncates */
    struct ubik_tid tid;	    /* transaction id of this trans (if write trans.) */
    int32 minCommitTime;		    /* time before which this trans can't commit */
    int32 seekFile;		    /* seek ptr: file number */
    int32 seekPos;		    /* seek ptr: offset therein */
    short flags;		    /* trans flag bits */
    char type;			    /* type of trans */
};

/* representation of a truncation operation */
struct ubik_trunc {
    struct ubik_trunc *next;
    int32 file;			    /* file to truncate */
    int32 length;		    /* new size */
};

/* representation of ubik byte-range lock */
struct ubik_lock {
    struct ubik_lock *next;	/* next lock same trans */
    int32 start;			/* byte range */
    int32 length;
    char type;			/* read or write lock */
    char flags;			/* useful flags */
};

struct ubik_stat {
    int32 size;
    int32 mtime;
};

#include <lock.h>			/* just to make sure we've go this */

/* representation of a ubik database.  Contains info on low-level disk access routines
    for use by disk transaction module.
*/
struct ubik_dbase {
    char *pathName;		    /* root name for dbase */
    struct ubik_trans *activeTrans; /* active transaction list */
    struct ubik_version	version;    /* version number */
    struct Lock versionLock;	    /* lock on version number */
    int32 tidCounter;                /* last RW or RO trans tid counter */
    int32 writeTidCounter;           /* last write trans tid counter */
    int32 flags;			    /* flags */
    int	(*read)();		    /* physio procedures */
    int (*write)();
    int (*truncate)();
    int (*sync)();
    int (*stat)();
    int (*open)();
    int	(*setlabel)();		    /* set the version label */
    int	(*getlabel)();		    /* retrieve the version label */
    int	(*getnfiles)();		    /* find out number of files */
    short readers;		    /* number of current read transactions */
    struct ubik_version	cachedVersion; /* version of caller's cached data */
};

/* procedures for automatically authenticating ubik connections */
extern int (*ubik_CRXSecurityProc)();
extern char *ubik_CRXSecurityRock;
extern int (*ubik_SRXSecurityProc)();
extern char *ubik_SRXSecurityRock;
extern int (*ubik_CheckRXSecurityProc)();
extern char *ubik_CheckRXSecurityRock;

/****************INTERNALS BELOW ****************/

#ifdef UBIK_INTERNALS
/* some ubik parameters */
#define	PAGESIZE	    1024	/* fits in current r packet */
#define	LOGPAGESIZE	    10		/* base 2 log thereof */
#define	NBUFFERS	    20		/* number of 1K buffers */
#define	HDRSIZE		    64		/* bytes of header per dbfile */

/* ubik_dbase flags */
#define	DBWRITING	    1		/* are any write trans. in progress */

/* ubik trans flags */
#define	TRDONE		    1		/* commit or abort done */
#define	TRABORT		    2		/* if TRDONE, tells if aborted */
#define TRREADANY	    4		/* read any data available in trans */

/* ubik_lock flags */
#define	LWANT		    1

/* ubik system database numbers */
#define	LOGFILE		    (-1)

/* define log opcodes */
#define	LOGNEW		    100	    /* start transaction */
#define	LOGEND		    101	    /* commit (good) end transaction */
#define	LOGABORT	    102	    /* abort (fail) transaction */
#define	LOGDATA		    103	    /* data */
#define	LOGTRUNCATE	    104	    /* truncate operation */

/* time constant for replication algorithms: the R time period is 20 seconds.  Both SMALLTIME
    and BIGTIME must be larger than RPCTIMEOUT+max(RPCTIMEOUT,POLLTIME),
    so that timeouts do not prevent us from getting through to our servers in time.

    We use multi-R to time out multiple down hosts concurrently.
    The only other restrictions:  BIGTIME > SMALLTIME and
    BIGTIME-SMALLTIME > MAXSKEW (the clock skew).
*/
#define MAXSKEW	10
#define POLLTIME 15
#define RPCTIMEOUT 20
#define BIGTIME 75
#define SMALLTIME 60

/* the per-server state, used by the sync site to keep track of its charges */
struct ubik_server {
    struct ubik_server *next;		/* next ptr */
    int32 addr;				/* network order host addr */
    int32 lastVoteTime;			/* last time yes vote received */
    int32 lastBeaconSent;		/* last time beacon attempted */
    struct ubik_version	version;	/* version, only used during recovery */
    struct rx_connection *vote_rxcid;	/* cid to use to contact dude for votes */
    struct rx_connection *disk_rxcid;	/* cid to use to contact dude for disk reqs */
    char lastVote;			/* true if last vote was yes */
    char up;				/* is it up? */
    char beaconSinceDown;		/* did beacon get through since last crash? */
    char currentDB;			/* is dbase up-to-date */
    char magic;				/* the one whose vote counts twice */
};

/* hold and release functions on a database */
#define	DBHOLD(a)	ObtainWriteLock(&((a)->versionLock))
#define	DBRELE(a)	ReleaseWriteLock(&((a)->versionLock))

/* globals */

/* list of all servers in the system */
extern struct ubik_server *ubik_servers;

/* network port info */
extern short ubik_callPortal;

/* urecovery state bits for sync site */
#define	UBIK_RECSYNCSITE	1	/* am sync site */
#define	UBIK_RECFOUNDDB		2	/* found acceptable dbase from quorum */
#define	UBIK_RECHAVEDB		4	/* fetched best dbase */
#define	UBIK_RECLABELDB		8	/* relabelled dbase */
#define	UBIK_RECSENTDB		0x10	/* sent best db to *everyone* */
#define	UBIK_RECSBETTER		UBIK_RECLABELDB	/* last state */

extern int32 ubik_quorum;			/* min hosts in quorum */
extern struct ubik_dbase *ubik_dbase;		/* the database handled by this server */
extern int32 ubik_host;				/* this host addr, in net order */
extern int ubik_amSyncSite;			/* sleep on this waiting to be sync site */
extern struct ubik_stats {		    	/* random stats */
    int32 escapes;
} ubik_stats;
extern int32 ubik_epochTime;			/* time when this site started */
extern int32 urecovery_state;			/* sync site recovery process state */
extern struct ubik_trans *ubik_currentTrans;	/* current trans */
extern struct ubik_version ubik_dbVersion;	/* sync site's dbase version */
extern int32 ubik_debugFlag;			/* ubik debug flag */

/* this extern gives the sync site's db version, with epoch of 0 if none yet */

extern int uphys_read();
extern int uphys_write();
extern int uphys_truncate();
extern int uphys_sync();
extern int uphys_open();
extern int uphys_stat();
extern int uphys_getlabel();
extern int uphys_setlabel();
extern int uphys_getnfiles();
extern int ubeacon_Interact();
extern int urecovery_Interact();
extern int sdisk_Interact();
extern int uvote_Interact();
extern int DISK_Abort();
extern int DISK_Begin();
extern int DISK_ReleaseLocks();
extern int DISK_Commit();
extern int DISK_Lock();
extern int DISK_Write();
extern int DISK_Truncate();
#endif /* UBIK_INTERNALS */

extern int32 ubik_nBuffers;

#endif /* UBIK_H */

/* End of prolog file ubik.p.h. */

#define UNOQUORUM                                (5376L)
#define UNOTSYNC                                 (5377L)
#define UNHOSTS                                  (5378L)
#define UIOERROR                                 (5379L)
#define UINTERNAL                                (5380L)
#define USYNC                                    (5381L)
#define UNOENT                                   (5382L)
#define UBADLOCK                                 (5383L)
#define UBADLOG                                  (5384L)
#define UBADHOST                                 (5385L)
#define UBADTYPE                                 (5386L)
#define UTWOENDS                                 (5387L)
#define UDONE                                    (5388L)
#define UNOSERVERS                               (5389L)
#define UEOF                                     (5390L)
#define ULOGIO                                   (5391L)
#define UBADFAM                                  (5392L)
#define UBADCELL                                 (5393L)
#define UBADSECGRP                               (5394L)
#define UBADGROUP                                (5395L)
#define UBADUUID                                 (5396L)
#define UNOMEM                                   (5397L)
#define UNOTMEMBER                               (5398L)
#define UNBINDINGS                               (5399L)
#define UBADPRINNAME                             (5400L)
#define UPIPE                                    (5401L)
#define UDEADLOCK                                (5402L)
#define UEXCEPTION                               (5403L)
#define UTPQFAIL                                 (5404L)
#define USKEWED                                  (5405L)
#define UNOLOCK                                  (5406L)
#define UNOACCESS                                (5407L)
#define UNOSPC                                   (5408L)
#define UBADPATH                                 (5409L)
#define UBADF                                    (5410L)
#define UREINITIALIZE                            (5411L)
extern void initialize_u_error_table ();
#define ERROR_TABLE_BASE_u (5376L)

/* for compatibility with older versions... */
#define init_u_err_tbl initialize_u_error_table
#define u_err_base ERROR_TABLE_BASE_u
