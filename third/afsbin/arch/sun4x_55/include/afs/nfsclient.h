/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1987, 1988
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */

#ifndef	__NFSCLIENT__
#define	__NFSCLIENT__

#define	NNFSCLIENTS	32	/* Hash table size for afs_nfspags table */
#define	NHash(host)	((host) & (NNFSCLIENTS-1))
#define NFSCLIENTGC     (10*3600) /* time after which to GC nfsclientpag structs */
#define	NFSXLATOR_CRED	0xaaaa

struct nfsclientpag {
    /* From here to .... */
    struct nfsclientpag	*next;	/* Next hash pointer */
    struct exporterops	*nfs_ops;
    int32 states;
    int32 type;
    struct exporterstats nfs_stats;
    /* .... here is also an overlay to the afs_exporter structure */

    int32 refCount;		/* Ref count for packages using this */
    int32 uid;			/* search based on uid and ... */
    int32 host;			/* ... nfs client's host ip address */
    int32 pag;			/* active pag for all  (uid, host) "unpaged" conns */
    char *sysname;		/* user's "@sys" value; also kept in unixuser */
    int32 lastcall;		/*  Used for timing out nfsclientpag structs  */
};


#endif /* __NFSCLIENT__ */

