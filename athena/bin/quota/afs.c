/*
 * AFS quota routines
 *
 * $Id: afs.c,v 1.12 1993-02-05 19:09:28 probe Exp $
 */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <afs/param.h>
#include <afs/venus.h>
#include <afs/afsint.h>

#ifdef _IBMR2
#include <sys/id.h>
#endif

extern int uflag,gflag,fsind;
extern char *fslist[];
extern int heading_printed;

#define user_and_groups (!uflag && !gflag)

void *
getafsquota(path, explicit)
    char *path;
    int explicit;
{
    static struct VolumeStatus vs;
    struct ViceIoctl ibuf;
    int code;

#ifdef _IBMR2
    setuidx(ID_EFFECTIVE, getuidx(ID_REAL));
#else
    setreuid(geteuid(), getuid());
#endif

    ibuf.out_size=sizeof(struct VolumeStatus);
    ibuf.in_size=0;
    ibuf.out=(caddr_t) &vs;
    code = pioctl(path,VIOCGETVOLSTAT,&ibuf,1);
    if (code) {
	if (explicit || ((errno != EACCES) && (errno != EPERM))) {
	    fprintf(stderr, "Error getting AFS quota: ");
	    perror(path);
	}
    }

#ifdef _IBMR2
    setuidx(ID_EFFECTIVE, getuidx(ID_SAVED));
#else
    setreuid(geteuid(), getuid());
#endif

    return(code ? (void *)0 : (void *)&vs);
}

void
prafsquota(path,foo,heading_id,heading_name)
    char *path;
    void *foo;
    int heading_id;
    char *heading_name;
{
    struct VolumeStatus *vs;
    char *cp;

    vs = (struct VolumeStatus *) foo;
    cp = path;
    if (!user_and_groups) {
	if (!heading_printed) simpleheading(heading_id,heading_name);
	if (strlen(path) > 15){
	    printf("%s\n",cp);
	    cp = "";
	}
	printf("%-14s %5d%7d%7d%12s\n",
	       cp,
	       vs->BlocksInUse,
	       vs->MaxQuota,
	       vs->MaxQuota,
	       (vs->MaxQuota && (vs->BlocksInUse >= vs->MaxQuota)
		? "Expired" : ""));
    } else {
	if (!heading_printed) heading(heading_id,heading_name);
	if (strlen(cp) > 15){
	    printf("%s\n", cp);
	    cp =  "";
	}
	printf("%-15s %-17s %6d %6d %6d%-2s\n",
	       cp, "volume",
	       vs->BlocksInUse,
	       vs->MaxQuota,
	       vs->MaxQuota,
	       (vs->MaxQuota && (vs->BlocksInUse >= vs->MaxQuota * 9 / 10)
		? "<<" : ""));
    }
}

void
afswarn(path,foo,name)
    char *path;
    void *foo;
    char *name;
{
    struct VolumeStatus *vs;
    char buf[1024];
    int i;
    uid_t uid=getuid();

    vs = (struct VolumeStatus *) foo;

    if (vs->MaxQuota && (vs->BlocksInUse > vs->MaxQuota)) {
	sprintf(buf,"Over disk quota on %s, remove %dK.\n",
		path, (vs->BlocksInUse - vs->MaxQuota));
	putwarning(buf);
    }
    else if (vs->MaxQuota && (vs->BlocksInUse >= vs->MaxQuota * 9 / 10)) {
	sprintf(buf,"%d%% of the disk quota on %s has been used.\n",
		(int)((vs->BlocksInUse*100.0/vs->MaxQuota)+0.5), path);
	putwarning(buf);
    }
}
