/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/SmplLabel.h,v $
 * $Author: vanharen $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#ifndef _Xj_SmplLabel_h
#define _Xj_SmplLabel_h

#include "Jets.h"

extern JetClass smplLabelJetClass;
extern void smplSetLabel();
extern void smplSetPixmap();
extern void smplSetState();

typedef struct {int littlefoo;} SmplLabelClassPart;

typedef struct _SmplLabelClassRec {
  CoreClassPart		core_class;
  SmplLabelClassPart	smpl_label_class;
} SmplLabelClassRec;

extern SmplLabelClassRec smplLabelClassRec;

typedef struct _SmplLabelPart {
  int foreground, background;
  Boolean reverseVideo;
  XFontStruct *font;
  int justifyX, justifyY;
  int x, y;
  char *label;
  GC gc, gc_reverse;
  XjPixmap *pixmap;
  int inverted, padding;
} SmplLabelPart;

typedef struct _SmplLabelRec {
  CorePart	core;
  SmplLabelPart	s_label;
} SmplLabelRec;

typedef struct _SmplLabelRec *SmplLabelJet;
typedef struct _SmplLabelClassRec *SmplLabelJetClass;

#define XjCLabel "Label"
#define XjNlabel "label"

#define XjCIcon "Icon"
#define XjNicon "icon"

#endif /* _Xj_SmplLabel_h */
