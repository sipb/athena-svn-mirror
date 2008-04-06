/* $Id: locker_private.h,v 1.8 2006-08-08 21:50:10 ghudson Exp $ */

/* Copyright 1998 by the Massachusetts Institute of Technology.
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

#include <sys/types.h>
#include <regex.h>
#include <stdarg.h>

#define LOCKER_PATH_ATTACHTAB "/var/run/attachtab"
#define LOCKER_PATH_ATTACH_CONF "/etc/athena/attach.conf"
#define LOCKER_PATH_LOCAL "/var/athena/local-validated"

#define LOCKER_AFS_MOUNT_DIR "/mit"
#define LOCKER_UFS_MOUNT_DIR "/mnt"
#define LOCKER_LOC_MOUNT_DIR "/mit"

#define LOCKER_NFS_KSERVICE "rvdsrv"

#define LOCKER_DEFAULT_GID 101

#define LOCKER_MOUNT_TIMEOUT 20

#define LOCKER_ZEPHYR_CLASS "filsrv"

enum explstate { LOCKER_DONT_CARE, LOCKER_EXPLICIT, LOCKER_NOEXPLICIT };

union locker__datum {
  char *string;
  int flag;
};

struct locker__regexp {
  int fstypes;
  enum explstate explicit;
  regex_t pattern;
  union locker__datum data;
};

struct locker__regexp_list {
  struct locker__regexp *tab;
  int size, num;
  int defflag;
};

struct locker_context {
  /* attach.conf variables */
  int exp_desc, exp_mountpoint, keep_mount, nfs_root_hack, ownercheck;
  int use_krb4;
  char *afs_mount_dir, *attachtab, *nfs_mount_dir, *local_dir;
  struct locker__regexp_list allow, setuid, mountpoint;
  struct locker__regexp_list allowopts, defopts, filesystem, reqopts;

  /* Defined fs types */
  struct locker_ops **fstype;
  int nfstypes;

  /* error reporting function and its cookie */
  locker_error_fun errfun;
  void *errdata;

  /* user we're acting on behalf of, and whether or not s/he is trusted */
  uid_t user;
  int trusted;

  /* Hesiod context */
  void *hes_context;

  /* Queued zephyr subs */
  char **zsubs;
  int nzsubs;

  /* Number of locks held on attachtab */
  int locks;
};

/* "kind"s for locker__attachtab_pathname */
#define LOCKER_LOCK 0
#define LOCKER_NAME 1
#define LOCKER_MOUNTPOINT 2
#define LOCKER_DIRECTORY 3
/* Already a full path: don't touch it. */
#define LOCKER_FULL_PATH 4

/* options for locker__canonicalize_path */
#define LOCKER_CANON_CHECK_NONE 0
#define LOCKER_CANON_CHECK_MOST 1
#define LOCKER_CANON_CHECK_ALL 2

/* Prototypes from attachtab.c */
locker_attachent *locker__new_attachent(locker_context context,
					struct locker_ops *type);
int locker__lookup_attachent(locker_context context, char *name,
			     char *mountpoint, int create,
			     locker_attachent **atp);
int locker__lookup_attachent_explicit(locker_context context, char *type,
				      char *desc, char *mountpoint,
				      int create, locker_attachent **atp);
void locker__update_attachent(locker_context context, locker_attachent *at);
char *locker__attachtab_pathname(locker_context context,
				 int kind, char *name);

/* Prototypes from conf.c */
int locker__fs_ok(locker_context context, struct locker__regexp_list list,
		  struct locker_ops *fs, char *filesystem);
char *locker__fs_data(locker_context context, struct locker__regexp_list list,
		      struct locker_ops *fs, char *filesystem);
struct locker_ops *locker__get_fstype(locker_context context, char *fstype);
void locker__error(locker_context context, char *fmt, ...);

/* Prototypes from mount.c */
int locker__mount(locker_context context, locker_attachent *at,
		  char *mountoptions);
int locker__unmount(locker_context context, locker_attachent *at);

/* Prototypes from mountpoint.c */
int locker__canonicalize_path(locker_context context, int check,
			      char **pathp, char **extp);
int locker__build_mountpoint(locker_context context, locker_attachent *at);
int locker__remove_mountpoint(locker_context context, locker_attachent *at);
void locker__put_mountpoint(locker_context context, locker_attachent *at);

/* Prototypes from util.c */
int locker__read_line(FILE *fp, char **buf, int *bufsize);

/* Prototypes from zephyr.c */
int locker__add_zsubs(locker_context context, char **subs, int nsubs);
void locker__free_zsubs(locker_context context);
