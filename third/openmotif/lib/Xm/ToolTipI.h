/**
 *
 * $Id: ToolTipI.h,v 1.1.1.1 2002-04-27 01:53:51 ghudson Exp $
 *
 **/

#ifndef _XmToolTip_h
#define _XmToolTip_h
#if 0 
#ifndef XmNtoolTipString
#define XmNtoolTipString "toolTipString"
#endif
#ifndef XmCToolTipString
#define XmCToolTipString "ToolTipString"
#endif
#ifndef XmNtoolTipPostDelay
#define XmNtoolTipPostDelay "toolTipPostDelay"
#endif
#ifndef XmCToolTipPostDelay
#define XmCToolTipPostDelay "ToolTipPostDelay"
#endif
#ifndef XmNtoolTipPostDuration
#define XmNtoolTipPostDuration "toolTipPostDuration"
#endif
#ifndef XmCToolTipPostDuration
#define XmCToolTipPostDuration "ToolTipPostDuration"
#endif
#ifndef XmNtoolTipEnable
#define XmNtoolTipEnable "toolTipEnable"
#endif
#ifndef XmCToolTipEnable
#define XmCToolTipEnable "ToolTipEnable"
#endif
#endif
#ifdef __cplusplus
extern "C" {
#endif

void _XmToolTipEnter(Widget wid, XEvent *event, String *params, Cardinal *num_params);
void _XmToolTipLeave(Widget wid, XEvent *event, String *params, Cardinal *num_params);

#ifdef __cplusplus
}
#endif

#endif
