/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/DClock.h,v $
 * $Author: vanharen $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#include "Jets.h"

extern JetClass dClockJetClass;

typedef struct {int littlefoo;} DClockClassPart;

typedef struct _DClockClassRec {
  CoreClassPart		core_class;
  DClockClassPart	dClock_class;
} DClockClassRec;

extern DClockClassRec dClockClassRec;

#define MAX_FMTS 32

typedef struct {
  GC gc, gc_bkgnd;
  int foreground;
  int background;
  Boolean reverseVideo;
  XFontStruct *font;
  Boolean centerY;
  int justify;
  int padding;
  Pixmap pmap;
  int pmap_ht;
  int timerid;
  int update;
  char *format[2];
  char fmts[2][1024];
  int fmt_type[2][MAX_FMTS];
  int num_fmts[2];
  int current_fmt;
  Boolean blink_colons;
  Boolean colons_on;
} DClockPart;

typedef struct _DClockRec {
  CorePart	core;
  DClockPart	dClock;
} DClockRec;

typedef struct _DClockRec *DClockJet;
typedef struct _DClockClassRec *DClockJetClass;

#define XjCFormat "Format"
#define XjNformat "format"
#define XjNformat2 "format2"
#define XjCUpdate "Update"
#define XjNupdate "update"
#define XjCInterval "Interval"
#define XjNinterval "interval"
#define XjCBlink "Blink"
#define XjNblinkColons "blinkColons"
