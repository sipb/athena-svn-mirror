/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/xcluster/registerCallbacks.c,v $
 * $Author: probe $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#ifndef	lint
static char rcsid[] =
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/xcluster/registerCallbacks.c,v 1.1 1993-10-12 05:32:27 probe Exp $";
#endif	lint

#include "mit-copyright.h"
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include "Jets.h"
#include "hash.h"

static struct hash *h = NULL;

void XjRegisterCallback(w, name)
XjCallbackProc w;
char *name;
{
  XrmQuark quark;
  int i;
  
  quark = XrmStringToQuark(name);
    
  if ( h == NULL ) 
    h = create_hash(HASHSIZE);

  i = hash_store(h, quark, (caddr_t)w);
  
  if (i == 1)		/* if a callback already exists by that name, warn */
    {
      char errtext[100];

      sprintf(errtext,
	      "XjRegisterCallbacks: duplicate procedure registration: %s",
	      name);
      XjWarning(errtext);
    }
}

XjCallbackProc XjGetCallback(string)
      char *string;
{
   XrmQuark quark;
   XjCallbackProc w;

   if (h == NULL)
     return NULL;

   quark = XrmStringToQuark(string);
   
   w = (XjCallbackProc)(hash_lookup(h, quark));
   return(w);
 }
   
void XjRegisterCallbacks(c, num)
XjCallbackRec c[];
int num;
{
  int i;
  XrmQuark quark;

  if (h == NULL)
    h = create_hash(HASHSIZE);

  for (i = 0; i < num; i++)
    {
      quark = XrmStringToQuark(c[i].name);
      if (1 == hash_store(h, quark, (caddr_t) c[i].proc))
        {
	  char errtext[100];

          sprintf(errtext,
		  "XjRegisterCallbacks: duplicate procedure registration: %s",
                  c[i].name);
          XjWarning(errtext);
        }
    }
}
