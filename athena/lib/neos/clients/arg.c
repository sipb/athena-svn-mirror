/**********************************************************************
 * File Exchange client routines
 *
 * $Author: ghudson $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/arg.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/arg.c,v 1.2 1996-09-20 04:34:21 ghudson Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_arg_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/arg.c,v 1.2 1996-09-20 04:34:21 ghudson Exp $";
#endif /* lint */

#include <stdio.h>
#include <fxcl.h>
#include <string.h>
#include <ctype.h>

/*** Global variables ***/

extern char Course[];
extern Paperlist_res *Plist;
extern Paper Criterion;

count_papers(plist)
     Paperlist plist;
{
  int i = 0;
  Paperlist node;

  for (node = plist; node; node = node->next) i++;
  return(i);
}

paper_named(s)
     char *s;
{
  int i=0;
  Paperlist node;

  for (node = Plist->Paperlist_res_u.list; node; node = node->next) {
    i++;
    if (strcmp(s, node->p.filename) == 0) return(i);
  }
  return(0);
}

low_bound(arg)
     char *arg;
{
  if (isalpha(arg[0])) return(paper_named(arg));
  if (arg[0] == ',' || arg[0] == ':' || strcmp(arg, "all") == 0
      || strcmp(arg, "*") == 0) return(1);
  return(atoi(arg));
}

high_bound(arg)
     char *arg;
{
  char *s;

  if (isalpha(arg[0])) return(paper_named(arg));
  if (arg[strlen(arg)-1] == ',' || arg[strlen(arg)-1] == ':'
      || strcmp(arg, "*") == 0 || strcmp(arg, "all") == 0)
    return(count_papers(Plist->Paperlist_res_u.list));
  
  if (strchr(arg, ',')) s = strchr(arg, ',');
  else s = strchr(arg, ':');

  if (s) return(atoi(s+1));
  else return(atoi(arg));
}
