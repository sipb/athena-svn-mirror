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

/* This is detach, which is used to detach lockers from workstations. */

static const char rcsid[] = "$Id: detach.c,v 1.21.2.1 1999-07-30 22:08:28 ghudson Exp $";

#include <netdb.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <locker.h>
#include "attach.h"
#include "agetopt.h"

static void usage(void);
static void detach_all(locker_context context, int options);
static void detach_by_host(locker_context context, char *host, int options);
static int detach_attachent(locker_context context, locker_attachent *at,
			    void *optionsp);

static struct agetopt_option detach_options[] = {
  { "all", 'a', 0 },
  { "clean", 'C', 0 },
  { "debug", 'd', 0 },
  { "explicit", 'e', 0 },
  { "force", 'f', 0 },
  { "nozephyr", 'h', 0 },
  { "host", 'H', 0 },
  { "lint", 'L', 0 },
  { "nomap", 'n', 0 },
  { "override", 'O', 0 },
  { "quiet", 'q', 0 },
  { "spoofhost", 's', 1 },
  { "type", 't', 1 },
  { "user", 'U', 1 },
  { "verbose", 'v', 0 },
  { "noexplicit", 'x', 0 },
  { "unmap", 'y', 0 },
  { "zephyr", 'z', 0 },
  { 0, 0, 0 }
};

static int verbose = 1;

enum { DETACH_FILESYSTEM, DETACH_EXPLICIT, DETACH_BY_HOST };

int detach_main(int argc, char **argv)
{
  locker_context context;
  locker_attachent *at;
  int options = LOCKER_DETACH_DEFAULT_OPTIONS;
  char *type = "nfs";
  int mode = DETACH_FILESYSTEM, opt, gotname = 0;
  int status, estatus = 0;

  if (locker_init(&context, getuid(), NULL, NULL))
    exit(1);

  /* Wrap another while around the getopt so we can go through
   * multiple cycles of "[options] lockers...".
   */
  while (optind < argc)
    {
      while ((opt = attach_getopt(argc, argv, detach_options)) != -1)
	{
	  switch (opt)
	    {
	    case 'a':
	      /* backward compatibility: "detach -O -a" implies
	       * override, but not unlock.
	       */
	      detach_all(context, options & ~LOCKER_DETACH_OPT_UNLOCK);
	      gotname++;
	      break;

	    case 'C':
	      options |= LOCKER_DETACH_OPT_CLEAN;
	      break;

	    case 'e':
	      mode = DETACH_EXPLICIT;
	      break;

	    case 'h':
	      options &= ~LOCKER_DETACH_OPT_UNZEPHYR;
	      break;

	    case 'H':
	      mode = DETACH_BY_HOST;
	      break;

	    case 'n':
	      options &= ~LOCKER_DETACH_OPT_UNAUTH;
	      break;

	    case 'O':
	      options |= LOCKER_DETACH_OPT_OVERRIDE | LOCKER_DETACH_OPT_UNLOCK;
	      break;

	    case 'q':
	      verbose = 0;
	      break;

	    case 't':
	      type = optarg;
	      break;

	    case 'U':
	      if (getuid() != 0)
		{
		  fprintf(stderr, "%s: You are not allowed to use the "
			  "--user option.\n", whoami);
		  exit(1);
		}
	      else
		{
		  struct passwd *pw;

		  pw = getpwnam(optarg);
		  if (!pw)
		    {
		      fprintf(stderr, "%s: No such user %s.\n",
			      whoami, optarg);
		      exit(1);
		    }
		  locker_end(context);
		  if (locker_init(&context, pw->pw_uid, NULL, NULL))
		    exit(1);
		}
	      break;

	    case 'v':
	      verbose = 1;
	      break;

	    case 'x':
	      mode = DETACH_FILESYSTEM;
	      break;

	    case 'y':
	      options |= LOCKER_DETACH_OPT_UNAUTH;
	      break;

	    case 'z':
	      options |= LOCKER_DETACH_OPT_UNZEPHYR;
	      break;

	    case 'd':
	    case 'f':
	    case 'L':
	    case 's':
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
	    case DETACH_FILESYSTEM:
	      if (*argv[optind] == '/')
		{
		  status = locker_detach(context, NULL, argv[optind],
					 options, &at);
		}
	      else
		{
		  status = locker_detach(context, argv[optind], NULL,
					 options, &at);
		}
	      if (status == LOCKER_SUCCESS)
		{
		  if (verbose)
		    printf("%s: %s detached\n", whoami, at->name);
		  locker_free_attachent(context, at);
		}
	      else if (!LOCKER_DETACH_SUCCESS(status))
		estatus = 2;
	      break;

	    case DETACH_EXPLICIT:
	      status = locker_detach_explicit(context, type, argv[optind],
					      NULL, options, &at); 
	      if (status == LOCKER_SUCCESS)
		{
		  if (verbose)
		    printf("%s: %s detached\n", whoami, at->name);
		  locker_free_attachent(context, at);
		}
	      else if (!LOCKER_DETACH_SUCCESS(status))
		estatus = 2;
	      break;

	    case DETACH_BY_HOST:
	      detach_by_host(context, argv[optind], options);
	      break;
	    }

	  optind++;
	}
    }

  if (!gotname)
    usage();

  locker_do_zsubs(context, LOCKER_ZEPHYR_UNSUBSCRIBE);
  locker_end(context);
  exit(0);
}

static void detach_by_host(locker_context context, char *host, int options)
{
  struct hostent *h;

  h = gethostbyname(host);
  if (!h)
    {
      fprintf(stderr, "%s: Could not resolve hostname \"%s\".\n",
	      whoami, host);
      exit(1);
    }
  locker_iterate_attachtab(context, locker_check_host, &(h->h_addr),
			   detach_attachent, &options);
}

static int detach_attachent(locker_context context, locker_attachent *at,
                     void *optionsp)
{
  int status, options = *(int *)optionsp;

  status = locker_detach_attachent(context, at, options);
  if (status == LOCKER_SUCCESS && verbose)
    printf("%s: %s detached\n", whoami, at->name);
  return 0;
}

static void detach_all(locker_context context, int options)
{
  locker_iterate_attachtab(context, NULL, NULL, detach_attachent, &options);
}


static void usage(void)
{
  fprintf(stderr, "Usage: detach [options] filesystem ... [options] filesystem ...\n");
  fprintf(stderr, "       detach [options] mountpoint ...\n");
  fprintf(stderr, "       detach [options] -H host ...\n");
  fprintf(stderr, "       detach [options] -a\n");
  exit(1);
}
