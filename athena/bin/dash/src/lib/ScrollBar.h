/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/ScrollBar.h,v $
 * $Author: vanharen $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#include "Jets.h"

extern JetClass scrollBarJetClass;
extern void SetScrollBar();
extern int GetScrollBarValue();

typedef struct {int littlefoo;} ScrollBarClassPart;

typedef struct _ScrollBarClassRec {
  CoreClassPart		core_class;
  ScrollBarClassPart	scrollBar_class;
} ScrollBarClassRec;

extern ScrollBarClassRec scrollBarClassRec;

typedef struct {
  XjCallback *changeProc;
  GC thumb_gc, line_gc;
  GC fg_gc, bg_gc;
  XjPixmap *thumb;
  int borderColor, foreground, background;
  int borderWidth;
  int borderThickness;
  int padding;
  Boolean inside;
  Boolean selected;
  Boolean pressed;
  int minimum, maximum;		/* minimum and maximum values */
  int visible, current;		/* visible amount, current value */
  int thumbX, thumbY;
  int thumbWidth, thumbHeight;	/* compute on resize... */
  int realized;
  Boolean reverseVideo;
  int vertCursorCode;
  Cursor vertCursor;
  int horizCursorCode;
  Cursor horizCursor;
  int upCursorCode;
  Cursor upCursor;
  int downCursorCode;
  Cursor downCursor;
  int leftCursorCode;
  Cursor leftCursor;
  int rightCursorCode;
  Cursor rightCursor;
  int orientation;
  Boolean showArrows;
  int arrowSize;
  int scrollBarSize;
} ScrollBarPart;

typedef struct _ScrollBarRec {
  CorePart	core;
  ScrollBarPart	scrollBar;
} ScrollBarRec;

typedef struct _ScrollBarRec *ScrollBarJet;
typedef struct _ScrollBarClassRec *ScrollBarJetClass;

#define XjCBorderThickness "BorderThickness"
#define XjNborderThickness "borderThickness"
#define XjCBorderWidth "BorderWidth"
#define XjNborderWidth "borderWidth"
#define XjCThumb "Thumb"
#define XjNthumb "thumb"
#define XjCChangeProc "ChangeProc"
#define XjNchangeProc "changeProc"
#define XjCBorderColor "BorderColor"
#define XjNborderColor "borderColor"
#define XjNverticalCursorCode "verticalCursorCode"
#define XjNhorizontalCursorCode "horizontalCursorCode"
#define XjNupCursorCode "upCursorCode"
#define XjNdownCursorCode "downCursorCode"
#define XjNleftCursorCode "leftCursorCode"
#define XjNrightCursorCode "rightCursorCode"
#define XjCCursorCode "CursorCode"
#define XjNorientation "orientation"
#define XjCOrientation "Orientation"
#define XjNshowArrows "showArrows"
#define XjCShowArrows "ShowArrows"
