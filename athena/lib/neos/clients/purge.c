/**********************************************************************
 * File Exchange purge module
 *
 * $Author: probe $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/purge.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/purge.c,v 1.1 1993-10-12 03:09:10 probe Exp $
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_collect_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/purge.c,v 1.1 1993-10-12 03:09:10 probe Exp $";
#endif /* lint */

#include <stdio.h>
#include <sys/time.h>
#include <memory.h>
#include <ctype.h>
#include <strings.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include "fxmain.h"

/*** Global variables ***/
Paper **paperv;
char *full_name();
char purge_err_context[1024];
int compar();
long prep_paper();

/*
 * do_dump -- dumps papers into files
 */

long
do_purge(fxp, criterion, flags, user)
     FX *fxp;
     Paper *criterion;
     int flags;
     char *user;
{
  long code;
  Paperlist_res *plist;
  int count, i, found = 0;

  criterion->author = user;
  /******** get list of papers from server ********/
  code = fx_list(fxp, criterion, &plist);
  if (code) {
    strcpy(purge_err_context, "while retrieving list");
    return(code);
  }

  count = get_array(plist->Paperlist_res_u.list, &paperv);

  /******** deal with empty list ********/
  if (count == 0) {
    if (flags & VERBOSE)
      empty_list(criterion);
    return(0L);
  }

  /******** main loop through list ********/
  for (i=0; i<count; i++) {

    /******* Skip papers not in time range ********/
    if (paperv[i]->modified.tv_sec < criterion->created.tv_sec ||
	paperv[i]->modified.tv_sec > criterion->modified.tv_sec) continue;

    found = 1;

    prep_paper(paperv[i], flags);

    if (flags & VERBOSE) {
      /******** print information about file ********/
      printf("%5d %-9s %9d  %-16.16s  %s\n", paperv[i]->assignment,
	     paperv[i]->owner, paperv[i]->size,
	     ctime(&(paperv[i]->modified.tv_sec)), paperv[i]->filename);
    }

    if (!(flags & LISTONLY)) {
      /******** delete file from server ********/
      code = fx_delete(fxp, paperv[i]);
      if (code) {
	strcpy(purge_err_context, "while deleting.");
	return(code);
      }
    }
  }

  /******** clean up ********/
  if (!found && (flags & VERBOSE)) empty_list(criterion);
  fx_list_destroy(&plist);
  free((char *) paperv);
  return(0L);
}
