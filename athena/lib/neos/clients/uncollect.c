/**********************************************************************
 * File Exchange uncollect client
 *
 * $Author: probe $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/uncollect.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/uncollect.c,v 1.1 1993-10-12 03:08:56 probe Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_uncollect_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/uncollect.c,v 1.1 1993-10-12 03:08:56 probe Exp $";
#endif /* lint */

#include <stdio.h>
#include <sys/time.h>
#include <memory.h>
#include <ctype.h>
#include <strings.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "fxmain.h"

/*
 * do_uncollect -- dumps papers into files
 */

long
do_uncollect(fxp, criterion, flags, user)
     FX *fxp;
     Paper *criterion;
     int flags;
     char *user;
{
  long code;
  Paperlist_res *plist;
  Paperlist l;
  PaperType newtype;
  Paper newp;

  newtype = criterion->type;
  criterion->type = TAKEN;
  criterion->author = user;
  /******** get list of papers from server ********/
  code = fx_list(fxp, criterion, &plist);
  criterion->type = newtype;
  if (code) {
    strcpy(fxmain_error_context, "while retrieving list");
    return(code);
  }

  /******** main loop through list ********/
  for (l = plist->Paperlist_res_u.list; l != NULL; l = l->next) {

    /******* Skip papers not in time range ********/
    if (l->p.modified.tv_sec < criterion->created.tv_sec ||
        l->p.modified.tv_sec > criterion->modified.tv_sec) continue;

    paper_copy(&(l->p), &newp);
    newp.type = newtype;
    if (flags & VERBOSE) {
      /******** print information about file ********/
      printf("%5d %-9s %9d  %-16.16s  %s\n", newp.assignment,
             newp.owner, newp.size, ctime(&(newp.created.tv_sec)),
	     newp.filename);
    }
    if (!(flags & LISTONLY)) {
      code = fx_move(fxp, &(l->p), &newp);
      if (code) {
	sprintf(fxmain_error_context, "while restoring %s by %s",
		l->p.filename, l->p.author);
	goto DO_UNCOLLECT_ABORT;
      }
    }
  }

DO_UNCOLLECT_ABORT:
  fx_list_destroy(&plist);
  return(code);
}

/*
 * main uncollect procedure
 */

main(argc, argv)
  int argc;
  char *argv[];
{
  Paper p;

  paper_clear(&p);
  p.type = TURNEDIN;

  if (fxmain(argc, argv,
             "Usage: %s [options] [assignment] [username ...]\n",
             &p, NULL, do_uncollect)) exit(1);
  exit(0);
}
