/* Based on xqcam.c by Paul Chinn <loomer@svpal.org> */

#include "config.h"

/* gcc -ansi -pedantic on GNU/Linux causes warnings and errors
 * unless this is defined:
 * warning: #warning "Files using this header must be compiled with _SVID_SOURCE or _XOPEN_SOURCE"
 */
#ifndef _XOPEN_SOURCE
#  define _XOPEN_SOURCE 1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xproto.h>
#include <X11/Xmd.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include "gstxwindow.h"

GstXWindow *
_gst_xwindow_new (gint width, gint height, gboolean toplevel)
{
  XGCValues values;
  GstXWindow *new;

  new = g_new0 (GstXWindow, 1);

  new->width = width;
  new->height = height;

  new->disp = XOpenDisplay (NULL);
  if (!new->disp) {
    return NULL;
  }

  new->screen = DefaultScreenOfDisplay (new->disp);
  new->screen_num = DefaultScreen (new->disp);
  new->white = XWhitePixel (new->disp, new->screen_num);
  new->black = XBlackPixel (new->disp, new->screen_num);

  new->root = DefaultRootWindow (new->disp);

  new->win = XCreateWindow (new->disp, DefaultRootWindow (new->disp), 
		            0, 0, new->width, new->height, 0, CopyFromParent, 
			    CopyFromParent, CopyFromParent, 0, NULL);

  if (!new->win) {
    XCloseDisplay (new->disp);
    g_free (new);
    return NULL;
  }

  XSelectInput (new->disp, new->win, ExposureMask | StructureNotifyMask);

  new->gc = XCreateGC (new->disp, new->win, 0, &values);
  new->depth = DefaultDepthOfScreen (new->screen);

  if (toplevel) {
    XMapRaised (new->disp, new->win);
  }

  return new;
}

void
_gst_xwindow_destroy (GstXWindow * window)
{
  XFreeGC (window->disp, window->gc);
  XCloseDisplay (window->disp);
  g_free (window);
}

void
_gst_xwindow_resize (GstXWindow * window, gint width, gint height)
{
  XResizeWindow (window->disp, window->win, width, height);
}
