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

/* Routines for retrieving local filesystem quotas. */

static const char rcsid[] = "$Id: ufs.c,v 1.1 1999-03-29 19:14:21 danw Exp $";

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "quota.h"

static void print_timeout(time_t time);
static int os_quotactl(struct quota_fs *fs, uid_t uid);

int get_local_quota(struct quota_fs *fs, uid_t uid, int verbose)
{
  int status;
#ifdef IRIX
  struct fs_disk_quota fdq;

  /* Irix uses a different quota structure for XFS only. */
  if (!strcmp(fs->type, "xfs"))
    {
      status = quotactl(Q_XGETQUOTA, fs->device, uid, (caddr_t)&fdq);
      if (status == 0)
	{
	  fs->dqb.dqb_curblocks = fdq.d_bcount;
	  fs->dqb.dqb_bsoftlimit = fdq.d_blk_softlimit;
	  fs->dqb.dqb_bhardlimit = fdq.d_blk_hardlimit;
	  fs->dqb.dqb_btimelimit = fdq.d_btimer;
	  fs->dqb.dqb_curfiles = fdq.d_icount;
	  fs->dqb.dqb_fsoftlimit = fdq.d_ino_softlimit;
	  fs->dqb.dqb_fhardlimit= fdq.d_ino_hardlimit;
	  fs->dqb.dqb_ftimelimit = fdq.d_itimer;
	  fs->have_quota = fs->have_files = fs->have_blocks = 1;
	}
    }
  else
#endif /* IRIX */
    {
      status = os_quotactl(fs, uid);
      if (status == 0)
	fs->have_quota = fs->have_files = fs->have_blocks = 1;
    }

  if (fs->have_quota)
    {
      if (fs->dqb.dqb_btimelimit)
	fs->warn_blocks = 1;
      if (fs->dqb.dqb_ftimelimit)
	fs->warn_files = 1;
    }

  /* Give an error message for anything but ESRCH ("Quotas not enabled
   * for this filesystem.") or EOPNOTSUPP ("No kernel support for
   * quotas.")
   */
  if (status == -1 && errno != ESRCH && errno != EOPNOTSUPP)
    {
      fprintf(stderr, "quota: Could not get quota for %s (%s): %s\n",
	      fs->mount, fs->device, strerror(errno));
      return -1;
    }

  return 0;
}

#ifndef SOLARIS

int os_quotactl(struct quota_fs *fs, uid_t uid)
{
#ifdef USRQUOTA
  return quotactl(QCMD(Q_GETQUOTA, USRQUOTA), fs->device, uid,
		  (caddr_t)&fs->dqb);
#else
  return quotactl(Q_GETQUOTA, fs->device, uid, (caddr_t)&fs->dqb);
#endif
}

#else

int os_quotactl(struct quota_fs *fs, uid_t uid)
{
  int fd, status;
  char *path;
  struct quotctl qctl;

  path = malloc(strlen(fs->mount) + 8);
  if (!path)
    return -1;
  sprintf(path, "%s/quotas", fs->mount);

  fd = open(path, O_RDONLY);
  free(path);
  if (fd == -1)
    {
      if (errno == ENOENT)
	errno = ESRCH;
      return -1;
    }

  qctl.op = Q_GETQUOTA;
  qctl.uid = uid;
  qctl.addr = (caddr_t)&fs->dqb;
  status = ioctl(fd, Q_QUOTACTL, &qctl);
  close(fd);
  return status;
}
#endif

/* This is used for both UFS and NFS. */
void print_mounted_warning(struct quota_fs *fs)
{
  time_t now = time(NULL);

  if (fs->dqb.dqb_bhardlimit &&
      (fs->dqb.dqb_curblocks >= fs->dqb.dqb_bhardlimit))
    printf("Block limit reached on %s\n", fs->mount);
  else if (fs->dqb.dqb_bsoftlimit &&
	   (fs->dqb.dqb_curblocks >= fs->dqb.dqb_bsoftlimit))
    {
      printf("Over disk quota on %s, remove %dK", fs->mount,
	     (fs->dqb.dqb_curblocks - fs->dqb.dqb_bsoftlimit + 1) / 2);

      if (fs->dqb.dqb_btimelimit > now)
	print_timeout(fs->dqb.dqb_btimelimit - now);
      else if (fs->dqb.dqb_btimelimit != 0)
	printf(" immediately");
      printf("\n");
    }

  if (fs->dqb.dqb_fhardlimit &&
      (fs->dqb.dqb_curfiles >= fs->dqb.dqb_fhardlimit))
    printf("File count limit reached on %s\n", fs->mount);
  else if (fs->dqb.dqb_fsoftlimit &&
	   (fs->dqb.dqb_curfiles >= fs->dqb.dqb_fsoftlimit))
    {
      int remfiles = fs->dqb.dqb_curfiles - fs->dqb.dqb_fsoftlimit + 1;

      printf("Over file quota on %s, remove %d file%s",
	      fs->mount, remfiles, remfiles > 1 ? "s" : "");

      if (fs->dqb.dqb_ftimelimit > now)
	print_timeout(fs->dqb.dqb_ftimelimit - now);
      else if (fs->dqb.dqb_ftimelimit != 0)
	printf(" immediately");
      printf("\n");
    }
}

static void print_timeout(time_t time)
{
  int i;
  static struct {
    int c_secs;				/* conversion units in secs */
    char *c_str;			/* unit string */
  } cunits [] = {
    {60 * 60 * 24 * 7 * 4, "months"},
    {60 * 60 * 24 * 7, "weeks"},
    {60 * 60 * 24, "days"},
    {60 * 60, "hours"},
    {60, "mins"},
    {1, "secs"}
  };

  for (i = 0; i < sizeof(cunits) / sizeof(cunits[0]); i++)
    {
      if (time >= cunits[i].c_secs)
	break;
    }
  printf(" within %.1f %s", (double)time / cunits[i].c_secs, cunits[i].c_str);
}
