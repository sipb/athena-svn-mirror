/*
 * $Id: Drawing.h,v 1.2 1999-01-22 23:16:51 ghudson Exp $
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#ifndef _Xj_Drawing_h
#define _Xj_Drawing_h

#include "Jets.h"

extern JetClass drawingJetClass;

typedef struct {int littlefoo;} DrawingClassPart;

typedef struct _DrawingClassRec {
  CoreClassPart		core_class;
  DrawingClassPart	drawing_class;
} DrawingClassRec;

extern DrawingClassRec drawingClassRec;

typedef struct {
  XjCallback *exposeProc;
  XjCallback *realizeProc;
  XjCallback *resizeProc;
  XjCallback *destroyProc;
  XjCallback *eventProc;
  GC foreground_gc;
  GC background_gc;
  GC invert_gc;
  int foreground, background;
  Boolean reverseVideo;
  int lineWidth;
  XFontStruct *font;
} DrawingPart;

typedef struct _DrawingRec {
  CorePart	core;
  DrawingPart	drawing;
} DrawingRec;

typedef struct _DrawingRec *DrawingJet;
typedef struct _DrawingClassRec *DrawingJetClass;

#define XjCExposeProc "ExposeProc"
#define XjNexposeProc "exposeProc"
#define XjCRealizeProc "RealizeProc"
#define XjNrealizeProc "realizeProc"
#define XjCResizeProc "ResizeProc"
#define XjNresizeProc "resizeProc"
#define XjCDestroyProc "DestroyProc"
#define XjNdestroyProc "destroyProc"
#define XjCEventProc "EventProc"
#define XjNeventProc "eventProc"
#define XjCLineWidth "LineWidth"
#define XjNlineWidth "lineWidth"

#endif /* _Xj_Drawing_h */
