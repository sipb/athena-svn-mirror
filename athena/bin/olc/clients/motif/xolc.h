#include <olc/olc.h>
#include <olc/olc_tty.h>
#include <olc/olc_parser.h>
#include "olc.h"

#include <zephyr/zephyr.h>
#include <X11/cursorfont.h>
#include <Mrm/MrmAppl.h>	/* Motif Toolkit */
#include <Mu.h>

#ifdef XTCOMM_CLIENT
#include <XtComm.h>
#endif XTCOMM_CLIENT

#include <signal.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <netdb.h>

/*  All the widget pointers that have been created  */

extern Widget			/* Widget ID's */
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

/*
 *  Callbacks that are attached to the buttons and widgets in the
 *   interface.
 *
 *   ... and other routines.
 */

extern void
  olc_new_ques(),
  olc_cont_ques(),
  olc_stock(),
  olc_help(),
  olc_quit(),
  olc_send(),
  olc_done(),
  olc_cancel(),
  olc_savelog(),
  olc_motd(),
  olc_update(),
  dlg_ok(),
  dlg_cancel(),
  olc_topic_select(),
  olc_clear_newq(),
  olc_send_newq(),
  olc_send_msg(),
  olc_clear_msg(),
  olc_close_msg(),

  Help(),	

  MakeInterface(),
  MakeContqForm(),
  MakeNewqForm(),
  MakeMotdForm(),
  MakeDialogs(),

  t_set_default_instance()
;

extern ERRCODE x_done(),
  x_cancel(),
  x_list_topics(),
  x_ask(),
  x_reply(),
  handle_response()
;

extern char *happy_message();

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
