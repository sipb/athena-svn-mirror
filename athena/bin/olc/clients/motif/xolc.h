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
 *      $Id: xolc.h,v 1.16 1999-06-28 22:51:59 ghudson Exp $
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

/* main.c */
int main (int argc, char *argv[]);
int olc_init (void);

/* procs.c */
void olc_new_ques     (Widget w, XtPointer tag, XtPointer callback_data);
void olc_clear_newq   (Widget w, XtPointer tag, XtPointer callback_data);
void olc_send_newq    (Widget w, XtPointer tag, XtPointer callback_data);
void olc_topic_select (Widget w, XtPointer tag, XtPointer callback_data);
void olc_cont_ques    (Widget w, XtPointer tag, XtPointer callback_data);
void olc_status (void);
void olc_replay (void);
void olc_done         (Widget w, XtPointer tag, XtPointer callback_data);
void olc_cancel       (Widget w, XtPointer tag, XtPointer callback_data);
void olc_savelog      (Widget w, XtPointer tag, XtPointer callback_data);
void save_cbk         (Widget w, XtPointer tag, XtPointer callback_data);
void olc_stock        (Widget w, XtPointer tag, XtPointer callback_data);
void olc_motd         (Widget w, XtPointer tag, XtPointer callback_data);
void olc_update       (Widget w, XtPointer tag, XtPointer callback_data);
void olc_help         (Widget w, XtPointer tag, XtPointer callback_data);
void olc_quit         (Widget w, XtPointer tag, XtPointer callback_data);
void dlg_ok           (Widget w, XtPointer tag, XtPointer callback_data);
void olc_send         (Widget w, XtPointer tag, XtPointer callback_data);
void olc_clear_msg    (Widget w, XtPointer tag, XtPointer callback_data);
void olc_send_msg     (Widget w, XtPointer tag, XtPointer callback_data);
void olc_close_msg    (Widget w, XtPointer tag, XtPointer callback_data);

/* visual.c */
void MakeInterface (void);
void MakeNewqForm (void);
void MakeContqForm (void);
void MakeMotdForm (void);
void MakeDialogs (void);

/* x_ask.c */
ERRCODE x_ask (REQUEST *Request, char *topic, char *question);

/* x_instance.c */
ERRCODE t_set_default_instance (REQUEST *Request);

/* x_motd.c */
ERRCODE x_get_motd (REQUEST *Request, int type, char *file, int dialog);

/* x_resolve.c */
void x_done (REQUEST *Request);
ERRCODE x_cancel (REQUEST *Request);

/* x_send.c */
ERRCODE x_reply (REQUEST *Request, char *message);

/* x_topic.c */
ERRCODE x_list_topics (REQUEST *Request, char *file);

/* x_utils.c */
ERRCODE handle_response (int response, REQUEST *req);
int popup_option (char *message);
