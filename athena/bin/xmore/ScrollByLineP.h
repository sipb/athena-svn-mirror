/*
 * xman - X window system manual page display program.
 *
 * $XConsortium: ScrollByLP.h,v 1.2 88/09/06 17:47:30 jim Exp $
 * $Athena: ScrollByLP.h,v 4.0 88/08/31 22:11:21 kit Exp $
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

#ifndef _XtScrollByLinePrivate_h
#define _XtScrollByLinePrivate_h

#define DEFAULT_WIDTH 300
#define DEFAULT_HEIGHT 200

/***********************************************************************
 *
 * ScrollByLine Widget Private Data
 *
 ***********************************************************************/

/* New fields for the ScrollByLine widget class record */
typedef struct {
     int mumble;   /* No new procedures */
} ScrollByLineClassPart;

/* Full class record declaration */
typedef struct _ScrollByLineClassRec {
    CoreClassPart	core_class;
    CompositeClassPart  composite_class;
    ScrollByLineClassPart scrolled_widget_class;
} ScrollByLineClassRec;

extern ScrollByLineClassRec scrollByLineClassRec;

/* New fields for the ScrollByLine widget record */
typedef struct _ScrollByLinePart {
  Pixel foreground;		/* The color for the forground of the text. */
  int inner_width, inner_height; /* The (viewable) size of the inner widget. */
  Boolean force_bars,		/* Must have bars visable */
    allow_horiz,		/* allow use of horizontal scroll bar. */
    allow_vert,			/* allow use of vertical scroll bar. */
    use_bottom,			/* put scroll bar on bottom of window. */
    use_right;			/* put scroll bar on right side of window. */
  int lines;			/* The number of lines in the text. */
  int font_height;		/* the height of the font. */
  XtCallbackList callbacks;	/* The callback list. */
  Boolean key;			/* which window will we size on
				   (TRUE == INNER). */

/* variables not in resource list. */

  int line_pointer;		/* The line that currently is at the top 
				   of the window being displayed. */
} ScrollByLinePart;


/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _ScrollByLineRec {
    CorePart	    core;
    CompositePart   composite;
    ScrollByLinePart  scroll_by_line;
} ScrollByLineRec;

#endif _XtScrollByLinePrivate_h
/* DON'T ADD STUFF AFTER THIS #endif */
