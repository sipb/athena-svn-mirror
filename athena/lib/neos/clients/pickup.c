/**********************************************************************
 * File Exchange pickup client
 *
 * $Author: ghudson $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/pickup.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/pickup.c,v 1.3 1996-09-20 04:34:45 ghudson Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_collect_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/pickup.c,v 1.3 1996-09-20 04:34:45 ghudson Exp $";
#endif /* lint */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "fxmain.h"

/*** Global variables ***/
long do_dump();
extern int verbose, errno;
int kbytes = 0;

/*
 * pickup_arg checks to see if the current argument indicates
 * a type for which PRESERVE should be set or a type for which
 * we pickup papers authored by others, e.g. handouts.
 */

/*ARGSUSED*/
int
pickup_arg(argc, argv, ip, p, flagp)
     int argc;
     char *argv[];
     int *ip;
     Paper *p;
     int *flagp;
{
  static int initialized = 0;

  if (!initialized) {
    *flagp |= ONE_AUTHOR;
    initialized = 1;
  }
  if (argv[*ip][0] == '-' && strchr("*TtPheAH", argv[*ip][1])) {
    *flagp |= PRESERVE;
    *flagp &= ~ONE_AUTHOR;
  }
  return(0);
}

/*
 * adjust_criterion -- put filename in criterion
 */

void
adjust_criterion(p, s)
     Paper *p;
     char *s;
{
  p->filename = s;
  return;
}

/*
 * compar -- compares two papers, used by qsort
 */

compar(p1, p2)
     Paper **p1, **p2;
{
  register int ret;

  ret = strcmp((*p1)->filename, (*p2)->filename);
  if (ret) return(ret);
  ret = (int) (*p1)->modified.tv_sec - (*p2)->modified.tv_sec;
  if (ret) return(ret);
  return((int) ((*p1)->modified.tv_usec - (*p2)->modified.tv_usec));
}

/*
 * prep_paper -- gets filename, disk usage for paper
 */

/*ARGSUSED*/
long
prep_paper(p, f, flags)
     Paper *p;			/* paper to retrieve */
     char *f;			/* filename (modified) */
     int flags;			/* flags (unused) */
{
  (void) strcpy(f, p->filename);
  kbytes += ((p->size + 1023) >> 10);
  return(0L);
}

empty_list(criterion)
     Paper *criterion;
{
  printf("No papers to pick up\n");
}

mark_retrieved(fxp, p)
     FX *fxp;
     Paper *p;
{
  Paper pickedup;
  static int warned = 0;
  long code;

  if (!warned) {
    /******** mark file on server as PICKEDUP ********/
    paper_copy(p, &pickedup);
    pickedup.type = PICKEDUP;
    if (!warned && (code = fx_move(fxp, p, &pickedup))) {
      com_err("Warning", code, "-- files not marked PICKEDUP on server.", "");
      warned = 1;
    }
  }
  return;
}

/*
 * main pickup procedure
 */

main(argc, argv)
  int argc;
  char *argv[];
{
  Paper p;

  paper_clear(&p);
  p.type = GRADED;

  if (fxmain(argc, argv,
	     "Usage: %s [options] [assignment] [filename ...]\n",
	     &p, pickup_arg, do_dump)) exit(1);
  if (verbose && kbytes) printf("%d kbytes total\n", kbytes);
  exit(0);
}
