/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/StripChart.h,v $
 * $Author: vanharen $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#include "Jets.h"

extern JetClass stripChartJetClass;

typedef struct {int littlefoo;} StripChartClassPart;

typedef struct _StripChartClassRec {
  CoreClassPart		core_class;
  StripChartClassPart	stripChart_class;
} StripChartClassRec;

extern StripChartClassRec stripChartClassRec;

typedef struct {
  GC gc, scalegc;
  int foreground, background, scaleColor;
  Boolean dotScale;
  Boolean reverseVideo;
  XjCallback *dataProc;
  int x, *data;
  int scaleInc, scale;
  int interval;
  int scrollDist;
  int parity;
} StripChartPart;

typedef struct _StripChartRec {
  CorePart	core;
  StripChartPart	stripChart;
} StripChartRec;

typedef struct _StripChartRec *StripChartJet;
typedef struct _StripChartClassRec *StripChartJetClass;

extern void XjStripChartData();

typedef struct _StripChartInitRec {
  StripChartJet j;
  int interval;
} StripChartInit;

#define XjCDataProc "DataProc"
#define XjNdataProc "dataProc"
#define XjCScale "Scale"
#define XjNscale "scale"
#define XjCScaleInc "ScaleInc"
#define XjNscaleInc "scaleInc"
#define XjCInterval "Interval"
#define XjNinterval "interval"
#define XjCScrollDistance "ScrollDistance"
#define XjNscrollDistance "scrollDistance"
#define XjNscaleColor "scaleColor"
#define XjCScaleColor "ScaleColor"
#define XjNdotScale "dotScale"
#define XjCDotScale "DotScale"
