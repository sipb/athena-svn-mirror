/**********************************************************************
 * File Exchange return client
 *
 * $Author: probe $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/return.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/return.c,v 1.2 1992-01-22 13:28:51 probe Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_return_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/return.c,v 1.2 1992-01-22 13:28:51 probe Exp $";
#endif /* lint */

#include <stdio.h>
#include <ctype.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include "fxmain.h"

/*
 * compar -- compares two papers by modify time, used by qsort
 */

compar(p1, p2)
     Paper **p1, **p2;
{
  int ret;

  ret = strcmp((*p1)->author, (*p2)->author);
  if (ret) return(ret);
  ret = strcmp((*p1)->filename, (*p2)->filename);
  if (ret) return(ret);
  ret = (int) (*p1)->modified.tv_sec - (*p2)->modified.tv_sec;
  if (ret) return(ret);
  return((int) ((*p1)->modified.tv_usec - (*p2)->modified.tv_usec));
}

/*
 * do_return -- returns papers from files
 */

long
do_return(fxp, criterion, flags, arg)
     FX *fxp;
     Paper *criterion;
     int flags;
     char *arg;
{
  extern int errno;
  long code;
  Paperlist_res *plist;
  int count, i;
  char *s;
  char filename[256];
  Paper **paperv;
  struct stat buf;
  Paper gpaper;
  PaperType newtype;

  newtype = criterion->type;
  criterion->type = TAKEN;
  /******** get list of papers from server ********/
  code = fx_list(fxp, criterion, &plist);
  criterion->type = newtype;
  if (code) {
    strcpy(fxmain_error_context, "while retrieving list");
    return(code);
  }

  count = get_array(plist->Paperlist_res_u.list, &paperv);

  /******** deal with empty list ********/
  if (count == 0) {
    if (flags & VERBOSE)
      printf("No papers to return.\n");
    return(0L);
  }

  /******** main loop through list ********/
  for (i=0; i<count; i++) {

    /*** Skip duplicates ***/
    if (i < count-1 &&
	!strcmp(paperv[i]->author, paperv[i+1]->author) &&
	!strcmp(paperv[i]->filename, paperv[i+1]->filename)) {
      if (!(flags & LISTONLY)) fx_delete(fxp, paperv[i]);
      continue;
    }

    /******* Skip papers not in time range ********/
    if (paperv[i]->modified.tv_sec < criterion->created.tv_sec ||
        paperv[i]->modified.tv_sec > criterion->modified.tv_sec) continue;

    /*** Form filename ***/
    (void) sprintf(filename, "%s/%s", paperv[i]->author,
		   paperv[i]->filename);
    /* change spaces to underscores */
    for (s=filename; *s != '\0'; s++)
      if (isspace(*s)) *s = '_';

    if (flags & VERBOSE) {
      /******** print information about file ********/
      printf("%5d %-9s %9d  %-16.16s  %s\n", paperv[i]->assignment,
	     paperv[i]->owner, paperv[i]->size,
	     ctime(&(paperv[i]->created.tv_sec)), filename);
    }

    if (stat(filename, &buf)) {
      printf("    Couldn't return %s (%s)\n", filename,
	     error_message((long) errno));
      continue;
    }

    if (buf.st_mtime == paperv[i]->created.tv_sec) {
      printf("    Won't return %s (not modified)\n", filename);
      continue;
    }
    if (!(flags & LISTONLY)) {
      /******** return file to server ********/
      paper_copy(paperv[i], &gpaper);
      gpaper.type = newtype;
      code = fx_send_file(fxp, &gpaper, filename);
      if (code) {
	sprintf(fxmain_error_context, "while returning \"%s\"", filename);
	return(code);
      }
      printf("    Returned %s to %s.\n", filename, full_name(gpaper.author));
      fx_delete(fxp, paperv[i]);
    }
  }


  /******** clean up ********/
  fx_list_destroy(&plist);
  free((char *) paperv);
  return(0L);
}

main(argc, argv)
  int argc;
  char *argv[];
{
  Paper p;

  paper_clear(&p);
  p.type = GRADED;
  if (fxmain(argc, argv,
             "Usage: %s [-c course] [options] [assignment] [student ...]\n",
             &p, NULL, do_return)) exit(1);
  exit(0);
}
