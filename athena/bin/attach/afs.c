/*
 * $Id: afs.c,v 1.13 1997-10-18 01:07:03 ghudson Exp $
 *
 * Copyright (c) 1990,1992 by the Massachusetts Institute of Technology.
 */

static char *rcsid = "$Id: afs.c,v 1.13 1997-10-18 01:07:03 ghudson Exp $";

#include "attach.h"

#ifdef AFS

#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/errno.h>

#include <krb.h>

extern char *krb_realmofhost();	/* <krb.h> doesn't declare this */

/* Flags to afs_auth_internal() */
#define AFSAUTH_DOAUTH		1
#define AFSAUTH_CELL		2
#define AFSAUTH_DOZEPHYR	4

#ifdef __STDC__
static int afs_auth_internal(const char * errorname, const char * afs_pathname,
			     struct in_addr hostaddr[], int flags);
#else
static int afs_auth_internal();
#endif


/*
 * The current implementation of AFS attaches is to make a symbolic
 * link.  This is NOT GUARANTEED to always be the case. 
 */

afs_attach(at, mopt, errorout)
	struct _attachtab *at;
	struct mntopts	*mopt;
	int errorout;
{
	struct	stat	statbuf;
	char	buf[BUFSIZ];
	int	len;
	int	afs_auth_flags = 0;

	if ((at->mode != 'n') && do_nfsid)
		afs_auth_flags |= AFSAUTH_DOAUTH;

#ifdef ZEPHYR
	if (use_zephyr)
		afs_auth_flags |= AFSAUTH_DOZEPHYR;
#endif	
	
	if (afs_auth_flags & (AFSAUTH_DOZEPHYR | AFSAUTH_DOAUTH)) {
		if (afs_auth_internal(at->hesiodname, at->hostdir,
				      at->hostaddr, afs_auth_flags) == FAILURE)
			return(FAILURE);
	}
	
	if (debug_flag)
		printf("lstating %s...\n", at->hostdir);
	seteuid(owner_uid);
	if (stat(at->hostdir, &statbuf)) {
		if (errno == ENOENT)
			fprintf(stderr, "%s: %s does not exist\n",
				at->hesiodname, at->hostdir);
		else
			perror(at->hostdir);
		error_status = ERR_ATTACHNOFILSYS;
		seteuid(effective_uid);
		return(FAILURE);
	}
	if ((statbuf.st_mode & S_IFMT) != S_IFDIR) {
		fprintf(stderr, "%s: %s is not a directory\n",
			at->hesiodname, at->hostdir);
		seteuid(effective_uid);

		error_status = ERR_ATTACHNOFILSYS;
		return(FAILURE);
	}
	
	if (debug_flag)
		printf("lstating %s....\n", at->mntpt);
	if (!lstat(at->mntpt, &statbuf)) {
		seteuid(effective_uid);
		if ((statbuf.st_mode & S_IFMT) == S_IFLNK) {
			len = readlink(at->mntpt, buf, sizeof(buf));
			buf[len] = '\0';
			(void) strcpy(buf, path_canon(buf));
			if (!strcmp(buf, path_canon(at->hostdir))) {
				if (!print_path && verbose)
				    fprintf(stderr,
					    "%s: %s already sym linked%s\n",
					    at->hesiodname, at->hostdir,
					    (afs_auth_flags & AFSAUTH_DOAUTH) ?
					    ", authenticating..." : "");
				if (keep_mount)
					at->flags |= FLAG_PERMANENT;
				return(SUCCESS);
			} else {
				if (keep_mount) {
					fprintf(stderr,
	"%s: Couldn't attach; symlink from %s to %s already exists.\n",
			at->hesiodname, at->mntpt, buf);
					error_status = ERR_ATTACHINUSE;
					return(FAILURE);
				}
				if (
#if defined(_AIX) && (AIXV==12)
				    /* AIX 1.x */
				    rmslink(at->mntpt) == -1 &&
#endif
				    unlink(at->mntpt) == -1)
				{
					fprintf(stderr,
	"%s: Couldn't remove existing symlink from %s to %s: %s\n",
			at->hesiodname, at->mntpt, buf, sys_errlist[errno]);
					error_status = ERR_ATTACHINUSE;
					return(FAILURE);
				}
			}
		} else if ((statbuf.st_mode & S_IFMT) == S_IFDIR) {
			if (rmdir(at->mntpt)) {
				if (errno == ENOTEMPTY)
					fprintf(stderr,
				"%s: %s is a non-empty directory.\n",
						at->hesiodname, at->mntpt);
				else
					fprintf(stderr,
						"%s: Couldn't rmdir %s (%s)\n",
						at->hesiodname, at->mntpt,
						sys_errlist[errno]);
				error_status = ERR_ATTACHINUSE;
				return(FAILURE);
			}
		} else {
			fprintf(stderr,
				"%s: Couldn't attach; %s already exists.\n",
				at->hesiodname, at->mntpt);
			error_status = ERR_ATTACHINUSE;
			return(FAILURE);
		}
	}
	seteuid(effective_uid);

	/*
	 * Note: we do our own path canonicalization here, since
	 * we have to check the sym link first.
	 */
	(void) strcpy(at->mntpt, path_canon(at->mntpt));
	if (debug_flag)
		printf("Mountpoint canonicalized as: %s\n", at->mntpt);
	
	if (!override && !check_mountpt(at->mntpt, at->fs->type)) {
		error_status = ERR_ATTACHBADMNTPT;
		return(FAILURE);
	}
	
	if (debug_flag)
		printf("symlinking %s to %s\n", at->hostdir, at->mntpt);
	if (symlink(at->hostdir, at->mntpt) < 0) {
		fprintf(stderr, "%s: ", at->hesiodname);
		perror(at->hostdir);
/*		error_status = ERR_; */
		return(FAILURE);
	}
	
	return (SUCCESS);
}

/*
 * afs_auth_internal --- authenticate oneself to the afs system
 *
 * Actually, it also does the zephyr subscriptions, but that's because
 * aklog does all of that stuff.
 */
static int afs_auth_internal(errorname, afs_pathname, hostlist, flags)
	const char	*errorname;
	const char	*afs_pathname;	/* For future expansion */
	struct in_addr hostlist[];
	int	flags;
{
#ifdef POSIX
	int	waitb;
#else
	union wait	waitb;
#endif
	int	error_ret;
	int	fds[2];
	FILE	*f;
	char	buff[512];
	int	host_idx = 0;
	
	if (debug_flag)
		printf("performing an %s %s %s -hosts -zsubs %s\n", aklog_fn,
		       flags & AFSAUTH_CELL ? "-cell" : "-path",
		       afs_pathname, flags & AFSAUTH_DOAUTH ? "" : "-noauth");
	
	if (pipe(fds)) {
		perror("afs_auth: pipe");
		return(FAILURE);
	}
	
	switch(vfork()) {
	case -1:
		close(fds[0]);
		close(fds[1]);
		perror("vfork: to aklog");
		error_status = ERR_AUTHFAIL;
		return(FAILURE);
	case 0:
		if (!debug_flag) {
			close(0);
			open("/dev/null", O_RDWR, 0644);
		}

		close(fds[0]);
		dup2(fds[1], 1);
		close(fds[1]);
		setuid(owner_uid);
		execl(aklog_fn, AKLOG_SHORTNAME,
		      flags & AFSAUTH_CELL ? "-cell" : "-path", 
		      afs_pathname, "-hosts", "-zsubs",
		      (flags & AFSAUTH_DOAUTH) ? 0 : "-noauth", 0);
		perror(aklog_fn);
		exit(1);
		/*NOTREACHED*/
	default:
		close(fds[1]);
		if ((f = fdopen(fds[0], "r")) == NULL) {
			perror("fdopen: to aklog output");
			return(FAILURE);
		} 
		while (fgets(buff, sizeof(buff), f)) {
			char	*cp;
			int	len;

			if (debug_flag)
				fputs(buff, stdout);
			if (!(cp = strchr(buff, ':')))
				continue;
			*cp = '\0';
#ifdef ZEPHYR
			if (!strcmp(buff, "zsub") &&
			    (flags & AFSAUTH_DOZEPHYR)) {
				cp++;
				while (*cp && isspace(*cp))
					cp++;
				for (len=strlen(cp)-1;
				     len>=0 && !isprint(cp[len])
				     ; len--)
					cp[len] = '\0';
				zephyr_addsub(cp);
			}
#endif
			if (hostlist && !strcmp(buff, "host")) {
				cp++;
				while (*cp && isspace(*cp))
					cp++;
				for (len=strlen(cp)-1;
				     len>=0 && !isprint(cp[len])
				     ; len--)
					cp[len] = '\0';
				hostlist[host_idx++].s_addr = inet_addr(cp);
			}
		}
		fclose(f);
		if (wait(&waitb) < 0) {
			perror("wait: for aklog");
			error_status = ERR_AUTHFAIL;
			return(FAILURE);
		}
	}

#ifdef POSIX
	error_ret = waitb;
#else
	error_ret = waitb.w_retcode;
#endif
	if (error_ret && (flags & AFSAUTH_DOAUTH)) {
		error_status = ERR_AUTHFAIL;
		return (FAILURE);
	}
	return(SUCCESS);
}

int afs_auth(hesname, afsdir)
const char *hesname, *afsdir;
{
    return(afs_auth_internal(hesname, afsdir, 0, AFSAUTH_DOAUTH));
}

/*
 * Parsing of explicit AFS file types
 */
char **afs_explicit(name)
	char *name;
{
  char *dir;
  char newmntpt[BUFSIZ];
  extern char *exp_hesptr[2];

  if (*name != '/')
    {
      fprintf(stderr, "%s: Illegal explicit definition \"%s\" for type %s\n",
	      progname, name, filsys_type);
      return (0);
    }
	
  dir = strrchr(name, '/');
  (void) strcpy(newmntpt, afs_mount_dir);
  (void) strcat(newmntpt, dir);
  
  sprintf(exp_hesline, "AFS %s %c %s", name, override_mode ?
	  override_mode : 'w', mntpt ? mntpt : newmntpt);
  exp_hesptr[0] = exp_hesline;
  exp_hesptr[1] = 0;
  return exp_hesptr;
}

int afs_detach(at)
	struct _attachtab *at;
{
    if(at->flags & FLAG_PERMANENT)
	return SUCCESS;
     
    if (
#if defined(_AIX) && (AIXV==12)
	/* AIX 1.x */
	rmslink(at->mntpt) == -1 &&
#endif
	unlink(at->mntpt) == -1)
	{
	    if(errno == ENOENT)
		{
		    fprintf(stderr, "%s: filesystem %s already detached\n",
			    progname, at->hesiodname);
		    return SUCCESS;
		}
	    fprintf(stderr, "%s: detach of filesystem %s failed, unable to remove mountpoint\n\terror is %s\n",
		    progname, at->hesiodname, errstr(errno));
	    /* Set error_status? */
	    return FAILURE;
	}
    return SUCCESS;
}

int afs_auth_to_cell(cell)
const char *cell;
{
    return(afs_auth_internal(cell, cell, 0, AFSAUTH_DOAUTH | AFSAUTH_CELL));
}

int afs_zinit(hesname, afsdir)
const char *hesname, *afsdir;
{
    return(afs_auth_internal(hesname, afsdir, 0, AFSAUTH_DOZEPHYR));
}
#endif	/* AFS */


