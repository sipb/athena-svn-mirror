/*	Created by:	Robert French
 *
 *	$Id: mount.c,v 1.10 1996-09-19 22:13:13 ghudson Exp $
 *
 *	Copyright (c) 1988 by the Massachusetts Institute of Technology.
 */

static char *rcsid_mount_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/attach/mount.c,v 1.10 1996-09-19 22:13:13 ghudson Exp $";

#include "attach.h"

#ifdef MOUNT_CMD

#ifdef _IBMR2
#include <sys/id.h>
#endif

mountfs(at, fsname, mopt, errorout)
	struct	_attachtab *at;
	char	*fsname;
	struct	mntopts *mopt;
	int errorout;
{
	int status;

#ifdef _IBMR2
	if (setuidx(ID_REAL|ID_EFFECTIVE, 0))
#else
#ifdef SOLARIS
	if (setuid(0))
#else
	if (setreuid(0,0))
#endif
#endif
	{
		fprintf(stderr, "Unable to change the uid to 0\n");
		return(FAILURE);
	}

	switch (fork()) {
	case -1:
		fprintf(stderr, "Unable to fork\n");
		return(FAILURE);
		/* NOTREACHED */
	case 0:
		execl(MOUNT_CMD, MOUNT_CMD,
		      "-o", stropt(*mopt),
		      fsname, at->mntpt,
		      (char *)0);
		exit(1);
		/* NOTREACHED */
	default:
		wait(&status);
		break;
	}

	return(status ? FAILURE : SUCCESS);
}


#else /* !MOUNT_CMD */

#ifndef ultrix
#include <mntent.h>
#endif
#if defined(_AIX) && (AIXV < 30)
#include <sys/dstat.h>
#include <rpc/rpcmount.h>
#include <rpc/nfsmount.h>
struct ufs_args { char *fspec;};
#endif
#ifdef _AUX_SOURCE
#define	mount(type,dir,flags,data)	fsmount(type,dir,flags,data)
#endif

extern int sys_nerr;
extern char *sys_errlist[];

/*
 * Mount a filesystem
 */
mountfs(at, fsname, mopt, errorout)
	struct	_attachtab	*at;
	char	*fsname;
	struct	mntopts	*mopt;
	int errorout;
{
	struct	mntent	mnt;
	union data {
#if defined(UFS) || defined(RVD)
		struct ufs_args	ufs_args;
#endif
#ifdef NFS
		struct nfs_args nfs_args;
#endif
	} data;

	mnt.mnt_fsname = fsname;
	mnt.mnt_dir = at->mntpt;
#if !defined(_AIX) && !defined(ultrix)
	mnt.mnt_type = (at->fs->mount_type==MOUNT_NFS) ? MNTTYPE_NFS
		: MNTTYPE_42;
#endif
	mnt.mnt_opts = stropt(*mopt);
	mnt.mnt_freq = 0;
#if defined(_AIX) && (AIXV<30)
	mnt.mnt_type = at->fs->mount_type == MOUNT_NFS ? "nfs" : "ufs";
	mnt.mnt_checkno = 0;
#else
	mnt.mnt_passno = 0;
#endif
#if defined(sun)
	mnt.mnt_type = at->fs->mount_type == MOUNT_NFS ? "nfs" : "ufs";
#endif		
	bzero(&data, sizeof(data));
	/* Already mounted? Why lose? */
	if (mounted(&mnt)) {
		fprintf(stderr,
			"%s: Already mounted, continuing...\n",
			at->hesiodname);
		if (keep_mount)
			at->flags |= FLAG_PERMANENT;
		return (SUCCESS);
	}
#if defined(UFS) || defined(RVD)
	if (at->fs->mount_type == MOUNT_UFS)
		if (mount_42(&mnt, &data.ufs_args) == FAILURE)
			return (FAILURE);
#endif
#ifdef NFS
	if (at->fs->mount_type == MOUNT_NFS) {
		if (mount_nfs(at, mopt, &data.nfs_args,
			      errorout) == FAILURE)
			return (FAILURE);
#ifdef _AIX
		if (nfs_mount(mnt.mnt_dir, &data.nfs_args, mopt->flags) != 0) {
			if (errorout)
				fprintf(stderr,
					"%s: Can't mount %s on %s - %s\n",
					at->hesiodname, fsname, mnt.mnt_dir,
					sys_errlist[errno]);
			return (FAILURE);
		} else {
		  /* We need to get the filesystem's gfs for mtab */
		  struct dstat st_buf;
		  if(dstat(mnt.mnt_dir, &st_buf, sizeof(st_buf)) != 0) {
		    if (errorout)
		      fprintf(stderr,
			      "%s: Can't stat %s to verify mount: %s\n",
			      at->hesiodname, mnt.mnt_dir, sys_errlist[errno]);
		    return (FAILURE);
		  } else {
		    mnt.mnt_gfs = st_buf.dst_gfs;
		    goto done;
		  }
		}
#endif
	}
#endif /* NFS */

#ifdef ultrix
	if (mount(mnt.mnt_fsname, mnt.mnt_dir, mopt->flags,
		  at->fs->mount_type, (char *)&data) < 0) {
#else /* !ultrix */
#ifdef _AIX
	if (mount(mnt.mnt_fsname, mnt.mnt_dir, mopt->flags) < 0) {
#else
#if defined(sun) 
	if (mount(mnt.mnt_type, mnt.mnt_dir, M_NEWTYPE | mopt->flags, &data) < 0) {
#else
	if (mount(at->fs->mount_type, mnt.mnt_dir, mopt->flags, &data) < 0) {
#endif /* sun */
#endif
#endif /* ultrix */
		if (errorout) {
			fprintf(stderr,
				"%s: Can't mount %s on %s - %s\n",
				at->hesiodname, fsname, mnt.mnt_dir,
				sys_errlist[errno]);
			error_status = ERR_SOMETHING;
		}
		return (FAILURE);
	} 

#ifdef _AIX
	else {
	  struct dstat st_buf;
	  if(dstat(mnt.mnt_dir, &st_buf, sizeof(st_buf)) != 0) {
	    if (errorout)
	      fprintf(stderr,
		      "%s: Can't stat %s to verify mount: %s\n",
		      at->hesiodname, mnt.mnt_dir, sys_errlist[errno]);
	    return (FAILURE);
	  } else {
	    mnt.mnt_gfs = st_buf.dst_gfs;
	  }
	}
#endif
#ifndef ultrix
      done:
	addtomtab(&mnt);
#endif /* !ultrix */
	return (SUCCESS);
}

#if defined(UFS) || defined(RVD)
/*
 * Prepare to mount a 4.2 style file system
 */

mount_42(mnt, args)
    struct mntent *mnt;
    struct ufs_args *args;
{
#ifndef ultrix
    args->fspec = mnt->mnt_fsname;
#endif
    return (SUCCESS);
}

#endif /* UFS || RVD */

#ifdef NFS
/*
 * Mount an NFS file system
 */
mount_nfs(at, mopt, args, errorout)
	struct	_attachtab	*at;
	struct	mntopts		*mopt;
	struct nfs_args *args;
	int errorout;
{
	static struct fhstatus fhs;
	static struct sockaddr_in sin;
	struct timeval timeout;
	CLIENT *client;
	enum clnt_stat rpc_stat;
	char	*hostdir = at->hostdir;

	if (errored_out(at->hostaddr[0])) {
		if (errorout)
			fprintf(stderr,
		"%s: Ignoring %s due to previous host errors\n",
				at->hesiodname, at->host);
		return (FAILURE);
	}
    
	if ((client = rpc_create(at->hostaddr[0], &sin)) == NULL) {
		if (errorout)
			fprintf(stderr, "%s: Server %s not responding\n",
				at->hesiodname, at->host);
		return (FAILURE);
	}
    
	client->cl_auth = spoofunix_create_default(spoofhost, real_uid);

	timeout.tv_usec = 0;
	timeout.tv_sec = 20;
	rpc_stat = clnt_call(client, MOUNTPROC_MNT, xdr_path, &hostdir,
			     xdr_fhstatus, &fhs, timeout);
	if (rpc_stat != RPC_SUCCESS) {
		mark_errored(at->hostaddr[0]);
		if (debug_flag)
			clnt_perror(client, "RPC return status");
		if (!errorout)
			return(FAILURE);
		switch (rpc_stat) {
		case RPC_TIMEDOUT:
			fprintf(stderr,
			"%s: Timeout while contacting mount daemon on %s\n",
				at->hesiodname, at->host);
			break;
		case RPC_AUTHERROR:
			fprintf(stderr, "%s: Authentication failed\n",
				at->hesiodname, at->host);
			break;
		case RPC_PMAPFAILURE:
			fprintf(stderr, "%s: Can't find mount daemon on %s\n",
				at->hesiodname, at->host);
			break;
		case RPC_PROGUNAVAIL:
		case RPC_PROGNOTREGISTERED:
			fprintf(stderr,
				"%s: Mount daemon not available on %s\n",
				at->hesiodname, at->host);
			break;
		default:
			fprintf(stderr,
				"%s: System error contacting server %s\n",
				at->hesiodname, at->host);
			break;
		} 
		return (FAILURE);
	} 

	if (errno = fhs.fhs_status) {
		if (errorout) {
			if (errno == EACCES) {
				fprintf(stderr,
				"%s: Mount access denied by server %s\n",
					at->hesiodname, at->host);
				error_status = ERR_ATTACHNOTALLOWED;
			} else if (errno < sys_nerr) {
				error_status = (errno == ENOENT) ? 
				  ERR_ATTACHNOFILSYS : ERR_ATTACHNOTALLOWED;
				fprintf(stderr,
			"%s: Error message returned from server %s: %s\n",
					at->hesiodname, at->host,
					sys_errlist[errno]);
			} else {
				error_status = ERR_ATTACHNOTALLOWED;
				fprintf(stderr,
			"%s: Error status %d returned from server %s\n",
					at->hesiodname, errno, at->host);
			}
		}
		return (FAILURE);
	}

	*args = mopt->tsa.nfs;
	args->hostname = at->host;
	args->fh = &fhs.fhs_fh;
	args->flags |= NFSMNT_HOSTNAME;
	if (mopt->nfs_port)
		sin.sin_port = mopt->nfs_port;
	else
		sin.sin_port = htons(NFS_PORT);	/* XXX should use portmapper */
	args->addr = &sin;
#ifdef ultrix 
	args->optstr = stropt(*mopt);
#endif
	return (SUCCESS);
}
#endif

#ifdef ultrix
mounted(mntck)
    struct mntent *mntck;
{
    int done = 0;
    int fsloc = 0;
    int found = 0;
    struct fs_data fsdata;

    while (getmountent(&fsloc, &fsdata, 1) > 0) {
	if (!strcmp(fsdata.fd_path, mntck->mnt_dir) &&
	    !strcmp(fsdata.fd_devname, mntck->mnt_fsname)) {
	    return(1);
	    break;
	}
    }
    return(0);
}

#else /* !ultrix */

/*
 * Add an entry to /etc/mtab
 */

addtomtab(mnt)
    struct mntent *mnt;
{
    FILE *mnted;

    lock_mtab();
    mnted = setmntent(mtab_fn, "r+");
    if (!mnted || addmntent(mnted, mnt)) {
	    fprintf(stderr, "Can't append to %s: %s\n", mtab_fn,
		    sys_errlist[errno]);
	    unlock_mtab();
	    exit(ERR_FATAL);
    }
    endmntent(mnted);
    unlock_mtab();
}

mounted(mntck)
    struct mntent *mntck;
{
    int found = 0;
    struct mntent *mnt;
    FILE *mnttab;

    lock_mtab();
    mnttab = setmntent(mtab_fn, "r");
    if (!mnttab) {
	    fprintf(stderr, "Can't open %s for read\n", mtab_fn);
	    exit(ERR_FATAL);
    }
    while (mnt = getmntent(mnttab)) {
	if (!strcmp(mnt->mnt_type, MNTTYPE_IGNORE))
	    continue;
	if ((!strcmp(mntck->mnt_dir, mnt->mnt_dir)) &&
	    (!strcmp(mntck->mnt_type, mnt->mnt_type))) {
		if (!strcmp(mnt->mnt_type, MNTTYPE_NFS)) {
			if (nfs_fsname_compare(mntck->mnt_fsname,
					       mnt->mnt_fsname)) {
				found = 1;
				break;
			}
		} else if (!strcmp(mntck->mnt_fsname,
				   mnt->mnt_fsname)) {
			found = 1;
			break;
		}
	}
    }
    endmntent(mnttab);
    unlock_mtab();
    return (found);
}

/*
 * Returns true if two NFS fsnames are the same.
 */
nfs_fsname_compare(fsname1, fsname2)
	char	*fsname1;
	char	*fsname2;
{
	char	host1[BUFSIZ], host2[BUFSIZ];
	char	*rmdir1, *rmdir2;

	(void) strcpy(host1, fsname1);
	(void) strcpy(host2, fsname2);
	if (rmdir1 = strchr(host1, ':'))
		*rmdir1++ = '\0';
	if (rmdir2 = strchr(host2, ':'))
		*rmdir2++ = '\0';
	if (host_compare(host1, host2)) {
		if (!rmdir1 || !rmdir2)
			return(0);
		return(!strcmp(rmdir1, rmdir2));
	} else
		return(0);
}

#endif /* !ultrix */

#endif /* !MOUNT_CMD */
