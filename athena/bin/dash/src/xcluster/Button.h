/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/xcluster/Button.h,v $
 * $Author: epeisach $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#include "Jets.h"

extern JetClass buttonJetClass;

typedef struct {int littlefoo;} ButtonClassPart;

typedef struct _ButtonClassRec {
  CoreClassPart		core_class;
  ButtonClassPart	button_class;
} ButtonClassRec;

extern ButtonClassRec buttonClassRec;

typedef struct {
  XjCallback *activateProc;
  GC gc;
  GC foreground_gc;
  GC background_gc;
  GC invert_gc;
  int foreground, background;
  Boolean reverseVideo;
  int borderWidth;
  int borderThickness;
  int padding;
  Boolean inside;
  Boolean selected;
  Boolean pressed;
} ButtonPart;

typedef struct _ButtonRec {
  CorePart	core;
  ButtonPart	button;
} ButtonRec;

typedef struct _ButtonRec *ButtonJet;
typedef struct _ButtonClassRec *ButtonJetClass;

#define XjCBorderThickness "BorderThickness"
#define XjNborderThickness "borderThickness"
#define XjCBorderWidth "BorderWidth"
#define XjNborderWidth "borderWidth"
