/*
 * Copyright 1989 by the Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that copyright notice and this permission
 * notice appear in supporting documentation, and that the name of
 * M.I.T. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission.  M.I.T. makes no representations about the suitability
 * of this software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 *
 * MotifUtils:   Utilities for use with Motif and UIL
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/Mu/MuSetCursor.c,v $
 * $Author: djf $
 * $Log: not supported by cvs2svn $
 */

#include "Mu.h"
#include <X11/cursorfont.h>
#include <X11/Shell.h>

static void SetCursor(w,c)
Widget w;
Cursor c;
{
    while (XtIsSubclass(w, shellWidgetClass) != True) {
	w = XtParent(w);
    }
    XDefineCursor(XtDisplay(w), XtWindow(w), c);
    XFlush(XtDisplay(w));
}

static Cursor ArrowCursor = (Cursor)NULL;

void MuSetStandardCursor(widget)
Widget widget;
{
    if (ArrowCursor == (Cursor)NULL) 
	ArrowCursor = XCreateFontCursor(XtDisplay(widget), XC_top_left_arrow);
    SetCursor(widget,ArrowCursor);
}


static Cursor WatchCursor = (Cursor)NULL;

void MuSetWaitCursor(widget)
Widget widget;
{	
    if (WatchCursor == (Cursor)NULL)
        WatchCursor = XCreateFontCursor(XtDisplay(widget), XC_watch);
    SetCursor(widget,WatchCursor);
}

void MuSetCursor(widget, cursorshape)
Widget widget;
unsigned int cursorshape;
{
    SetCursor(widget,XCreateFontCursor(XtDisplay(widget), cursorshape));
}





