/* Copyright 1999 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

/* Routines for finding mounted/attached filesystems */

static const char rcsid[] = "$Id: fslist.c,v 1.3 1999-10-07 16:59:29 rbasch Exp $";

/* There are two basic ways of reading the mounted-filesystems table:
 * the 4.3+BSD way (getfsstat), and the old way (reading a file in
 * /etc). NetBSD and OSF/1 differ in how you determine the type of
 * filesystems returned by getfsstat. The non-BSD systems differ in
 * what the file in /etc is called, and how much of an interface they
 * provide for reading it. Solaris defines a function getmntent which
 * is not the same as Irix and Linux's, so we #define around this.
 */

#include <sys/types.h>
#include <stdio.h>

#ifdef IRIX
#include <mntent.h>
#define MOUNTTAB MOUNTED
#define GETMNTENT getmntent
#endif

#ifdef SOLARIS
#include <sys/mnttab.h>

#define MOUNTTAB MNTTAB
#define GETMNTENT solaris_compat_getmntent
#define setmntent fopen
#define endmntent fclose

struct mntent {
  char *mnt_fsname;
  char *mnt_dir;
  char *mnt_type;
};

static struct mntent *solaris_compat_getmntent(FILE *);
#endif

#ifdef LINUX
#include <mntent.h>
#define MOUNTTAB _PATH_MOUNTED
#define GETMNTENT getmntent
#endif

#ifdef NETBSD
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#define HAVE_GETFSSTAT
#endif

#ifdef OSF
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/fs_types.h>
#define HAVE_GETFSSTAT
#endif


#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <locker.h>
#include "quota.h"

extern char **fsnames;
extern locker_context context;

static int numfs, fssize;
static int check_fs(locker_context context, locker_attachent *at, void *arg);
static int add_fs(locker_context context, locker_attachent *at, void *arg);
static int okname(char *name, char **list);

struct quota_fs *get_fslist(uid_t uid)
{
  struct quota_fs *fslist = NULL;

#ifdef HAVE_GETFSSTAT

  struct statfs *mntbuf;
  int i;

  fssize = getfsstat(NULL, 0, 0);
  if (fssize)
    {
      mntbuf = malloc(fssize * sizeof(struct statfs));
      if (!mntbuf)
	{
	  fprintf(stderr, "quota: Out of memory.\n");
	  exit(1);
	}
      fssize = getfsstat(mntbuf, fssize * sizeof(struct statfs), MNT_WAIT);
    }
  if (!fssize)
    {
      fprintf(stderr, "quota: Could not get list of mounted filesystems: %s\n",
	      strerror(errno));
      exit(1);
    }

  fslist = malloc(fssize * sizeof(struct quota_fs));
  if (!fslist)
    {
      fprintf(stderr, "quota: Out of memory.\n");
      exit(1);
    }
  memset(fslist, 0, fssize * sizeof(struct quota_fs));

  for (i = numfs = 0; i < fssize; i ++)
    {
      if ((
#ifdef NETBSD
	   !strcmp(mntbuf[i].f_fstypename, "ffs") ||
	   !strcmp(mntbuf[i].f_fstypename, "lfs") ||
	   !strcmp(mntbuf[i].f_fstypename, "nfs")
#endif
#ifdef OSF
	   !strcmp(mnt_names[mntbuf[i].f_type], "ufs") ||
	   !strcmp(mnt_names[mntbuf[i].f_type], "nfs")
#endif
	   ) && okname(mntbuf[i].f_mntonname, fsnames))
	{
	  fslist[numfs].mount = strdup(mntbuf[i].f_mntonname);
	  fslist[numfs].device = strdup(mntbuf[i].f_mntfromname);
#ifdef NETBSD
	  fslist[numfs++].type = strdup(mntbuf[i].f_fstypename);
#endif
#ifdef OSF
	  fslist[numfs++].type = strdup(mnt_names[mntbuf[i].f_type]);
#endif
	}
    }
  free(mntbuf);

#else /* HAVE_GETFSSTAT */

  struct mntent *mnt;
  FILE *mtab;

  mtab = setmntent(MOUNTTAB, "r");
  while ((mnt = GETMNTENT(mtab)) != NULL)
    {
      if ((
#ifdef IRIX
	   !strcmp(mnt->mnt_type, "xfs") ||
	   !strcmp(mnt->mnt_type, "efs") ||
#endif
#ifdef SOLARIS
	   !strcmp(mnt->mnt_type, "ufs") ||
#endif
#ifdef LINUX
	   !strcmp(mnt->mnt_type, "e2fs") ||
#endif
	   !strcmp(mnt->mnt_type, "nfs"))
	  && okname(mnt->mnt_dir, fsnames))
	{
	  if (numfs == fssize)
	    {
	      fssize = 2 * (fssize + 1);
	      fslist = realloc(fslist, fssize * sizeof(struct quota_fs));
	      if (!fslist)
		{
		  fprintf(stderr, "quota: Out of memory.\n");
		  exit(1);
		}
	    }
	  memset(&fslist[numfs], 0, sizeof(struct quota_fs));
	  fslist[numfs].device = strdup(mnt->mnt_fsname);
	  fslist[numfs].mount = strdup(mnt->mnt_dir);
	  fslist[numfs++].type = strdup(mnt->mnt_type);
	}
    }
  endmntent(mtab);

#endif /* HAVE_GETFSSTAT */

  /* Now get AFS filesystems, */
  locker_iterate_attachtab(context, check_fs, uid ? &uid : NULL,
			   add_fs, &fslist);

  fslist = realloc(fslist, (numfs + 1) * sizeof(struct quota_fs));
  if (!fslist)
    {
      fprintf(stderr, "quota: Out of memory.\n");
      exit(1);
    }
  fslist[numfs].device = fslist[numfs].mount = fslist[numfs].type = NULL;

  return fslist;
}

/* Check an attachtab filesystem to see if we want to remember it. */
static int check_fs(locker_context context, locker_attachent *at, void *arg)
{
  /* Check in list. */
  if (!okname(at->mountpoint, fsnames))
    return 0;

  /* Check type. */
  if (strcmp(at->fs->name, "AFS") != 0)
    return 0;

  /* If listing all, or this was explicitly specified, then it's ok. */
  if (!arg || fsnames)
    return 1;

  /* Else check owners. */
  if (!locker_check_owner(context, at, arg))
    return 0;

  /* Check access */
  return access(at->mountpoint, W_OK) == 0;
}

static int add_fs(locker_context context, locker_attachent *at, void *arg)
{
  struct quota_fs **fslistp = arg;

  if (numfs == fssize)
    {
      fssize = 2 * (fssize + 1);
      *fslistp = realloc(*fslistp, fssize * sizeof(struct quota_fs));
      if (!*fslistp)
	{
	  fprintf(stderr, "quota: Out of memory.\n");
	  exit(1);
	}
    }
  memset(&(*fslistp)[numfs], 0, sizeof(struct quota_fs));
  (*fslistp)[numfs].device = strdup(at->hostdir);
  (*fslistp)[numfs].mount = strdup(at->mountpoint);
  (*fslistp)[numfs++].type = strdup(at->fs->name);

  return 0;
}

/* Check if a filesystem mountpoint is in the list to check. */
static int okname(char *name, char **list)
{
  int i;

  if (!list)
    return 1;

  for (i = 0; list[i]; i++)
    {
      if (!strcmp(name, list[i]))
	return 1;
    }
  return 0;
}

#ifdef SOLARIS
/* Solaris has a mostly compatible getmntent that just handles its
 * arguments and names its structure elements differently.
 */
static struct mntent *solaris_compat_getmntent(FILE *mtab)
{
  struct mnttab mnt_sol;
  static struct mntent mnt_compat;
  
  if (getmntent(mtab, &mnt_sol) == 0)
    {
      mnt_compat.mnt_fsname = mnt_sol.mnt_special;
      mnt_compat.mnt_dir = mnt_sol.mnt_mountp;
      mnt_compat.mnt_type = mnt_sol.mnt_fstype;
      return &mnt_compat;
    }
  else
    return NULL;
}
#endif
