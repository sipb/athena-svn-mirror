#include <olc/olc.h>
#include <olc/olc_tty.h>
#include <olc/olc_parser.h>
#include "olc.h"

#include <zephyr/zephyr.h>
#include "XmAppl.h"          /*  Motif Toolkit  */

#include <signal.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <netdb.h>

#define  NEWQ_BTN	1
#define  CONTQ_BTN	2
#define  STOCK_BTN	3
#define  QUIT_BTN	4
#define  HELP_BTN	5
#define  CONTQ_FORM	6
#define  CONNECT_LBL	7
#define  TOPIC_LBL	8
#define  REPLAY_SCRL	9
#define  SEND_BTN	10
#define  DONE_BTN	11
#define  CANCEL_BTN	12
#define  SAVELOG_BTN	13
#define  MOTD_BTN	14
#define  UPDATE_BTN	15
#define  MOTD_DLG	16
#define  HELP_DLG	17
#define  QUIT_DLG	18
#define  ERROR_DLG	19
#define  MOTD_FORM	20
#define  MOTD_SCRL	21

Widget			/* Widget ID's */
  w_newq_btn,
  w_contq_btn,
  w_stock_btn,
  w_quit_btn,
  w_help_btn,
  w_contq_form,
  w_connect_lbl,
  w_topic_lbl,
  w_replay_scrl,
  w_send_btn,
  w_done_btn,
  w_cancel_btn,
  w_savelog_btn,
  w_motd_btn,
  w_update_btn,
  w_motd_dlg,
  w_help_dlg,
  w_quit_dlg,
  w_error_dlg,
  w_motd_form,
  w_motd_scrl,
  toplevel,
  main_form;
