/*	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/attach/unmount.c,v $
 *	$Author: epeisach $
 *
 *	Copyright (c) 1988 by the Massachusetts Institute of Technology.
 */

/*
 * NOTE!  A good deal of this file was stolen from BSD 4.3's umount.c.  It
 * is probably still protected under standard Berkeley copyright agreements.
 */

static char *rcsid_mount_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/attach/unmount.c,v 1.4 1991-03-04 12:58:13 epeisach Exp $";

#include "attach.h"
#include <sys/file.h>
#include <sys/param.h>
#ifndef ultrix
#include <mntent.h>
#endif
#if defined(_AIX) && (AIXV < 30)
#include <rpc/nfsmount.h>
#include <rpc/rpcmount.h>
#else
#include <rpcsvc/mount.h>
#endif
#ifdef AIX
#define unmount(x) umount(x)
#endif
/*
 * Unmount a filesystem.
 */

unmount_42(errname, mntpt, dev)
    char *errname;
    char *mntpt;
    char *dev;
{
    FILE *tmpmnt, *mnted;
    char *tmpname;
#ifndef ultrix
    struct mntent *mnt;
#endif
    int tmpfd;

#ifdef ultrix
    int fsloc = 0, found = 0;
    struct fs_data fsdata;

    while (getmountent(&fsloc, &fsdata, 1) > 0) {
	if (!strcmp(fsdata.fd_path, mntpt)) {
	    found = 1;
	    break;
	}
    }
    if (!found) {
	fprintf(stderr,
		"%s: Directory %s appears to already be unmounted\n",
		errname, mntpt);
	return(SUCCESS);
    }
/* this hack to avoid ugly ifdef's */
#define unmount(x) umount(fsdata.fd_dev)
#endif /* ultrix */

    if (unmount(dev ? dev : mntpt) < 0) {
	if (errno == EINVAL || errno == ENOENT
#ifdef _AIX
	    || errno == ENOTBLK
#endif
	    ) {
		fprintf(stderr,
			"%s: Directory %s appears to already be unmounted\n",
			errname, mntpt);
		/* Continue on, to flush mtab if necessary */
	} else {
		fprintf(stderr, "%s: Unable to unmount %s: %s\n", errname,
			mntpt, sys_errlist[errno]);
		return (FAILURE);
	}
    }

#ifdef ultrix
#undef unmount
    return(SUCCESS);
#else /* !ultrix */

    lock_mtab();
    if (!(tmpname = malloc(strlen(mtab_fn)+7))) {
	    fprintf(stderr, "Can't malloc temp filename for unmount!\n");
	    exit(ERR_FATAL);
    }
    (void) strcpy(tmpname, mtab_fn);
    (void) strcat(tmpname, "XXXXXX");
    mktemp(tmpname);
    if ((tmpfd = open(tmpname, O_RDWR|O_CREAT|O_TRUNC, 0644)) < 0) {
	    fprintf(stderr, "Can't open temporary file for umount!\n");
	    exit(ERR_FATAL);
    }
    close(tmpfd);
    tmpmnt = setmntent(tmpname, "w");
    if (!tmpmnt) {
	    fprintf(stderr,
		    "Can't open temporary file for writing in umount!\n");
	    exit(ERR_FATAL);
    }
    mnted = setmntent(mtab_fn, "r");
    if (!mnted) {
	    fprintf(stderr, "Can't open %s for read:%s\n", mtab_fn,
		    sys_errlist[errno]);
	    exit(ERR_FATAL);
    }
    /* Delete filesystem from /etc/mtab */
    while (mnt = getmntent(mnted))
	if (strcmp(mnt->mnt_dir, mntpt))
	    addmntent(tmpmnt, mnt);

    endmntent(tmpmnt);
    endmntent(mnted);
    if (rename(tmpname, mtab_fn) < 0) {
	    fprintf(stderr, "Unable to rename %s to %s: %s\n", tmpname,
		    mtab_fn, sys_errlist[errno]);
	    exit(ERR_FATAL);
    }
    unlock_mtab();
    
    return (SUCCESS);
#endif /* ultrix */
}

#ifdef NFS
/*
 * Unmount an NFS filesystem
 */
nfs_unmount(errname, host, hostaddr, mntpt, rmntpt)
    char *errname;
    char *host;
    struct in_addr hostaddr;
    char *mntpt;
    char *rmntpt;
{
    static struct sockaddr_in sin;
    struct timeval timeout;
    CLIENT *client;
    enum clnt_stat rpc_stat;

    if (unmount_42(errname, mntpt, NULL) == FAILURE)
	return (FAILURE);

    /*
     * If we can't contact the host, don't bother complaining;
     * it won't actually hurt anything except that hosts rmtab.
     */
    if (errored_out(hostaddr))
	return (SUCCESS);

    if ((client = (CLIENT *)rpc_create(hostaddr, &sin)) == NULL) {
	fprintf(stderr,
		"%s: Server %s not responding\n",
		errname, host);
	return (SUCCESS);
    }

    client->cl_auth = spoofunix_create_default(spoofhost, real_uid);

    timeout.tv_usec = 0;
    timeout.tv_sec = 20;
    rpc_stat = clnt_call(client, MOUNTPROC_UMNT, xdr_path, &rmntpt,
			 xdr_void, NULL, timeout);
    if (rpc_stat != RPC_SUCCESS) {
	mark_errored(hostaddr);
	switch (rpc_stat) {
	case RPC_TIMEDOUT:
	    fprintf(stderr, "%s: Timeout while contacting mount daemon on %s\n",
		    errname, host);
	    break;
	case RPC_AUTHERROR:
	    fprintf(stderr, "%s: Authentication failed\n",
		    errname, host);
	    break;
	case RPC_PMAPFAILURE:
	    fprintf(stderr, "%s: Can't find mount daemon on %s\n",
		    errname, host);
	    break;
	case RPC_PROGUNAVAIL:
	case RPC_PROGNOTREGISTERED:
	    fprintf(stderr, "%s: Mount daemon not available on %s\n",
		    errname, host);
	    break;
	default:
	    fprintf(stderr, "%s: System error contacting server %s\n",
		    errname, host);
	    break;
	}
	if (debug_flag)
	    clnt_perror(client, "RPC return status");
	return (SUCCESS);
    } 

    return (SUCCESS);
}
#endif
