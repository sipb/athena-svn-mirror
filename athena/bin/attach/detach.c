/*	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/attach/detach.c,v $
 *	$Author: jfc $
 *
 *	Copyright (c) 1988 by the Massachusetts Institute of Technology.
 */

#ifndef lint
static char rcsid_detach_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/attach/detach.c,v 1.2 1990-04-19 12:04:37 jfc Exp $";
#endif lint

#include "attach.h"

/*
 * Detach the filesystem called name.
 */

detach(name)
    char *name;
{
    struct _attachtab *atp, at;
    int status, bymntpt;
#ifdef ZEPHYR
    char instbfr[BUFSIZ];
#endif /* ZEPHYR */
	
    lock_attachtab();
    get_attachtab();
    bymntpt = 0;
    if (!(atp = attachtab_lookup(name))) {
	if (!(atp = attachtab_lookup_mntpt(name))) {
	    unlock_attachtab();
	    free_attachtab();
	    fprintf(stderr, "%s: Not attached\n", name);
	    error_status = ERR_DETACHNOTATTACHED;
	    return (FAILURE);
	} else
		bymntpt = 1;
    }

    switch (atp->status) {
    case STATUS_ATTACHING:
	if (!force && really_in_use(name)) {
	    fprintf(stderr, "%s: Being attached by another process\n",
		    name);
	    error_status = ERR_DETACHINUSE;
	    unlock_attachtab();
	    free_attachtab();
	    return (FAILURE);
	}
	/*
	 * Derelict entry...here might be a good place for attachtab/mtab
	 * reconciliation.
	 */
	break;
    case STATUS_DETACHING:
	if (!force && really_in_use(name)) {
	    fprintf(stderr, "%s: Already being detached by another process\n",
		    name);
	    error_status = ERR_DETACHINUSE;
	    unlock_attachtab();
	    free_attachtab();
	    return (FAILURE);
	}
	break;
    } 

    /* Note: attachtab still locked at this point */
    
    start_critical_code();
    if (clean_detach) {
	    if (clean_attachtab(atp)) {
		    put_attachtab();
		    unlock_attachtab();
		    free_attachtab();
		    end_critical_code();
		    return(SUCCESS);
	    }
	    /*
	     * If we fall through, it means we should detach the filesystem
	     */
    }
    if (atp->status == STATUS_ATTACHED) {
	    if (override) {	/* Override _all_ owners and detach */
		    atp->nowners = 0;
		    atp->flags &= ~FLAG_LOCKED;
		    status = SUCCESS;
	    } else if (owner_check && !clean_detach) {
		    if (del_an_owner(atp, owner_uid)) {
			    int ret = SUCCESS;
			    fprintf(stderr, "%s: Filesystem wanted by others, not unmounted.\n", name);
			    error_status = ERR_ATTACHINUSE;
			    put_attachtab();
			    if (atp->fs->type == TYPE_NFS && do_nfsid)
				    ret = nfsid(atp->host, atp->hostaddr,
						MOUNTPROC_KUIDUMAP, 1,
						atp->hesiodname, 0, real_uid);
			    unlock_attachtab();
			    free_attachtab();
			    end_critical_code();
			    return(ret);
		    }
		    status = SUCCESS;
	    }
	    if (!override && atp->flags & FLAG_LOCKED) {
		    error_status = ERR_DETACHNOTALLOWED;
		    fprintf(stderr, "%s: Filesystem locked, use -override to detach it.\n", name);
		    put_attachtab();
		    unlock_attachtab();
		    free_attachtab();
		    end_critical_code();
		    return(FAILURE);
	    }
	    
    }
    atp->status = STATUS_DETACHING;
    put_attachtab();
    mark_in_use(name);
    at = *atp;
    unlock_attachtab();
    free_attachtab();

    if (at.fs->detach)
	    status = (at.fs->detach)(&at);
    else {
	    fprintf(stderr,
		    "Sorry, I don't know how to detach %s type filesystems\n",
		    at.fs->name);
	    status = ERR_FATAL;
	    return(FAILURE);
    }
    
    if (status == SUCCESS) {
	if (verbose)
		printf("%s: detached\n", at.hesiodname);
	if (at.fs->flags & FS_MNTPT)
		rm_mntpt(&at);
	lock_attachtab();
	get_attachtab();
	if (bymntpt)
	    attachtab_delete(attachtab_lookup_mntpt(at.mntpt));
	else
	    attachtab_delete(attachtab_lookup(at.hesiodname));
	put_attachtab();
	mark_in_use(NULL);
	unlock_attachtab();
	/*
	 * Do Zephyr stuff as necessary
	 */
#ifdef ZEPHYR
	if (use_zephyr && at.fs->flags & FS_REMOTE) {
		sprintf(instbfr, "%s:%s", at.host, at.hostdir);
		zephyr_addsub(instbfr);
		if (!host_occurs(at.host))
			zephyr_addsub(at.host);
        }
#endif ZEPHYR
	free_attachtab();
    } else {
	lock_attachtab();
	get_attachtab();
	at.status = STATUS_ATTACHED;
	attachtab_replace(&at);
	put_attachtab();
	mark_in_use(NULL);
	unlock_attachtab();
	free_attachtab();
    }
    end_critical_code();
    return (status);
}

/*
 * Detach all filesystems.  Read through the attachtab and call detach
 * on each one.
 */

detach_all()
{
    struct _attachtab	*atp, *next;
    int tempexp;
    extern struct _attachtab	*attachtab_last, *attachtab_first;
    struct _fstypes	*fs_type = NULL;

    lock_attachtab();
    get_attachtab();
    unlock_attachtab();
    
    if (filsys_type)
	    fs_type = get_fs(filsys_type);
    tempexp = explicit;
    atp = attachtab_last;
    attachtab_last = attachtab_first = NULL;
    while (atp) {
	    next = atp->prev;
	    explicit = atp->explicit;
	    if ((override || !owner_check || clean_detach
		 || is_an_owner(atp,owner_uid)) &&
		(!filsys_type || fs_type == atp->fs) &&
		!(atp->flags & FLAG_LOCKED))
		    detach(atp->hesiodname);
	    free(atp);
	    atp = next;
    }
    free_attachtab();
    explicit = tempexp;
}

/*
 * Detach routine for the NULL filesystem.  This is for cleanup
 * purposes only.
 */

null_detach(at, mopt, errorout)
	struct _attachtab *at;
	struct mntopts	*mopt;
	int errorout;
{
	if (debug_flag)
		printf("Detaching null filesystem %s...\n", at->hesiodname);

	return(SUCCESS);
}

	
/*
 * Detach all filesystems from a specified host.  Read through the
 * attachtab and call detach on each one.
 */

detach_host(host)
	char	*host;
{
	struct _attachtab	*atp, *next;
	int tempexp;
	extern struct _attachtab	*attachtab_last, *attachtab_first;
	struct _fstypes	*fs_type = NULL;

	lock_attachtab();
	get_attachtab();
	unlock_attachtab();
    
	if (filsys_type)
		fs_type = get_fs(filsys_type);
	tempexp = explicit;
	atp = attachtab_last;
	attachtab_last = attachtab_first = NULL;
	while (atp) {
		next = atp->prev;
		explicit = atp->explicit;
		if ((override || !owner_check || clean_detach
		     || is_an_owner(atp,owner_uid)) &&
		    (!filsys_type || fs_type == atp->fs) &&
		    (host_compare(host, atp->host)) &&
		    !(atp->flags & FLAG_LOCKED))
			detach(atp->hesiodname);
		free(atp);
		atp = next;
	}
	free_attachtab();
	explicit = tempexp;
}

