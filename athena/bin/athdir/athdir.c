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

static const char rcsid[] = "$Id: athdir.c,v 1.4 1999-09-15 23:57:14 danw Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <athdir.h>

char *progName;

void usage(void);
void repeatflag(char *option);

int main(int argc, char **argv)
{
  int num_dirs = 0, flags = 0;
  char **dir_list, *type = NULL, *athsys = NULL, *hosttype = NULL;
  char *auxconvention = NULL, *recsep = NULL, **ptr;
  char **path_list;
  int i;
  int match = 0;

  progName = strrchr(argv[0], '/');
  if (progName != NULL)
    progName++;
  else
    progName = argv[0];

  if (argc == 1)
    usage();

  if (argc)
    dir_list = malloc((argc + 1) * sizeof(char *));

  if (dir_list == NULL)
    {
      fprintf(stderr, "%s: out of memory\n", progName);
      exit(1);
    }

  if (argv[1][0] != '-')
    {
      if (argc > 3)
	usage();

      dir_list[num_dirs++] = argv[1];

      if (argv[2])
	{
	  if (argv[2][0] == '-')
	    usage();

	  type = argv[2];
	}
      else
	type = "bin";
    }
  else
    {
      argv++;

      while (*argv)
	{
	  if (**argv != '-' || (*argv)[2] != '\0')
	    {
	      fprintf(stderr, "%s: unknown option: %s\n", progName, *argv);
	      usage();
	    }

	  switch((*argv)[1])
	    {
	    case 't':
	      if (type)
		repeatflag(*argv);
	      argv++;
	      if (*argv == NULL)
		usage();
	      type = *argv++;
	      break;

	    case 'p':
	      if (num_dirs != 0)
		repeatflag(*argv);

	      argv++;
	      if (*argv == NULL)
		usage();
	      while (*argv != NULL && **argv != '-')
		dir_list[num_dirs++] = *argv++;
	      break;

	    case 'e':
	      if (flags & ATHDIR_SUPPRESSEDITORIALS)
		repeatflag(*argv);
	      argv++;
	      flags |= ATHDIR_SUPPRESSEDITORIALS;
	      break;

	    case 'c':
	      if (flags & ATHDIR_SUPPRESSSEARCH)
		repeatflag(*argv);
	      argv++;
	      flags |= ATHDIR_SUPPRESSSEARCH;
	      break;

	    case 'l':
	      if (flags & ATHDIR_LISTSEARCHDIRECTORIES)
		repeatflag(*argv);
	      argv++;
	      flags |= ATHDIR_LISTSEARCHDIRECTORIES;
	      break;

	    case 's':
	      if (athsys)
		repeatflag(*argv);
	      argv++;
	      if (*argv == NULL)
		usage();
	      athsys = *argv++;
	      break;

	    case 'm':
	      if (hosttype != NULL)
		repeatflag(*argv);
	      argv++;
	      if (*argv == NULL)
		usage();
	      hosttype = *argv++;
	      break;

	    case 'f':
	      if (auxconvention)
		repeatflag(*argv);
	      argv++;
	      if (*argv == NULL)
		usage();
	      auxconvention = *argv++;
	      break;

	    case 'r':
	      if (recsep != NULL)
		repeatflag(*argv);
	      argv++;
	      if (*argv == NULL)
		usage();
	      recsep = *argv++;
	      break;

	    case 'd':
	      if (flags & ATHDIR_MACHINEDEPENDENT)
	        repeatflag(*argv);
	      argv++;
	      flags |= ATHDIR_MACHINEDEPENDENT;
	      break;

	    case 'i':
	      if (flags & ATHDIR_MACHINEINDEPENDENT)
	        repeatflag(*argv);
	      argv++;
	      flags |= ATHDIR_MACHINEINDEPENDENT;
	      break;

	    default:
	      fprintf(stderr, "%s: unknown option: %s\n", progName, *argv);
	      usage();
	      break;
	    }
	}
    }

  if (!num_dirs)
    dir_list[num_dirs++] = NULL;

  /* Default record separator is a newline. */
  if (!recsep)
    recsep = "\n";

  for (i = 0; i < num_dirs; i++)
    {
      path_list = athdir_get_paths(dir_list[i], type, athsys, NULL, hosttype,
				   auxconvention, flags);
      if (path_list != NULL)
	{
	  for (ptr = path_list; *ptr != NULL; ptr++)
	    {
	      if (match == 1)
		fprintf(stdout, "%s", recsep);
	      match = 1;
	      fprintf(stdout, "%s", *ptr);
	    }

	  athdir_free_paths(path_list);
	}
    }

  if (match)
    fprintf(stdout, "\n");

#ifdef DEBUG
  fprintf(stdout, "%s ", progName);
  if (type != NULL)
    fprintf(stdout, "-t %s ", type);
  if (num_dirs)
    {
      fprintf(stdout, "-p ");
      for (i = 0; i < num_dirs; i++)
	fprintf(stdout, "%s ", dir_list[i]);
    }
  if (ATHDIR_SUPPRESSEDITORIALS & flags)
    fprintf(stdout, "-e ");
  if (ATHDIR_SUPPRESSSEARCH & flags)
    fprintf(stdout, "-c ");
  if (ATHDIR_LISTSEARCHDIRECTORIES & flags)
    fprintf(stdout, "-l ");
  fprintf(stdout, "\n");
#endif

  return match == 0;
}

void usage(void)
{
  fprintf(stderr, "usage: %s path [type]\n", progName);
  fprintf(stderr,
	  "   or: %s [-t type] [-p path ...] [-e] [-c] [-l] [-d | -i]\n",
	  progName);
  fprintf(stderr,
	  "       [-r recsep] [-f format] [-s sysname] [-m machtype]\n");
  exit(1);
}

void repeatflag(char *option)
{
  fprintf(stderr, "%s: %s already specified.\n", progName, option);
  usage();
}
