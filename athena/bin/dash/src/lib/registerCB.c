/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/registerCB.c,v $
 * $Author: vanharen $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#if  (!defined(lint))  &&  (!defined(SABER))
static char rcsid[] =
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/registerCB.c,v 1.2 1991-12-17 11:37:18 vanharen Exp $";
#endif

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
  
  quark = XrmStringToQuark(name);
    
  if ( h == NULL ) 
    h = create_hash(HASHSIZE);

  if (hash_store(h, quark, (caddr_t)w) == 1)	/* if a callback already */
    {						/* exists by that name, */
      char errtext[100];			/* give a warning */ 

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

  for (i = 0; i < num; i++)
    XjRegisterCallback(c[i].proc, c[i].name);
}
