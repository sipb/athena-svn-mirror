/**********************************************************************
 * File Exchange fxmain module
 *
 * $Author: probe $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/fxmain.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/fxmain.c,v 1.1 1993-10-12 03:09:18 probe Exp $
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>
#include <ctype.h>
#include <sys/types.h>
#include "fxmain.h"

#define DAY (86400L)            /* number of seconds in one day */

/*** Global variables ***/
char fxmain_error_context[256];

FX *
fxmain_open(progname, course)
     char *progname, *course;
{
  FX *fxp;
  long code;

  if (!course) {
    fprintf(stderr, "%s: No course specified.\n", progname);
    exit(1);
  }
  fxp = fx_open(course, &code);
  if (!fxp) {
    com_err(progname, code, "trying to open %s", course);
    exit(1);
  }

  if (code)
    fprintf(stderr, "%s: Warning: %s at %s\n", progname,
	    error_message(code), fxp->host);
  return(fxp);
}

long
fxmain(argc, argv, usage, p, special_arg, doproc)
     int argc;
     char *argv[];
     char *usage;
     Paper *p;
#ifdef __STDC__
     int (*special_arg)(int, char *[], int *, Paper *, int *);
     long (*doproc)(FX *, Paper *, int, char *);
#else /* __STDC__ */
     int (*special_arg)();
     long (*doproc)();
#endif /* __STDC__ */
{
  FX *fxp = NULL;
  long code = 0L;
  int asgn_found = 0, specific = 0;
  int i;
  char *course;
  int flags = VERBOSE;

  course = (char *) getenv("COURSE");
  if (!p->modified.tv_sec) p->modified.tv_sec = time(0);

  for (i=1; i<argc; i++) {

    /* Deal with arguments specific to one application */
    if (special_arg)
      if (special_arg(argc, argv, &i, p, &flags)) continue;

    /* Time ceiling (for programs that choose to use it) */
    if (argv[i][0] == '+' && isdigit(argv[i][1])) {
      p->modified.tv_sec = time(0) - (DAY * (long) atol(argv[i]+1));
      continue;
    }

    /* Hyphenated options */
    if (argv[i][0] == '-') {
      switch(argv[i][1]) {
      case 'a':
	p->assignment = atoi(argv[++i]);
	asgn_found = 1;
	break;
      case 'c':
	course = argv[++i];
	if (fxp) {
	  fx_close(fxp);
	  fxp = NULL;
	}
	break;
      case 'd':
	if (chdir(argv[++i])) {
          perror(argv[i]);
          exit(1);
        }
        break;
      case 'f':
	p->filename = argv[++i];
	break;
      case 'u':
	p->author = argv[++i];
	break;
      case 'o':
	p->owner = argv[++i];
      case 's':
        p->desc = argv[++i];
        break;
      case 'q':
	flags &= ~VERBOSE;
	break;
      case 'v':
	flags |= VERBOSE;
	break;
      case 'l':
	flags |= LISTONLY;
	break;
      case 'w':
	flags &= ~LISTONLY;
	break;
      case 'p':
	flags |= PRESERVE;
	break;
      case 'm':
	flags &= ~PRESERVE;
	break;
      case '*':
	p->type = TYPE_WILDCARD;
	break;
      case 'T':
	p->type = TURNEDIN;
	break;
      case 't':
	p->type = TAKEN;
	break;
      case 'g':
        p->type = GRADED;
        break;
      case 'P':
	p->type = PICKEDUP;
	break;
      case 'h':
        p->type = HANDOUT;
        break;
      case 'e':
        p->type = EXCHANGE;
        break;
      case 'A':
	p->type = TEACHERS_ARCHIVE;
	break;
      case 'H':
	p->type = TEACHERS_HANDOUT;
	break;
      default:
	/* Time floor (for programs that choose to use it) */
	if (isdigit(argv[i][1])) {
	  p->created.tv_sec = time(0) - (DAY * (long) atol(argv[i]+1));
	  break;
	}
	fprintf(stderr, usage, argv[0]);
	exit(1);
      }
      continue;
    }

    /* Assignment number */
    if (!asgn_found && isdigit(argv[i][0])) {
      asgn_found = 1;
      p->assignment = atoi(argv[i]);
      continue;
    }

    /* specific argument (user or filename) */
    if (!fxp) fxp = fxmain_open(argv[0], course);
    specific = 1;
    code = doproc(fxp, p, flags, argv[i]);
    if (code == ERR_USAGE) {
      fprintf(stderr, usage, argv[0]);
      fx_close(fxp);
      return(0L);
    }
    if (code) {
      com_err(argv[0], code, "%s", fxmain_error_context);
      goto FXMAIN_ABORT;
    }
  }
  if (!specific) {
    if (!fxp) fxp = fxmain_open(argv[0], course);
    code = doproc(fxp, p, flags, NULL);
    if (code == ERR_USAGE) {
      fprintf(stderr, usage, argv[0]);
      fx_close(fxp);
      return(0L);
    }
    if (code) com_err(argv[0], code, "%s", fxmain_error_context);
  }

FXMAIN_ABORT:
  fx_close(fxp);
  return(code);
}
