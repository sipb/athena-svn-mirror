/* Copyright 1997 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

static const char rcsid[] = "$Id: Snoop.c,v 1.1 1997-02-25 19:09:17 ghudson Exp $";

#include <stdio.h>
#include <Jets.h>
#include "Snoop.h"

#define offset(field) XjOffset(Snoop_jet,field)

static XjResource resources[] = {
  { XjNeventProc, XjCEventProc, XjRCallback,
    sizeof(XjCallback *), offset(snoop.event_proc), XjRString, NULL },
};

#undef offset

static void realize(Snoop_jet), destroy(Snoop_jet);
static Boolean event_handler(Snoop_jet, XEvent *);

Snoop_class_rec snoop_class_rec = {
  {
    /* class name */		"Snoop",
    /* jet size   */		sizeof(Snoop_rec),
    /* classInitialize */	NULL,
    /* classInitialized? */	1,
    /* initialize */		NULL,
    /* prerealize */    	NULL,
    /* realize */		realize,
    /* event */			event_handler,
    /* expose */		NULL,
    /* querySize */     	NULL,
    /* move */			NULL,
    /* resize */        	NULL,
    /* destroy */       	NULL,
    /* resources */		resources,
    /* number of 'em */		XjNumber(resources)
  }
};

JetClass snoop_jet_class = (JetClass)&snoop_class_rec;

static void realize(Snoop_jet me)
{
  /* If this were general purpose, we'd have to have XjGetOwner
     in the library. But since it isn't (it doesn't have to be
     deinstalled) it doesn't matter. */

  /* me->snoop.lastOwner = XjGetOwner(me->core.window); */
  XjRegisterWindow(me->core.window, (Jet)me);
}

static void destroy(Snoop_jet me)
{
  /* XjRegisterWindow(me->core.window, me->snoop.lastOwner); */
}

static Boolean event_handler(Snoop_jet me, XEvent *event)
{
  XjCallCallbacks(me, me->snoop.event_proc, event);
  return False;
}
