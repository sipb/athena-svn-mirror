/*
 *   Disk quota reporting program.
 *
 *   $Author jnrees $
 *   $Header: /afs/dev.mit.edu/source/repository/athena/bin/quota/quota.c,v 1.7 1990-05-24 10:58:11 jnrees Exp $
 *   $Log: not supported by cvs2svn $
 * Revision 1.6  90/05/23  12:25:41  jnrees
 * Changed output format.  '-i' flag no longer needed.
 * Added a usage message if command line is improper.
 * 
 * Revision 1.5  90/05/22  12:12:09  jnrees
 * Fixup of permissions, plus fallback to old rpc call
 * if the first call fails because the rpc server is not
 * registered on the server machine.
 * 
 * Revision 1.4  90/05/17  15:04:02  jnrees
 * Fixed bug where unknown uid or gid would result in a bus error
 * 
 *   
 *   Uses the rcquota rpc call for group and user quotas
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
#include <rpcsvc/rquota.h>
#include <rpcsvc/rcquota.h>

static char *warningstring = NULL;

static int	vflag, localflag, gflag;
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
	vflag=1;
	break;

      case 'g':
	gflag=1;
	break;

      default:
	fprintf(stderr, "quota: %c: unknown option\n",
		*cp);
	usage();
	exit(1);
      }
    else
      break;
    argc--, argv++;
  }
  if (argc == 0){
    if (gflag){
      fprintf(stderr, "A group name or id must be specified with the -g option.\n");
      usage();
      exit(1);
    }
    
    showid(getuid());
    exit(0);
  }

  localflag++;
  for (; argc > 0; argc--, argv++) {
    if (alldigits(*argv))
      showid(atoi(*argv));
    else
      showname(*argv);
  }
  exit(0);
}

showid(id)
	int id;
{
  struct passwd *pwd = getpwuid(id);
  struct group  *grp = getgrgid(id);

  if (id == 0) {
    if (vflag)
      printf("no disk quota for %s 0\n",
	     (gflag ? "gid": "uid"));
    return;
  }
  if (gflag){
    if (grp == NULL) showquotas(id, "(no group)");
    else showquotas(id, grp->gr_name);
  }
  else{
    if (pwd == NULL) showquotas(id, "(no account)");
    else showquotas(id, pwd->pw_name);
  }
}

showname(name)
	char *name;
{
  struct passwd *pwd = getpwnam(name);
  struct group  *grp = getgrnam(name);

  if (gflag){
    if (grp == NULL){
      fprintf(stderr, "quota: %s: unknown group\n", name);
      return;
    }
    if (grp->gr_gid == 0){
      if (vflag) printf("no disk quota for %s (gid 0)\n", name);
      return;
    }
    showquotas(grp->gr_gid, name);
  }
  else{
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
}

showquotas(id, name)
	int id;
	char *name;
{
  register struct mntent *mntp;
  FILE *mtab;
  struct getcquota_rslt qvalues;
  int myuid, ngroups, gidset[NGROUPS];

  myuid = getuid();

  if (gflag){			/* User must be in group or be root */
    if ((ngroups = getgroups(NGROUPS, gidset)) == -1){
      perror("Couldn't get groups you are in.");
      exit(1);
    }
    while(ngroups){
      if (id == gidset[ngroups-1]) break;
      --ngroups;
    }
    if (!ngroups && myuid != 0){
      printf("quota: %s (gid %d): permission denied\n", name, id);
      return;
    }
  }
  else{
    if (id != myuid && myuid != 0) {
      printf("quota: %s (uid %d): permission denied\n", name, id);
      return;
    }
  }

  if (vflag && !localflag)
    heading(id, name);
  if (vflag && localflag) localheading(id,name);

  mtab = setmntent(MOUNTED, "r");
  while (mntp = getmntent(mtab)) {
    if (strcmp(mntp->mnt_type, MNTTYPE_42) == 0
	&& hasmntopt(mntp, MNTOPT_QUOTA)){
      if (getlocalquota(mntp, id, &qvalues)) continue;
    }
    else if (!localflag && (strcmp(mntp->mnt_type, MNTTYPE_NFS) == 0)){
      if (!getnfsquota(mntp, id, &qvalues)) continue;
    }
    else {
      continue;
    }
    if (vflag){
      prquota(mntp, &qvalues);
      if (!localflag) warn(mntp, &qvalues);
    }
    else warn(mntp, &qvalues);
  }
  endmntent(mtab);
  if (warningstring) printf("\n%s\n", warningstring);
  
}

getlocalquota(mntp, uid, qvp)
     struct mntent *mntp;
     int uid;
     struct getcquota_rslt *qvp;
{
  struct qoent *qoent;
  FILE *qotab;
  struct dqblk dqblk;

  /* First get the options */
  qotab = setqoent(QOTAB);
  while(qoent = getqoent(qotab)){
    if (!strcmp(mntp->mnt_dir, qoent->mnt_dir))
      break;
    else continue;
  }
  if (!qoent){ /* this partition has no quota options, use defaults */
    qvp->rq_group = !strcmp(QOTYPE_DEFAULT, QOTYPE_GROUP);
    qvp->rq_bsize = DEV_BSIZE;
    qvp->gqr_zm.rq_bhardlimit = QO_BZHL_DEFAULT;
    qvp->gqr_zm.rq_bsoftlimit = QO_BZSL_DEFAULT;
    qvp->gqr_zm.rq_fhardlimit = QO_FZHL_DEFAULT;
    qvp->gqr_zm.rq_fsoftlimit = QO_FZSL_DEFAULT;
  }
  else{
    /* qoent contains options */
    qvp->rq_group = QO_GROUP(qoent);
    qvp->rq_bsize = DEV_BSIZE;
    qvp->gqr_zm.rq_bhardlimit = qoent->qo_bzhl;
    qvp->gqr_zm.rq_bsoftlimit = qoent->qo_bzsl;
    qvp->gqr_zm.rq_fhardlimit = qoent->qo_fzhl;
    qvp->gqr_zm.rq_fsoftlimit = qoent->qo_fzsl;
  }

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
    qvp->rq_ngrps = 1;
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
  char buf[1024];
  char *id_name, *id_type;
  struct rcquota *rqp;

  id_type = (qvp->rq_group? "Group" : "User");
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
    if(rqp->rq_bhardlimit &&
       rqp->rq_curblocks >= rqp->rq_bhardlimit){
      sprintf(buf,
	      "Block limit reached for %s %s on %s\n",
	     id_type, id_name, mntp->mnt_dir);
      putwarning(buf);
    }

    else if (rqp->rq_bsoftlimit &&
	     rqp->rq_curblocks >= rqp->rq_bsoftlimit){
      if (rqp->rq_btimeleft == 0) {
	sprintf(buf,
		"%s %s over disk quota on %s, remove %dK\n",
	       id_type, id_name, mntp->mnt_dir,
	       kb(rqp->rq_curblocks - rqp->rq_bsoftlimit + 1));
	putwarning(buf);
      }
      else if (rqp->rq_btimeleft > tv.tv_sec) {
	char btimeleft[80];

	fmttime(btimeleft, rqp->rq_btimeleft - tv.tv_sec);
	sprintf(buf,
		"%s %s over disk quota on %s, remove %dK within %s\n",
	       id_type, id_name, mntp->mnt_dir,
	       kb(rqp->rq_curblocks - rqp->rq_bsoftlimit + 1),
	       btimeleft);
	putwarning(buf);
      }
      else {
	sprintf(buf,
		"%s %s over disk quota on %s, time limit has expired, remove %dK\n",
	       id_type, id_name, mntp->mnt_dir,
	       kb(rqp->rq_curblocks - rqp->rq_bsoftlimit + 1));
	putwarning(buf);
      }
    }

    if (rqp->rq_fhardlimit &&
	rqp->rq_curfiles >= rqp->rq_fhardlimit){
      sprintf(buf,
	      "File count limit reached for %s %s on %s\n",
	     id_type, id_name, mntp->mnt_dir);
      putwarning(buf);
    }

    else if (rqp->rq_fsoftlimit &&
	     rqp->rq_curfiles >= rqp->rq_fsoftlimit) {
      if (rqp->rq_ftimeleft == 0) {
	sprintf(buf,
		"%s %s over file quota on %s, remove %d file%s\n",
	       id_type, id_name, mntp->mnt_dir,
	       rqp->rq_curfiles - rqp->rq_fsoftlimit + 1,
	       ((rqp->rq_curfiles - rqp->rq_fsoftlimit + 1) > 1 ?
		"s" : "" ));
	putwarning(buf);
      }

      else if (rqp->rq_ftimeleft > tv.tv_sec) {
	char ftimeleft[80];

	fmttime(ftimeleft, rqp->rq_ftimeleft - tv.tv_sec);
	sprintf(buf,
		"%s %s over file quota on %s, remove %d file%s within %s\n",
	       id_type,id_name,mntp->mnt_dir,
	       rqp->rq_curfiles - rqp->rq_fsoftlimit + 1,
	       ((rqp->rq_curfiles - rqp->rq_fsoftlimit + 1) > 1 ?
		"s" : "" ), ftimeleft);
	putwarning(buf);
      }
      else {
	sprintf(buf,
		"%s %s over file quota on %s, time limit has expired, remove %d file%s\n",
	       id_type, id_name, mntp->mnt_dir,
	       rqp->rq_curfiles - rqp->rq_fsoftlimit + 1,
	       ((rqp->rq_curfiles - rqp->rq_fsoftlimit + 1) > 1 ?
		"s" : "" ));
	putwarning(buf);
      }
    }
  }
}

heading(uid,name)
     int uid;
     char *name;
{
  printf("Disk quotas for %s (uid %d):\n",name,uid);
  printf("%-16s%-6s%-12s%6s%7s%7s  %7s%7s%7s\n"
	 , "Filesystem"
	 , "Type"
	 , "ID"
	 , "usage", "quota", "limit"
	 , "files", "quota", "limit"
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
  char ftimeleft[80], btimeleft[80], idbuf[20];
  char *cp;
  int i;
  char *id_name = "", *id_type;
  struct rcquota *rqp;

  id_type = (qvp->rq_group? "group" : "user");
  if (!qvp->rq_group || localflag) qvp->rq_ngrps = 1;

  cp = mntp->mnt_dir;
  if (strlen(cp) > 15){
    printf("%s\n", cp);
    cp = "";
  }

  gettimeofday(&tv, NULL);

  for(i=0; i<qvp->rq_ngrps; i++){
    rqp = &(qvp->gqr_rcquota[i]);

    /* We're not interested in this group if all is zero */
    if (!rqp->rq_bsoftlimit && !rqp->rq_bhardlimit
        && !rqp->rq_curblocks && !rqp->rq_fsoftlimit
        && !rqp->rq_fhardlimit && !rqp->rq_curfiles) continue;

    if (!localflag){
      if (qvp->rq_group){
	if (getgrgid(qvp->gqr_rcquota[i].rq_id))
	  id_name = getgrgid(qvp->gqr_rcquota[i].rq_id)->gr_name;
	else{
	  sprintf(idbuf, "#%d", qvp->gqr_rcquota[i].rq_id);
	  id_name = idbuf;
	}
      }
      else{
	if (getpwuid(qvp->gqr_rcquota[i].rq_id))
	  id_name = getpwuid(qvp->gqr_rcquota[i].rq_id)->pw_name;
	else{
	  sprintf(idbuf, "#%d", qvp->gqr_rcquota[i].rq_id);
	  id_name = idbuf;
	}
      }
    }
    
    /* Correct for zero quotas... */
    if(!rqp->rq_bsoftlimit)
      rqp->rq_bsoftlimit = qvp->gqr_zm.rq_bsoftlimit;
    if(!rqp->rq_bhardlimit)
      rqp->rq_bhardlimit = qvp->gqr_zm.rq_bhardlimit;
    if(!rqp->rq_fsoftlimit)
      rqp->rq_fsoftlimit = qvp->gqr_zm.rq_fsoftlimit;
    if(!rqp->rq_fhardlimit)
      rqp->rq_fhardlimit = qvp->gqr_zm.rq_fhardlimit;

    if (!rqp->rq_bsoftlimit && !rqp->rq_bhardlimit &&
	!rqp->rq_fsoftlimit && !rqp->rq_fhardlimit)
      /* Skip this entirely for compatibility */
      continue;

    if (rqp->rq_bsoftlimit &&
	rqp->rq_curblocks >= rqp->rq_bsoftlimit) {
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
    if (rqp->rq_fsoftlimit &&
	rqp->rq_curfiles >= rqp->rq_fsoftlimit) {
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
      printf("%-16s%-6s%-12.12s%6d%7d%7d%-2s%7d%7d%7d%-2s\n",
	     cp, id_type, id_name,
	     kb(rqp->rq_curblocks),
	     kb(rqp->rq_bsoftlimit),
	     kb(rqp->rq_bhardlimit),
	     (btimeleft[0]? "<<" : ""),
	     rqp->rq_curfiles,
	     rqp->rq_fsoftlimit,
	     rqp->rq_fhardlimit,
	     (ftimeleft[0]? "<<" : ""));
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
	struct getcquota_args gq_args;
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
	if ((enum clnt_stat)
	    callrpc(hostp, RCQUOTAPROG, RCQUOTAVERS,
		    (vflag? RCQUOTAPROC_GETQUOTA: RCQUOTAPROC_GETACTIVEQUOTA),
		    xdr_getcquota_args, &gq_args, xdr_getcquota_rslt, qvp) ==
	    RPC_PROGNOTREGISTERED) {
	  /* Fallback on old rpc */
	  struct getquota_rslt oldquota_result;

	  if (callaurpc(hostp, RQUOTAPROG, RQUOTAVERS,
			(vflag? RQUOTAPROC_GETQUOTA:
			 RQUOTAPROC_GETACTIVEQUOTA),
			xdr_getquota_args, &gq_args,
			xdr_getquota_rslt, &oldquota_result) != 0){
	    /* Okay, it really failed */
	    *cp = ':';
	    return (0);
	  }
	  else{
	    /* We have to convert the old return structure to
	       a new format structure.*/
	    qvp->gqr_status = (enum gcqr_status) oldquota_result.gqr_status;
	    qvp->rq_group = 0;
	    qvp->rq_ngrps = 0;
	    qvp->rq_bsize = oldquota_result.gqr_rquota.rq_bsize;

	    bzero(&qvp->gqr_zm, sizeof(struct rcquota));

	    qvp->gqr_rcquota[0].rq_id = getuid();
	    qvp->gqr_rcquota[0].rq_bhardlimit =
	       oldquota_result.gqr_rquota.rq_bhardlimit;
	    qvp->gqr_rcquota[0].rq_bsoftlimit =
	       oldquota_result.gqr_rquota.rq_bsoftlimit;
	    qvp->gqr_rcquota[0].rq_curblocks =
	      oldquota_result.gqr_rquota.rq_curblocks;
	    qvp->gqr_rcquota[0].rq_fhardlimit =
	       oldquota_result.gqr_rquota.rq_fhardlimit;
	    qvp->gqr_rcquota[0].rq_fsoftlimit =
	       oldquota_result.gqr_rquota.rq_fsoftlimit;
	    qvp->gqr_rcquota[0].rq_curfiles = 
	      oldquota_result.gqr_rquota.rq_curfiles;
	    qvp->gqr_rcquota[0].rq_btimeleft = 
	      oldquota_result.gqr_rquota.rq_btimeleft;
	    qvp->gqr_rcquota[0].rq_ftimeleft = 
	      oldquota_result.gqr_rquota.rq_ftimeleft;
	  }
	}
	
	switch (qvp->gqr_status) {
	case QC_OK:
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

	case QC_NOQUOTA:
		break;

	case QC_EPERM:
		if (vflag)
		  fprintf(stderr, "Warning: no NFS mapping on host: %s\n", hostp);
		break;

	default:
		fprintf(stderr, "bad rpc result, host: %s\n",  hostp);
		break;
	}
	*cp = ':';
	return (0);
}

callaurpc(host, prognum, versnum, procnum, inproc, in, outproc, out)
	char *host;
	xdrproc_t inproc, outproc;
	char *in, *out;
{
	struct sockaddr_in server_addr;
	enum clnt_stat clnt_stat;
	struct hostent *hp;
	struct timeval timeout, tottimeout;

	static CLIENT *client = NULL;
	static int socket = RPC_ANYSOCK;
	static int valid = 0;
	static int oldprognum, oldversnum;
	static char oldhost[256];

	if (valid && oldprognum == prognum && oldversnum == versnum
		&& strcmp(oldhost, host) == 0) {
		/* reuse old client */		
	}
	else {
		valid = 0;
		close(socket);
		socket = RPC_ANYSOCK;
		if (client) {
			clnt_destroy(client);
			client = NULL;
		}
		if ((hp = gethostbyname(host)) == NULL)
			return ((int) RPC_UNKNOWNHOST);
		timeout.tv_usec = 0;
		timeout.tv_sec = 6;
		bcopy(hp->h_addr, &server_addr.sin_addr, hp->h_length);
		server_addr.sin_family = AF_INET;
		/* ping the remote end via tcp to see if it is up */
		server_addr.sin_port =  htons(PMAPPORT);
		if ((client = clnttcp_create(&server_addr, PMAPPROG,
		    PMAPVERS, &socket, 0, 0)) == NULL) {
			return ((int) rpc_createerr.cf_stat);
		} else {
			/* the fact we succeeded means the machine is up */
			close(socket);
			socket = RPC_ANYSOCK;
			clnt_destroy(client);
			client = NULL;
		}
		/* now really create a udp client handle */
		server_addr.sin_port =  0;
		if ((client = clntudp_create(&server_addr, prognum,
		    versnum, timeout, &socket)) == NULL)
			return ((int) rpc_createerr.cf_stat);
		client->cl_auth = authunix_create_default();
		valid = 1;
		oldprognum = prognum;
		oldversnum = versnum;
		strcpy(oldhost, host);
	}
	tottimeout.tv_sec = 25;
	tottimeout.tv_usec = 0;
	clnt_stat = clnt_call(client, procnum, inproc, in,
	    outproc, out, tottimeout);
	/* 
	 * if call failed, empty cache
	 */
	if (clnt_stat != RPC_SUCCESS)
		valid = 0;
	return ((int) clnt_stat);
}

putwarning(string)
     char *string;
{
  static warningmaxsize = 0;
  
  if (warningmaxsize == 0){
    warningstring = (char*)malloc(10);
    warningstring[0] = '\0';
    warningmaxsize = 10;
  }

  while (strlen(warningstring) + strlen(string) + 1 > warningmaxsize){
    warningstring = (char*)realloc(warningstring, (warningmaxsize * 3)/2);
    warningmaxsize = (warningmaxsize * 3) / 2;
  }

  sprintf(&warningstring[strlen(warningstring)], "%s", string);
}

usage()
{
  fprintf(stderr, "Usage: quota [-v] [ user | uid ] ...\n");
  fprintf(stderr, "       quota -g [-v] [ group | gid ] ...\n");
}
