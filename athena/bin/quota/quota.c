/*
 * Disk quota reporting program.
 *    This version handles groups quotas and version 2 of the quota rpc.
 *      (-JR)
 */
#include <stdio.h>
#include <mntent.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>

#include <sys/param.h>
#include <sys/file.h>
#include <sys/time.h>
#include <ufs/quota.h>
#include <qoent.h>

#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <sys/socket.h>
#include <netdb.h>
#include <rpcsvc/rcquota.h>

int	vflag, iflag, localflag, gflag;
#define QFNAME	"quotas"

#define kb(n)   (howmany(dbtob(n), 1024))

main(argc, argv)
	char *argv[];
{
  register char *cp;

  argc--,argv++;
  while (argc > 0) {
    if (argv[0][0] == '-')
      for (cp = &argv[0][1]; *cp; cp++) switch (*cp) {

      case 'v':
	vflag++;
	break;

      case 'i':
	iflag++;
	break;
	
      case 'g':
	gflag++;
	break;

      default:
	fprintf(stderr, "quota: %c: unknown option\n",
		*cp);
	exit(1);
      }
    else
      break;
    argc--, argv++;
  }
  if (argc == 0) {
    showuid(getuid());
    exit(0);
  }

  localflag++;
  for (; argc > 0; argc--, argv++) {
    if (alldigits(*argv))
      showuid(atoi(*argv));
    else
      showname(*argv);
  }
  exit(0);
}

showuid(uid)
	int uid;
{
	struct passwd *pwd = getpwuid(uid);

	if (uid == 0) {
		if (vflag)
			printf("no disk quota for uid 0\n");
		return;
	}
	if (pwd == NULL)
		showquotas(uid, "(no account)");
	else
		showquotas(uid, pwd->pw_name);
}

showname(name)
	char *name;
{
	struct passwd *pwd = getpwnam(name);

	if (pwd == NULL) {
		fprintf(stderr, "quota: %s: unknown user\n", name);
		return;
	}
	if (pwd->pw_uid == 0) {
		if (vflag)
			printf("no disk quota for %s (uid 0)\n", name);
		return;
	}
	showquotas(pwd->pw_uid, name);
}

showquotas(uid, name)
	int uid;
	char *name;
{
  register struct mntent *mntp;
  FILE *mtab;
  struct getcquota_rslt qvalues;
  int myuid;

  myuid = getuid();
  if (uid != myuid && myuid != 0) {
    printf("quota: %s (uid %d): permission denied\n", name, uid);
    return;
  }
  if (vflag && !localflag)
    heading(uid, name);
  if (localflag) localheading(uid,name);

  mtab = setmntent(MOUNTED, "r");
  while (mntp = getmntent(mtab)) {
    if (strcmp(mntp->mnt_type, MNTTYPE_42) == 0) {
      if (getlocalquota(mntp, uid, &qvalues)){
	continue;
      }
    }
    else if (!localflag && (strcmp(mntp->mnt_type, MNTTYPE_NFS) == 0)){
      if (!getnfsquota(mntp, uid, &qvalues))
	continue;
    }
    else {
      continue;
    }
    if (vflag || localflag)
      prquota(mntp, &qvalues);
    else
      warn(mntp, &qvalues);
  }
  endmntent(mtab);
}

getlocalquota(mntp, uid, qvp)
     struct mntent *mntp;
     int uid;
     struct getcquota_rslt *qvp;
{
  struct qoent *qoent;
  FILE *qotab;
  struct dqblk dqblk;

  /* Make sure quotas are actually on */
  if (!hasmntopt(mntp, MNTOPT_QUOTA)) return(-1);

  /* First get the options */
  qotab = setqoent(QOTAB);
  while(qoent = getqoent(qotab)){
    if (!strcmp(mntp->mnt_dir, qoent->mnt_dir))
      break;
    else continue;
  }
  if (!qoent){ /* this partition has no quota options */
    fprintf(stderr, "Missing quota options : %s\n", mntp->mnt_dir);
    return(-1);
  }
  /* qoent contains options */
  qvp->rq_group = QO_GROUP(qoent);
  qvp->rq_bsize = DEV_BSIZE;
  qvp->gqr_zm.rq_bhardlimit = qoent->qo_bzhl;
  qvp->gqr_zm.rq_bsoftlimit = qoent->qo_bzsl;
  qvp->gqr_zm.rq_fhardlimit = qoent->qo_fzhl;
  qvp->gqr_zm.rq_fsoftlimit = qoent->qo_fzsl;

  if (localflag){
    if ((qvp->rq_group && !gflag) ||
	(!qvp->rq_group && gflag))
      return(-1);
  }
  
  if (!qvp->rq_group || localflag){

    if (quotactl(Q_GETQUOTA, mntp->mnt_fsname, uid, &dqblk)){
      /* ouch! quotas are on, but this failed */
      fprintf(stderr, "quotactl: %s %d\n", mntp->mnt_fsname, uid);
      return(-1);
    }
    qvp->gqr_rcquota[0].rq_id = uid;
    qvp->gqr_rcquota[0].rq_bhardlimit = dqblk.dqb_bhardlimit;
    qvp->gqr_rcquota[0].rq_bsoftlimit = dqblk.dqb_bsoftlimit;
    qvp->gqr_rcquota[0].rq_curblocks  = dqblk.dqb_curblocks;
    qvp->gqr_rcquota[0].rq_fhardlimit = dqblk.dqb_fhardlimit;
    qvp->gqr_rcquota[0].rq_fsoftlimit = dqblk.dqb_fsoftlimit;
    qvp->gqr_rcquota[0].rq_curfiles   = dqblk.dqb_curfiles;
    qvp->gqr_rcquota[0].rq_btimeleft  = dqblk.dqb_btimelimit;
    qvp->gqr_rcquota[0].rq_ftimeleft  = dqblk.dqb_ftimelimit;

    return(0);
  }

  else {
    int groups[NGROUPS];
    int i;

    qvp->rq_ngrps = getgroups(NGROUPS, groups);
    for(i=0;i<qvp->rq_ngrps;i++){
      if (quotactl(Q_GETQUOTA, mntp->mnt_fsname, groups[i], &dqblk)){
	/* ouch again! */
	fprintf(stderr, "quotactl: %s %d\n", mntp->mnt_fsname, groups[i]);
	return(-1);
      }
      qvp->gqr_rcquota[i].rq_id = groups[i];
      qvp->gqr_rcquota[i].rq_bhardlimit = dqblk.dqb_bhardlimit;
      qvp->gqr_rcquota[i].rq_bsoftlimit = dqblk.dqb_bsoftlimit;
      qvp->gqr_rcquota[i].rq_curblocks  = dqblk.dqb_curblocks;
      qvp->gqr_rcquota[i].rq_fhardlimit = dqblk.dqb_fhardlimit;
      qvp->gqr_rcquota[i].rq_fsoftlimit = dqblk.dqb_fsoftlimit;
      qvp->gqr_rcquota[i].rq_curfiles   = dqblk.dqb_curfiles;
      qvp->gqr_rcquota[i].rq_btimeleft  = dqblk.dqb_btimelimit;
      qvp->gqr_rcquota[i].rq_ftimeleft  = dqblk.dqb_ftimelimit;
    }
    return(0);
  }
}


warn(mntp, qvp)
     register struct mntent *mntp;
     register struct getcquota_rslt *qvp;
{
  struct timeval tv;
  int i;
  char *id_name, *id_type;
  struct rcquota *rqp;

  id_type = (qvp->rq_group? "group" : "user");
  if (!qvp->rq_group) qvp->rq_ngrps = 1;

  gettimeofday(&tv, NULL);

  for(i=0; i<qvp->rq_ngrps; i++){

    id_name = (qvp->rq_group ?
	       getgrgid(qvp->gqr_rcquota[i].rq_id)->gr_name :
	       getpwuid(qvp->gqr_rcquota[i].rq_id)->pw_name);
  
    rqp = &(qvp->gqr_rcquota[i]);

    /* Correct for zero quotas... */
    if(!rqp->rq_bsoftlimit)
      rqp->rq_bsoftlimit = qvp->gqr_zm.rq_bsoftlimit;
    if(!rqp->rq_bhardlimit)
      rqp->rq_bhardlimit = qvp->gqr_zm.rq_bhardlimit;
    if(!rqp->rq_fsoftlimit)
      rqp->rq_fsoftlimit = qvp->gqr_zm.rq_fsoftlimit;
    if(!rqp->rq_fhardlimit)
      rqp->rq_fhardlimit = qvp->gqr_zm.rq_fhardlimit;

    /* Now check for over...*/
    if(rqp->rq_curblocks >= rqp->rq_bhardlimit)
      printf("Block limit reached for %s %s on %s\n",
	     id_type, id_name, mntp->mnt_dir);

    else if (rqp->rq_curblocks >= rqp->rq_bsoftlimit){
      if (rqp->rq_btimeleft == 0) {
	printf("%s %s over disk quota on %s, remove %dK\n",
	       id_type, id_name, mntp->mnt_dir,
	       kb(rqp->rq_curblocks - rqp->rq_bsoftlimit + 1));
      }
      else if (rqp->rq_btimeleft > tv.tv_sec) {
	char btimeleft[80];

	fmttime(btimeleft, rqp->rq_btimeleft - tv.tv_sec);
	printf("%s %s over disk quota on %s, remove %dK within %s\n",
	       id_type, id_name, mntp->mnt_dir,
	       kb(rqp->rq_curblocks - rqp->rq_bsoftlimit + 1),
	       btimeleft);
      }
      else {
	printf("%s %s over disk quota on %s, time limit has expired, remove %dK\n",
	       id_type, id_name, mntp->mnt_dir,
	       kb(rqp->rq_curblocks - rqp->rq_bsoftlimit + 1));
      }
    }

    if (rqp->rq_curfiles >= rqp->rq_fhardlimit)
      printf("File count limit reached for %s %s on %s\n",
	     id_type, id_name, mntp->mnt_dir);

    else if (rqp->rq_curfiles >= rqp->rq_fsoftlimit) {
      if (rqp->rq_ftimeleft == 0) {
	printf("%s %s over file quota on %s, remove %d file%s\n",
	       id_type, id_name, mntp->mnt_dir,
	       rqp->rq_curfiles - rqp->rq_fsoftlimit + 1,
	       ((rqp->rq_curfiles - rqp->rq_fsoftlimit + 1) > 1 ?
		"s" : "" ));
      }

      else if (rqp->rq_ftimeleft > tv.tv_sec) {
	char ftimeleft[80];

	fmttime(ftimeleft, rqp->rq_ftimeleft - tv.tv_sec);
	printf("%s %s over file quota on %s, remove %d file%s within %s\n",
	       id_type,id_name,mntp->mnt_dir,
	       rqp->rq_curfiles - rqp->rq_fsoftlimit + 1,
	       ((rqp->rq_curfiles - rqp->rq_fsoftlimit + 1) > 1 ?
		"s" : "" ), ftimeleft);
      }
      else {
	printf("%s %s over file quota on %s, time limit has expired, remove %d file%s\n",
	       id_type, id_name, mntp->mnt_dir,
	       rqp->rq_curfiles - rqp->rq_fsoftlimit + 1,
	       ((rqp->rq_curfiles - rqp->rq_fsoftlimit + 1) > 1 ?
		"s" : "" ));
      }
    }
  }
}

heading(uid,name)
     int uid;
     char *name;
{
  printf("Disk quotas for %s (uid %d):\n",name,uid);
  printf("%-27s%-6s%-15s%6s%7s%7s%12s\n"
	 , "Filesystem"
	 , "Type"
	 , "ID"
	 , (iflag ? "files" : "usage")
	 , "quota"
	 , "limit"
	 , "timeleft"
	 );
}
  


localheading(uid, name)
	int uid;
	char *name;
{
	printf("Disk quotas for %s %s (%s %d):\n",
	       (gflag? "group":"user"), name,
	       (gflag? "gid": "uid"), uid);
	printf("%-12s %7s%7s%7s%12s%7s%7s%7s%12s\n"
		, "Filesystem"
		, "usage"
		, "quota"
		, "limit"
		, "timeleft"
		, "files"
		, "quota"
		, "limit"
		, "timeleft"
	);
}

prquota(mntp, qvp)
	register struct mntent *mntp;
	register struct getcquota_rslt *qvp;
{
  struct timeval tv;
  char ftimeleft[80], btimeleft[80];
  char *cp;
  int i;
  char *id_name, *id_type;
  struct rcquota *rqp;

  id_type = (qvp->rq_group? "group" : "user");
  if (!qvp->rq_group || localflag) qvp->rq_ngrps = 1;

  gettimeofday(&tv, NULL);

  if (strlen(mntp->mnt_dir) > 26) {
    printf("%s\n", mntp->mnt_dir);
    cp = "";
  } else {
    cp = mntp->mnt_dir;
  }

  for(i=0; i<qvp->rq_ngrps; i++){
    rqp = &(qvp->gqr_rcquota[i]);
    /* We're not interested in this group if all is zero */
    if (!rqp->rq_bsoftlimit && !rqp->rq_bhardlimit
	&& !rqp->rq_curblocks && !rqp->rq_fsoftlimit
	&& !rqp->rq_fhardlimit && !rqp->rq_curfiles) continue;

    if (!localflag)
      id_name = (qvp->rq_group ?
	       getgrgid(qvp->gqr_rcquota[i].rq_id)->gr_name :
	       getpwuid(qvp->gqr_rcquota[i].rq_id)->pw_name);
    
    /* Correct for zero quotas... */
    if(!rqp->rq_bsoftlimit)
      rqp->rq_bsoftlimit = qvp->gqr_zm.rq_bsoftlimit;
    if(!rqp->rq_bhardlimit)
      rqp->rq_bhardlimit = qvp->gqr_zm.rq_bhardlimit;
    if(!rqp->rq_fsoftlimit)
      rqp->rq_fsoftlimit = qvp->gqr_zm.rq_fsoftlimit;
    if(!rqp->rq_fhardlimit)
      rqp->rq_fhardlimit = qvp->gqr_zm.rq_fhardlimit;

    if (rqp->rq_curblocks >= rqp->rq_bsoftlimit) {
      if (rqp->rq_btimeleft == 0) {
	strcpy(btimeleft, "NOT STARTED");
      } else if (rqp->rq_btimeleft > tv.tv_sec) {
	fmttime(btimeleft, rqp->rq_btimeleft - tv.tv_sec);
      } else {
	strcpy(btimeleft, "EXPIRED");
      }
    } else {
      btimeleft[0] = '\0';
    }
    if (rqp->rq_fsoftlimit && rqp->rq_curfiles >= rqp->rq_fsoftlimit) {
      if (rqp->rq_ftimeleft == 0) {
	strcpy(ftimeleft, "NOT STARTED");
      } else if (rqp->rq_ftimeleft > tv.tv_sec) {
	fmttime(ftimeleft, rqp->rq_ftimeleft - tv.tv_sec);
      } else {
	strcpy(ftimeleft, "EXPIRED");
      }
    } else {
      ftimeleft[0] = '\0';
    }

    if (localflag){
      printf("%-12.12s %7d%7d%7d%12s%7d%7d%7d%12s\n",
	     cp,
	     kb(rqp->rq_curblocks),
	     kb(rqp->rq_bsoftlimit),
	     kb(rqp->rq_bhardlimit),
	     btimeleft,
	     rqp->rq_curfiles,
	     rqp->rq_fsoftlimit,
	     rqp->rq_fhardlimit,
	     ftimeleft
	     );
    }
    else{
      printf("%-27s%-6s%-15s%6d%7d%7d%12s\n",
	     cp,
	     id_type,
	     id_name,
	     (iflag? rqp->rq_curfiles : kb(rqp->rq_curblocks)),
	     (iflag? rqp->rq_fsoftlimit : kb(rqp->rq_bsoftlimit)),
	     (iflag? rqp->rq_fhardlimit : kb(rqp->rq_bhardlimit)),
	     (iflag? ftimeleft : btimeleft));
    }
  }
}

fmttime(buf, time)
	char *buf;
	register long time;
{
	int i;
	static struct {
		int c_secs;		/* conversion units in secs */
		char * c_str;		/* unit string */
	} cunits [] = {
		{60*60*24*28, "months"},
		{60*60*24*7, "weeks"},
		{60*60*24, "days"},
		{60*60, "hours"},
		{60, "mins"},
		{1, "secs"}
	};

	if (time <= 0) {
		strcpy(buf, "EXPIRED");
		return;
	}
	for (i = 0; i < sizeof(cunits)/sizeof(cunits[0]); i++) {
		if (time >= cunits[i].c_secs)
			break;
	}
	sprintf(buf, "%.1f %s", (double)time/cunits[i].c_secs, cunits[i].c_str);
}

alldigits(s)
	register char *s;
{
	register c;

	c = *s++;
	do {
		if (!isdigit(c))
			return (0);
	} while (c = *s++);
	return (1);
}

int
getnfsquota(mntp, uid, qvp)
	struct mntent *mntp;
	int uid;
	struct getcquota_rslt *qvp;
{
	char *hostp;
	char *cp;
	struct getquota_args gq_args;
	extern char *index();

	hostp = mntp->mnt_fsname;
	cp = index(mntp->mnt_fsname, ':');
	if (cp == 0) {
		fprintf(stderr, "cannot find hostname for %s\n", mntp->mnt_dir);
		return (0);
	}
	*cp = '\0';
	gq_args.gqa_pathp = cp + 1;
	gq_args.gqa_uid = uid;
	if (callrpc(hostp, RCQUOTAPROG, RCQUOTAVERS,
	    (vflag? RCQUOTAPROC_GETQUOTA: RCQUOTAPROC_GETACTIVEQUOTA),
	    xdr_getquota_args, &gq_args, xdr_getcquota_rslt, qvp) != 0) {
		*cp = ':';
		return (0);
	}
	switch (qvp->gqr_status) {
	case Q_OK:
		{
		struct timeval tv;
		int i;
		float blockconv;

		gettimeofday(&tv, NULL);
		blockconv = (float)qvp->rq_bsize / DEV_BSIZE;
		qvp->gqr_zm.rq_bhardlimit *= blockconv;
		qvp->gqr_zm.rq_bsoftlimit *= blockconv;
		qvp->gqr_zm.rq_curblocks  *= blockconv;
		if (!qvp->rq_group) qvp->rq_ngrps = 1;
		for(i=0;i<qvp->rq_ngrps;i++){
		  qvp->gqr_rcquota[i].rq_bhardlimit *= blockconv;
		  qvp->gqr_rcquota[i].rq_bsoftlimit *= blockconv;
		  qvp->gqr_rcquota[i].rq_curblocks  *= blockconv;
		  qvp->gqr_rcquota[i].rq_btimeleft += tv.tv_sec;
		  qvp->gqr_rcquota[i].rq_ftimeleft += tv.tv_sec;
		}
		*cp = ':';
		return (1);
		}

	case Q_NOQUOTA:
		break;

	case Q_EPERM:
		fprintf(stderr, "Warning: no NFS mapping on host: %s\n", hostp);
		break;

	default:
		fprintf(stderr, "bad rpc result, host: %s\n",  hostp);
		break;
	}
	*cp = ':';
	return (0);
}
