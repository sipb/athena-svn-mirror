/*
 *   Disk quota reporting program.
 *
 *   $Id: quota.c,v 1.17 1992-04-10 20:27:39 probe Exp $
 */

#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/time.h>
#include <pwd.h>
#include <grp.h>

#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <rpcsvc/rquota.h>
#include <rpcsvc/rcquota.h>

#ifdef _IBMR2
#include <sys/select.h>
#include <sys/mntctl.h>
#include <sys/vmount.h>
#define dbtob(db) ((unsigned)(db) << UBSHIFT)

#else /* !_IBMR2 */

#if defined(ultrix) || defined(_I386)
#include <sys/stat.h>
#include <sys/quota.h>
#else
#include <ufs/quota.h>
#endif

#ifdef ultrix
#include <sys/mount.h>
#include <fstab.h>
#include <sys/fs_types.h>
#define mntent fs_data
struct fs_data mountbuffer[NMOUNT];
#else
#include <mntent.h>
#endif

#endif /* !_IBMR2 */

#ifdef ultrix
#undef mntent
#endif
#include "attach.h"
#ifdef ultrix
#define mntent fs_data
#endif

#if defined(vax) || defined(ibm032)
#include <qoent.h>
#endif


static char *warningstring = NULL;

int	vflag=0, uflag=0, gflag=0, aflag=0;
#if defined(ultrix) || defined(_I386)
int	qflag=0, done=0;
#endif

#define user_and_groups (!uflag && !gflag)
#define QFNAME	"quotas"

#define kb(n)   (howmany(dbtob(n), 1024))

#define MAXFS 16
#define MAXID 32
/* List of id's and filesystems to check */
char *fslist[MAXFS], *idlist[MAXID];
int fsind = 0, idind = 0, heading_printed;

main(argc, argv)
     char *argv[];
{
    register char *cp;
    register int i;

    argc--,argv++;
    while (argc > 0) {
	if (argv[0][0] == '-')
	    for (cp = &argv[0][1]; *cp; cp++) switch (*cp) {

	    case 'a':
		aflag=1;
		break;

	    case 'v':
		vflag=1;
		break;

#ifdef ultrix
	    case 'q':
		qflag=1;
		break;
#endif

	    case 'g':
		gflag=1;
		break;

	    case 'u':
		uflag=1;
		break;

	    case 'f':
		if (fsind < MAXFS)
		    {
			if (argc == 1){
			    fprintf(stderr, "quota: No filesystem argument\n");
			    usage();
			    exit(1);
			}
			fslist[fsind++] = argv[1];
			argv++;
			argc--;
			break;
		    }
		else{
		    fprintf(stderr, "quota: too many filesystems\n");
		    exit(1);
		}

	    default:
		fprintf(stderr, "quota: %c: unknown option\n",
			*cp);
		usage();
		exit(1);
	    }
	else if (idind < MAXID)
	    {
		idlist[idind++] = argv[0];
	    }
	else{
	    fprintf(stderr, "quota: too many id's\n");
	    exit(1);
	}
	argv++;
	argc--;
    }
  
    if (gflag && uflag) {
	fprintf(stderr, "quota: Can't use both -g and -u\n");
	usage();
	exit(1);
    }

    if (aflag && fsind) {
	fprintf(stderr, "quota: Can't use both -a and -f\n");
	usage();
	exit(1);
    }

    if (uflag && idind == 0){
	idlist[0] = (char*)malloc(20);
	sprintf(idlist[0],"%d",getuid());
	idind = 1;
    }

    if (gflag  && (idind == 0)){
	fprintf(stderr, "quota: No groups specified\n");
	usage();
	exit(1);
    }

    lock_attachtab();
    get_attachtab();
    unlock_attachtab();

    if (fsind) verify_filesystems();

    if (idind == 0) showid(getuid());
    else
	for(i=0;i<idind;i++) {
	    heading_printed = 0;
	    if ((!gflag && getpwnam(idlist[i])) ||
		(gflag && getgrnam(idlist[i])))
		showname(idlist[i]);
	    else if (alldigits(idlist[i])) showid(atoi(idlist[i]));
	    else showname(idlist[i]);
	}

    exit(0);
}

showid(id)
     int id;
{
    register struct passwd *pwd = getpwuid(id);
    register struct group  *grp = getgrgid(id);

    if (id == 0){
	if (vflag) printf("no disk quota for %s 0\n",
			  (gflag ? "gid" : "uid"));
	return;
    }

    if (gflag) {
	if (grp == NULL) showquotas(id, "(no group)");
	else showquotas(id, grp->gr_name);
    } else { 
	if (pwd == NULL) showquotas(id, "(no account)");
	else showquotas(id, pwd->pw_name);
    }
    return;
}

showname(name)
     char *name;
{
    register struct passwd *pwd = getpwnam(name);
    register struct group  *grp = getgrnam(name);

    if (gflag){
	if (grp == NULL){
	    fprintf(stderr, "quota: %s: unknown group\n", name);
	    exit(7);
	    return;
	}
	if (grp->gr_gid == 0){
	    if (vflag) printf("no disk quota for %s (gid 0)\n", name);
	    return;
	}
	showquotas(grp->gr_gid,name);
    }
    else{
	if (pwd == NULL){
	    fprintf(stderr, "quota: %s: unknown user\n", name);
	    exit(8);
	}
	if (pwd->pw_uid == 0){
	    if (vflag)
		printf("no disk quota for %s (uid 0)\n", name);
	    return;
	}
	showquotas(pwd->pw_uid,name);
    }
}

/* Different enough that it's a mess to ifdef it all individually */
showquotas(id,name)
     int id;
     char *name;
{
    int myuid, ngroups, gidset[NGROUPS];
    struct getcquota_rslt qvalues;
    struct _attachtab *p;
    void *v;				/* Pointer to AFS VolumeStatus */
    extern void *getafsquota();

#ifdef _IBMR2
    int mntsize;
    char *mntbuf;
    int nmount;
    struct vmount *vmt;
    int j;
#else
    
    register struct mntent *mntp;
    FILE *mtab;
#ifdef ultrix
#define mnt_dir fd_path
#define mnt_fsname fd_devname
#define Q_GETQUOTA Q_GETDLIM
    int ultlocalquotas=0;
    int loc=0, ret;
#endif
#ifdef _I386
#define MNTTYPE_42 MNTTYPE_UFS
    int ultlocalquotas=0;
#endif

#endif /* !_IBMR2 */
    
    myuid = getuid();

    if (gflag) {			/* Must be in group or super-user */
	if ((ngroups = getgroups(NGROUPS, gidset)) == -1){
	    perror("quota: Couldn't get list of groups.");
	    exit(9);
	}
	while(ngroups){ if (id == gidset[ngroups-1]) break; --ngroups;}
	if (!ngroups && myuid != 0){
	    printf("quota: %s (gid %d): permission denied\n", name, id);
	    return;
	}
    }
    else{
	if (id != myuid && myuid != 0){
	    printf("quota: %s (uid %d): permission denied\n", name, id);
	    return;
	}
    }

#ifdef _IBMR2
    mntsize = 4096;
    if ((mntbuf = (char *)malloc(mntsize)) == NULL) {
	perror("quota: error allocating memory");
	exit(9);
    }
    nmount = mntctl(MCTL_QUERY,mntsize,mntbuf);
    if (nmount == -1) {
	perror("quota: mntctl failed");
	exit(9);
    }
    if (nmount == 0) {
	mntsize = *((int *) mntbuf);
	if ((mntbuf = (char *)realloc(mntbuf, mntsize)) == NULL) {
	    perror("quota: error reallocing memory");
	    exit(9);
	}
	nmount = mntctl(MCTL_QUERY,mntsize,mntbuf);
	if (nmount == -1) {
	    perror("quota: mntctl failed");
	    exit(9);
	}
    }

    vmt = (struct vmount *)mntbuf;
    for(j=0; j<nmount; j++,vmt=(struct vmount *)((char *)vmt+vmt->vmt_length))
    {
	char *dirp;

	dirp = ((char *)vmt+vmt->vmt_data[VMT_STUB].vmt_off);
	if (fsind) {
	    int i, l;
	    for(i=0;i<fsind;i++){
		l = strlen(fslist[i]);
		if(!strncmp(((char *)vmt + vmt->vmt_data[VMT_STUB].vmt_off),
			    fslist[i], l))
		    break;
	    }
	    if (i == fsind) continue;	/* Punt filesystems not in fslist */
	}

	if (!fsind && !aflag && !own(gflag?myuid:id, dirp))
	    continue;
	
	if (vmt->vmt_gfstype == MNT_NFS) {
	    if (!getnfsquota(((char *)vmt+vmt->vmt_data[VMT_HOSTNAME].vmt_off),
			     ((char *)vmt+vmt->vmt_data[VMT_OBJECT].vmt_off),
			     id, &qvalues))
		continue;
	}
	else
	    continue;
	if (vflag) prquota(dirp, &qvalues, id, name);      
	if (user_and_groups || !vflag) warn(dirp, &qvalues);
    }

#else /* !_IBMR2  */

#ifndef ultrix
    mtab = setmntent(MOUNTED, "r");
#else
    ret = getmountent(&loc, mountbuffer, NMOUNT);
    if (ret == 0) {
	perror("getmountent");
	exit(3);
    }
#endif

#ifndef ultrix
    while(mntp = getmntent(mtab))
#else
    for(mntp = mountbuffer; mntp < &mountbuffer[ret]; mntp++)
#endif
    {
	if (fsind)
	{
	    int i, l;

	    for(i=0;i<fsind;i++){
		l = strlen(fslist[i]);
		if(!strncmp(mntp->mnt_dir, fslist[i], l))
		    break;
	    }
	    if (i == fsind) continue;		/* Punt filesystems not
						 * in the fslist */
	}

#ifndef ultrix
	if (strcmp(mntp->mnt_type, MNTTYPE_42) == 0 &&
	    hasmntopt(mntp, MNTOPT_QUOTA)){
#ifdef _I386
	    ultlocalquotas++;
#else
	    if (getlocalquota(mntp,id,&qvalues)) continue;
	}
#endif						/* _I386 */
#else						/* ultrix */
	if (mntp->fd_fstype == GT_ULTRIX &&
	    (mntp->fd_flags & M_QUOTA)) {
	    ultlocalquotas++;
	    continue;
	}
#endif						/* ultrix */

	else if (!fsind && !aflag && !own(gflag?myuid:id, mntp->mnt_dir))
	    continue;

#ifndef ultrix
	else if (!strcmp(mntp->mnt_type, MNTTYPE_NFS))
#else
	else if (mntp->fd_fstype == GT_NFS)
#endif
	{
	    char host[128];
	    char path[128];
	    int i;

	    i = (char *)index(mntp->mnt_fsname, ':') - mntp->mnt_fsname;
	    bcopy(mntp->mnt_fsname, host, i);
	    host[i] = 0;
	    strcpy(path, mntp->mnt_fsname+i+1);
	    if (!getnfsquota(host, path, id, &qvalues))
		continue;
	}
	else continue;

	if (vflag) prquota(mntp->mnt_dir, &qvalues, id, name);      
	if (user_and_groups || !vflag) warn(mntp->mnt_dir, &qvalues);
    }
#ifndef ultrix
    endmntent(mtab);
#endif
#if defined(ultrix) || defined(_I386)
    if (ultlocalquotas) 
	ultprintquotas(id,name);
#endif
#endif /* !_IBMR2 */
  
    /* Check afs volumes */
    for(p = attachtab_first; p!=NULL; p=p->next) {
	/* Only look at AFS mountpoints; others already done */
	if ((p->fs->type != TYPE_AFS) || (p->status != STATUS_ATTACHED))
	    continue;
	if (fsind) {
	    int i, l;

	    for(i=0;i<fsind;i++){
		l = strlen(fslist[i]);
		if(!strncmp(p->mntpt, fslist[i], l))
		    break;
	    }
	    if (i == fsind) continue;	/* Punt filesystems not in fslist */
	}

	if (!fsind && !aflag && !own(gflag?myuid:id, p->mntpt))
	    continue;
    
	if ((v = getafsquota(p->mntpt, fsind)) == NULL)
	    continue;
	if (vflag) prafsquota(p->mntpt, v, id, name);
	if (user_and_groups || !vflag) afswarn(p->mntpt,v,name);
    }

    if (warningstring) {
	printf("\n%s\n", warningstring);
	free(warningstring);
	warningstring = NULL;
    }
}

int own(id, dir)
    int id;
    char *dir;
{
    struct _attachtab *atp;
    int i;

    atp = attachtab_lookup_mntpt(dir);
    if (!atp)
	return 0;

    for(i=0; i<atp->nowners; i++)
	if (atp->owners[i] == id) {
	    return (access(dir,W_OK) == 0);
	}

    return 0;
}

#ifdef QOTAB
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

    if (!qoent) {	/* this partition has no quota options, use defaults */
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

    /* If this filesystem is group controlled and user quota is being
     * requested, or this is a user controlled filesystem and group
     * quotas are requested, then punt */
    if ((qvp->rq_group && uflag) || 
	(!qvp->rq_group && gflag))
	return(-1);

    if (uflag || gflag || !qvp->rq_group){
	if (quotactl(Q_GETQUOTA, mntp->mnt_fsname, uid, &dqblk)){
	    /* ouch! quotas are on, but this failed */
	    fprintf(stderr, "quotactl: %s %d\n", mntp->mnt_fsname, uid);
	    return(-1);
	}

	qvp->rq_ngrps = 1;
	dqblk2rcquota(&dqblk,&(qvp->gqr_rcquota[0]), uid);
	return(0);
    }
    else{
	int groups[NGROUPS], i;
    
	qvp->rq_ngrps = getgroups(NGROUPS,groups);
	for(i=0;i<qvp->rq_ngrps;i++){
	    if (quotactl(Q_GETQUOTA, mntp->mnt_fsname, groups[i], &dqblk)){
		/* ouch again! */
		fprintf(stderr, "quotactl: %s %d\n", mntp->mnt_fsname, groups[i]);
		return(-1);
	    }
	    dqblk2rcquota(&dqblk, &(qvp->gqr_rcquota[i]), groups[i]);
	}
	return(0);
    }
}
#endif

int
simpleheading(id,name)
     int id;
     char *name;
{
    printf("Disk quotas for %s %s (%s %d):\n",
	   (gflag? "group":"user"), name,
	   (gflag? "gid": "uid"), id);
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
    heading_printed = 1;
}

heading(id,name)
     int id;
     char *name;
{
    printf("Disk quotas for %s (uid %d):\n",name,id);
    printf("%-16s%-6s%-12s%6s%7s%7s  %7s%7s%7s\n"
	   , "Filesystem"
	   , "Type"
	   , "ID"
	   , "usage", "quota", "limit"
	   , "files", "quota", "limit"
	   );
    heading_printed = 1;
}

prquota(dir, qvp, heading_id, heading_name)
    char *dir;
    register struct getcquota_rslt *qvp;
    int heading_id;
    char *heading_name;
{
    struct timeval tv;
    char ftimeleft[80], btimeleft[80], idbuf[20];
    char *cp, *id_name = "", *id_type;
    int i;
    struct rcquota *rqp;

    id_type = (qvp->rq_group? "group" : "user");


    gettimeofday(&tv, NULL);

    for(i=0; i<qvp->rq_ngrps; i++){

	/* If this is root or wheel, then skip */
	if (qvp->gqr_rcquota[i].rq_id == 0) continue;

	rqp = &(qvp->gqr_rcquota[i]);

	/* We're not interested in this group if all is zero */
	if (!rqp->rq_bsoftlimit && !rqp->rq_bhardlimit
	    && !rqp->rq_curblocks && !rqp->rq_fsoftlimit
	    && !rqp->rq_fhardlimit && !rqp->rq_curfiles) continue;

	if (user_and_groups){
	    if (qvp->rq_group){
		getgroupname(qvp->gqr_rcquota[i].rq_id, idbuf);
		id_name = idbuf;
	    }
	    else{				/* this is a user quota */
		getusername(qvp->gqr_rcquota[i].rq_id, idbuf);
		id_name = idbuf;
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

	cp = dir;

	if (!user_and_groups){
	    if (!heading_printed) simpleheading(heading_id,heading_name);
	    if (strlen(cp) > 15){
		printf("%s\n",cp);
		cp = "";
	    }
	    printf("%-14s %5d%7d%7d%12s%7d%7d%7d%12s\n",
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
	    if (!heading_printed) heading(heading_id,heading_name);
	    if (strlen(cp) > 16){
		printf("%s\n", cp);
		cp =  "";
	    }
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
  
warn(dir, qvp)
    char *dir;
    register struct getcquota_rslt *qvp;
{
    struct timeval tv;
    int i;
    char buf[1024], idbuf[20];
    char *id_name, *id_type;
    struct rcquota *rqp;

    id_type = (qvp->rq_group? "Group" : "User");

    gettimeofday(&tv, NULL);

    for(i=0; i<qvp->rq_ngrps; i++){

	/* If this is root or wheel, then skip */
	if (qvp->gqr_rcquota[i].rq_id == 0) continue;

	if (qvp->rq_group){
	    getgroupname(qvp->gqr_rcquota[i].rq_id, idbuf);
	    id_name = idbuf;
	}
	else{
	    getusername(qvp->gqr_rcquota[i].rq_id, idbuf);
	    id_name = idbuf;
	}

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
		    id_type, id_name, dir);
	    putwarning(buf);
	}

	else if (rqp->rq_bsoftlimit &&
		 rqp->rq_curblocks >= rqp->rq_bsoftlimit){
	    if (rqp->rq_btimeleft == 0) {
		sprintf(buf,
			"%s %s over disk quota on %s, remove %dK\n",
			id_type, id_name, dir,
			kb(rqp->rq_curblocks - rqp->rq_bsoftlimit + 1));
		putwarning(buf);
	    }
	    else if (rqp->rq_btimeleft > tv.tv_sec) {
		char btimeleft[80];

		fmttime(btimeleft, rqp->rq_btimeleft - tv.tv_sec);
		sprintf(buf,
			"%s %s over disk quota on %s, remove %dK within %s\n",
			id_type, id_name, dir,
			kb(rqp->rq_curblocks - rqp->rq_bsoftlimit + 1),
			btimeleft);
		putwarning(buf);
	    }
	    else {
		sprintf(buf,
			"%s %s over disk quota on %s, time limit has expired, remove %dK\n",
			id_type, id_name, dir,
			kb(rqp->rq_curblocks - rqp->rq_bsoftlimit + 1));
		putwarning(buf);
	    }
	}

	if (rqp->rq_fhardlimit &&
	    rqp->rq_curfiles >= rqp->rq_fhardlimit){
	    sprintf(buf,
		    "File count limit reached for %s %s on %s\n",
		    id_type, id_name, dir);
	    putwarning(buf);
	}

	else if (rqp->rq_fsoftlimit &&
		 rqp->rq_curfiles >= rqp->rq_fsoftlimit) {
	    if (rqp->rq_ftimeleft == 0) {
		sprintf(buf,
			"%s %s over file quota on %s, remove %d file%s\n",
			id_type, id_name, dir,
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
			id_type, id_name, dir,
			rqp->rq_curfiles - rqp->rq_fsoftlimit + 1,
			((rqp->rq_curfiles - rqp->rq_fsoftlimit + 1) > 1 ?
			 "s" : "" ), ftimeleft);
		putwarning(buf);
	    }
	    else {
		sprintf(buf,
			"%s %s over file quota on %s, time limit has expired, remove %d file%s\n",
			id_type, id_name, dir,
			rqp->rq_curfiles - rqp->rq_fsoftlimit + 1,
			((rqp->rq_curfiles - rqp->rq_fsoftlimit + 1) > 1 ?
			 "s" : "" ));
		putwarning(buf);
	    }
	}
    }
}

usage()
{
    fprintf(stderr,
	    "Usage: quota [-v] [user] [-g group] [-u user] [-f filesystem]\n");
}

alldigits(s)
	register char *s;
{
	register int c;

	c = *s++;
	do {
		if (!isdigit(c))
			return (0);
	} while (c = *s++);
	return (1);
}

#if !defined(ultrix) && !defined(_I386) && !defined(_IBMR2)
dqblk2rcquota(dqblkp, rcquotap, uid)
     struct dqblk *dqblkp;
     struct rcquota *rcquotap;
     int uid;
{
    rcquotap->rq_id = uid;
    rcquotap->rq_bhardlimit = dqblkp->dqb_bhardlimit;
    rcquotap->rq_bsoftlimit = dqblkp->dqb_bsoftlimit;
    rcquotap->rq_curblocks  = dqblkp->dqb_curblocks;
    rcquotap->rq_fhardlimit = dqblkp->dqb_fhardlimit;
    rcquotap->rq_fsoftlimit = dqblkp->dqb_fsoftlimit;
    rcquotap->rq_curfiles   = dqblkp->dqb_curfiles;
    rcquotap->rq_btimeleft  = dqblkp->dqb_btimelimit;
    rcquotap->rq_ftimeleft  = dqblkp->dqb_ftimelimit;
}
#endif
    
getgroupname(id,buffer)
     int id;
     char *buffer;
{
    if (getgrgid(id))
	strcpy(buffer, (getgrgid(id))->gr_name);
    else{
	sprintf(buffer, "G%d", id);
    }
}

getusername(id,buffer)
     int id;
     char *buffer;
{
    if (getpwuid(id))
	strcpy(buffer, (getpwuid(id))->pw_name);
    else{
	sprintf(buffer, "#%d", id);
    }
}

putwarning(string)
     char *string;
{
    static int warningmaxsize = 0;
  
    if (warningstring == 0){
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

fmttime(buf, time)
	char *buf;
	register long time;
{
    int i;
    static struct {
	int c_secs;				/* conversion units in secs */
	char * c_str;				/* unit string */
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

#ifdef _IBMR2
verify_filesystems()
{
    int i, j, found, l;
    int mntsize;
    char *mntbuf;
    int nmount;
    struct vmount *vmt;
    struct _attachtab *atp;

    mntsize = 4096;
    if ((mntbuf = (char *)malloc(mntsize)) == NULL) {
	perror("quota: error allocating memory");
	exit(9);
    }
    nmount = mntctl(MCTL_QUERY,mntsize,mntbuf);
    if (nmount == -1) {
	perror("quota: mntctl failed");
	exit(9);
    }
    if (nmount == 0) {
	mntsize = *((int *) mntbuf);
	if ((mntbuf = (char *)realloc(mntbuf, mntsize)) == NULL) {
	    perror("quota: error reallocing memory");
	    exit(9);
	}
	nmount = mntctl(MCTL_QUERY,mntsize,mntbuf);
	if (nmount == -1) {
	    perror("quota: mntctl failed");
	    exit(9);
	}
    }

    for(i=0;i<fsind;i++){
	l = strlen(fslist[i]);
	found = 0;
	vmt = (struct vmount *)mntbuf;
	for(j=0;j<nmount;j++) {
	    if (!strncmp(fslist[i],
			 (char *)vmt+vmt->vmt_data[VMT_STUB].vmt_off,l)) { 
		found = 1;
		break;
	    }
	    vmt = (struct vmount *)((char *)vmt + vmt->vmt_length);
	}
	if (!found && (atp = attachtab_lookup(fslist[i]))) {
	    fslist[i] = atp->mntpt;
	    found = 1;
	    break;
	}
	if (!found && (attachtab_lookup_mntpt(fslist[i]) == NULL)) {
	    fprintf(stderr, "quota: '%s' matches no mounted filesystems.\n",
		    fslist[i]);
	    exit(10);
	}
    }
}

#else /* _IBMR2 */

verify_filesystems()
{
    struct _attachtab *atp;
    struct mntent *mntp;
    FILE *mtab;
    int i,l, found;
#ifdef ultrix
    int loc=0, ret;
#endif
  
    for(i=0;i<fsind;i++){
	l = strlen(fslist[i]);
	found = 0;
#ifndef ultrix
	mtab = setmntent(MOUNTED, "r");
#else
	ret = getmountent(&loc, mountbuffer, NMOUNT);
	if (ret == 0) {
	    perror("getmountent");
	    exit(3);
	}
#endif
	
#ifndef ultrix
	while(mntp = getmntent(mtab))
#else
	for(mntp = mountbuffer; mntp < &mountbuffer[ret]; mntp++)
#endif
	{
	    if (!strncmp(fslist[i], mntp->mnt_dir,l)){
		found = 1;
		break;
	    }
	}
#ifndef ultrix
	endmntent(mtab);
#endif
	if (!found && (atp = attachtab_lookup(fslist[i]))) {
	    fslist[i] = atp->mntpt;
	    found = 1;
	    break;
	}
	if (!found && (attachtab_lookup_mntpt(fslist[i]) == NULL)) {
	    fprintf(stderr, "quota: '%s' matches no mounted filesystems.\n",
		    fslist[i]);
	    exit(10);
	}
    }
}
#endif /* IBMR2 */

#if defined(_I386) || defined(ultrix)
ultprintquotas(uid, name)
	int uid;
	char *name;
{
	register char c, *p;
	register struct fstab *fs;
	int myuid;
#ifdef _I386
	FILE	*fptr;
	struct	mntent 	*buf;
#endif

	myuid = getuid();
	if (uid != myuid && myuid != 0) {
		printf("quota: %s (uid %d): permission denied\n", name, uid);
		return;
	}
	done = 0;
#ifdef ultrix
	setfsent();
	while (fs = getfsent()) {
#else
	/* Deal with FSTAB */
	fptr = setmntent("/etc/mtab","r");
	while((buf = getmntent(fptr)) != NULL) {
		dev_t 		qf_gfs;
#endif
		register char *msgi = (char *)0, *msgb = (char *)0;
		register int enab = 1;
		dev_t	fsdev;
		struct	stat statb;
		struct	dqblk dqblk;
		char qfilename[MAXPATHLEN + 1], iwarn[8], dwarn[8];
		char		*vol;

#ifdef _I386
		if (hasmntopt(buf, MNTOPT_QUOTA) == NULL)
		    continue;
		vol = buf->mnt_dir;
		(void) sprintf(qfilename, "%s/%s", buf->mnt_dir, QFNAME);
		if (stat(qfilename, &statb) < 0) 
		         continue; 
		qf_gfs = buf->mnt_gfs;
		if (quota(Q_SYNC, 0, (gfs_t) qf_gfs, (caddr_t) 0) < 0 &&				 errno == EINVAL)
			continue;
		sync();
		if (quota(Q_GETDLIM, uid, qf_gfs, (caddr_t) &dqblk) != 0)
#else
		if (stat(fs->fs_spec, &statb) < 0)
			continue;
		fsdev = statb.st_rdev;
		vol = fs->fs_file;
		(void) sprintf(qfilename, "%s/%s", fs->fs_file, QFNAME);
		if (stat(qfilename, &statb) < 0 || statb.st_dev != fsdev)
			continue;
		if (quota(Q_GETDLIM, uid, fsdev, &dqblk) != 0)
#endif
		{
		    register fd = open(qfilename, O_RDONLY);

		    if (fd < 0)
			continue;
		    lseek(fd, (long)(uid * sizeof (dqblk)), L_SET);
		    if (read(fd, &dqblk, sizeof dqblk) != sizeof (dqblk)) {
			close(fd);
			continue;
		    }
		    close(fd);
		    if (dqblk.dqb_isoftlimit == 0 &&
			dqblk.dqb_bsoftlimit == 0)
			continue;
		    enab = 0;
		}
		if (dqblk.dqb_ihardlimit &&
		    dqblk.dqb_curinodes >= dqblk.dqb_ihardlimit)
			msgi = "File count limit reached on %s";
		else if (enab && dqblk.dqb_iwarn == 0)
			msgi = "Out of inode warnings on %s";
		else if (dqblk.dqb_isoftlimit &&
		    dqblk.dqb_curinodes >= dqblk.dqb_isoftlimit)
			msgi = "Too many files on %s";
		if (dqblk.dqb_bhardlimit &&
		    dqblk.dqb_curblocks >= dqblk.dqb_bhardlimit)
			msgb = "Block limit reached on %s";
		else if (enab && dqblk.dqb_bwarn == 0)
			msgb = "Out of block warnings on %s";
		else if (dqblk.dqb_bsoftlimit &&
		    dqblk.dqb_curblocks >= dqblk.dqb_bsoftlimit)
			msgb = "Over disc quota on %s";
		if (dqblk.dqb_iwarn < MAX_IQ_WARN)
			sprintf(iwarn, "%d", dqblk.dqb_iwarn);
		else
			iwarn[0] = '\0';
		if (dqblk.dqb_bwarn < MAX_DQ_WARN)
			sprintf(dwarn, "%d", dqblk.dqb_bwarn);
		else
			dwarn[0] = '\0';
		if (qflag) {
			if (msgi != (char *)0 || msgb != (char *)0)
				ultheading(uid, name);
			if (msgi != (char *)0)
				xprintf(msgi, vol);
			if (msgb != (char *)0)
				xprintf(msgb, vol);
			continue;
		}
		if (vflag || dqblk.dqb_curblocks || dqblk.dqb_curinodes) {
			ultheading(uid, name);
			printf("%10s%8d%c%7d%8d%8s%8d%c%7d%8d%8s\n"
				, vol
				, (dqblk.dqb_curblocks / btodb(1024)) 
				, (msgb == (char *)0) ? ' ' : '*'
				, (dqblk.dqb_bsoftlimit / btodb(1024)) 
				, (dqblk.dqb_bhardlimit / btodb(1024)) 
				, dwarn
				, dqblk.dqb_curinodes
				, (msgi == (char *)0) ? ' ' : '*'
				, dqblk.dqb_isoftlimit
				, dqblk.dqb_ihardlimit
				, iwarn
			);
		}
	}
#ifdef _I386
	endmntent(fptr);
#else
	endfsent();
#endif
	if (!done && !qflag) {
		if (idind)
			putchar('\n');
		xprintf("Disc quotas for %s (uid %d):", name, uid);
		xprintf("none.");
	}
	xprintf(0);
}

ultheading(uid, name)
	int uid;
	char *name;
{

    if (done++)
	return;
    xprintf(0);
    if (qflag) {
	if (!idind)
	    return;
	xprintf("User %s (uid %d):", name, uid);
	xprintf(0);
	return;
    }
    putchar('\n');
#if 0
    xprintf("Disc quotas for %s (uid %d):", name, uid);
#endif
    xprintf(0);
    printf("%10s%8s %7s%8s%8s%8s %7s%8s%8s\n"
	   , "Filsys"
	   , "current"
	   , "quota"
	   , "limit"
	   , "#warns"
	   , "files"
	   , "quota"
	   , "limit"
	   , "#warns"
	   );
}

xprintf(fmt, arg1, arg2, arg3, arg4, arg5, arg6)
    char *fmt;
{
    char	buf[100];
    static int column;

    if (fmt == 0 && column || column >= 40) {
	putchar('\n');
	column = 0;
    }
    if (fmt == 0)
	return;
    sprintf(buf, fmt, arg1, arg2, arg3, arg4, arg5, arg6);
    if (column != 0 && strlen(buf) < 39)
	while (column++ < 40)
	    putchar(' ');
    else if (column) {
	putchar('\n');
	column = 0;
    }
    printf("%s", buf);
    column += strlen(buf);
}
#endif /* _I386 || ultrix */
