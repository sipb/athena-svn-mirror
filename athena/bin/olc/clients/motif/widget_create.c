/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains the create procedures for the widgets used in the
 * X-based interface.
 *
 *      Chris VanHaren
 *      MIT Project Athena
 *
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/widget_create.c,v $
 *      $Author: vanharen $
 */

#ifndef lint
static char rcsid[]="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/widget_create.c,v 1.3 1989-09-13 03:18:31 vanharen Exp $";
#endif

#include "xolc.h"

void widget_create (w, tag, callback_data)
     Widget w;
     int *tag;
     XmAnyCallbackStruct *callback_data;
{
  Arg arg;

  switch (*tag) {
  case NEWQ_BTN:
    w_newq_btn = w;
    break;
  case CONTQ_BTN:
    w_contq_btn = w;
    break;
  case STOCK_BTN:
    w_stock_btn = w;
    break;
  case QUIT_BTN:
    w_quit_btn = w;
    break;
  case HELP_BTN:
    w_help_btn = w;
    break;
  case CONTQ_FORM:
    w_contq_form = w;
    break;
  case CONNECT_LBL:
    w_connect_lbl = w;
    break;
  case TOPIC_LBL:
    w_topic_lbl = w;
    break;
  case REPLAY_SCRL:
    w_replay_scrl = w;
    break;
  case SEND_BTN:
    w_send_btn = w;
    break;
  case DONE_BTN:
    w_done_btn = w;
    break;
  case CANCEL_BTN:
    w_cancel_btn = w;
    break;
  case SAVELOG_BTN:
    w_savelog_btn = w;
    break;
  case MOTD_BTN:
    w_motd_btn = w;
    break;
  case UPDATE_BTN:
    w_update_btn = w;
    break;
  case MOTD_DLG:
    w_motd_dlg = w;
    XtDestroyWidget(XmMessageBoxGetChild(w, XmDIALOG_CANCEL_BUTTON));
    XtDestroyWidget(XmMessageBoxGetChild(w, XmDIALOG_HELP_BUTTON));
    break;
  case HELP_DLG:
    w_help_dlg = w;
    XtDestroyWidget(XmMessageBoxGetChild(w, XmDIALOG_CANCEL_BUTTON));
    XtDestroyWidget(XmMessageBoxGetChild(w, XmDIALOG_HELP_BUTTON));
    break;
  case QUIT_DLG:
    w_quit_dlg = w;
    XtSetArg(arg, XmNmessageString,
	     XmStringLtoRCreate("`Quit' means that you simply want to get out of this program.\nTo continue this question, just type `xolc' again.\n\nRemember, your question is still active until you mark it\n`done' or `cancel' it.  Otherwise, it will remain active until a\nconsultant can answer it.\n\nIf you logout, a consultant will send you mail.\n\nDo you wish to quit OLC at this time?",
				""));
    XtSetValues(w_quit_dlg, &arg, 1);
    XtDestroyWidget(XmMessageBoxGetChild(w, XmDIALOG_HELP_BUTTON));
    break;
  case ERROR_DLG:
    w_error_dlg = w;
    XtDestroyWidget(XmMessageBoxGetChild(w, XmDIALOG_CANCEL_BUTTON));
    XtDestroyWidget(XmMessageBoxGetChild(w, XmDIALOG_HELP_BUTTON));
    break;
  case MOTD_FORM:
    w_motd_form = w;
    break;
  case MOTD_SCRL:
    w_motd_scrl = w;
    break;
  default:
    break;
  }
}
