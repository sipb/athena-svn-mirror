/*
 * 	$Id: nfs.c,v 1.11 1999-01-22 23:08:34 ghudson Exp $
 *
 *	Copyright (c) 1988 by the Massachusetts Institute of Technology.
 */

static char *rcsid_nfs_c = "$Id: nfs.c,v 1.11 1999-01-22 23:08:34 ghudson Exp $";

#include "attach.h"
#ifdef NFS

nfs_attach(at, mopt, errorout, mountpoint_list)
	struct _attachtab *at;
	struct mntopts	*mopt;
	int errorout;
	string_list **mountpoint_list;
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
			if (!override) {
				fprintf(stderr,
					"%s: (filesystem %s) NFS self mount not allowed.\n",
					progname, at->hesiodname);
				error_status = ERR_ATTACHNOTALLOWED;
				return(FAILURE);
			}
			fprintf(stderr, "%s: (filesystem %s) warning: NFS self mount\n",
				progname, at->hesiodname);
		}
	}

	if ((at->mode != 'n') && do_nfsid)
		if (nfsid(at->host, at->hostaddr[0], MOUNTPROC_KUIDMAP,
			  errorout, at->hesiodname, 1, owner_uid) == FAILURE) {
			if (mopt->flags & M_RDONLY) {
				printf("%s: Warning, mapping failed for filesystem %s,\n\tcontinuing with read-only mount.\n",
				       progname, at->hesiodname);
				/* So the mount rpc wins */
				clear_errored(at->hostaddr[0]); 
			} else if(at->mode == 'm') {
				printf("%s: Warning, mapping failed for filesystem %s.\n", 
				       progname, at->hesiodname);
				error_status = 0;
				clear_errored(at->hostaddr[0]);
			} else
				return (FAILURE);
		}

	/* XXX This is kind of bogus, because if a filesystem has a number
	 * of hesiod entries, and the mount point is busy, each one will
	 * be tried until the last one fails, then an error printed.
	 * C'est la vie.
	 */
	
	sprintf(fsname, "%s:%s", at->host, at->hostdir);

	if (mountfs(at, fsname, mopt, errorout) == FAILURE) {
		if ((at->mode != 'n') && do_nfsid)
			nfsid(at->host, at->hostaddr[0], MOUNTPROC_KUIDUMAP,
			      errorout, at->hesiodname, 1, owner_uid);
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
	    nfsid(at->host, at->hostaddr[0], MOUNTPROC_KUIDUMAP, 1,
		  at->hesiodname,0, owner_uid) == FAILURE)
		printf("%s: Warning: couldn't unmap filesystem %s/host %s\n",
		       progname, at->hesiodname, at->host);

	if (at->flags & FLAG_PERMANENT) {
		if (debug_flag)
			printf("Permanent flag on, skipping umount.\n");
		return(SUCCESS);
	}
	
	if (nfs_unmount(at->hesiodname, at->host, at->hostaddr[0],
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

    filsys_type = "NFS";
    strcpy(host, name);
    dir = strchr(host, ':');
    if (!dir) {
	fprintf(stderr, "%s: Illegal explicit definition \"%s\" for type %s\n",
		progname, name, filsys_type); 
	return (0);
    }
    *dir = '\0';
    dir++;
    if (*dir != '/') {
	fprintf(stderr, "%s: Illegal explicit definition \"%s\" for type %s\n",
		progname, name, filsys_type);
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
	    if (cp = strchr(temp, '.'))
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
