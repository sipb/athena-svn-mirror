#include <Xm/Form.h>
#include <Xm/PushBG.h>
#include <Xm/SeparatoG.h>
#include <Xm/LabelG.h>
#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/MessageB.h>
#include <Xm/Text.h>
#include <Xm/List.h>
#include <Xm/PanedW.h>
#include <Xm/DialogS.h>

#include "visual.h"

#include "xolc.h"

Widget				/* Widget ID's */
  toplevel,
  main_form,

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

int NEWQ_Btn	= 1,
  CONTQ_Btn	= 2,
  STOCK_Btn	= 3,
  QUIT_Btn	= 4,
  HELP_Btn	= 5,
  CONTQ_Form	= 6,
  CONNECT_Lbl	= 7,
  TOPIC_Lbl	= 8,
  REPLAY_Scrl	= 9,
  SEND_Btn	= 10,
  DONE_Btn	= 11,
  CANCEL_Btn	= 12,
  SAVELOG_Btn	= 13,
  MOTD_Btn	= 14,
  UPDATE_Btn	= 15,
  MOTD_Dlg	= 16,
  HELP_Dlg	= 17,
  QUIT_Dlg	= 18,
  ERROR_Dlg	= 19,
  MOTD_Form	= 20,
  MOTD_Scrl	= 21
;

void MakeInterface()
{
  Arg args[100];
  int n = 0;

/*
 * The main form of the interface.  This initially displays the MOTD and
 *  lets the user select whether to ask a question or go somewhere else
 *  for help.  It also provides a "quit" and "help" button.
 */

  n=0;
/*XtSetArg(args[n], XmNheight, DEFAULT_HEIGHT);  n++;
  XtSetArg(args[n], XmNwidth, DEFAULT_WIDTH);  n++; */
  XtSetArg(args[n], XmNverticalSpacing, VERT_SPACING);  n++;
  XtSetArg(args[n], XmNhorizontalSpacing, HORIZ_SPACING);  n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNshadowThickness, NORMAL_SHADOW);  n++;
  XtSetArg(args[n], XmNborderWidth, NORMAL_BORDER);  n++;
  main_form = XmCreateForm(toplevel, "main", args, n);
  XtManageChild(main_form);

/*  Buttons along the top row:  [new_ques, cont_ques], stock, quit, help  */

  n=0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
  w_newq_btn = XmCreatePushButtonGadget(main_form, "new_ques_btn", args, n);
  XtAddCallback(w_newq_btn, XmNactivateCallback, olc_new_ques, NULL);
  XtAddCallback(w_newq_btn, XmNhelpCallback, Help, &NEWQ_Btn);

  n=0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
  w_contq_btn = XmCreatePushButtonGadget(main_form, "cont_ques_btn", args, n);
  XtAddCallback(w_contq_btn, XmNactivateCallback, olc_cont_ques, NULL);
  XtAddCallback(w_contq_btn, XmNhelpCallback, Help, &CONTQ_Btn);

  n=0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET);  n++;
  XtSetArg(args[n], XmNleftWidget, w_contq_btn);  n++;
  w_stock_btn = XmCreatePushButtonGadget(main_form, "stock_btn", args, n);
  XtAddCallback(w_stock_btn, XmNactivateCallback, olc_stock, NULL);
  XtAddCallback(w_stock_btn, XmNhelpCallback, Help, &STOCK_Btn);
  XtManageChild(w_stock_btn);

  n=0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
  w_help_btn = XmCreatePushButtonGadget(main_form, "help_btn", args, n);
  XtAddCallback(w_help_btn, XmNactivateCallback, olc_help, NULL);
  XtAddCallback(w_help_btn, XmNhelpCallback, Help, &HELP_Btn);
  XtManageChild(w_help_btn);
  
  n=0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET);  n++;
  XtSetArg(args[n], XmNrightWidget, w_help_btn);  n++;
  w_quit_btn = XmCreatePushButtonGadget(main_form, "quit_btn", args, n);
  XtAddCallback(w_quit_btn, XmNactivateCallback, olc_quit, NULL);
  XtAddCallback(w_quit_btn, XmNhelpCallback, Help, &QUIT_Btn);
  XtManageChild(w_quit_btn);
  
/*  Separator along the top, below buttons  */

  n=0;
  XtSetArg(args[n], XmNseparatorType, XmSHADOW_ETCHED_IN);  n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET);  n++;
  XtSetArg(args[n], XmNtopWidget, w_help_btn);  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
  w_button_sep = XmCreateSeparatorGadget(main_form, "buttonSep", args, n);
  XtManageChild(w_button_sep);
}


void
MakeNewqForm()
{
  Arg args[100];
  int n = 0;

/*
 * The "new_ques_form" will contain a scrolled_list widget for selecting
 *  the topic, as well as a scrolled_text region for entry of the initial
 *  question.
 */

  n=0;
  XtSetArg(args[n], XmNmarginWidth, NONE);  n++;
  XtSetArg(args[n], XmNmarginHeight, NONE);  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET);  n++;
  XtSetArg(args[n], XmNtopWidget, w_button_sep);  n++;
  XtSetArg(args[n], XmNshadowThickness, NONE);  n++;
  XtSetArg(args[n], XmNborderWidth, NONE);  n++;
  w_newq_form = XmCreateForm(main_form, "new_ques_form", args, n);
/*XtManageChild(w_newq_form);*/

/*
 *  Paned Window widget will hold scrolled list widget (on top) and
 *   scrolled text widget on bottom.
 */

  n=0;
  XtSetArg(args[n], XmNmarginWidth, NONE);  n++;
  XtSetArg(args[n], XmNmarginHeight, NONE);  n++;
  XtSetArg(args[n], XmNseparatorOn, TRUE);  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNminimum, MINIMUM);  n++;
  w_pane = XmCreatePanedWindow(w_newq_form, "pane", args, n);
  XtManageChild(w_pane);

/*  Top Form holds a title and scrolled list widget.  */

  n=0;
  XtSetArg(args[n], XmNmarginWidth, NONE);  n++;
  XtSetArg(args[n], XmNmarginHeight, NONE);  n++;
  XtSetArg(args[n], XmNverticalSpacing, VERT_SPACING);  n++;
  XtSetArg(args[n], XmNhorizontalSpacing, HORIZ_SPACING);  n++;
  XtSetArg(args[n], XmNminimum, MINIMUM);  n++;
  w_top_form = XmCreateForm(w_pane, "top_form", args, n);
  XtManageChild(w_top_form);

/*  Top label.  Used to prompt user to choose a topic.  */

  n=0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNalignment, XmALIGNMENT_CENTER);  n++;
  w_top_lbl = XmCreateLabelGadget(w_top_form, "top_lbl", args, n);
  XtManageChild(w_top_lbl);

/*  Frame to hold scrolled list widget  */

  n=0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET);  n++;
  XtSetArg(args[n], XmNtopWidget, w_top_lbl);  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNborderWidth, NONE);  n++;
  XtSetArg(args[n], XmNshadowThickness, NONE);  n++;
  w_list_frame = XmCreateFrame(w_top_form, "list_frame", args, n);
  XtManageChild(w_list_frame);

/*  List widget on top.  */

  n=0;
  XtSetArg(args[n], XmNlistMarginWidth, NORMAL_SPACING);  n++;
  XtSetArg(args[n], XmNlistMarginHeight, NORMAL_SPACING);  n++;
/*XtSetArg(args[n], XmNlistSpacing, NORMAL_SPACING);  n++;*/
  XtSetArg(args[n], XmNselectionPolicy, XmSINGLE_SELECT);  n++;
  XtSetArg(args[n], XmNscrollBarDisplayPolicy, XmAS_NEEDED);  n++;
  XtSetArg(args[n], XmNscrollBarPlacement, XmBOTTOM_LEFT);  n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM);  n++;
  w_list = XmCreateScrolledList(w_list_frame, "list", args, n);
  XtAddCallback(w_list, XmNsingleSelectionCallback, olc_topic_select, NULL);
  XtAddCallback(w_list, XmNdefaultActionCallback, olc_topic_select, NULL);
  XtManageChild(w_list);

/*  Bottom Form holds a title, scrolled text widget, and rowcolumn.  */

  n=0;

  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNmarginWidth, NONE);  n++;
  XtSetArg(args[n], XmNmarginHeight, NONE);  n++;
  XtSetArg(args[n], XmNverticalSpacing, VERT_SPACING);  n++;
  XtSetArg(args[n], XmNhorizontalSpacing, HORIZ_SPACING);  n++;
  XtSetArg(args[n], XmNminimum, MINIMUM);  n++;
  w_bottom_form = XmCreateForm(w_pane, "bottom_form", args, n);
  XtManageChild(w_bottom_form);

/*  Bottom label.  Used to prompt user to type in initial question.  */

  n=0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNalignment, XmALIGNMENT_CENTER);  n++;
  w_bottom_lbl = XmCreateLabelGadget(w_bottom_form, "bottom_lbl", args, n);
  XtManageChild(w_bottom_lbl);

/*  RowColumn containing buttons along bottom of text-entry area:
        send, clear   */

  n=0;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNborderWidth, NONE);  n++;
  XtSetArg(args[n], XmNmarginWidth, NONE);  n++;
  XtSetArg(args[n], XmNmarginHeight, NONE);  n++;
  XtSetArg(args[n], XmNorientation, XmHORIZONTAL);  n++;
  XtSetArg(args[n], XmNpacking, XmPACK_TIGHT);  n++;
  w_newq_rowcol = XmCreateRowColumn(w_bottom_form, "newqRowCol", args, n);
  XtManageChild(w_newq_rowcol);

  n=0;
  w_send_newq_btn = XmCreatePushButtonGadget(w_newq_rowcol,
					     "send_newq_btn", args, n);
  XtAddCallback(w_send_newq_btn, XmNactivateCallback, olc_send_newq, NULL);
  XtAddCallback(w_send_newq_btn, XmNhelpCallback, Help, &SEND_Btn);
  XtManageChild(w_send_newq_btn);

  n=0;
  w_clear_btn = XmCreatePushButtonGadget(w_newq_rowcol,
					 "clear_btn", args, n);
  XtAddCallback(w_clear_btn, XmNactivateCallback, olc_clear_newq, NULL);
  XtAddCallback(w_clear_btn, XmNhelpCallback, Help, &DONE_Btn);
  XtManageChild(w_clear_btn);

/*  Frame to hold scrolled text widget.  Scrolled text widget is for
 *   entering initial question.
 */

  n=0;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET);  n++;
  XtSetArg(args[n], XmNtopWidget, w_bottom_lbl);  n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET);  n++;
  XtSetArg(args[n], XmNbottomWidget, w_newq_rowcol);  n++;
  XtSetArg(args[n], XmNborderWidth, NONE);  n++;
  XtSetArg(args[n], XmNshadowThickness, NONE);  n++;
  w_newq_frame = XmCreateFrame(w_bottom_form, "newq_frame", args, n);
  XtManageChild(w_newq_frame);

  n=0;
  XtSetArg(args[n], XmNeditable, TRUE);  n++;
  XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT);  n++;
  XtSetArg(args[n], XmNwordWrap, TRUE);  n++;
  XtSetArg(args[n], XmNscrollHorizontal, FALSE);  n++;
  XtSetArg(args[n], XmNscrollVertical, TRUE);  n++;
  w_newq_scrl = XmCreateScrolledText(w_newq_frame, "newq_scrl", args, n);
  XtManageChild(w_newq_scrl);
  MuSetEmacsBindings(w_newq_scrl);
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

  n=0;
  XtSetArg(args[n], XmNmarginWidth, NORMAL_SPACING);  n++;
  XtSetArg(args[n], XmNmarginHeight, NONE);  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET);  n++;
  XtSetArg(args[n], XmNtopWidget, w_button_sep);  n++;
/*XtSetArg(args[n], XmNverticalSpacing, VERT_SPACING);  n++;*/
  XtSetArg(args[n], XmNhorizontalSpacing, HORIZ_SPACING);  n++;
  XtSetArg(args[n], XmNshadowThickness, NONE);  n++;
  w_contq_form = XmCreateForm(main_form, "cont_ques_form", args, n);
/*XtManageChild(w_contq_form);*/

/*  Status form contains connect_lbl, your_topic_lbl, topic_lbl  */

  n=0;
  XtSetArg(args[n], XmNmarginWidth, NONE);  n++;
  XtSetArg(args[n], XmNmarginHeight, NONE);  n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
  w_status_form = XmCreateForm(w_contq_form, "status_form", args, n);
  XtManageChild(w_status_form);

  n=0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNalignment, XmALIGNMENT_CENTER);  n++;
  XtSetArg(args[n], XmNlabelString, MotifString(STATUS_UNKNOWN_LABEL));  n++;
  w_connect_lbl = XmCreateLabelGadget(w_status_form, "connect_lbl", args, n);
  XtManageChild(w_connect_lbl);

  n=0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNalignment, XmALIGNMENT_CENTER);  n++;
  XtSetArg(args[n], XmNlabelString, MotifString(TOPIC_UNKNOWN_LABEL));  n++;
  w_topic_lbl = XmCreateLabelGadget(w_status_form, "topic_lbl", args, n);
  XtManageChild(w_topic_lbl);

  n=0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET);  n++;
  XtSetArg(args[n], XmNrightWidget, w_topic_lbl);  n++;
  XtSetArg(args[n], XmNalignment, XmALIGNMENT_CENTER);  n++;
  XtSetArg(args[n], XmNlabelString, MotifString(YOUR_TOPIC_LABEL));  n++;
  XtManageChild( XmCreateLabelGadget(w_status_form,
				     "your_topic_lbl", args, n));


/*  RowColumn containing buttons along bottom:
        send, done, cancel, savelog, motd, update   */

  n=0;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNborderWidth, NONE);  n++;
  XtSetArg(args[n], XmNmarginWidth, NONE);  n++;
  XtSetArg(args[n], XmNmarginHeight, NONE);  n++;
  XtSetArg(args[n], XmNorientation, XmHORIZONTAL);  n++;
  XtSetArg(args[n], XmNpacking, XmPACK_TIGHT);  n++;
  w_options_rowcol = XmCreateRowColumn(w_contq_form, "optionsRowCol", args, n);
  XtManageChild(w_options_rowcol);

  n=0;
  w_send_btn = XmCreatePushButtonGadget(w_options_rowcol, "send_btn", args, n);
  XtAddCallback(w_send_btn, XmNactivateCallback, olc_send, NULL);
  XtAddCallback(w_send_btn, XmNhelpCallback, Help, &SEND_Btn);
  XtManageChild(w_send_btn);

  n=0;
  w_done_btn = XmCreatePushButtonGadget(w_options_rowcol, "done_btn", args, n);
  XtAddCallback(w_done_btn, XmNactivateCallback, olc_done, NULL);
  XtAddCallback(w_done_btn, XmNhelpCallback, Help, &DONE_Btn);
  XtManageChild(w_done_btn);

  n=0;
  w_cancel_btn = XmCreatePushButtonGadget(w_options_rowcol,
					  "cancel_btn", args, n);
  XtAddCallback(w_cancel_btn, XmNactivateCallback, olc_cancel, NULL);
  XtAddCallback(w_cancel_btn, XmNhelpCallback, Help, &CANCEL_Btn);
  XtManageChild(w_cancel_btn);

  n=0;
  w_savelog_btn = XmCreatePushButtonGadget(w_options_rowcol,
					   "savelog_btn", args, n);
  XtAddCallback(w_savelog_btn, XmNactivateCallback, olc_savelog, NULL);
  XtAddCallback(w_savelog_btn, XmNhelpCallback, Help, &SAVELOG_Btn);
  XtManageChild(w_savelog_btn);

  n=0;
  w_motd_btn = XmCreatePushButtonGadget(w_options_rowcol, "motd_btn", args, n);
  XtAddCallback(w_motd_btn, XmNactivateCallback, olc_motd, NULL);
  XtAddCallback(w_motd_btn, XmNhelpCallback, Help, &MOTD_Btn);
  XtManageChild(w_motd_btn);

  n=0;
  w_update_btn = XmCreatePushButtonGadget(w_options_rowcol,
					  "update_btn", args, n);
  XtAddCallback(w_update_btn, XmNactivateCallback, olc_update, NULL);
  XtAddCallback(w_update_btn, XmNhelpCallback, Help, &UPDATE_Btn);
  XtManageChild(w_update_btn);
  
/*  Frame to hold scrolled text replay_frame  */

  n=0;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET);  n++;
  XtSetArg(args[n], XmNtopWidget,  w_status_form);  n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET);  n++;
  XtSetArg(args[n], XmNbottomWidget, w_options_rowcol);  n++;
  XtSetArg(args[n], XmNborderWidth, NONE);  n++;
  XtSetArg(args[n], XmNshadowThickness, NONE);  n++;
  XtSetArg(args[n], XmNmarginHeight, NORMAL_SPACING);  n++;
  w_replay_frame = XmCreateFrame(w_contq_form, "replay_frame", args, n);
  XtManageChild(w_replay_frame);

  n=0;
  XtSetArg(args[n], XmNeditable, FALSE);  n++;
  XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT);  n++;
  XtSetArg(args[n], XmNwordWrap, TRUE);  n++;
  XtSetArg(args[n], XmNscrollHorizontal, FALSE);  n++;
  XtSetArg(args[n], XmNscrollVertical, TRUE);  n++;
  w_replay_scrl = XmCreateScrolledText(w_replay_frame, "replay_scrl", args, n);
  XtManageChild(w_replay_scrl);
  MuSetEmacsBindings(w_replay_scrl);
}

void
MakeMotdForm()
{
  Arg args[100];
  int n = 0;

/*
 * The "motd_form" will contain the motd at start up time.  The MOTD
 *  will be displayed initially until the user wants to do something else,
 *  then it will be unmanaged and the new thing popped in it's place.
 */

  n=0;
/*XtSetArg(args[n], XmNmarginWidth, NONE);  n++;
  XtSetArg(args[n], XmNmarginHeight, NONE);  n++;*/
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET);  n++;
  XtSetArg(args[n], XmNtopWidget, w_button_sep);  n++;
  XtSetArg(args[n], XmNverticalSpacing, VERT_SPACING);  n++;
  XtSetArg(args[n], XmNhorizontalSpacing, HORIZ_SPACING);  n++;
  XtSetArg(args[n], XmNshadowThickness, NONE);  n++;
  XtSetArg(args[n], XmNborderWidth, NONE);  n++;
  w_motd_form = XmCreateForm(main_form, "motd_form", args, n);
  XtManageChild(w_motd_form);

/*  "Welcome to Project Athena's OLC" label  */

  n=0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNalignment, XmALIGNMENT_CENTER);  n++;
  XtSetArg(args[n], XmNlabelString, MotifString(WELCOME_LABEL));  n++;
  w_welcome_lbl = XmCreateLabelGadget(w_motd_form, "welcome_lbl", args, n);
  XtManageChild(w_welcome_lbl);

/*  "Copyright MIT" label  */

  n=0;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNalignment, XmALIGNMENT_CENTER);  n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET);  n++;
  XtSetArg(args[n], XmNtopWidget, w_welcome_lbl);  n++;
  XtSetArg(args[n], XmNlabelString, MotifString(COPYRIGHT_LABEL));  n++;
  w_copyright_lbl = XmCreateLabelGadget(w_motd_form, "copyright_lbl", args, n);
  XtManageChild(w_copyright_lbl);

/*  Frame to hold scrolled text motd_frame  */

  n=0;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET);  n++;
  XtSetArg(args[n], XmNtopWidget, w_copyright_lbl);  n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNborderWidth, NONE);  n++;
  XtSetArg(args[n], XmNshadowThickness, NONE);  n++;
  w_motd_frame = XmCreateFrame(w_motd_form, "motd_frame", args, n);
  XtManageChild(w_motd_frame);

  n=0;
  XtSetArg(args[n], XmNeditable, FALSE);  n++;
  XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT);  n++;
  XtSetArg(args[n], XmNwordWrap, TRUE);  n++;
  XtSetArg(args[n], XmNscrollHorizontal, FALSE);  n++;
  XtSetArg(args[n], XmNscrollVertical, TRUE);  n++;
  w_motd_scrl = XmCreateScrolledText(w_motd_frame, "motd_scrl", args, n);
  XtManageChild(w_motd_scrl);
  MuSetEmacsBindings(w_motd_scrl);
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

  n=0;
  XtSetArg(args[n], XmNdialogStyle, XmDIALOG_MODELESS);  n++;
  XtSetArg(args[n], XmNdialogType, XmDIALOG_INFORMATION);  n++;
  XtSetArg(args[n], XmNmessageAlignment, XmALIGNMENT_BEGINNING);  n++;
  XtSetArg(args[n], XmNborderWidth, NORMAL_BORDER);  n++;
  w_motd_dlg = XmCreateInformationDialog(main_form, "motd_dlg", args, n);
  XtAddCallback(w_motd_dlg, XmNokCallback, dlg_ok, &MOTD_Dlg);
  XtDestroyWidget(XmMessageBoxGetChild(w_motd_dlg, XmDIALOG_CANCEL_BUTTON));
  XtDestroyWidget(XmMessageBoxGetChild(w_motd_dlg, XmDIALOG_HELP_BUTTON));

  n=0;
  XtSetArg(args[n], XmNdialogStyle, XmDIALOG_MODELESS);  n++;
  XtSetArg(args[n], XmNmessageAlignment, XmALIGNMENT_BEGINNING);  n++;
  XtSetArg(args[n], XmNborderWidth, NORMAL_BORDER);  n++;
  w_help_dlg = XmCreateInformationDialog(main_form, "help_dlg", args, n);
  XtAddCallback(w_help_dlg, XmNokCallback, dlg_ok, &HELP_Dlg);
  XtDestroyWidget(XmMessageBoxGetChild(w_help_dlg, XmDIALOG_CANCEL_BUTTON));
  XtDestroyWidget(XmMessageBoxGetChild(w_help_dlg, XmDIALOG_HELP_BUTTON));

/*  Send Form holds a title, scrolled text widget, and rowcolumn.  */

  n=0;

  XtSetArg(args[n], XmNmarginWidth, NONE);  n++;
  XtSetArg(args[n], XmNmarginHeight, NONE);  n++;
  XtSetArg(args[n], XmNborderWidth, NORMAL_BORDER);  n++;
  XtSetArg(args[n], XmNverticalSpacing, VERT_SPACING);  n++;
  XtSetArg(args[n], XmNhorizontalSpacing, 2 * HORIZ_SPACING);  n++;
  w_send_form = XmCreateFormDialog(toplevel, "send_form", args, n);

/*  Send label.  Used to prompt user to type in message.  */

  n=0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNalignment, XmALIGNMENT_CENTER);  n++;
  w_send_lbl = XmCreateLabelGadget(w_send_form, "send_lbl", args, n);
  XtManageChild(w_send_lbl);

/*  RowColumn containing buttons along bottom of text-entry area:
        send, clear, close.   */

  n=0;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNborderWidth, NONE);  n++;
  XtSetArg(args[n], XmNmarginWidth, NONE);  n++;
  XtSetArg(args[n], XmNmarginHeight, NONE);  n++;
  XtSetArg(args[n], XmNorientation, XmHORIZONTAL);  n++;
  XtSetArg(args[n], XmNpacking, XmPACK_TIGHT);  n++;
  w_send_rowcol = XmCreateRowColumn(w_send_form, "sendRowCol", args, n);
  XtManageChild(w_send_rowcol);

  n=0;
  w_send_msg_btn = XmCreatePushButtonGadget(w_send_rowcol,
					    "send_msg_btn", args, n);
  XtAddCallback(w_send_msg_btn, XmNactivateCallback, olc_send_msg, NULL);
  XtManageChild(w_send_msg_btn);

  n=0;
  w_clear_msg_btn = XmCreatePushButtonGadget(w_send_rowcol,
					     "clear_msg_btn", args, n);
  XtAddCallback(w_clear_msg_btn, XmNactivateCallback, olc_clear_msg, NULL);
  XtManageChild(w_clear_msg_btn);

  n=0;
  w_close_msg_btn = XmCreatePushButtonGadget(w_send_rowcol,
					     "close_msg_btn", args, n);
  XtAddCallback(w_close_msg_btn, XmNactivateCallback, olc_close_msg, NULL);
  XtManageChild(w_close_msg_btn);

/*  Frame to hold scrolled text widget.  Scrolled text widget is for
 *   entering message.
 */

  n=0;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET);  n++;
  XtSetArg(args[n], XmNtopWidget, w_send_lbl);  n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET);  n++;
  XtSetArg(args[n], XmNbottomWidget, w_send_rowcol);  n++;
  XtSetArg(args[n], XmNborderWidth, NONE);  n++;
  XtSetArg(args[n], XmNshadowThickness, NONE);  n++;
  w_send_frame = XmCreateFrame(w_send_form, "send_frame", args, n);
  XtManageChild(w_send_frame);

  n=0;
  XtSetArg(args[n], XmNeditable, TRUE);  n++;
  XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT);  n++;
  XtSetArg(args[n], XmNwordWrap, TRUE);  n++;
  XtSetArg(args[n], XmNscrollHorizontal, FALSE);  n++;
  XtSetArg(args[n], XmNscrollVertical, TRUE);  n++;
  w_send_scrl = XmCreateScrolledText(w_send_frame, "send_scrl", args, n);
  XtManageChild(w_send_scrl);
  MuSetEmacsBindings(w_send_scrl);
}
