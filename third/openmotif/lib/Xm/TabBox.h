#ifndef __TabBox_h__
#define __TabBox_h__

#if defined(VMS) || defined(__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/Ext.h>
#include <Xm/DrawUtils.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {XmTABS_SQUARED, XmTABS_ROUNDED, XmTABS_BEVELED} XmTabStyle;
typedef enum {XmTABS_BASIC, XmTABS_STACKED, XmTABS_STACKED_STATIC,
	      XiTABS_SCROLLED, XiTABS_OVERLAYED} XmTabMode;

typedef enum {XmTAB_ORIENTATION_DYNAMIC, XmTABS_RIGHT_TO_LEFT,
	      XmTABS_LEFT_TO_RIGHT, XmTABS_TOP_TO_BOTTOM,
	      XmTABS_BOTTOM_TO_TOP} XmTabOrientation;

typedef enum {XmTAB_EDGE_TOP_LEFT, XmTAB_EDGE_BOTTOM_RIGHT} XiTabEdge;

typedef enum {XiTAB_ARROWS_ON_RIGHT, XiTAB_ARROWS_ON_LEFT,
	      XiTAB_ARROWS_SPLIT} XiTabArrowPlacement;

enum {XiCR_TAB_SELECTED, XiCR_TAB_UNSELECTED};

typedef struct _XiTabBoxCallbackStruct {
    int       reason;
    XEvent    *event;
    int       tab_index;
    int       old_index;
} XiTabBoxCallbackStruct;

typedef struct _XiTabBoxRec      *XiTabBoxWidget;
typedef struct _XmTabBoxClassRec *XiTabBoxWidgetClass;

extern WidgetClass               xiTabBoxWidgetClass;

#ifndef XiIsTabBox
#define XiIsTabBox(w) XtIsSubclass(w, xiTabBoxWidgetClass)
#endif /* XiIsTabBox */

#ifdef _NO_PROTO
extern Widget XiCreateTabBox();
extern int XiTabBoxGetIndex();
extern int XiTabBoxGetNumRows();
extern int XiTabBoxGetNumColumns();
extern int XiTabBoxGetNumTabs();
extern int XiTabBoxGetRow();
extern int XiTabBoxXYToIndex();

#else
extern Widget XiCreateTabBox(Widget, String, ArgList, Cardinal);
extern int XiTabBoxGetIndex(Widget, int, int);
extern int XiTabBoxGetNumRows(Widget);
extern int XiTabBoxGetNumColumns(Widget);
extern int XiTabBoxGetNumTabs(Widget);
extern int XiTabBoxGetTabRow(Widget, int);
extern int XiTabBoxXYToIndex(Widget, int, int);

#endif

#ifdef __cplusplus
} /* Close scope of 'extern "C"' declaration */
#endif

#if defined(VMS) || defined(__VMS)
#include <X11/apienvrst.h>
#endif

#endif /* __TabBox_h__ */