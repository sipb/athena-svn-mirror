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

/* This is zinit, which is run by zwgc to get subs for lockers that
   were attached before zwgc started. */

static const char rcsid[] = "$Id: zinit.c,v 1.3 1999-03-23 18:24:40 danw Exp $";

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <locker.h>
#include "attach.h"
#include "agetopt.h"

static void usage(void);
static int zinit_attachent(locker_context context, locker_attachent *at,
			   void *opp);

static struct agetopt_option zinit_options[] = {
  { "all", 'a', 0 },
  { "debug", 'd', 0 },
  { "me", 'm', 0 },
  { "quiet", 'q', 0 },
  { "verbose", 'v', 0 },
  { 0, 0, 0 }
};

int zinit_main(int argc, char **argv)
{
  locker_context context;
  int opt, all = 0, op = LOCKER_ZEPHYR_SUBSCRIBE;
  uid_t uid = getuid();

  if (locker_init(&context, uid, NULL, NULL))
    exit(1);

  while ((opt = attach_getopt(argc, argv, zinit_options)) != -1)
    {
      switch (opt)
	{
	case 'a':
	  all = 1;
	  break;

	case 'm':
	  all = 0;
	  break;

	case 'q':
	case 'v':
	case 'd':
	  fprintf(stderr, "%s: The '%c' flag is no longer supported.\n",
		  whoami, opt);
	  break;

	default:
	  usage();
	}
    }

  if (optind != argc)
    usage();

  if (all)
    locker_iterate_attachtab(context, NULL, NULL, zinit_attachent, &op);
  else
    {
      locker_iterate_attachtab(context, locker_check_owner, &uid,
			       zinit_attachent, &op);
    }
  return 0;
}

static int zinit_attachent(locker_context context, locker_attachent *at,
			   void *opp)
{
  return at->fs->zsubs(context, at, *(int *)opp);
}

static void usage(void)
{
  fprintf(stderr, "Usage: zinit [-a | -m]\n");
  exit(1);
}
