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

/* This is attach, which is used to attach lockers to workstations. */

static const char rcsid[] = "$Id: attach.c,v 1.27 1999-03-24 16:02:33 danw Exp $";

#include <netdb.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <locker.h>
#include "attach.h"
#include "agetopt.h"

static void usage(void);
static void attach_list(locker_context context, char *host);
static int print_callback(locker_context context, locker_attachent *at,
			  void *val);
static void attach_print_entry(char *fs, char *mp, char *user, char *mode);
static int attach_print(locker_context context, locker_attachent *at,
			void *data);
static void attach_lookup(locker_context context, char *filesystem);

static struct agetopt_option attach_options[] = {
  { "noremap", 'a', 0 },
  { "debug", 'd', 0 },
  { "explicit", 'e', 0 },
  { "force", 'f', 0 },
  { "skipfsck", 'F', 0 },
  { "remap", 'g', 0 },
  { "nozephyr", 'h', 0 },
  { "host", 'H', 0 },
  { "lookup", 'l', 0 },
  { "lock", 'L', 0 },
  { "mountpoint", 'm', 1 },
  { "nomap", 'n', 0 },
  { "nosetuid", 'N', 0 },
  { "nosuid", 'N', 0 },
  { "mountoptions", 'o', 1 },
  { "override", 'O', 0 },
  { "printpath", 'p', 0 },
  { "quiet", 'q', 0 },
  { "readonly", 'r', 0 },
  { "spoofhost", 's', 1 },
  { "setuid", 'S', 0 },
  { "suid", 'S', 0 },
  { "type", 't', 1 },
  { "user", 'U', 1 },
  { "verbose", 'v', 0 },
  { "write", 'w', 0 },
  { "noexplicit", 'x', 0 },
  { "map", 'y', 0 },
  { "zephyr", 'z', 0 },
  { 0, 0, 0 }
};

locker_callback attach_callback = print_callback;

enum { ATTACH_FILESYSTEM, ATTACH_EXPLICIT, ATTACH_LOOKUP, ATTACH_LIST_HOST };
enum { ATTACH_QUIET, ATTACH_VERBOSE, ATTACH_PRINTPATH };

int attach_main(int argc, char **argv)
{
  locker_context context;
  locker_attachent *at;
  int options = LOCKER_ATTACH_DEFAULT_OPTIONS;
  char *type = "nfs", *mountpoint = NULL, *mountoptions = NULL;
  int mode = ATTACH_FILESYSTEM, auth = LOCKER_AUTH_DEFAULT;
  int output = ATTACH_VERBOSE, opt, gotname = 0;
  int status, estatus = 0;

  if (locker_init(&context, getuid(), NULL, NULL))
    exit(1);

  /* Wrap another while around the getopt so we can go through
   * multiple cycles of "[options] lockers...".
   */
  while (optind < argc)
    {
      while ((opt = attach_getopt(argc, argv, attach_options)) != -1)
	{
	  switch (opt)
	    {
	    case 'a':
	      options &= ~LOCKER_ATTACH_OPT_REAUTH;
	      break;

	    case 'e':
	      mode = ATTACH_EXPLICIT;
	      break;

	    case 'g':
	      options |= LOCKER_ATTACH_OPT_REAUTH;
	      break;

	    case 'h':
	      options &= ~LOCKER_ATTACH_OPT_ZEPHYR;
	      break;

	    case 'H':
	      mode = ATTACH_LIST_HOST;
	      break;

	    case 'l':
	      mode = ATTACH_LOOKUP;
	      break;

	    case 'L':
	      options |= LOCKER_ATTACH_OPT_LOCK;
	      break;

	    case 'm':
	      mountpoint = optarg;
	      break;

	    case 'n':
	      auth = LOCKER_AUTH_NONE;
	      break;

	    case 'N':
	      options &= ~LOCKER_ATTACH_OPT_ALLOW_SETUID;
	      break;

	    case 'o':
	      mountoptions = optarg;
	      break;

	    case 'O':
	      options |= LOCKER_ATTACH_OPT_OVERRIDE;
	      break;

	    case 'p':
	      output = ATTACH_PRINTPATH;
	      break;

	    case 'q':
	      output = ATTACH_QUIET;
	      break;

	    case 'r':
	      auth = LOCKER_AUTH_READONLY;
	      break;

	    case 'S':
	      options |= LOCKER_ATTACH_OPT_ALLOW_SETUID;
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
	      output = ATTACH_VERBOSE;
	      break;

	    case 'w':
	      auth = LOCKER_AUTH_READWRITE;
	      break;

	    case 'x':
	      mode = ATTACH_FILESYSTEM;
	      break;

	    case 'y':
	      auth = LOCKER_AUTH_DEFAULT;
	      break;

	    case 'z':
	      options |= LOCKER_ATTACH_OPT_ZEPHYR;
	      break;

	    case 'd':
	    case 'f':
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
	    case ATTACH_FILESYSTEM:
	      status = locker_attach(context, argv[optind], mountpoint,
				     auth, options, mountoptions, &at);
	      if (LOCKER_ATTACH_SUCCESS(status))
		{
		  locker_attachent *ai;

		  for (ai = at->next; ai; ai = ai->next)
		    attach_callback(context, ai, &output);
		  attach_callback(context, at, &output);
		  locker_free_attachent(context, at);
		}
	      else
		estatus = 2;
	      break;

	    case ATTACH_EXPLICIT:
	      status = locker_attach_explicit(context, type, argv[optind],
					      mountpoint, auth, options,
					      mountoptions, &at);
	      if (LOCKER_ATTACH_SUCCESS(status))
		{
		  attach_callback(context, at, &output);
		  locker_free_attachent(context, at);
		}
	      else
		estatus = 2;
	      break;

	    case ATTACH_LOOKUP:
	      attach_lookup(context, argv[optind]);
	      break;

	    case ATTACH_LIST_HOST:
	      attach_list(context, argv[optind]);
	      break;
	    }

	  /* -m only applies to the first locker after it is specified. */
	  mountpoint = NULL;

	  optind++;
	}
    }

  /* If no locker names, and no mode options given, list attached
   * lockers. Otherwise, if we didn't attach anything, give an error.
   */
  if (!gotname)
    {
      if (argc == optind && mode == ATTACH_FILESYSTEM)
	attach_list(context, NULL);
      else
	usage();
    }

  locker_end(context);
  return(estatus);
}


static void attach_list(locker_context context, char *host)
{
  struct hostent *h;

  if (host)
    {
      h = gethostbyname(host);
      if (!h)
	{
	  fprintf(stderr, "%s: Could not resolve hostname \"%s\".\n",
		  whoami, host);
	  exit(1);
	}
    }

  attach_print_entry("filesystem", "mountpoint", "user", "mode");
  attach_print_entry("----------", "----------", "----", "----");
  locker_iterate_attachtab(context,
			   host ? locker_check_host : NULL, &(h->h_addr),
			   attach_print, NULL);
}

static int print_callback(locker_context context, locker_attachent *at,
			  void *val)
{
  int *output = val;

  if (*output == ATTACH_VERBOSE)
    {
      if (*at->hostdir)
	{
	  printf("%s: %s attached to %s for filesystem %s\n",
		 whoami, at->hostdir, at->mountpoint, at->name);
	}
      else
	{
	  printf("%s: %s (%s) attached\n", whoami, at->name,
		 at->mountpoint);
	}
    }
  else if (*output == ATTACH_PRINTPATH)
    printf("%s\n", at->mountpoint);

  return LOCKER_SUCCESS;
}

static void attach_print_entry(char *fs, char *mp, char *user, char *mode)
{
  printf("%-22.22s %-22.22s  %-18.18s%s\n", fs, mp, user, mode);
}

static int attach_print(locker_context context, locker_attachent *at,
			void *data)
{
  char *ownerlist, *p, optstr[32], name[32];
  struct passwd *pw;
  int i;

  /* Build name. */
  if (at->flags & LOCKER_FLAG_NAMEFILE)
    strcpy(name, at->name);
  else
    sprintf(name, "(%s)", at->name);

  /* Build ownerlist. */
  p = ownerlist = malloc(9 * at->nowners);
  if (!ownerlist)
    {
      fprintf(stderr, "%s: Out of memory.\n", whoami);
      exit(1);
    }
  for (i = 0; i < at->nowners; i++)
    {
      if (i)
	*p++ = ',';
      pw = getpwuid(at->owners[i]);
      if (pw)
	p += sprintf(p, "%s", pw->pw_name);
      else
	p += sprintf(p, "#%lu", (unsigned long) at->owners[i]);
    }

  /* Build optstr. (32 characters is "long enough".) */
  optstr[0] = at->mode;
  optstr[1] = '\0';
  if (at->flags & LOCKER_FLAG_NOSUID)
    strcat(optstr, ",nosuid");
  if (at->flags & LOCKER_FLAG_LOCKED)
    strcat(optstr, ",locked");
  if (at->flags & LOCKER_FLAG_KEEP)
    strcat(optstr, ",perm");

  attach_print_entry(name, at->mountpoint[0] == '/' ? at->mountpoint : "-",
		     ownerlist, optstr);
  free(ownerlist);

  return LOCKER_SUCCESS;
}

static void attach_lookup(locker_context context, char *filesystem)
{
  int status, i;
  char **fs;
  void *cleanup;

  status = locker_lookup_filsys(context, filesystem, &fs, &cleanup);
  if (status == LOCKER_SUCCESS)
    {
      printf("%s resolves to:\n", filesystem);
      for (i = 0; fs[i]; i++)
	printf("%s", fs[i]);
      printf("\n");
      locker_free_filesys(context, fs, cleanup);
    }
}

static void usage(void)
{
  fprintf(stderr, "Usage: attach [options] filesystem ... [options] filesystem ...\n");
  fprintf(stderr, "       attach -l filesystem\n");
  fprintf(stderr, "       attach -H host\n");
  fprintf(stderr, "       attach\n");
  exit(1);
}
