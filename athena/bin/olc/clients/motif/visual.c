#include <Xm/Form.h>
#include <Xm/PushB.h>
#include <Xm/SeparatoG.h>
#include <Xm/LabelG.h>
#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/MessageB.h>
#include <Xm/Text.h>
#include <Xm/List.h>
#include <Xm/PanedW.h>
#include <Xm/DialogS.h>

#include <sys/param.h>

#include "visual.h"
#include "buttons.h"
#include "xolc.h"

#include "xolc.xbm"

Widget				/* Widget ID's */
  toplevel,
  main_form,
  w,

  w_newq_btn,
  w_contq_btn,
  w_stock_btn,
  w_quit_btn,
  w_help_btn,
  w_button_sep,

  w_newq_form,
  w_newq_sep,
  w_pane,
  w_top_form,
  w_top_lbl,
  w_list_frame,
  w_list,
  w_bottom_form,
  w_bottom_lbl,
  w_newq_rowcol,
  w_send_newq_btn,
  w_clear_btn,
  w_newq_frame,
  w_newq_scrl,

  w_contq_form,
  w_status_form,
  w_connect_lbl,
  w_topic_lbl,
  w_replay_frame,
  w_replay_scrl,
  w_options_rowcol,
  w_send_btn,
  w_done_btn,
  w_cancel_btn,
  w_savelog_btn,
  w_motd_btn,
  w_update_btn,

  w_motd_form,
  w_welcome_lbl,
  w_copyright_lbl,
  w_motd_frame,
  w_motd_scrl,

  w_motd_dlg,
  w_help_dlg,

  w_send_form,
  w_send_lbl,
  w_send_rowcol,
  w_send_msg_btn,
  w_clear_msg_btn,
  w_close_msg_btn,
  w_send_frame,
  w_send_scrl
;

void MakeInterface()
{
  Arg args[1];
  Pixmap icon_pixmap = None;
  Widget wl[10];
  int n;

  n = 0;

/*
 * The main form of the interface.  This initially displays the MOTD and
 *  lets the user select whether to ask a question or go somewhere else
 *  for help.  It also provides a "quit" and "help" button.
 */

  XtSetArg(args[0], XtNiconPixmap, &icon_pixmap);
  XtGetValues(toplevel,args,1);
  if (icon_pixmap == None) {
    XtSetArg(args[0], XtNiconPixmap,
             XCreateBitmapFromData(XtDisplay(toplevel),
                                   XtScreen(toplevel)->root,
                                   xolc_bits, xolc_width, xolc_height));
    XtSetValues (toplevel, args, 1);
  }


  w = main_form = XmCreateForm(toplevel, "main", NULL, 0);
  XtManageChild(w);

/*  Buttons along the top row:  [new_ques, cont_ques], stock, quit, help  */

  w = w_newq_btn = XmCreatePushButton(main_form, "new_ques_btn", NULL, 0);
  XtAddCallback(w, XmNactivateCallback, olc_new_ques, NULL);
  XtAddCallback(w, XmNhelpCallback, Help, NEWQ_BTN);

  w = w_contq_btn = XmCreatePushButton(main_form, "cont_ques_btn", NULL, 0);
  XtAddCallback(w, XmNactivateCallback, olc_cont_ques, NULL);
  XtAddCallback(w, XmNhelpCallback, Help, CONTQ_BTN);

  w = w_stock_btn = XmCreatePushButton(main_form, "stock_btn", NULL, 0);
  XtAddCallback(w, XmNactivateCallback, olc_stock, NULL);
  XtAddCallback(w, XmNhelpCallback, Help, STOCK_BTN);
  wl[n++] = w;

  w = w_help_btn = XmCreatePushButton(main_form, "help_btn", NULL, 0);
  XtAddCallback(w, XmNactivateCallback, olc_help, NULL);
  XtAddCallback(w, XmNhelpCallback, Help, HELP_BTN);
  wl[n++] = w;
  
  w = w_quit_btn = XmCreatePushButton(main_form, "quit_btn", NULL, 0);
  XtAddCallback(w, XmNactivateCallback, olc_quit, NULL);
  XtAddCallback(w, XmNhelpCallback, Help, QUIT_BTN);
  wl[n++] = w;
  
/*  Separator along the top, below buttons  */

  w = w_button_sep = XmCreateSeparatorGadget(main_form, "buttonSep", NULL, 0);
  wl[n++] = w;

  XtManageChildren(wl,(Cardinal) n);

}

void
MakeNewqForm()
{
  Widget wl[20];
  int n;

  n = 0;

/*
 * The "new_ques_form" will contain a scrolled_list widget for selecting
 *  the topic, as well as a scrolled_text region for entry of the initial
 *  question.
 */

  w_newq_form = XmCreateForm(main_form, "new_ques_form", NULL, 0);

/*
 *  Paned Window widget will hold scrolled list widget (on top) and
 *   scrolled text widget on bottom.
 */

  w = w_pane = XmCreatePanedWindow(w_newq_form, "nq_pane", NULL, 0);
  XtManageChild(w);

/*  Top Form holds a title and scrolled list widget.  */

  w = w_top_form = XmCreateForm(w_pane, "top_form", NULL, 0);
  XtManageChild(w);

/*  Top label.  Used to prompt user to choose a topic.  */

  w = w_top_lbl = XmCreateLabelGadget(w_top_form, "top_lbl", NULL, 0);
  XtManageChild(w);


/*  Frame to hold scrolled list widget  */
  w = w_list_frame = XmCreateFrame(w_top_form, "list_frame", NULL, 0);
  XtManageChild(w);

/*  List widget on top.  */

  w = w_list = XmCreateScrolledList(w_list_frame, "topic_list", NULL, 0);
  XtAddCallback(w, XmNsingleSelectionCallback, olc_topic_select, NULL);
  XtAddCallback(w, XmNdefaultActionCallback, olc_topic_select, NULL);
  XtManageChild(w);

/*  Bottom Form holds a title, scrolled text widget, and rowcolumn.  */

  w = w_bottom_form = XmCreateForm(w_pane, "bottom_form", NULL, 0);
  XtManageChild(w);

/*  Bottom label.  Used to prompt user to type in initial question.  */

  w = w_bottom_lbl = XmCreateLabelGadget(w_bottom_form, "bottom_lbl", NULL, 0);
  wl[n++] = w;

/*  RowColumn containing buttons along bottom of text-entry area:
        send, clear   */

  w = w_newq_rowcol = XmCreateRowColumn(w_bottom_form, "newqRowCol", NULL, 0);
  wl[n++] = w;
  XtManageChildren(wl, (Cardinal) n);

  n = 0;

  w = w_send_newq_btn = XmCreatePushButton(w_newq_rowcol,
					   "send_newq_btn", NULL, 0);
  XtAddCallback(w, XmNactivateCallback, olc_send_newq, NULL);
  XtAddCallback(w, XmNhelpCallback, Help, SEND_NEWQ_BTN);
  wl[n++] = w;

  w = w_clear_btn = XmCreatePushButton(w_newq_rowcol,
				       "clear_btn", NULL, 0);
  XtAddCallback(w, XmNactivateCallback, olc_clear_newq, NULL);
  XtAddCallback(w, XmNhelpCallback, Help, CLEAR_BTN);
  wl[n++] = w;
  XtManageChildren(wl, (Cardinal) n);
  n = 0;

/*  Frame to hold scrolled text widget.  Scrolled text widget is for
 *   entering initial question.
 */

  w = w_newq_frame = XmCreateFrame(w_bottom_form, "newq_frame", NULL, 0);
  XtManageChild(w);

  w = w_newq_scrl = XmCreateScrolledText(w_newq_frame, "newq_scrl", NULL, 0);
  XtManageChild(w);
  MuSetEmacsBindings(w);

}


void
MakeContqForm()
{
  Arg args[100];
  int n = 0;

/*
 * The "cont_ques_form" will contain regions for showing the replay of the log
 *  in progress, as well as frames for sending a message to the
 *  consultant and buttons for canceling, don'ing, and getting the MOTD.
 */

  w_contq_form = XmCreateForm(main_form, "cont_ques_form", NULL, 0);

/*  Status form contains connect_lbl, your_topic_lbl, topic_lbl  */

  w = w_status_form = XmCreateForm(w_contq_form, "status_form", NULL, 0);
  XtManageChild(w);

  w = w_connect_lbl = XmCreateLabelGadget(w_status_form, "connect_lbl",
					  NULL, 0);
  XtManageChild(w);

  w = w_topic_lbl = XmCreateLabelGadget(w_status_form, "topic_lbl", NULL, 0);
  XtManageChild(w);

  XtManageChild( XmCreateLabelGadget(w_status_form,
				     "your_topic_lbl",NULL, 0));


/*  RowColumn containing buttons along bottom:
        send, done, cancel, savelog, motd, update   */

  w = w_options_rowcol = XmCreateRowColumn(w_contq_form,
					   "optionsRowCol", args, n);
  XtManageChild(w);

  n=0;
  w = w_send_btn = XmCreatePushButton(w_options_rowcol, "send_btn", args, n);
  XtAddCallback(w, XmNactivateCallback, olc_send, NULL);
  XtAddCallback(w, XmNhelpCallback, Help, SEND_BTN);
  XtManageChild(w);

  n=0;
  w = w_done_btn = XmCreatePushButton(w_options_rowcol, "done_btn", args, n);
  XtAddCallback(w, XmNactivateCallback, olc_done, NULL);
  XtAddCallback(w, XmNhelpCallback, Help, DONE_BTN);
  XtManageChild(w);

  n=0;
  w = w_cancel_btn = XmCreatePushButton(w_options_rowcol,
					"cancel_btn", args, n);
  XtAddCallback(w, XmNactivateCallback, olc_cancel, NULL);
  XtAddCallback(w, XmNhelpCallback, Help, CANCEL_BTN);
  XtManageChild(w);

  n=0;
  w = w_savelog_btn = XmCreatePushButton(w_options_rowcol,
					 "savelog_btn", args, n);
  XtAddCallback(w, XmNactivateCallback, olc_savelog, NULL);
  XtAddCallback(w, XmNhelpCallback, Help, SAVELOG_BTN);
  XtManageChild(w);

  n=0;
  w = w_motd_btn = XmCreatePushButton(w_options_rowcol, "motd_btn", args, n);
  XtAddCallback(w, XmNactivateCallback, olc_motd, NULL);
  XtAddCallback(w, XmNhelpCallback, Help, MOTD_BTN);
  XtManageChild(w);

  n=0;
  w = w_update_btn = XmCreatePushButton(w_options_rowcol,
					  "update_btn", args, n);
  XtAddCallback(w, XmNactivateCallback, olc_update, NULL);
  XtAddCallback(w, XmNhelpCallback, Help, UPDATE_BTN);
  XtManageChild(w);
  
/*  Frame to hold scrolled text replay_frame  */

  w = w_replay_frame = XmCreateFrame(w_contq_form, "replay_frame", NULL, 0);
  XtManageChild(w);

  w = w_replay_scrl = XmCreateScrolledText(w_replay_frame,
					   "replay_scrl", NULL, 0);
  XtManageChild(w);
  MuSetEmacsBindings(w);
}

void
MakeMotdForm()
{

/*
 * The "motd_form" will contain the motd at start up time.  The MOTD
 *  will be displayed initially until the user wants to do something else,
 *  then it will be unmanaged and the new thing popped in it's place.
 */

  w = w_motd_form = XmCreateForm(main_form, "motd_form", NULL, 0);
  XtManageChild(w);

/*  "Welcome to Project Athena's OLC" label  */

  w = w_welcome_lbl = XmCreateLabelGadget(w_motd_form, "welcome_lbl", NULL, 0);
  XtManageChild(w);

/*  "Copyright MIT" label  */

  w = w_copyright_lbl = XmCreateLabelGadget(w_motd_form,
					    "copyright_lbl", NULL,0);
  XtManageChild(w);

/*  Frame to hold scrolled text motd_frame  */

  w = w_motd_frame = XmCreateFrame(w_motd_form, "motd_frame", NULL, 0);
  XtManageChild(w);

  w = w_motd_scrl = XmCreateScrolledText(w_motd_frame, "motd_scrl", NULL, 0);
  XtManageChild(w);
  MuSetEmacsBindings(w);
}


void
MakeDialogs()
{
  Arg args[100];
  int n = 0;

/*
 *  Unmanaged dialog boxes for communication with user:
 *   motd, help.
 */

  w = w_motd_dlg = XmCreateInformationDialog(main_form, "motd_dlg", NULL, 0);
  XtAddCallback(w, XmNokCallback, dlg_ok, MOTD_BTN);
  XtDestroyWidget(XmMessageBoxGetChild(w, XmDIALOG_CANCEL_BUTTON));
  XtDestroyWidget(XmMessageBoxGetChild(w, XmDIALOG_HELP_BUTTON));

  w = w_help_dlg = XmCreateInformationDialog(main_form, "help_dlg", NULL, 0);
  XtAddCallback(w, XmNokCallback, dlg_ok, HELP_BTN);
  XtDestroyWidget(XmMessageBoxGetChild(w, XmDIALOG_CANCEL_BUTTON));
  XtDestroyWidget(XmMessageBoxGetChild(w, XmDIALOG_HELP_BUTTON));

/*  Send Form holds a title, scrolled text widget, and rowcolumn.  */

  w_send_form = XmCreateFormDialog(toplevel, "send_form", NULL, 0);

/*  Send label.  Used to prompt user to type in message.  */

  w = w_send_lbl = XmCreateLabelGadget(w_send_form, "send_lbl", NULL, 0);
  XtManageChild(w);

/*  RowColumn containing buttons along bottom of text-entry area:
        send, clear, close.   */

  w = w_send_rowcol = XmCreateRowColumn(w_send_form, "sendRowCol", NULL, 0);
  XtManageChild(w);

  w = w_send_msg_btn = XmCreatePushButton(w_send_rowcol,
					  "send_msg_btn", NULL, 0);
  XtAddCallback(w, XmNactivateCallback, olc_send_msg, NULL);
  XtAddCallback(w, XmNhelpCallback, Help, SEND_MSG_BTN);
  XtManageChild(w);

  w = w_clear_msg_btn = XmCreatePushButton(w_send_rowcol,
					     "clear_msg_btn", NULL, 0);
  XtAddCallback(w, XmNactivateCallback, olc_clear_msg, NULL);
  XtAddCallback(w, XmNhelpCallback, Help, CLEAR_MSG_BTN);
  XtManageChild(w);

  w = w_close_msg_btn = XmCreatePushButton(w_send_rowcol,
					   "close_msg_btn", NULL, 0);
  XtAddCallback(w, XmNactivateCallback, olc_close_msg, NULL);
  XtAddCallback(w, XmNhelpCallback, Help, CLOSE_MSG_BTN);
  XtManageChild(w);

/*  Frame to hold scrolled text widget.  Scrolled text widget is for
 *   entering message.
 */

  w = w_send_frame = XmCreateFrame(w_send_form, "send_frame", NULL, 0);
  XtManageChild(w);

  w = w_send_scrl = XmCreateScrolledText(w_send_frame, "send_scrl", NULL, 0);
  XtManageChild(w);
  MuSetEmacsBindings(w);
}
