/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains definitions and function prototypes for XOLH
 *
 *      Chris VanHaren
 *      Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1991 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/xolc.h,v $
 *      $Id: xolc.h,v 1.14 1997-04-30 17:42:04 ghudson Exp $
 *      $Author: ghudson $
 */

#include <mit-copyright.h>

#include <olc/olc.h>
#include <olc/olc_tty.h>
#include <olc/olc_parser.h>

#include <Mu.h>

/*  All the widget pointers that have been created  */

extern Widget			/* Widget ID's */
  xolc,
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
  w_save_dlg,

  w_send_form,
  w_send_lbl,
  w_send_rowcol,
  w_send_msg_btn,
  w_clear_msg_btn,
  w_close_msg_btn,
  w_send_frame,
  w_send_scrl
;


/*
 *  Global variables.
 */

extern char current_topic[];

typedef struct tTOPIC {
  char topic[TOPIC_SIZE];
} TOPIC;

extern TOPIC TopicTable[256];

extern int has_question,
  init_screen,
  ask_screen,
  replay_screen
  ;

/*  Useful Macros  */

#define  MotifString(s)		XmStringLtoRCreate(s, XmSTRING_DEFAULT_CHARSET)
#define  AddItemToList(l, s)	XmListAddItem(l, MotifString(s), 0);
#define STANDARD_CURSOR	SetCursor(0)
#define WAIT_CURSOR	SetCursor(1)

/*
 * Function Prototypes
 */

#if defined(__STDC__)
# define P_(s) s
#else
# define P_(s) ()
#endif

/* main.c */
int main P_((int argc, char *argv[]));
int olc_init P_((void));

/* procs.c */
void olc_new_ques     P_((Widget w, XtPointer tag, XtPointer callback_data));
void olc_clear_newq   P_((Widget w, XtPointer tag, XtPointer callback_data));
void olc_send_newq    P_((Widget w, XtPointer tag, XtPointer callback_data));
void olc_topic_select P_((Widget w, XtPointer tag, XtPointer callback_data));
void olc_cont_ques    P_((Widget w, XtPointer tag, XtPointer callback_data));
void olc_status P_((void));
void olc_replay P_((void));
void olc_done         P_((Widget w, XtPointer tag, XtPointer callback_data));
void olc_cancel       P_((Widget w, XtPointer tag, XtPointer callback_data));
void olc_savelog      P_((Widget w, XtPointer tag, XtPointer callback_data));
void save_cbk         P_((Widget w, XtPointer tag, XtPointer callback_data));
void olc_stock        P_((Widget w, XtPointer tag, XtPointer callback_data));
void olc_motd         P_((Widget w, XtPointer tag, XtPointer callback_data));
void olc_update       P_((Widget w, XtPointer tag, XtPointer callback_data));
void olc_help         P_((Widget w, XtPointer tag, XtPointer callback_data));
void olc_quit         P_((Widget w, XtPointer tag, XtPointer callback_data));
void dlg_ok           P_((Widget w, XtPointer tag, XtPointer callback_data));
void olc_send         P_((Widget w, XtPointer tag, XtPointer callback_data));
void olc_clear_msg    P_((Widget w, XtPointer tag, XtPointer callback_data));
void olc_send_msg     P_((Widget w, XtPointer tag, XtPointer callback_data));
void olc_close_msg    P_((Widget w, XtPointer tag, XtPointer callback_data));

/* visual.c */
void MakeInterface P_((void));
void MakeNewqForm P_((void));
void MakeContqForm P_((void));
void MakeMotdForm P_((void));
void MakeDialogs P_((void));

/* x_ask.c */
ERRCODE x_ask P_((REQUEST *Request, char *topic, char *question));

/* x_instance.c */
ERRCODE t_set_default_instance P_((REQUEST *Request));

/* x_motd.c */
ERRCODE x_get_motd P_((REQUEST *Request, int type, char *file, int dialog));

/* x_resolve.c */
void x_done P_((REQUEST *Request));
ERRCODE x_cancel P_((REQUEST *Request));

/* x_send.c */
ERRCODE x_reply P_((REQUEST *Request, char *message));

/* x_topic.c */
ERRCODE x_list_topics P_((REQUEST *Request, char *file));

/* x_utils.c */
ERRCODE handle_response P_((int response, REQUEST *req));
int popup_option P_((char *message));

#undef P_
