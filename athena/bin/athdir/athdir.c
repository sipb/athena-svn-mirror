#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>		/* MAXPATHLEN */
#include <sys/types.h>
#include <sys/stat.h>

#define NUMCONVENTIONS 5

#ifdef ultrix
#define HOSTTYPESET 1
#define HOSTTYPE "decmips"
#endif

#ifdef _IBMR2
#define HOSTTYPESET 1
#define HOSTTYPE "rsaix"
#endif

#ifdef SOLARIS
#define HOSTTYPESET 1
#define HOSTTYPE "sun4"
#endif

#ifdef linux
#define HOSTTYPESET 1
#define HOSTTYPE "linux"
#endif

/*
 * Don't define HOSTTYPE on platforms for which `machtype`bin was
 * never widely used.
 */

/*
 * This should be passed in as a compile flag.
 */
#ifndef ATHSYS
#define ATHSYS "@sys"
#endif

char *athsys = ATHSYS;

#ifdef HOSTTYPE
int hosttypeset = 1;
char *hosttype = HOSTTYPE;
#else
int hosttypeset = 0;
char *hosttype = NULL;
#endif

char *progName;

/*
 * Definition of known conventions and what flavors they are.
 */

typedef struct {
  char *name;
  int flavor;
} Convention;

#define ARCHflavor (1<<0)
#define MACHflavor (1<<1)
#define PLAINflavor (1<<2)
#define SYSflavor (1<<3)
#define DEPENDENTflavor (1<<8)
#define INDEPENDENTflavor (1<<9)

Convention conventions[NUMCONVENTIONS] = {
  { NULL,		ARCHflavor | MACHflavor | PLAINflavor |
      			DEPENDENTflavor | INDEPENDENTflavor },
  { "%p/arch/%s/%t",	ARCHflavor | DEPENDENTflavor },
  { "%p/%s/%t",		SYSflavor | DEPENDENTflavor },
  { "%p/%m%t",		MACHflavor | DEPENDENTflavor },
  { "%p/%t",		PLAINflavor | INDEPENDENTflavor }
};

/*
 * Editorial tagging for what conventions are acceptable or
 * preferable for what types.
 */

typedef struct {
  char *type;
  int allowedFlavors;	/* searching parameters */
  int preferredFlavor;	/* creating paramaters */
} Editorial;

Editorial editorials[] = {
  { "bin",	ARCHflavor | SYSflavor | MACHflavor,	DEPENDENTflavor },
  { "lib",	ARCHflavor | SYSflavor | MACHflavor,	DEPENDENTflavor },
  { "etc",	ARCHflavor | SYSflavor | MACHflavor,	DEPENDENTflavor },
  { "man",	ARCHflavor | PLAINflavor,		INDEPENDENTflavor },
  { "include",	ARCHflavor | PLAINflavor,		INDEPENDENTflavor },
  { NULL,	ARCHflavor | PLAINflavor,		DEPENDENTflavor }
};

usage()
{
  fprintf(stderr, "usage: %s path [type]\n", progName);
  fprintf(stderr,
	  "   or: %s [-t type] [-p path ...] [-e] [-c] [-l] [-d | -i]\n",
	  progName);
  fprintf(stderr,
	  "       [-r recsep] [-f format] [-s sysname] [-m machtype]\n");
  exit(1);
}

/*
 * path = template(dir, type, sys, machine)
 *	%p = path (dir)
 *	%t = type
 *	%s = sys
 *	%m = machine
 * %foo is inserted if the corresponding string is NULL.
 * If this happens, expand returns 1 (not a valid path).
 * Otherwise, expand returns 0.
 */
expand(path, template, dir, type, sys, machine)
     char *path, *template, *dir, *type, *sys, *machine;
{
  char *src, *dst;
  int somenull = 0;

  src = template;
  dst = path;
  *dst = '\0';

  while (*src != '\0')
    {
      if (*src != '%' && *src != '\0')
	*dst++ = *src++;
      else
	{

#define copystring(casec, cases, string)			\
	case casec:						\
	  src++;						\
	  if (string)						\
	    {							\
	      strcpy(dst, string);				\
	      dst += strlen(string);				\
	    }							\
	  else							\
	    {							\
	      strcpy(dst, cases);				\
	      dst += 2;						\
	      somenull = 1;					\
	    }							\
	  break;

	  src++;
	  switch(*src)
	    {
	      copystring('p', "%p", dir);
	      copystring('t', "%t", type);
	      copystring('s', "%s", sys);
	      copystring('m', "%m", machine);

#undef copystring

	    case '\0':
	      break;

	    default:
	      *dst++ = '%';
	      *dst++ = *src++;
	      break;
	    }
	}
    }

  *dst = '\0';
  return somenull;
}

main(argc, argv)
     int argc;
     char **argv;
{
  int numPaths = 0, eflag = 0, cflag = 0, lflag = 0, tflag = 0,
    sflag = 0, mflag = 0, dflag = 0, iflag = 0;
  int preferredFlavor = 0;
  char **pathList, *type = NULL, *tmp;
  char *recsep = NULL;
  char path[MAXPATHLEN];
  struct stat statbuf;
  int i, j, t;
  int complete;
  int failed, status = 0, first = 1;

  progName = strrchr(argv[0], '/');
  if (progName != NULL)
    progName++;
  else
    progName = argv[0];

  if (argc == 1)
    usage();

  if (argc)
    pathList = malloc((argc + 1) * sizeof(char *));

  /*
   * ATHENA_SYS environment variable overrides hard-coded
   * value.
   */
  tmp = getenv("ATHENA_SYS");
  if (tmp != NULL)
    athsys = tmp;

  if (argv[1][0] != '-')
    {
      if (argc > 3)
	usage();

      pathList[numPaths++] = argv[1];

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

#define repeatflag							\
		{							\
		  fprintf(stderr, "%s: %s already specified.\n",	\
			  progName, *argv);				\
		  usage();						\
		}

	    case 't':
	      if (tflag)
		repeatflag;
	      argv++;
	      if (*argv == NULL)
		usage();
	      type = *argv++;
	      tflag++;
	      break;

	    case 'p':
	      if (numPaths != 0)
		repeatflag;

	      argv++;
	      if (*argv == NULL)
		usage();
	      while (*argv != NULL && **argv != '-')
		pathList[numPaths++] = *argv++;
	      break;

	    case 'e':
	      if (eflag)
		repeatflag;
	      argv++;
	      eflag++;
	      break;

	    case 'c':
	      if (cflag)
		repeatflag;
	      argv++;
	      cflag++;
	      break;

	    case 'l':
	      if (lflag)
		repeatflag;
	      argv++;
	      lflag++;
	      break;

	    case 's':
	      if (sflag)
		repeatflag;
	      argv++;
	      if (*argv == NULL)
		usage();
	      athsys = *argv++;
	      sflag++;
	      break;

	    case 'm':
	      if (mflag)
		repeatflag;
	      argv++;
	      if (*argv == NULL)
		usage();
	      hosttype = *argv++;
	      hosttypeset = 1;
	      mflag++;
	      break;

	    case 'f':
	      if (conventions[0].name)
		repeatflag;
	      argv++;
	      if (*argv == NULL)
		usage();
	      conventions[0].name = *argv++;
	      break;

	    case 'r':
	      if (recsep != NULL)
		repeatflag;
	      argv++;
	      if (*argv == NULL)
		usage();
	      recsep = *argv++;
	      break;

	    case 'd':
	      if (dflag)
	        repeatflag;
	      argv++;
	      dflag++;
	      preferredFlavor = DEPENDENTflavor;
	      break;

	    case 'i':
	      if (iflag)
	        repeatflag;
	      argv++;
	      iflag++;
	      preferredFlavor = INDEPENDENTflavor;
	      break;

	    default:
	      fprintf(stderr, "%s: unknown option: %s\n", progName, *argv);
	      usage();
	      break;
	    }
	}
    }

  if (!numPaths)
    pathList[numPaths++] = NULL;

  if (!recsep)
    recsep = "\n";

  /*
   * You are in a twisty little maze of interconnecting
   * command line options, all different.
   */
  for (i = 0; i < numPaths; i++)
    {
      /* Find matching editorial. */
      for (t = 0; editorials[t].type != NULL; t++)
	if (type != NULL &&
	    !strcmp(type, editorials[t].type))
	  break;

      if (!preferredFlavor)
	preferredFlavor = editorials[t].preferredFlavor;

      failed = 1;

      /* Cycle through matching conventions */
      for (j = 0; j < NUMCONVENTIONS; j++)
	{
	  if (conventions[j].name == NULL)
	    continue; /* User specified convention is not set. */

	  if (/* -e: explicit editorial override */
	      eflag || 

	      /* -d, -i also imply override, but only have meaning with -c */
	      ((dflag || iflag) && cflag) || 

	      /* otherwise, we make our editorial comments */
	      (editorials[t].allowedFlavors & conventions[j].flavor))
	    {
	      if (cflag &&
		  !(preferredFlavor & conventions[j].flavor))
		continue;

	      complete = !expand(path, conventions[j].name,
				pathList[i], type, athsys, hosttype);

	      if (lflag ||
		  (cflag && complete))
		{
		  if (!first)
		    fprintf(stdout, "%s", recsep);
		  fprintf(stdout, "%s", path);
		  first = 0;
		  failed = 0;
		  if (cflag)
		    break;
		}

	      if (complete && !lflag && !cflag &&
		  !stat(path, &statbuf))
		{
		  if (!first)
		    fprintf(stdout, "%s", recsep);
		  fprintf(stdout, "%s", path);
		  first = 0;
		  failed = 0;
		  break;
		}
	    }
	}
      if (failed)
	status = 1;
    }

  if (!first)
    fprintf(stdout, "\n");

#ifdef DEBUG
  fprintf(stdout, "%s ", progName);
  if (type != NULL)
    fprintf(stdout, "-t %s ", type);
  if (numPaths)
    {
      fprintf(stdout, "-p ");
      for (i = 0; i < numPaths; i++)
	fprintf(stdout, "%s ", pathList[i]);
    }
  if (eflag)
    fprintf(stdout, "-e ");
  if (cflag)
    fprintf(stdout, "-c ");
  if (lflag)
    fprintf(stdout, "-l ");
  fprintf(stdout, "\n");
#endif

  exit(status);
}
