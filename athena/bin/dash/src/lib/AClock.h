/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/AClock.h,v $
 * $Author: vanharen $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#include "Jets.h"

#define ANGLES 256 /* 1K of data for 90 degrees */
#define CIRCLE (4 * ANGLES)
#define SCALE 65536
#define PI 3.14159365358979
#define HALFPI (PI / 2.0)

extern JetClass aClockJetClass;

typedef struct {int littlefoo;} AClockClassPart;

typedef struct _AClockClassRec {
  CoreClassPart		core_class;
  AClockClassPart	aClock_class;
} AClockClassRec;

extern AClockClassRec aClockClassRec;

typedef struct {
  GC foregroundGC, backgroundGC, handsGC, highlightGC;
  int foreground, background, hands, highlight;
  Boolean reverseVideo;
  int centerx, centery;
  int xradius, yradius;
  int minorTick, majorTick;
  int minorStart, minorEnd;
  int majorStart, majorEnd;
  int padding;
  int secondStart, secondEnd, secondArc;
  int minuteStart, minuteEnd, minuteArc;
  int hourStart, hourEnd, hourArc;
  int h, m, s;
  int realized;
  int timerid;
  int update;
  Boolean keepRound;
#ifdef SHAPE
  Boolean round;
  Pixmap mask;
  GC setGC, eraseGC;
#endif
} AClockPart;

typedef struct _AClockRec {
  CorePart	core;
  AClockPart	aClock;
} AClockRec;

typedef struct _AClockRec *AClockJet;
typedef struct _AClockClassRec *AClockJetClass;

#define XjCTick "Tick"
#define XjNminorTick "minorTick"
#define XjNmajorTick "majorTick"
#define XjNmajorStart "majorStart"
#define XjNmajorEnd "majorEnd"
#define XjNminorStart "minorStart"
#define XjNminorEnd "minorEnd"
#define XjCArc "Arc"
#define XjNsecondArc "secondArc"
#define XjCHand "Hand"
#define XjNsecondStart "secondStart"
#define XjNsecondEnd "secondEnd"
#define XjNminuteStart "minuteStart"
#define XjNminuteEnd "minuteEnd"
#define XjNhourStart "hourStart"
#define XjNhourEnd "hourEnd"
#define XjNminuteArc "minuteArc"
#define XjNhourArc "hourArc"
#define XjCUpdate "Update"
#define XjNupdate "update"
#define XjCInterval "Interval"
#define XjNinterval "interval"
#define XjCKeepRound "KeepRound"
#define XjNkeepRound "keepRound"
#define XjNhands "hands"
#define XjNhighlight "highlight"
#ifdef SHAPE
#define XjCRound "Round"
#define XjNround "round"
#endif
