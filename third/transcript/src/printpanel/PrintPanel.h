/*
  Copyright (C) 1991,1992 Adobe Systems Incorporated. All rights reserved.
  RCSID: $Header: /afs/dev.mit.edu/source/repository/third/transcript/src/printpanel/PrintPanel.h,v 1.1.1.1 1996-10-07 20:25:55 ghudson Exp $
*/
#ifndef _PrintPanel_h
#define _PrintPanel_h

#define XtNokCallback "okCallback"
#define XtNcancelCallback "cancelCallback"
#define XtNinputFileName "inputFileName"
#define XtNfilterName "filterName"
#define XtNprinterName "printerName"


extern WidgetClass printPanelWidgetClass;

typedef struct _PrintPanelRec *PrintPanelWidget;
#endif






