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

/* This contains the AFS quota-checking routines. */

static const char rcsid[] = "$Id: afs.c,v 1.15 1999-03-29 19:14:14 danw Exp $";

#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include <afs/param.h>
#include <afs/vice.h>
#include <afs/venus.h>
#include <afs/afsint.h>

#include "quota.h"

int get_afs_quota(struct quota_fs *fs, uid_t uid, int verbose)
{
  static struct VolumeStatus vs;
  struct ViceIoctl ibuf;
  int code;
  uid_t euid = geteuid();

  ibuf.out_size = sizeof(struct VolumeStatus);
  ibuf.in_size = 0;
  ibuf.out = (caddr_t)&vs;
  seteuid(getuid());
  code = pioctl(fs->device, VIOCGETVOLSTAT, &ibuf, 1);
  seteuid(euid);
  if (code)
    {
      if (verbose || ((errno != EACCES) && (errno != EPERM)))
	{
	  fprintf(stderr, "Error getting AFS quota: ");
	  perror(fs->device);
	}
      return -1;
    }
  else
    {
      fs->have_quota = fs->have_blocks = 1;
      fs->have_files = 0;
      memset(&fs->dqb, 0, sizeof(fs->dqb));

      /* Need to multiply by 2 to get 512 byte blocks instead of 1k */
      fs->dqb.dqb_curblocks = vs.BlocksInUse * 2;
      fs->dqb.dqb_bsoftlimit = fs->dqb.dqb_bhardlimit = vs.MaxQuota * 2;

      if (vs.MaxQuota && (vs.BlocksInUse >= vs.MaxQuota * 9 / 10))
	fs->warn_blocks = 1;
      return 0;
    }
}

void print_afs_warning(struct quota_fs *fs)
{
  if (fs->dqb.dqb_curblocks > fs->dqb.dqb_bhardlimit)
    {
      printf("Over disk quota on %s, remove %dK.\n", fs->mount,
	     (fs->dqb.dqb_curblocks - fs->dqb.dqb_bhardlimit) / 2);
    }
  else if (fs->dqb.dqb_curblocks > (fs->dqb.dqb_bhardlimit * 9 / 10))
    {
      printf("%d%% of the disk quota on %s has been used.\n",
	     (int)((fs->dqb.dqb_curblocks * 100.0 /
		    fs->dqb.dqb_bhardlimit) + 0.5),
	     fs->mount);
    }
}
