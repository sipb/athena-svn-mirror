/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/Button.h,v $
 * $Author: vanharen $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#ifndef _Xj_Button_h
#define _Xj_Button_h

#include "Jets.h"

extern void SetToggleState();
extern Boolean GetToggleState();

extern JetClass buttonJetClass;

typedef struct {int littlefoo;} ButtonClassPart;

typedef struct _ButtonClassRec {
  CoreClassPart		core_class;
  ButtonClassPart	button_class;
} ButtonClassRec;

extern ButtonClassRec buttonClassRec;

typedef struct {
  XjCallback *activateProc;
  XjCallback *deactivateProc;
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
  Boolean toggle;
  Boolean state;
  int repeatDelay;
  int initialDelay;
  int timerid;
  Boolean highlightOnEnter;
  XjCallbackProc hilite, unhilite;
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
#define XjCToggle "Toggle"
#define XjNtoggle "toggle"
#define XjCState "State"
#define XjNstate "state"
#define XjCDeactivateProc "DeactivateProc"
#define XjNdeactivateProc "deactivateProc"
#define XjCInterval "Interval"
#define XjNinterval "interval"
#define XjNrepeatDelay "repeatDelay"
#define XjNinitialDelay "initialDelay"
#define XjCHighlightOnEnter "HighlightOnEnter"
#define XjNhighlightOnEnter "highlightOnEnter"

#endif /* _Xj_Button_h */
