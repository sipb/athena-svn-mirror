/*	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/attach/attach.c,v $
 *	$Author: miki $
 *
 *	Copyright (c) 1988 by the Massachusetts Institute of Technology.
 */

#ifndef lint
static char rcsid_attach_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/attach/attach.c,v 1.18 1994-03-25 15:57:14 miki Exp $";
#endif

#include "attach.h"
#include <signal.h>

extern int mul_attach();

/*
 * Attempt to attach a filesystem.  Lookup the name with Hesiod as
 * necessary, and try all possible results in order.  Also handles
 * the explicit flag for explicitly defined hostname/directory
 * arguments.  Branches to NFS or RVD attach routines as necessary.
 */

attach(name)
    const char *name;
{
    struct _attachtab at, *atp;
    char **hes;
    int i;
    int	print_wait;
    extern int caught_signal;
	
    hes = build_hesiod_line(name);

    if (!hes || !*hes) {
	error_status = ERR_ATTACHBADFILSYS;
	return (FAILURE);
    }

    if (lookup || debug_flag) {
	printf("%s resolves to:\n", name);
	for (i=0;hes[i];i++)
	    printf("%s\n", hes[i]);
	putchar('\n');
    }

    if (lookup)
	    return(SUCCESS);

    print_wait = 2;
retry:
    lock_attachtab();
    get_attachtab();
    if (atp = attachtab_lookup(name)) {
	switch (atp->status) {
	case STATUS_ATTACHING:
	    if (!force && really_in_use(name)) {
		unlock_attachtab();
		free_attachtab();
		if (print_path) {
			sleep(print_wait);  /* Wait a few seconds */
			if (print_wait < 30)
				print_wait *= 2;
			goto retry;
		}
		fprintf(stderr,
			"%s: Filesystem \"%s\" already being attached by another process\n",
			progname, name);
		error_status = ERR_ATTACHINUSE;
		return (FAILURE);
	    }
	    attachtab_delete(atp);
	    put_attachtab();
	    break;
	case STATUS_DETACHING:
	    if (!force && really_in_use(name)) {
		fprintf(stderr, "%s: Filesystem \"%s\" is being detached by another process\n",
			progname, name);
		error_status = ERR_ATTACHINUSE;
		unlock_attachtab();
		free_attachtab();
		return (FAILURE);
	    }
	    attachtab_delete(atp);
	    put_attachtab();
	    break;
	case STATUS_ATTACHED:
	    if (force)
		break;
	    if (lock_filesystem)
		    atp->flags |= FLAG_LOCKED;
	    if (owner_list)
		    add_an_owner(atp,owner_uid);
	    if (owner_list || lock_filesystem)
		    put_attachtab();
	    unlock_attachtab();

	    if (atp->fs->type == TYPE_MUL)
		return mul_attach(atp, (struct mntopts *)0, 0);
	    
	    if (print_path)
		printf("%s\n", atp->mntpt);
	    else if(verbose)
		printf("%s: Filesystem \"%s\" is already attached", progname, name);
#ifdef NFS
	    if (map_anyway && atp->mode != 'n' && atp->fs->type == TYPE_NFS) {
		int ret;
		if (verbose && !print_path)
		    printf(" (mapping)\n");
		
		ret = nfsid(atp->host, atp->hostaddr[0],
			    MOUNTPROC_KUIDMAP, 1, name, 1, real_uid);
		if(atp->mode != 'm')
		  return ret;
		if (ret == FAILURE)
		  {
		    error_status = 0;
		    clear_errored(atp->hostaddr[0]);
		  }
		return SUCCESS;
	    }
#endif
#ifdef AFS
	    if (map_anyway && atp->mode != 'n' && atp->fs->type == TYPE_AFS) {
		    if (verbose && !print_path)
			    printf(" (authenticating)\n");
		    return(afs_auth(atp->hesiodname, atp->hostdir));
	    }
#endif
	    if (verbose && !print_path)
		putchar('\n');
	    /* No error code set on already attached */
	    free_attachtab();
	    return (FAILURE);
	}
    }

    /* Note: attachtab is still locked at this point */
    
    /* We don't really care about ANY of the values besides status */
    at.status = STATUS_ATTACHING;
    at.explicit = explicit;
    at.fs = NULL;
    strcpy(at.hesiodname, name);
    strcpy(at.host, "?");
    strcpy(at.hostdir, "?");
    memset((char *)&at.hostaddr, 0, sizeof(at.hostaddr));
    at.rmdir = 0;
    at.drivenum = 0;
    at.mode = 'r';
    at.flags = 0;
    at.nowners = 1;
    at.owners[0] = owner_uid;
    strcpy(at.mntpt, "?");

    /*
     * Mark the filsys as "being attached".
     */
    start_critical_code();	/* Don't let us be interrupted */
    mark_in_use(name);
    attachtab_append(&at);
    put_attachtab();
    unlock_attachtab();

   for (i=0;hes[i];i++) {
	if (debug_flag)
	    printf("Processing line %s\n", hes[i]);
	if (caught_signal) {
		if (debug_flag)
			printf("Caught signal; cleaning up attachtab....\n");
		free_attachtab();
		lock_attachtab();
		get_attachtab();
		attachtab_delete(attachtab_lookup(at.hesiodname));
		put_attachtab();
		unlock_attachtab();
		free_attachtab();
		terminate_program();
	}
	/*
	 * Note try_attach will change attachtab appropriately if
	 * successful.
	 */
	if (try_attach(name, hes[i], !hes[i+1]) == SUCCESS) {
		free_attachtab();
		mark_in_use(NULL);
		end_critical_code();
		return (SUCCESS);
	}
    }
    free_attachtab();

    if (error_status == ERR_ATTACHNOTALLOWED)
	    fprintf(stderr, "%s: You are not allowed to attach %s.\n",
		    progname, name);

    if (error_status == ERR_ATTACHBADMNTPT)
	    fprintf(stderr,
		    "%s: You are not allowed to attach a filesystem here\n",
		    progname);
    /*
     * We've failed -- delete the partial entry and unlock the in-use file.
     */
    lock_attachtab();
    get_attachtab();
    attachtab_delete(attachtab_lookup(at.hesiodname));
    put_attachtab();
    mark_in_use(NULL);
    unlock_attachtab();
    free_attachtab();
    end_critical_code();

    /* Assume error code already set by whatever made us fail */
    return (FAILURE);
}

/*
 * Given a Hesiod line and a filesystem name, try to attach it.  If
 * successful, change the attachtab accordingly.
 */

try_attach(name, hesline, errorout)
    char *name, *hesline;
    int errorout;
{
    struct _attachtab at, *atp;
    int status;
    int	attach_suid;
#ifdef ZEPHYR
    char instbfr[BUFSIZ];
#endif
    struct mntopts	mopt;
    char	*default_options;

    if (parse_hes(hesline, &at, name)) {
	    error_status = ERR_BADFSDSC;
	    return(FAILURE);
    }
    if (filsys_type && *filsys_type && strcasecmp(filsys_type, at.fs->name)) {
	    error_status = ERR_ATTACHBADFILSYS;
	    return FAILURE;
    }
    if (!override && !allow_filsys(name, at.fs->type)) {
	    error_status = ERR_ATTACHNOTALLOWED;
	    return(FAILURE);
    }

    at.status = STATUS_ATTACHING;
    at.explicit = explicit;
    strcpy(at.hesiodname, name);
    add_an_owner(&at, owner_uid);

    if (mntpt)
	strcpy(at.mntpt, mntpt);

    /*
     * Note if a filesystem does nothave AT_FS_MNTPT_CANON as a property,
     * it must also somehow call check_mntpt, if it wants mountpoint
     * checking to happen at all.
     */
    if (at.fs->flags & AT_FS_MNTPT_CANON) {
	    char *path;

	    /* Perform path canonicalization */
	    if ((path = path_canon(at.mntpt)) == NULL) {
		    fprintf(stderr, "%s: Cannot get information about the mountpoint %s\n",
			    at.hesiodname, at.mntpt);
		    error_status = ERR_SOMETHING;
		    return(FAILURE);
	    }
	    strcpy(at.mntpt, path);
	    if (debug_flag)
		    printf("Mountpoint canonicalized as: %s\n", at.mntpt);
	    if (!override && !check_mountpt(at.mntpt, at.fs->type)) {
		    error_status = ERR_ATTACHBADMNTPT;
		    return(FAILURE);
	    }
    }
    
    if (at.fs->flags & AT_FS_MNTPT && (atp=attachtab_lookup_mntpt(at.mntpt))) {
	    fprintf(stderr,"%s: Filesystem %s is already mounted on %s\n",
		    at.hesiodname, atp->hesiodname, at.mntpt);
	    error_status = ERR_ATTACHDIRINUSE;
	    return(FAILURE);
    }

    if (override_mode)
	at.mode = override_mode;
	
    if (override_suid == -1)
	    attach_suid = !nosetuid_filsys(name, at.fs->type);
    else
	    attach_suid = override_suid;
    at.flags = (attach_suid ? 0 : FLAG_NOSETUID) +
	    (lock_filesystem ? FLAG_LOCKED : 0);

    /* Prepare mount options structure */
    memset(&mopt, 0, sizeof(mopt));
    mopt.type = at.fs->mount_type;
    
    /* Read in default options */
    default_options = filsys_options(at.hesiodname, at.fs->type);
    add_options(&mopt, "soft");		/* Soft by default */
    if (mount_options && *mount_options)
	    add_options(&mopt, mount_options);
    if (default_options && *default_options)
	    add_options(&mopt, default_options);
    
    if (at.mode == 'r')
	    add_options(&mopt, "ro");
    if (at.flags & FLAG_NOSETUID)
	    add_options(&mopt, "nosuid");
	
    if (at.fs->attach) {
	    if (at.fs->flags & AT_FS_MNTPT) {
		    if (make_mntpt(&at) == FAILURE) {
			    rm_mntpt(&at);
			    return (FAILURE);
		    }
	    }
	    status = (at.fs->attach)(&at, &mopt, errorout);
    } else {
	    fprintf(stderr,
		    "%s: Can't attach filesystem type \"%s\"\n", 
		    progname, at.fs->name);
	    status = ERR_FATAL;
	    return(FAILURE);
    }
    
    if (status == SUCCESS) {
	char	tmp[BUFSIZ];
	if (at.fs->flags & AT_FS_REMOTE)
		sprintf(tmp, "%s:%s", at.host, at.hostdir);
	else
		strcpy(tmp, at.hostdir);
	if (verbose)
		if(at.fs->type == TYPE_AFS)
		  printf("%s: %s linked to %s for filesystem %s\n", progname,
			 tmp, at.mntpt, at.hesiodname);
		else
		  printf("%s: filesystem %s (%s) mounted on %s (%s)\n",
			 progname, at.hesiodname, tmp,
			 at.mntpt, (mopt.flags & M_RDONLY) ? "read-only" :
			 "read-write");
	if (print_path)
		printf("%s\n", at.mntpt);
	at.status = STATUS_ATTACHED;
	lock_attachtab();
	get_attachtab();
	attachtab_replace(&at);
	put_attachtab();
	unlock_attachtab();
	/*
	 * Do Zephyr stuff as necessary
	 */
#ifdef ZEPHYR
	if (use_zephyr && at.fs->flags & AT_FS_REMOTE) {
		if(at.fs->type == TYPE_AFS) {
			afs_zinit(at.hesiodname, at.hostdir);
		} else {
			sprintf(instbfr, "%s:%s", at.host, at.hostdir);
			zephyr_addsub(instbfr);
			zephyr_addsub(at.host);
		}
	}
#endif
	free_attachtab();
    } else
	    if (at.fs->flags & AT_FS_MNTPT)
		    rm_mntpt(&at);
    return (status);
}


char *attach_list_format = "%-22s %-22s %c%-18s%s\n";

int attach_print(host)
    char *host;
{
    static int print_banner = 1;
    struct _attachtab *atp;
    extern struct _attachtab *attachtab_first;
    char optstr[40];
    int bad = 0;
    int i;

    lock_attachtab();
    get_attachtab();
    unlock_attachtab();
    atp = attachtab_first;
    if (!atp) {
	printf("No filesystems currently attached.\n");
	free_attachtab();
	return(ERR_NONE);
    }
    if (print_banner) {
	printf(attach_list_format, "filesystem", "mountpoint",
	       ' ', "user", "mode");
	printf(attach_list_format, "----------", "----------",
	       ' ', "----", "----");
	print_banner = 0;
    }
    while (atp) {
	optstr[0] = atp->mode;
	optstr[1] = '\0';
	if (atp->flags & FLAG_NOSETUID)
	    strcat(optstr, ",nosuid");
	if (atp->flags & FLAG_LOCKED)
	    strcat(optstr, ",locked");
	if (atp->flags & FLAG_PERMANENT)
	    strcat(optstr, ",perm");
	if (host) {
	    bad = 1;
	    for (i=0; i<MAXHOSTS && atp->hostaddr[i].s_addr; i++) {
		if (host_compare(host, inet_ntoa(atp->hostaddr[i]))) {
		    bad = 0;
		    break;
		}
	    }
	}
	if (!bad && atp->status == STATUS_ATTACHED) {
	    printf(attach_list_format, atp->hesiodname,
		   (atp->fs->type&TYPE_MUL) ? "-" : atp->mntpt,
		   atp->flags & FLAG_ANYONE ? '*' : ' ',
		   ownerlist(atp), optstr);
	}
	atp = atp->next;
    }
    free_attachtab();
    return (ERR_NONE);
}
