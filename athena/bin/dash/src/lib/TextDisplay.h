/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/TextDisplay.h,v $
 * $Author: cfields $ 
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
extern void SetLine();
extern void AddText();

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
  int charHeight;
  int displayWidth, displayHeight;
  int columns;
  char **lineStarts;
  int lineStartsSize;
  char *startSelect, *endSelect, *startPivot, *endPivot;
  char *selection;
  int startEnd;
  char *realStart, *realEnd;
  Boolean buttonDown;
  int whichButton;
  GC gc, selectgc, gc_clear, gc_fill;
  int foreground, background;
  int hl_foreground, hl_background;
  char *hl_fg_name, *hl_bg_name;
  Boolean reverseVideo;
  XFontStruct *font;
  XjCallback *resizeProc;
  XjCallback *scrollProc;
  int realized;
  int internalBorder;
  int multiClickTime;
  Time lastUpTime;
  short clickTimes;
  char *charClass;
  int scrollDelay1, scrollDelay2;
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
#define XjCScrollProc "ScrollProc"
#define XjNscrollProc "scrollProc"
#define XjCBorderWidth "BorderWidth"
#define XjNinternalBorder "internalBorder"
#define XjCMultiClickTime "MultiClickTime"
#define XjNmultiClickTime "multiClickTime"
#define XjCCharClass "CharClass"
#define XjNcharClass "charClass"
#define XjCScrollDelay "ScrollDelay"
#define XjNscrollDelay "scrollDelay"
#define XjNscrollDelay2 "scrollDelay2"
#define XjNhighlightForeground "highlightForeground"
#define XjNhighlightBackground "highlightBackground"
#define XjNdisplayWidth "displayWidth"
#define XjNdisplayHeight "displayHeight"
#define XjCDisplayWidth "DisplayWidth"
#define XjCDisplayHeight "DisplayHeight"

#endif /* _Xj_TextDisplay_h */
