/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/Arrow.h,v $
 * $Author: vanharen $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#ifndef _Xj_Arrow_h
#define _Xj_Arrow_h

#include "Jets.h"

extern JetClass arrowJetClass;
extern void setDirection();

typedef struct {int littlefoo;} ArrowClassPart;

typedef struct _ArrowClassRec {
  CoreClassPart		core_class;
  ArrowClassPart	arrow_class;
} ArrowClassRec;

extern ArrowClassRec arrowClassRec;

typedef struct _ArrowPart {
  int foreground, background, fillColor;
  Boolean reverseVideo;
  int direction;
  int x, y;
  GC gc, gc_reverse, gc_fill;
  int padding;
  XPoint pt[11];
  short num_pts;
  XjCallback *highlightProc;
  XjCallback *unHighlightProc;
} ArrowPart;

typedef struct _ArrowRec {
  CorePart	core;
  ArrowPart	arrow;
} ArrowRec;

typedef struct _ArrowRec *ArrowJet;
typedef struct _ArrowClassRec *ArrowJetClass;

#define XjCArrow "Arrow"
#define XjNarrow "arrow"

#define XjCDirection "Direction"
#define XjNdirection "direction"
#define XjNfillColor "fillColor"

#endif /* _Xj_Arrow_h */
