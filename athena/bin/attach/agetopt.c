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

/* This file defines attach_getopt, a version of getopt with support
 * for long arguments in two different styles.
 */

static const char rcsid[] = "$Id: agetopt.c,v 1.1 1999-02-26 23:12:58 danw Exp $";

#include <stdio.h>
#include <string.h>

#include "agetopt.h"

extern char *whoami;

int attach_getopt(int argc, char **argv, struct agetopt_option *options)
{
  int i;
  char *name;
  static char *last = NULL;

  if (optind >= argc || argv[optind][0] != '-' || argv[optind][1] == '\0')
    return -1;

  if (!strcmp(argv[optind], "--"))
    {
      optind++;
      return -1;
    }

  /* Remaining possibilities:
   * Long arg name: 		--foo
   * Short arg name:		-f
   * Short arg name with data:	-foo
   * Old-style long arg name:	-foo
   * Short args concatenated:	-foo
   *
   * Too bad we can't necessarily tell the last few apart. We assume
   * that anything that matches a long arg is a long arg, not a
   * bunch of short args. Also, if we're in the midst of parsing
   * concatenated short args, we can't switch to long args in mid word.
   */

  if (last)
    {
      /* We are parsing concatenated short args. Pick up where we
       * left off.
       */
      name = last;
    }
  else
    {
      /* This is a new argument. Figure out what kind. */
      int doubledash = 0;

      name = argv[optind] + 1;
      if (*name == '-')
	{
	  doubledash = 1;
	  name++;
	}

      /* Look for a long arg. */
      for (i = 0; options[i].longname; i++)
	{
	  if (!strcmp(name, options[i].longname))
	    {
#ifdef DEPRECATE_SINGLEDASH
	      if (!doubledash)
		{
		  fprintf(stderr, "%s: -%s is deprecated. Use --%s instead.\n",
			  whoami, name, name);
		}
#endif
	      optarg = argv[optind + 1];
	      optind += 1 + options[i].arg;
	      return options[i].shortname;
	    }
	}

      /* If we had a "--option" and didn't match any long arg, we lose. */
      if (doubledash)
	return '?';
    }

  /* Look for a short arg. */
  for (i = 0; options[i].longname; i++)
    {
      if (*name == options[i].shortname)
	{
	  if (*(name + 1))
	    {
	      if (options[i].arg)
		{
		  /* Option argument is rest of argv[optind]. */
		  optarg = name + 1;
		  last = NULL;
		  optind++;
		}
	      else
		{
		  /* There are more options to decode in argv[optind]. */
		  last = name + 1;
		}
	    }
	  else
	    {
	      /* Done with argv[optind]. Move on. */
	      last = NULL;
	      optarg = argv[optind + 1];
	      optind += 1 + options[i].arg;
	    }
	  return options[i].shortname;
	}
    }

  /* No match. */
  return '?';
}
