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

/* This is the part of attach that is used by the "add" alias. */

static const char rcsid[] = "$Id: add.c,v 1.15 2005-09-15 14:22:00 rbasch Exp $";

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <athdir.h>
#include <locker.h>

#include "agetopt.h"
#include "attach.h"

static void usage(void);
static void modify_path(char **path, char *elt);
static void print_readable_path(char *path);
static int add_callback(locker_context context, locker_attachent *at,
			void *arg);
static int ruid_stat(const char *path, struct stat *st);

static struct agetopt_option add_options[] = {
  { "verbose", 'v', 0 },
  { "quiet", 'q', 0 },
  { "debug", 'd', 0 },
  { "front", 'f', 0 },
  { "remove", 'r', 0 },
  { "print", 'p', 0 },
  { "warn", 'w', 0 },
  { "bourne", 'b', 0 },
  { "path", 'P', 1 },
  { "attachopts", 'a', 0 }
};

static char *shell_templates[2][2] =
{
  {
    "setenv PATH %s; setenv MANPATH %s; setenv INFOPATH %s\n",
    "PATH=%s; export PATH; MANPATH=%s; export MANPATH; INFOPATH=%s; export INFOPATH\n"
  },
  {
    "set athena_path=(%s); setenv MANPATH %s; setenv INFOPATH %s\n",
    "athena_path=%s; MANPATH=%s; export MANPATH; INFOPATH=%s; export INFOPATH\n"
  }
};

extern locker_callback attach_callback;

static int quiet = 0, give_warnings = 0, remove_from_path = 0;
static int add_to_front = 0, bourne_shell = 0, use_athena_path = 0;
static char *path, *manpath, *infopath;

int add_main(int argc, char **argv)
{
  int print_path, end_args;
  int opt;

  print_path = end_args = 0;

  while (!end_args && (opt = attach_getopt(argc, argv, add_options)) != -1)
    {
      switch (opt)
	{
	case 'q':
	  quiet = 1;
	  break;

	case 'f':
	  add_to_front = 1;
	  break;

	case 'r':
	  remove_from_path = 1;
	  break;

	case 'p':
	  print_path = 1;
	  break;

	case 'w':
	  give_warnings = 1;
	  break;

	case 'b':
	  bourne_shell = 1;
	  break;

	case 'P':
	  use_athena_path = 1;
	  path = optarg;
	  break;

	case 'a':
	  if (remove_from_path || print_path)
	    {
	      fprintf(stderr, "%s: can't use -a with -r or -p.\n", whoami);
	      usage();
	    }
	  end_args = 1;
	  break;

	case 'd':
	case 'v':
	  fprintf(stderr, "%s: The '%c' flag is no longer supported.\n",
		  whoami, opt);
	  break;

	case '?':
	  usage();
	}
    }

  if (!path)
    path = getenv("PATH");
  if (!path)
    path = "";
  path = strdup(path);
  if (!path)
    {
      fprintf(stderr, "%s: Out of memory.\n", whoami);
      exit(1);
    }
  if (use_athena_path && !bourne_shell)
    {
      char *p;
      for (p = path; *p; p++)
	{
	  if (*p == ' ')
	    *p = ':';
	}
    }
  manpath = getenv("MANPATH");
  if (manpath)
    manpath = strdup(manpath);
  else
    manpath = strdup(":");

  infopath = getenv("INFOPATH");
  if (infopath)
    infopath = strdup(infopath);
  else
    infopath = strdup("");

  /* If no arguments have been directed to attach, or -p was
   * specified, we output the path in an easier-to-read format and
   * we're done.
   */
  if (argc == optind || print_path)
    {
      print_readable_path(path);
      exit(0);
    }

  /* If the following args weren't explicitly handed to attach (via
   * -a), and the first one looks like a pathname, then assume we've
   * been passed a bunch of pathnames rather than lockernames.
   */
  if (!end_args && (argv[optind][0] == '.' || argv[optind][0] == '/'))
    {
      struct stat st;

      for (; optind < argc; optind++)
	{
	  if (argv[optind][0] != '.' && argv[optind][0] != '/')
	    {
	      fprintf(stderr, "%s: only pathnames may be specified when "
		      "pathnames are being added\n", whoami);
	      usage();
	    }

	  /* Make sure the directory exists, if we're adding it to the
	   * path. Otherwise we don't care.
	   */
	  if (!remove_from_path && ruid_stat(argv[optind], &st) == -1)
	    {
	      fprintf(stderr, "%s: no such path: %s\n", whoami,
		      argv[optind]);
	    }
	  else
	    modify_path(&path, argv[optind]);
	}
    }
  else if (remove_from_path)
    {
      locker_context context;
      int status;

      if (locker_init(&context, getuid(), NULL, NULL))
	exit(1);

      for (; optind < argc; optind++)
	{
	  locker_attachent *at;

	  /* Ignore flags, just look at lockers */
	  if (argv[optind][0] == '-')
	    continue;

	  status = locker_read_attachent(context, argv[optind], &at);
	  if (status != LOCKER_SUCCESS)
	    continue;
	  add_callback(context, at, NULL);
	  locker_free_attachent(context, at);
	}

      locker_end(context);
    }
  else
    {
      /* We are adding lockers. */

      attach_callback = add_callback;

      /* Reinvoke attach's main: optind now points to either the first
       * attach command-line argument or the first locker. Either way,
       * let attach deal (using our callback), and return to us when
       * it's done.
       */
      attach_main(argc, argv);
    }

  if (use_athena_path && !bourne_shell)
    {
      char *p;
      for (p = path; *p; p++)
	{
	  if (*p == ':')
	    *p = ' ';
	}
    }

  printf(shell_templates[use_athena_path][bourne_shell],
	 path, manpath, infopath);
  free(path);
  free(manpath);
  free(infopath);
  exit(0);
}

static int add_callback(locker_context context, locker_attachent *at,
			void *arg)
{
  char **found, **ptr;

  /* Find the binary directories we want to add to/remove from the path. */
  found = athdir_get_paths(at->mountpoint, "bin", NULL, NULL, NULL, NULL, 0);
  if (found)
    {
      for (ptr = found; *ptr; ptr++)
	{
	  if (!remove_from_path && !athdir_native(*ptr, NULL) && give_warnings)
	    {
	      fprintf(stderr, "%s: warning: using compatibility for %s\n",
		      whoami, at->mountpoint);
	    }
	  modify_path(&path, *ptr);
	}
      athdir_free_paths(found);
    }
  else
    {
      if (give_warnings)
	{
	  fprintf(stderr, "%s: warning: %s has no binary directory\n",
		  whoami, at->mountpoint);
	}
    }

  /* Find the man directories we want to add to/remove from the manpath. */
  found = athdir_get_paths(at->mountpoint, "man", NULL, NULL, NULL, NULL, 0);
  if (found)
    {
      for (ptr = found; *ptr; ptr++)
	modify_path(&manpath, *ptr);
      athdir_free_paths(found);
    }

  /* Find the info directories we want to add to/remove from the infopath. */
  found = athdir_get_paths(at->mountpoint, "info", NULL, NULL, NULL, NULL, 0);
  if (found)
    {
      for (ptr = found; *ptr; ptr++)
	modify_path(&infopath, *ptr);
      athdir_free_paths(found);
    }

  return 0;
}

static void modify_path(char **pathp, char *elt)
{
  char *p;
  int len = strlen(elt);

  /* If we're adding a string to the front of the path, we need
   * to remove it from the middle first, if it's already there.
   */
  if (remove_from_path || add_to_front)
    {
      p = *pathp;
      while (p)
	{
	  if (!strncmp(p, elt, len) && (p[len] == ':' || p[len] == '\0'))
	    {
	      if (p[len] == ':')
		len++;
	      else if (p != *pathp)
		{
		  p--;
		  len++;
		}

	      memmove(p, p + len, strlen(p + len) + 1);
	    }

	  p = strchr(p, ':');
	  if (p)
	    p++;
	}
    }
  else
    {
      /* Adding to end, so make sure the path element isn't already in
       * the middle.
       */
      if ((p = strstr(*pathp, elt)) &&
	  (p[len] == ':' || p[len] == '\0') &&
	  (p == *pathp || *(p - 1) == ':'))
	return;
    }


  if (!remove_from_path)
    {
      p = malloc(strlen(*pathp) + len + 2);
      if (!p)
	{
	  fprintf(stderr, "%s: Out of memory.\n", whoami);
	  exit(1);
	}
      if (add_to_front)
	sprintf(p, "%s%s%s", elt, **pathp ? ":" : "", *pathp);
      else
	sprintf(p, "%s%s%s", *pathp, **pathp ? ":" : "", elt);
      free(*pathp);
      *pathp = p;
    }
}

/* print_readable_path
 *
 * A hack to print out a readable version of the user's path.
 *
 * It's a hack because it's not currently a nice thing to do
 * correctly. So, if any path element starts with "/mit" and ends with
 * "bin" such as "/mit/gnu/arch/sun4x_55/bin," we print "{add gnu}"
 * instead.  This is not always correct; it misses things that are not
 * mounted under /mit, and is misleading for lockers that do not mount
 * under /mit/lockername as well as MUL type filesystems. However,
 * these occasions are infrequent.
 *
 * In addition, each path starting with "/mit" and ending with "bin"
 * is tested for the substring of the machine's $ATHENA_SYS value. If
 * absent, it is assumed that some form of compatibility system is
 * being used, and a * is added to the shortened path string. So if
 * ATHENA_SYS_COMPAT is set to sun4x_55 while ATHENA_SYS is set to
 * sun4x_56, in the example above "{add gnu*}" would be printed
 * instead of "{add gnu}."
 *
 * XXX We could do a less hacky version of this using
 * locker_iterate_attachtab to get all of the mountpoints. It's not clear
 * that there's a lot of benefit to this though.  */
static void print_readable_path(char *path)
{
  char *p, *name, *name_end;

  for (p = strtok(path, ":"); p; p = strtok(NULL, ":"))
    {
      if (p != path)
	putc(bourne_shell ? ':' : ' ', stderr);

      if (!strncmp(p, "/mit/", 5))
	{
	  name = p + 5;
	  name_end = strchr(name, '/');
	  if (name_end && !strcmp(p + strlen(p) - 3, "bin"))
	    {
	      if (athdir_native(name, NULL))
		fprintf(stderr, "{add %.*s}", name_end - name, name);
	      else
		fprintf(stderr, "{add %.*s*}", name_end - name, name);
	    }
	  else
	    fprintf(stderr, "%s", p);
	}
      else
	fprintf(stderr, "%s", p);
    }

  fprintf(stderr, "\n");
}

/* stat() the given path after setting the effective UID to the
 * real UID (if necessary), and return the value returned by stat().
 */
static int ruid_stat(const char *path, struct stat *st)
{
  uid_t euid = geteuid();
  uid_t ruid = getuid();
  int status;

  if (euid != ruid)
    seteuid(ruid);
  status = stat(path, st);
  if (euid != ruid)
    seteuid(euid);
  return status;
}

static void usage(void)
{
  fprintf(stderr, "Usage: add [-vfrpwbq] [-P $athena_path] [-a attachflags] [lockername ...]\n");
  fprintf(stderr, "       add [-dfrb] [-P $athena_path] pathname ...\n");
  exit(1);
}
