/* This file is part of the Project Athena Zephyr Notification System.
 * It is one of the source files comprising zwgc, the Zephyr WindowGram
 * client.
 *
 *      Created by:     Marc Horowitz <marc@athena.mit.edu>
 *
 *      $Id: xerror.c,v 1.4 1999-01-22 23:20:43 ghudson Exp $
 *
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 */

#include <sysdep.h>

#if (!defined(lint) && !defined(SABER))
static const char rcsid_xerror_c[] = "$Id: xerror.c,v 1.4 1999-01-22 23:20:43 ghudson Exp $";
#endif

#include <zephyr/mit-copyright.h>

#ifndef X_DISPLAY_MISSING

#include <X11/Xlib.h>
#include "mux.h"

int xerror_happened;

/*ARGSUSED*/
static int xerrortrap(dpy,xerrev)
     Display *dpy;
     XErrorEvent *xerrev;
{
   xerror_happened = 1;
}

/*ARGSUSED*/
void begin_xerror_trap(dpy)
     Display *dpy;
{
   xerror_happened = 0;
   XSetErrorHandler(xerrortrap);
}

void end_xerror_trap(dpy)
     Display *dpy;
{
   XSync(dpy,False);
   XSetErrorHandler(NULL);
}

#endif

