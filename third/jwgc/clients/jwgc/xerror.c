/*
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 *
 *      Modified for jwgc by Daniel Henninger.
 */

#include "mit-copyright.h"

#ifndef X_DISPLAY_MISSING

#include <X11/Xlib.h>
#include "mux.h"

int xerror_happened;

/* ARGSUSED */
static int 
xerrortrap(dpy, xerrev)
	Display *dpy;
	XErrorEvent *xerrev;
{
	xerror_happened = 1;
	return (1);
}

/* ARGSUSED */
void 
begin_xerror_trap(dpy)
	Display *dpy;
{
	xerror_happened = 0;
	XSetErrorHandler(xerrortrap);
}

void 
end_xerror_trap(dpy)
	Display *dpy;
{
	XSync(dpy, False);
	XSetErrorHandler(NULL);
}

#endif
