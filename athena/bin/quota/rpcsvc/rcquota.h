/*
 * remote quota inquiry protocol
 */

#define RQUOTAPROG	100011
#define RQUOTAVERS_ORIG	1
#define RQUOTAVERS	2

/*
 * inquire about quotas for uid (assume AUTH_UNIX)
 *	input - getquota_args			(xdr_getquota2_args)
 *	output - getquota2_rslt			(xdr_getquota2_rslt)
 */
#define RQUOTAPROC_GETQUOTA		1	/* get quota */
#define RQUOTAPROC_GETACTIVEQUOTA	2	/* get only active quotas */

/*
 * args to RQUOTAPROC_GETQUOTA and RQUOTAPROC_GETACTIVEQUOTA
 */
struct getquota_args {
	char *gqa_pathp;		/* path to filesystem of interest */
	int gqa_uid;			/* inquire about quota for uid */
};

/*
 * remote quota structure
 */
struct rquota2 {
  int    rq_id;                 /* Which id (uid or gid) this record is for */
  u_long rq_bhardlimit;		/* absolute limit on disk blks alloc */
  u_long rq_bsoftlimit;		/* preferred limit on disk blks */
  u_long rq_curblocks;		/* current block count */
  u_long rq_fhardlimit;		/* absolute limit on allocated files */
  u_long rq_fsoftlimit;		/* preferred file limit */
  u_long rq_curfiles;		/* current # allocated files */
  u_long rq_btimeleft;		/* time left for excessive disk use */
  u_long rq_ftimeleft;		/* time left for excessive files */
};	

enum gqr_status {
  Q_OK = 1,			/* quotas returned */
  Q_NOQUOTA = 2,		/* noquota for uid */
  Q_EPERM = 3,			/* no permission to access quota */
};

struct getquota2_rslt {
  enum gqr_status gqr_status;	/* discriminant */
  bool_t rq_group;              /* inidicates group quotas instead of user */
  int  rq_ngrps;		/* number of groups */
  int rq_bsize;			/* block size for block counts */
  struct rquota2 gqr_zm;        /* What zero means */
  struct rquota2 gqr_rquota[NGROUPS+1]; /* valid if status == Q_OK */
};

extern bool_t xdr_getquota_args();
extern bool_t xdr_getquota2_rslt();
extern bool_t xdr_rquota2();
