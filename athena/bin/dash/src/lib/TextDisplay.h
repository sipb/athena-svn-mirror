/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/TextDisplay.h,v $
 * $Author: vanharen $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#ifndef _Xj_TextDisplay_h
#define _Xj_TextDisplay_h

#include "Jets.h"

extern JetClass textDisplayJetClass;
extern void SetText();
extern void MoveText();
extern int CountLines();
extern int VisibleLines();

typedef struct {int littlefoo;} TextDisplayClassPart;

typedef struct _TextDisplayClassRec {
  CoreClassPart		core_class;
  TextDisplayClassPart	textDisplay_class;
} TextDisplayClassRec;

extern TextDisplayClassRec textDisplayClassRec;

typedef struct {
  char *text;
  int topLine, numLines, visLines;
  int charWidth;
  int columns;
  char **lineStarts;
  int lineStartsSize;
  char *startSelect, *endSelect, *startPivot, *endPivot;
  char *selection;
  int startEnd;
  char *realStart, *realEnd;
  Boolean buttonDown;
  int whichButton;
  GC gc, selectgc;
  int foreground, background;
  Boolean reverseVideo;
  XFontStruct *font;
  XjCallback *resizeProc;
  int realized;
  int internalBorder;
  int multiClickTime;
  Time lastUpTime;
  short clickTimes;
} TextDisplayPart;

typedef struct _TextDisplayRec {
  CorePart	core;
  TextDisplayPart	textDisplay;
} TextDisplayRec;

typedef struct _TextDisplayRec *TextDisplayJet;
typedef struct _TextDisplayClassRec *TextDisplayJetClass;

#define XjCText "Text"
#define XjNtext "text"
#define XjCResizeProc "ResizeProc"
#define XjNresizeProc "resizeProc"
#define XjCBorderWidth "BorderWidth"
#define XjNinternalBorder "internalBorder"
#define XjCMultiClickTime "MultiClickTime"
#define XjNmultiClickTime "multiClickTime"

#endif /* _Xj_TextDisplay_h */
