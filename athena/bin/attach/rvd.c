/*	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/attach/rvd.c,v $
 *	$Author: jfc $
 *
 *	Copyright (c) 1988 by the Massachusetts Institute of Technology.
 */

#ifndef lint
static char rcsid_rvd_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/attach/rvd.c,v 1.2 1990-04-19 12:15:30 jfc Exp $";
#endif lint

#include "attach.h"

#ifdef RVD			/* Don't do anything if RVD not defined */
#include "rvdlib.h"
#include <sys/wait.h>

extern char	*rvd_error();

/*
 * Attach an RVD
 */

rvd_attach(at, mopt, errorout)
	struct _attachtab *at;
	struct mntopts	*mopt;
	int errorout;
{
    char buf[BUFSIZ];
    union wait waitb;
    int vddrive, pid;
    char *passwd;
    
    if (avail_drive(at->hostaddr, at->hostdir, &vddrive) == FAILURE) {
	if (errorout)
	    fprintf(stderr, "%s: No free RVD drives\n", at->hesiodname);
	return (FAILURE);
    }

    if (debug_flag)
	printf("avail_drive returned %d\n", vddrive);

    at->drivenum = vddrive;
    
    passwd = NULL;
    
    while (rvd_spinup(at->hostaddr, at->hostdir,
		      vddrive, at->mode, at->host, passwd) == FAILURE &&
	   rvderrno == RVDEBPWD) {
	fprintf(stderr, "%s: Need password for RVD spinup\n",
		at->hesiodname);
	if (!isatty(fileno(stdin))) {
	    error_status = ERR_ATTACHNEEDPW;
	    return (FAILURE);
	}
	passwd = (char *)getpass("Password:");
	if (!*passwd) {
	    fprintf(stderr, "%s: Null password, ignoring filesystem\n",
		    at->hesiodname);
	    return (FAILURE);
	}
    }

    if (rvderrno && rvderrno != RVDEIDA &&
	rvderrno != EBUSY /* kernel bug */) {
	if (errorout)
	    fprintf(stderr, "%s: Error in RVD spinup: %s\n",
		    at->hesiodname, rvd_error(rvderrno));
	return (FAILURE);
    }

    if((pid = vfork()) == 0) {
	execl(RVDGETM_FULLNAME, RVDGETM_SHORTNAME, at->host, 0);
	_exit(0);
    } else if(pid == -1) {
	perror("vfork");
    } else {
	(void) wait(&waitb);
    }
    
    if (at->mode == 'w' && rvderrno != RVDEIDA &&
	rvderrno != EBUSY && !skip_fsck) {
	    if (perform_fsck(vdnam(buf, vddrive),
			     at->hesiodname, 1) == FAILURE) {
		    rvd_spindown(vddrive);
		    return(FAILURE);
	    }
    }
	    
    /* XXX This is kind of bogus, because if a filesystem has a number
     * of hesiod entries, and the mount point is busy, each one will
     * be tried until the last one fails, then an error printed.
     * C'est la vie.
     */

    if (mountfs(at, vdnam(buf, vddrive), mopt, errorout) == FAILURE) {
	rvd_spindown(vddrive);
	return (FAILURE);
    }

    return (SUCCESS);
}

/*
 * Detach a RVD
 */
rvd_detach(at)
    struct _attachtab *at;
{
	if (at->flags & FLAG_PERMANENT) {
		if (debug_flag)
			printf("Permanent flag on, skipping umount.\n");
		return(SUCCESS);
	}
	
	if (unmount_42(at->hesiodname, at->mntpt) == FAILURE)
		return (FAILURE);
	rvd_spindown(at->drivenum);
	return (SUCCESS);
}

/*
 * Parsing of explicit RVD file types
 */
char **rvd_explicit(name)
    char *name;
{
    char temp[BUFSIZ];
    char *pack;
    char newmntpt[BUFSIZ];
    extern char *exp_hesptr[2];
	
    strcpy(temp, name);
    pack = index(temp, ':');
    if (!pack) {
	fprintf(stderr, "%s: Illegal explicit definition for type %s\n",
		name, filsys_type);
	return (0);
    }
    *pack = '\0';
    pack++;
    if (!mntpt) {
	sprintf(newmntpt, "/%s/%s", temp, pack);
    }
    sprintf(exp_hesline, "RVD %s %s %c %s", pack, temp, override_mode ?
	    override_mode : 'w', mntpt ? mntpt : newmntpt);
    exp_hesptr[0] = exp_hesline;
    exp_hesptr[1] = 0;
    return(exp_hesptr);
}

#endif /* RVD */
