/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/Label.h,v $
 * $Author: vanharen $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#ifndef _Xj_Label_h
#define _Xj_Label_h

#include "Jets.h"

extern JetClass labelJetClass;
extern void setLabel();

typedef struct {int littlefoo;} LabelClassPart;

typedef struct _LabelClassRec {
  CoreClassPart		core_class;
  LabelClassPart	label_class;
} LabelClassRec;

extern LabelClassRec labelClassRec;

typedef struct {
  char *label;
  GC gc, gc_clear;
  int foreground, background;
  Boolean reverseVideo;
  XFontStruct *font;
  Boolean centerX, centerY;
  int x, y;
} LabelPart;

typedef struct _LabelRec {
  CorePart	core;
  LabelPart	label;
} LabelRec;

typedef struct _LabelRec *LabelJet;
typedef struct _LabelClassRec *LabelJetClass;

#define XjCLabel "Label"
#define XjNlabel "label"

#endif /* _Xj_Label_h */
