/*
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/xmore/ScrollByLine.h,v 1.1 1989-10-15 04:32:30 probe Exp $
 */

/* 
 * ScrollByLine.h - Public definitions for ScrollByLine widget.
 * 
 * Author:	Chris Peterson
 * 		M. I. T. Project Athena.
 *              Cambridge, Mass.
 * Date:	12/5/87
 *
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
