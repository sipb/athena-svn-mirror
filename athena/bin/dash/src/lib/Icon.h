/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/Icon.h,v $
 * $Author: vanharen $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#include "Jets.h"

extern JetClass iconJetClass;
extern void SetIcon();

typedef struct {int littlefoo;} IconClassPart;

typedef struct _IconClassRec {
  CoreClassPart		core_class;
  IconClassPart	icon_class;
} IconClassRec;

extern IconClassRec iconClassRec;

typedef struct {
  XjPixmap *pixmap;
  GC gc, reversegc;
  int inverted;
  int foreground, background;
  Boolean reverseVideo;
  Boolean centerX, centerY;
  int x, y;
} IconPart;

typedef struct _IconRec {
  CorePart	core;
  IconPart	icon;
} IconRec;

typedef struct _IconRec *IconJet;
typedef struct _IconClassRec *IconJetClass;

#define XjCIcon "Icon"
#define XjNicon "icon"
