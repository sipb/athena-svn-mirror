#ifndef	__EXPORTER__
#define	__EXPORTER__

#ifdef UID_NOBODY
#define	NFS_NOBODY	UID_NOBODY
#endif
#ifndef	NFS_NOBODY
#define NFS_NOBODY      -2	/* maps Nfs's "nobody" but since not declared by some systems (i.e. Ultrix) we use a constant  */
#endif
#define	RMTUSER_REQ		0xabc
#if defined(AFS_OSF_ENV) || (defined(AFS_SGI61_ENV) && (_MIPS_SZPTR==64))
#define	AFS_XLATOR_MAGIC	0x8765		/* XXX */
#else
#define	AFS_XLATOR_MAGIC	0x87654321
#endif

#ifdef	AFS_OSF_ENV
#define	AFS_NFSXLATORREQ(cred)    ((cred)->cr_ruid == NFSXLATOR_CRED)
#else
#define	AFS_NFSXLATORREQ(cred)    ((cred)->cr_rgid == NFSXLATOR_CRED)
#endif

struct	exporterops {
    int	    (*export_reqhandler)();
    int	    (*export_hold)();
    int	    (*export_rele)();
    int	    (*export_sysname)();
    int	    (*export_garbagecollect)();
    int	    (*export_statistics)();
};

struct exporterstats {
    int32 calls;			/* # of calls to the exporter */
    int32 rejectedcalls;		/* # of afs rejected  calls */
    int32 nopag;			/* # of unpagged remote calls */
    int32 invalidpag;		/* # of invalid pag calls */
};

struct afs_exporter {
    struct  afs_exporter   *exp_next;
    struct  exporterops	    *exp_op;
    int32		    exp_states;
    int32		    exp_type;
    struct  exporterstats   exp_stats;
    char		    *exp_data;
};

/* exp_type values */
#define	EXP_NULL    0	    /* Undefined */
#define	EXP_NFS	    1	    /* Nfs/Afs translator */

/* exp_states values */
#define	EXP_EXPORTED	1
#define	EXP_UNIXMODE	2
#define	EXP_PWSYNC	4
#define	EXP_SUBMOUNTS	8


#define	AFS_NFSFULLFID	1

#define	EXP_REQHANDLER(EXP, CRED, HOST, PAG, EXPP) \
        (*(EXP)->exp_op->export_reqhandler)(EXP, CRED, HOST, PAG, EXPP)
#define	EXP_HOLD(EXP)	\
        (*(EXP)->exp_op->export_hold)(EXP)
#define	EXP_RELE(EXP)	\
        (*(EXP)->exp_op->export_rele)(EXP)
#define	EXP_SYSNAME(EXP, INNAME, OUTNAME)   \
        (*(EXP)->exp_op->export_sysname)(EXP, INNAME, OUTNAME)
#define	EXP_GC(EXP, param)	\
        (*(EXP)->exp_op->export_garbagecollect)(EXP, param)
#define	EXP_STATISTICS(EXP)	\
        (*(EXP)->exp_op->export_statistics)(EXP)

struct afs3_fid {
    u_short len;
    u_short padding;
    u_int32 Cell;
    u_int32 Volume;
    u_int32 Vnode;
    u_int32 Unique;
};

struct Sfid {
    u_int32 padding;
    u_int32 Cell;
    u_int32 Volume;
    u_int32 Vnode;
    u_int32 Unique;
#ifdef	AFS_SUN5_ENV
    struct cred *credp;
#endif
};


#endif /* __EXPORTER__ */
