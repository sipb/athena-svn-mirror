/**********************************************************************
 * File Exchange client library
 *
 * $Author: probe $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_host_list.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_host_list.c,v 1.1 1993-10-12 03:03:16 probe Exp $
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_fx_host_list_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/fx_host_list.c,v 1.1 1993-10-12 03:03:16 probe Exp $";
#endif /* lint */

#include <strings.h>
#ifdef HESIOD
#include <hesiod.h>
#endif /* HESIOD */
#include <fxcl.h>
#include <memory.h>

char *getenv();			/* this ought to be in a system .h file */

stringlist
fx_host_list()
{
  stringlist ret = NULL;
  stringlist node, newret;
  char *fxpath, *s;
  int i, count=0;
#ifdef HESIOD
  char **hesinfo;
#endif /* HESIOD */

  /* allocate first node of stringlist */
  if ((ret = New(stringnode)) == NULL) return(NULL);
  ret->next = NULL;
  ret->s = NULL;
  
  /* if FXPATH environment variable is set, use it */
  if ((fxpath = getenv(ENV_FXPATH)) != NULL) {

    for (node = ret; node; node = node->next = New(stringnode)) {
      node->next = NULL;

      /* find the next host */
      if (*fxpath == '\0') break;
      s = index(fxpath, ':');
      if (s) i = (int) (s - fxpath);
      else i = strlen(fxpath);

      /* allocate space to copy it into */
      if ((node->s = NewArray(char, i+1)) == NULL) {
	fx_host_list_destroy(ret);
	return(NULL);
      }

      /* copy it */
      (void) strncpy(node->s, fxpath, i);
      node->s[i] = '\0';
      count++;

      /* get ready to copy the next name */
      if (!s) break;
      fxpath = 1 + s;
    }
  }
#ifdef HESIOD
  else {
    /* Get server locations from hesiod */
    if ((hesinfo = hes_resolve("turnin", "sloc")) == NULL) {
      fx_host_list_destroy(ret);
      return(NULL);
    }

    for (node = ret; node; node = node->next = New(stringnode)) {
      node->next = NULL;

      if ((node->s = newstring(*hesinfo)) == NULL) {
	fx_host_list_destroy(ret);
	return(NULL);
      }
      (void) strcpy(node->s, *hesinfo++);
      count++;
      if (*hesinfo == NULL) break;
    }
  }
#endif /* HESIOD */
  /*
   * Now we rotate the list of server hosts based on pid.  This will
   * cause fxopen to choose a server approximately at random, evening
   * the load among servers.
   */
  newret = ret;
  for(i = getpid()%count ; i>0 ; i--) {
    node = newret;
    if (newret->next) newret = newret->next;
    if (i == 1) node->next = NULL;
  }
  if (newret != ret) {
    node = newret;
    while (node->next) node = node->next;
    node->next = ret;
    ret = newret;
  }
  return(ret);
}
