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

/* This is quota, which abstracts quota-checking mechanisms for
 * various types of lockers.
 */

static const char rcsid[] = "$Id: quota.c,v 1.24.4.1 1999-11-09 16:11:46 ghudson Exp $";

#include <ctype.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <locker.h>
#include "quota.h"

char **fsnames;
locker_context context;

static void usage(void);
static void print_quota(struct quota_fs *fs);
static void heading(uid_t uid, char *name);
static int alldigits(char *s);

int main(int argc, char **argv)
{
  int opt, i, status;
  struct passwd *pw;
  uid_t myuid;
  int all = 0;
  char *user;
  uid_t uid;
  int fsind = 0, fssize = 0, heading_printed = 0;
  int verbose = 0;
  struct quota_fs *fslist = NULL;
  sigset_t mask;

  myuid = getuid();
  if (locker_init(&context, myuid, NULL, NULL) != LOCKER_SUCCESS)
    {
      fprintf(stderr, "quota: Could not initialize locker library.\n");
      exit(1);
    }

  /* Block ^Z to prevent holding locks on the attachtab. */
  sigemptyset(&mask);
  sigaddset(&mask, SIGTSTP);
  sigaddset(&mask, SIGTTOU);
  sigaddset(&mask, SIGTTIN);
  sigprocmask(SIG_BLOCK, &mask, NULL);

  while ((opt = getopt(argc, argv, "af:guv")) != -1)
    {
      switch (opt)
	{
	case 'a':
	  all = 1;
	  break;

	case 'v':
	  verbose = 1;
	  break;

	case 'g':
	  fprintf(stderr, "quota: Group quotas no longer supported.\n");
	  exit(1);
	  break;

	case 'u':
	  /* Backward-compatibility option. */
	  verbose = 1;
	  break;

	case 'f':
	  if (fsind == fssize)
	    {
	      fssize = 2 * (fssize + 1);
	      fsnames = realloc(fsnames, fssize);
	      if (!fsnames)
		{
		  fprintf(stderr, "quota: Out of memory.\n");
		  exit(1);
		}
	    }

	  if (!strchr(optarg, '/'))
	    {
	      locker_attachent *at;

	      if (locker_read_attachent(context, optarg, &at) != 
		  LOCKER_SUCCESS)
		{
		  fprintf(stderr, "quota: Unknown filesystem %s.\n", optarg);
		  exit(1);
		}
	      fsnames[fsind++] = strdup(at->mountpoint);
	      locker_free_attachent(context, at);
	    }
	  else
	    fsnames[fsind++] = optarg;
	  break;

	default:
	  fprintf(stderr, "quota: %s: unknown option\n", argv[optind - 1]);
	  usage();
	  exit(1);
	}
    }

  if (all && fsind)
    {
      fprintf(stderr, "quota: Can't use both -a and -f\n");
      usage();
      exit(1);
    }

  if (optind < argc)
    {
      if (optind < argc - 1)
	{
	  fprintf(stderr, "quota: Can only specify a single user.\n");
	  exit(1);
	}

      if (alldigits(argv[optind]))
	{
	  uid = atoi(argv[optind]);

	  pw = getpwuid(uid);
	  if (pw)
	    user = strdup(pw->pw_name);
	  else
	    user = "(no account)";
	}
      else
	{
	  user = argv[optind];

	  pw = getpwnam(user);
	  if (pw)
	    uid = pw->pw_uid;
	  else
	    {
	      fprintf(stderr, "quota: No passwd entry for user %s.\n", user);
	      exit(1);
	    }
	}

      /* Check permission if not root. */
      if (myuid != 0 && uid != myuid)
	{
	  fprintf(stderr, "quota: %s (uid %lu): permission denied\n",
		  user, (unsigned long) uid);
	  exit(1);
	}
    }
  else
    {
      /* Use real uid. */
      uid = myuid;
      pw = getpwuid(myuid);
      if (!pw)
	{
	  fprintf(stderr, "quota: Could not get password entry for uid %lu.\n",
		  (unsigned long) myuid);
	  exit(1);
	}
      user = strdup(pw->pw_name);
    }

  if (uid == 0)
    {
      if (verbose)
	printf("no disk quota for %s (uid 0)\n", user);
      exit(0);
    }

  fslist = get_fslist(all ? 0 : uid);

  /* Now print quotas */
  for (i = 0; fslist[i].type; i++)
    {
      if (!strcasecmp(fslist[i].type, "afs"))
	status = get_afs_quota(&fslist[i], uid, verbose);
      else if (!strcasecmp(fslist[i].type, "nfs"))
	status = get_nfs_quota(&fslist[i], uid, verbose);
      else
	status = get_local_quota(&fslist[i], uid, verbose);

      if (!status && fslist[i].have_quota && verbose)
	{
	  if (!heading_printed)
	    {
	      heading(uid, user);
	      heading_printed = 1;
	    }
	  print_quota(&fslist[i]);
	}
    }
  printf("\n");

  for (i = 0; fslist[i].type; i++)
    {
      if (fslist[i].warn_blocks || fslist[i].warn_files)
	{
	  if (!strcasecmp(fslist[i].type, "afs"))
	    print_afs_warning(&fslist[i]);
	  else
	    print_mounted_warning(&fslist[i]);
	}
    }

  exit(0);
}

static void heading(uid_t uid, char *name)
{
  printf("Disk quotas for %s (uid %lu):\n", name, (unsigned long) uid);
  printf("%-16s %8s %8s %8s    %8s %8s %8s\n",
	 "Filesystem",
	 "usage", "quota", "limit",
	 "files", "quota", "limit");
}

static void print_quota(struct quota_fs *fs)
{
  /* Ignore all-zero quotas */
  if (!fs->dqb.dqb_bsoftlimit && !fs->dqb.dqb_bhardlimit
      && !fs->dqb.dqb_curblocks && !fs->dqb.dqb_fsoftlimit
      && !fs->dqb.dqb_fhardlimit && !fs->dqb.dqb_curfiles)
    return;

  if (strlen(fs->mount) > 16)
    printf("%s\n%-16s ", fs->mount, "");
  else
    printf("%-16s ", fs->mount);

  if (fs->have_blocks)
    {
      printf("%8u %8u %8u %2s ",
	     fs->dqb.dqb_curblocks / 2,
	     fs->dqb.dqb_bsoftlimit / 2,
	     fs->dqb.dqb_bhardlimit / 2,
	     fs->warn_blocks ? "<<" : "");
    }
  else
    printf("%30s", "");

  if (fs->have_files)
    {
      printf("%8u %8u %8u %2s ",
	     fs->dqb.dqb_curfiles,
	     fs->dqb.dqb_fsoftlimit,
	     fs->dqb.dqb_fhardlimit,
	     fs->warn_files ? "<<" : "");
    }

  printf("\n");
}

static int alldigits(char *s)
{
  int c;

  c = *s++;
  do {
    if (!isdigit(c))
      return 0;
  } while ((c = *s++));
  return 1;
}

static void usage(void)
{
  fprintf(stderr, "Usage: quota [-v] [-f filesystem...] [-u] [user]\n");
  exit(1);
}
