/**********************************************************************
 * File Exchange purge client
 *
 * $Author: probe $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/fxpurge.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/fxpurge.c,v 1.1 1993-10-12 03:09:12 probe Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_fxpurge_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/fxpurge.c,v 1.1 1993-10-12 03:09:12 probe Exp $";
#endif /* lint */

#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <sys/errno.h>
#include "fxmain.h"

#define DAY (86400L)		/* number of seconds in one day */

/*** Global variables ***/
long do_purge();
char *full_name();
extern int errno;

/*
 * compar -- compares two papers, used by qsort
 */

compar(p1, p2)
     Paper **p1, **p2;
{
  register int ret;

  ret = strcmp((*p1)->author, (*p2)->author);
  if (ret) return(ret);
  ret = strcmp((*p1)->filename, (*p2)->filename);
  if (ret) return(ret);
  ret = (int) (*p1)->modified.tv_sec - (*p2)->modified.tv_sec;
  if (ret) return(ret);
  return((int) ((*p1)->modified.tv_usec - (*p2)->modified.tv_usec));
}

/*
 * prep_paper -- prints info before paper is purged
 */

long
prep_paper(p, flags)
     Paper *p;			/* paper to purge */
     int flags;
{
  static char *old_author = NULL;

  if (flags & VERBOSE) {
    if (!old_author || strcmp(p->author, old_author))
      printf("%s:\n", full_name(p->author));
    old_author = p->author;
  }

  return(0L);
}

empty_list(criterion)
     Paper *criterion;
{
  if (criterion->author)
    printf("%s:\n", full_name(criterion->author));
  printf("No papers to purge\n");
}

/*
 * main purge procedure
 */

main(argc, argv)
  int argc;
  char *argv[];
{
  Paper p;

  paper_clear(&p);
  p.type = PICKEDUP;
  if (fxmain(argc, argv,
             "Usage: %s [-c course] [options] [assignment] [student ...]\n",
             &p, NULL, do_purge)) exit(1);
  exit(0);
}
