/**********************************************************************
 * File Exchange collect client
 *
 * $Author: ghudson $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/dump.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/dump.c,v 1.5 1996-09-20 04:34:31 ghudson Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_collect_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/dump.c,v 1.5 1996-09-20 04:34:31 ghudson Exp $";
#endif /* lint */

#include <stdio.h>
#include <sys/time.h>
#include <memory.h>
#include <ctype.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#ifdef POSIX 
#include <unistd.h>
#endif /* POSIX */
#include "fxmain.h"

/*** Global variables ***/
Paper **paperv;
char *full_name();
int compar();
long prep_paper();
void adjust_criterion();
int verbose;

/*
 * do_dump -- dumps papers into files
 */

long
do_dump(fxp, criterion, flags, string)
     FX *fxp;
     Paper *criterion;
     int flags;
     char *string;
{
  extern int errno;
  long code;
  Paperlist_res *plist;
  int count, i;
  char *s;
  char filename[256], newfilename[256];
  int tilde;			/* number for .~n~ backup extension */
  struct timeval tvp[2];	/* for changing mod time */

  verbose = flags & VERBOSE;
  adjust_criterion(criterion, string);
  if ((flags & ONE_AUTHOR) && !(criterion->author))
    criterion->author = fxp->owner;

  /******** get list of papers from server ********/
  code = fx_list(fxp, criterion, &plist);
  if (code) {
    strcpy(fxmain_error_context, "while retrieving list");
    return(code);
  }

  count = get_array(plist->Paperlist_res_u.list, &paperv);

  /******** deal with empty list ********/
  if (count == 0) {
    if (verbose)
      empty_list(criterion);
    goto DUMP_CLEANUP;
  }

  /******** main loop through list ********/
  for (i=0; i<count; i++) {

    /******* Skip papers not in time range ********/
    if (paperv[i]->modified.tv_sec < criterion->created.tv_sec ||
        paperv[i]->modified.tv_sec > criterion->modified.tv_sec) continue;

    /*** do things particular to pickup or collect ***/
    if (code=prep_paper(paperv[i], filename, flags)) goto DUMP_CLEANUP;

    /*** change spaces to underscores ***/
    for (s=filename; *s != '\0'; s++)
      if (isspace(*s)) *s = '_';

    /******** rename local file of same name ********/
    if (access(filename, F_OK) == 0) {
      tilde = 0;
      do {
	sprintf(newfilename, "%s.~%d~", filename, ++tilde);
      } while (access(newfilename, F_OK) == 0);
      if (!(flags & LISTONLY)) {
	if (rename(filename, newfilename)) {
	  sprintf(fxmain_error_context, "renaming %s to %s",
		  filename, newfilename);
	  code = (long) errno;
	  goto DUMP_CLEANUP;
	}
      }
    }

    if (verbose) {
      /******** print information about file ********/
      printf("%5d %-9s %9d  %-16.16s  %s\n", paperv[i]->assignment,
	     paperv[i]->owner, paperv[i]->size,
	     ctime(&(paperv[i]->created.tv_sec)), filename);
    }

    if (!(flags & LISTONLY)) {
      /******** retrieve file from server ********/
      code = fx_retrieve_file(fxp, paperv[i], filename);
      if (code) {
	sprintf(fxmain_error_context, "while retrieving \"%s\"", filename);
	goto DUMP_CLEANUP;
      }

      if (!(flags & PRESERVE)) mark_retrieved(fxp, paperv[i]);

      /******** change accessed, updated times of local file ********/
      tvp[0].tv_sec = paperv[i]->modified.tv_sec;
      tvp[0].tv_usec = paperv[i]->modified.tv_usec;
      tvp[1].tv_sec = paperv[i]->created.tv_sec;
      tvp[1].tv_usec = paperv[i]->created.tv_usec;
      utimes(filename, tvp);	/* Do we care if this fails? */
    }
  }


 DUMP_CLEANUP:
  fx_list_destroy(&plist);
  free((char *) paperv);
  return(code);
}
