/*
 * 	$Source: /afs/dev.mit.edu/source/repository/athena/bin/attach/nfs.c,v $
 *	$Author: jfc $
 *
 *	Copyright (c) 1988 by the Massachusetts Institute of Technology.
 */

#ifndef lint
static char rcsid_nfs_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/attach/nfs.c,v 1.3 1990-04-21 17:40:58 jfc Exp $";
#endif lint

#include "attach.h"
#ifdef NFS

/* If the timeout is not explicitly specified by the attach command,
 * it is set to 20 tenths of a second (i.e. 2 seconds).  This should
 * really be set in the kernel, but due to release shakedown timing,
 * it's kludged in here.
 * Similarly with retransmissions.
 * 
 * The kernel will double the timeout on each retry until it reaches a
 * max of 60 seconds, at which point it uses 60 seconds for the timeout.
 *
 * current kernel defaults: 4 retrans, 7 tenths sec timeout.
 * -JTK, 24 Oct 88
 *
 * The new values are: 6 retrans, 8 tenths sec timeout.
 * The total timeout period is approximately 100 seconds, thus
 * compensating for a gateway rebooting (~40 seconds).
 * -RPB, 9 Feb 88
 * 
 * Calculations:
 *    total time = timeout * (2^(retrans) - 1)
 *    [derived from sum of geometric series = a(r^n-1)/(r-1)]
 *       a = initial timeout
 *       r = 2
 *       n = retrans
 *
 *    This holds true while timeout * 2^retrans <= 60 seconds
 */

#define	TIMEO_DEFAULT	8
#define	RETRANS_DEFAULT	7

nfs_attach(at, mopt, errorout)
	struct _attachtab *at;
	struct mntopts	*mopt;
	int errorout;
{
	char fsname[BUFSIZ];
	static char myhostname[BUFSIZ] = "";

	/*
	 * Handle the 'n' mode.
	 */
	if (at->mode == 'n') {
		add_options(mopt, "ro");
	}
	
	/*
	 * Try to figure out information about myself.  Use fsname
	 * as a temporary buffer.
	 */
	if (!myhostname[0]) {
		if (gethostname(myhostname, sizeof(myhostname))) {
			if (debug_flag)
				perror("gethostname");
		}
	}

	if (myhostname[0]) {
		if (host_compare(myhostname, at->host)) {
			fprintf(stderr,
				"Doing an NFS self mount.  %s\n",
				override ? "Error overridden..." :
				"Bad news!");
			if (!override) {
				error_status = ERR_ATTACHNOTALLOWED;
				return(FAILURE);
			}
		}
	}

	if ((at->mode != 'n') && do_nfsid)
		if (nfsid(at->host, at->hostaddr, MOUNTPROC_KUIDMAP,
			  errorout, at->hesiodname, 1, real_uid) == FAILURE) {
			if (mopt->flags & M_RDONLY) {
				printf("%s: Warning, mapping failed, continuing with read-only mount.\n",
				       at->hesiodname);
				/* So the mount rpc wins */
				clear_errored(at->hostaddr); 
			} else if(at->mode == 'm') {
				printf("%s: Warning, mapping failed.\n", 
				       at->hesiodname);
				error_status = 0;
				clear_errored(at->hostaddr);
			} else
				return (FAILURE);
		}

	if (!(mopt->tsa.nfs.flags & NFSMNT_RETRANS)) {
		mopt->tsa.nfs.flags |= NFSMNT_RETRANS;
		mopt->tsa.nfs.retrans = RETRANS_DEFAULT;
	}
	    
	if (!(mopt->tsa.nfs.flags & NFSMNT_TIMEO)) {
		mopt->tsa.nfs.flags |= NFSMNT_TIMEO;
		mopt->tsa.nfs.timeo = TIMEO_DEFAULT;
	}
    
	/* XXX This is kind of bogus, because if a filesystem has a number
	 * of hesiod entries, and the mount point is busy, each one will
	 * be tried until the last one fails, then an error printed.
	 * C'est la vie.
	 */
	
	sprintf(fsname, "%s:%s", at->host, at->hostdir);

	if (mountfs(at, fsname, mopt, errorout) == FAILURE) {
		if ((at->mode != 'n') && do_nfsid)
			nfsid(at->host, at->hostaddr, MOUNTPROC_KUIDUMAP,
			      errorout, at->hesiodname, 1, real_uid);
		return (FAILURE);
	}

	return (SUCCESS);
}

/*
 * Detach an NFS filesystem.
 */
nfs_detach(at)
    struct _attachtab *at;
{
	if ((at->mode != 'n') && do_nfsid &&
	    nfsid(at->host, at->hostaddr, MOUNTPROC_KUIDUMAP, 1,
		  at->hesiodname,0, real_uid) == FAILURE)
		printf("Couldn't unmap %s, continuing.....\n", at->host);

	if (at->flags & FLAG_PERMANENT) {
		if (debug_flag)
			printf("Permanent flag on, skipping umount.\n");
		return(SUCCESS);
	}
	
	if (nfs_unmount(at->hesiodname, at->host, at->hostaddr,
			at->mntpt, at->hostdir) == FAILURE)
		return (FAILURE);
    
	return (SUCCESS);
}

/*
 * Parsing of explicit NFS file types
 */
char **nfs_explicit(name)
    char *name;
{
    char temp[BUFSIZ], host[BUFSIZ];
    char *dir, *cp;
    char newmntpt[BUFSIZ];
    extern char *exp_hesptr[2];

    strcpy(host, name);
    dir = index(host, ':');
    if (!dir) {
	fprintf(stderr, "%s: Illegal explicit definition for type %s\n",
		name, filsys_type);
	return (0);
    }
    *dir = '\0';
    dir++;
    if (*dir != '/') {
	fprintf(stderr, "%s: Illegal explicit definition for type %s\n",
		name, filsys_type);
	return (0);
    }
    if (!nfs_mount_dir)
	    nfs_mount_dir = strdup("");
    if (!mntpt) {
	    strcpy(temp, host);
	    /*
	     * Zero out any domain names, since they're ugly as mount
	     * points.
	     */
	    if (cp = index(temp, '.'))
		    *cp = '\0';
	    if (!strcmp(dir, "/")) {
		    if (nfs_root_hack)
			    sprintf(newmntpt, "%s/%s/root",
				    nfs_mount_dir, temp);
		    else
			    sprintf(newmntpt, "%s/%s", nfs_mount_dir, temp);
	    } else
		    sprintf(newmntpt, "%s/%s%s", nfs_mount_dir, temp, dir);
    }
    
    sprintf(exp_hesline, "NFS %s %s %c %s", dir, host, override_mode ?
	    override_mode : 'w',mntpt ? mntpt : newmntpt);
    exp_hesptr[0] = exp_hesline;
    exp_hesptr[1] = 0;
    return (exp_hesptr);
}
#endif
