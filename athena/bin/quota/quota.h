/* $Id: quota.h,v 1.1 1999-03-29 19:14:20 danw Exp $ */

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

/* Include the appropriate files to get the definition of a
 * struct dqblk (and the quotactl function).
 */

#ifdef IRIX
#include <sys/types.h>
#include <sys/quota.h>
#endif

#ifdef SOLARIS
#include <sys/fs/ufs_quota.h>
#endif

#ifdef LINUX
#include <linux/quota.h>
#endif

#ifdef NETBSD
#include <ufs/ufs/quota.h>
#endif

#ifdef OSF
#include <ufs/quota.h>
#endif

struct quota_fs {
  char *device;		/* Device special file, or /afs pathname */
  char *mount;		/* Mountpoint */
  char *type;		/* Filesystem type */
  int have_quota;	/* User has a quota for this filesystem */
  int have_blocks;	/* Filesystem has block quotas */
  int warn_blocks;	/* User is near or over block quota */
  int have_files;	/* Filesystem has file quotas */
  int warn_files;	/* User is near or over file quota */
  struct dqblk dqb;	/* Quota details */
};

struct quota_fs *get_fslist(uid_t uid);

int get_afs_quota(struct quota_fs *fs, uid_t uid, int verbose);
int get_nfs_quota(struct quota_fs *fs, uid_t uid, int verbose);
int get_local_quota(struct quota_fs *fs, uid_t uid, int verbose);

void print_afs_warning(struct quota_fs *fs);
void print_mounted_warning(struct quota_fs *fs);
