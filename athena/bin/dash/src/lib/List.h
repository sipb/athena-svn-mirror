/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/List.h,v $
 * $Author: probe $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#ifndef _Xj_List_h
#define _Xj_List_h

#include "Jets.h"

extern JetClass listJetClass;

typedef struct {int littlefoo;} ListClassPart;

typedef struct _ListClassRec {
  CoreClassPart		core_class;
  ListClassPart	list_class;
} ListClassRec;

extern ListClassRec listClassRec;

typedef struct {
  XjCallback *resizeProc;
  XjCallback *scrollProc;
  int internalBorder;
  int foreground, background;
  Boolean reverseVideo, buttonDown;
  int whichButton;
  int multiClickTime;
  Time lastUpTime;
  short clickTimes;
  int scrollDelay1, scrollDelay2;
  GC invert_gc, foreground_gc, background_gc;
  Jet focus, selected;
  int listMode;
  Boolean autoSelect, alwaysOne;
} ListPart;

typedef struct _ListRec {
  CorePart	core;
  ListPart	list;
} ListRec;

typedef struct _ListRec *ListJet;
typedef struct _ListClassRec *ListJetClass;

#define XjCResizeProc "ResizeProc"
#define XjNresizeProc "resizeProc"
#define XjCBorderWidth "BorderWidth"
#define XjNinternalBorder "internalBorder"
#define XjCResizeProc "ResizeProc"
#define XjNresizeProc "resizeProc"
#define XjCScrollProc "ScrollProc"
#define XjNscrollProc "scrollProc"
#define XjCMultiClickTime "MultiClickTime"
#define XjNmultiClickTime "multiClickTime"
#define XjCScrollDelay "ScrollDelay"
#define XjNscrollDelay "scrollDelay"
#define XjNscrollDelay2 "scrollDelay2"
#define XjNlistMode "listMode"
#define XjCListMode "ListMode"
#define XjRListMode "ListMode"
#define XjNautoSelect "autoSelect"
#define XjCAutoSelect "AutoSelect"
#define XjNalwaysOne "alwaysOne"
#define XjCAlwaysOne "AlwaysOne"

#define XjSingleSelect "SingleSelect"
#define XjMultiSelect "MultiSelect"

#define SingleSelect	0
#define MultiSelect	1

#endif /* _Xj_List_h */
