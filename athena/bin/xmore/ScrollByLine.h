/*
 * xman - X window system manual page display program.
 *
 * $XConsortium: ScrollByL.h,v 1.2 88/09/06 17:47:28 jim Exp $
 * $Athena: ScrollByL.h,v 4.0 88/08/31 22:11:16 kit Exp $
 *
 * Copyright 1987, 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:    Chris D. Peterson, MIT Project Athena
 * Created:   December 5, 1987
 */

#ifndef _XtScrollByLine_h
#define _XtScrollByLine_h

#define XtNlines "lines"
#define XtCLine  "line_class"
#define XtNfontHeight "font_height"
#define XtNformOnInner "form_on_inner"

/***********************************************************************
 *
 * ScrollByLine Widget (subclass of CompositeClass)
 *
 ***********************************************************************/

/* Class record constants */

extern WidgetClass scrollByLineWidgetClass;

typedef struct _ScrollByLineClassRec *ScrollByLineWidgetClass;
typedef struct _ScrollByLineRec      *ScrollByLineWidget;

typedef struct _ScrollByLineStruct {
  int location;			/* The location so start writing text in the
				   child window widget. */
  int start_line;		/* The line to start printing text. */
  int num_lines;		/* The number of lines to print. */
} ScrollByLineStruct;

/* public routines */

extern Widget XtScrollByLineWidget(); /* the ScrollByLine Widget. */
/* Widget w; */

extern void XtResetScrollByLine();
/* Widget w; */

#endif _XtScrollByLine_h
/* DON'T ADD STUFF AFTER THIS #endif */
