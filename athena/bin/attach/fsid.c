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

/* This is fsid, which is used to authenticate/unauthenticate to
 * lockers.
 */

static const char rcsid[] = "$Id: fsid.c,v 1.7 1999-09-19 23:51:03 danw Exp $";

#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <locker.h>
#include "attach.h"
#include "agetopt.h"

static void usage(void);
static int fsid_attachent(locker_context context, locker_attachent *at,
			  void *opp);
static char *opped(int op);

static struct agetopt_option fsid_options[] = {
  { "all", 'a', 0 },
  { "cell", 'c', 0 },
  { "debug", 'd', 0 },
  { "filsys", 'f', 0 },
  { "host", 'h', 0 },
  { "map", 'm', 0 },
  { "purge", 'p', 0 },
  { "quiet", 'q', 0 },
  { "purgeuser", 'r', 0 },
  { "user", 'U', 1 },
  { "verbose", 'v', 0 },
  { "unmap", 'u', 0 },
  { 0, 0, 0 }
};

static int verbose = 1;

enum { FSID_WHATEVER, FSID_FILESYSTEM, FSID_HOST, FSID_CELL };

int fsid_main(int argc, char **argv)
{
  locker_context context;
  int mode = FSID_WHATEVER, op = LOCKER_AUTH_AUTHENTICATE;
  struct hostent *h;
  int status, estatus = 0, opt, gotname = 0;
  uid_t uid = getuid();

  if (locker_init(&context, uid, NULL, NULL))
    exit(1);

  while (optind < argc)
    {
      while ((opt = attach_getopt(argc, argv, fsid_options)) != -1)
	{
	  switch (opt)
	    {
	    case 'a':
	      if (op == LOCKER_AUTH_PURGE ||
		  op == LOCKER_AUTH_PURGEUSER)
		{
		  locker_iterate_attachtab(context, NULL, NULL,
					   fsid_attachent, &op);
		}
	      else
		{
		  locker_iterate_attachtab(context, locker_check_owner, &uid,
					   fsid_attachent, &op);
		}
	      gotname++;
	      break;

	    case 'c':
	      mode = FSID_CELL;
	      break;

	    case 'f':
	      mode = FSID_FILESYSTEM;
	      break;

	    case 'h':
	      mode = FSID_HOST;
	      break;

	    case 'm':
	      op = LOCKER_AUTH_AUTHENTICATE;
	      break;

	    case 'p':
	      op = LOCKER_AUTH_PURGE;
	      break;

	    case 'q':
	      verbose = 0;
	      break;

	    case 'r':
	      op = LOCKER_AUTH_PURGEUSER;
	      break;

	    case 'u':
	      op = LOCKER_AUTH_UNAUTHENTICATE;
	      break;

	    case 'v':
	      verbose = 1;
	      break;

	    case 'd':
	    case 'U':
	      fprintf(stderr, "%s: The '%c' flag is no longer supported.\n",
		      whoami, opt);
	      break;

	    default:
	      usage();
	    }
	}

      while (optind < argc && argv[optind][0] != '-')
	{
	  gotname++;
	  switch (mode)
	    {
	    case FSID_WHATEVER:
	    case FSID_HOST:
	      h = gethostbyname(argv[optind]);
	      if (h)
		{
		  status = locker_auth_to_host(context, whoami,
					       argv[optind], op);
		  if (status != LOCKER_SUCCESS)
		    estatus = 2;
		  break;
		}
	      else if (mode == FSID_HOST)
		{
		  fprintf(stderr, "%s: Could not resolve hostname \"%s\".\n",
			  whoami, argv[optind]);
		  estatus = 2;
		}
	      /* else if (mode == FSID_WHATEVER), fall through */

	    case FSID_FILESYSTEM:
	      status = locker_auth(context, argv[optind], op);
	      if (status != LOCKER_SUCCESS)
		estatus = 2;
	      break;

	    case FSID_CELL:
	      status = locker_auth_to_cell(context, whoami, argv[optind], op);
	      if (status != LOCKER_SUCCESS)
		estatus = 2;
	      break;
	    }

	  if (verbose && status == LOCKER_SUCCESS)
	    printf("%s: %s %s\n", whoami, argv[optind], opped(op));

	  optind++;
	}
    }

  if (!gotname)
    usage();
  locker_end(context);
  exit(estatus);
}

static int fsid_attachent(locker_context context, locker_attachent *at,
			  void *opp)
{
  int status;

  status = at->fs->auth(context, at, LOCKER_AUTH_DEFAULT, *(int *)opp);
  if (verbose && status == LOCKER_SUCCESS)
    printf("%s: %s %s\n", whoami, at->name, opped(*(int *)opp));
  return 0;
}

static char *opped(int op)
{
  switch (op)
    {
    case LOCKER_AUTH_AUTHENTICATE:
      return "mapped";
    case LOCKER_AUTH_UNAUTHENTICATE:
      return "unmapped";
    case LOCKER_AUTH_PURGE:
      return "purged";
    case LOCKER_AUTH_PURGEUSER:
      return "user-purged";
    default:
      return "(unknown)";
    }
}

static void usage(void)
{
  fprintf(stderr, "Usage: fsid [-q | -v] [-m | -p | -r | -u] [ filesystem | host ] ...\n");
  fprintf(stderr, "       fsid [-q | -v] [-m | -p | -r | -u] -f filesystem ...\n");
  fprintf(stderr, "       fsid [-q | -v] [-m | -p | -r | -u] -h host ...\n");
  fprintf(stderr, "       fsid [-q | -v] [-m      |      -u] -c cell ...\n");
  fprintf(stderr, "       fsid [-q | -v] [-m | -p | -r | -u] -a\n");
  exit(1);
}
