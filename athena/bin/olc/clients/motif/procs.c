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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/procs.c,v $
 *      $Author: vanharen $
 */

#ifndef lint
static char rcsid[]="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/procs.c,v 1.2 1989-07-31 15:01:26 vanharen Exp $";
#endif

#include "xolc.h"

/*extern Widget			/* Widget ID's */
/*  w_newq_btn,
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
  w_error_dlg,
  w_motd_form,
  w_motd_scrl,
  toplevel,
  main_form;
*/
  /**************************************************************/
  /*  Procedures						*/
  /**************************************************************/

void olc_new_ques (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
  XtSetSensitive(w_newq_btn, FALSE);
  if ( XtIsManaged(w_motd_form) )
    XtUnmanageChild(w_motd_form);
  if (! XtIsManaged(w_contq_form) )
    XtManageChild(w_contq_form);
}

void olc_cont_ques (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
  XtSetSensitive(w_contq_btn, FALSE);
  if ( XtIsManaged(w_motd_form) )
    XtUnmanageChild(w_motd_form);
  olc_replay();
  olc_status();
  olc_topic();
  if (! XtIsManaged(w_contq_form) )
    XtManageChild(w_contq_form);
}

olc_topic()
{
  REQUEST Request;
  int status;
  char topic[TOPIC_SIZE];
  Arg arg;

  fill_request(&Request);
  status = OGetTopic(&Request,topic);
  switch(status) {
  case SUCCESS:
    XtSetArg(arg, XmNlabelString, XmStringLtoRCreate(topic, ""));
    XtSetValues(w_topic_lbl, &arg, 1);
    break;
  case PERMISSION_DENIED:
    XtSetArg(arg, XmNlabelString, XmStringLtoRCreate("unauth", ""));
    XtSetValues(w_topic_lbl, &arg, 1);
    break;
  case FAILURE:
    XtSetArg(arg, XmNlabelString, XmStringLtoRCreate("unknown", ""));
    XtSetValues(w_topic_lbl, &arg, 1);
    break;
  default:
    XtSetArg(arg, XmNlabelString, XmStringLtoRCreate("unknown", ""));
    XtSetValues(w_topic_lbl, &arg, 1);
    status = handle_response(status, Request);
    break;
  }
}


olc_status()
{
  REQUEST Request;
  int status;
  LIST list;
  Arg arg;
  char connect[256];

  fill_request(&Request);
  status = OWho(&Request, &list);
  switch (status) {
  case SUCCESS:
    if(list.connected.uid >= 0) {
      XtSetArg(arg, XmNlabelString,
	       XmStringLtoRCreate(sprintf(connect,
					  "You are connected to %s %s.",
					  list.connected.title,
					  list.connected.realname /*,
					  list.connected.username,
					  list.connected.machine */),
				  ""));
      XtSetValues(w_connect_lbl, &arg, 1);
    }
    else {
      XtSetArg(arg, XmNlabelString,
	       XmStringLtoRCreate("You are not connected to a consultant.",
				  ""));
      XtSetValues(w_connect_lbl, &arg, 1);
    }
    break;
  default:
    XtSetArg(arg, XmNlabelString, XmStringLtoRCreate("Status unknown.", ""));
    XtSetValues(w_connect_lbl, &arg, 1);
    status = handle_response(status, &Request);
    break;
  }
}

olc_replay()
{
  REQUEST Request;
  char file[NAME_LENGTH];
  int status;
  struct stat buf;
  char *log;
  int fd;
  Arg arg;

  fill_request(&Request);
  make_temp_name(file);
  status = OReplayLog(&Request,file);
  stat(file, &buf);
  if ((log = malloc((1 + buf.st_size) * sizeof(char))) == (char *) NULL)
    fprintf(stderr, "olc_replay: unable to malloc space for log.\n");
  if ((fd = open(file, O_RDONLY, 0)) < 0)
    fprintf(stderr, "olc_replay: unable to open log file for read.\n");
  if ((read(fd, log, buf.st_size)) != buf.st_size)
    fprintf(stderr, "olc_replay: unable to read log correctly.\n");
  close(fd);
  unlink(file);
  log[buf.st_size] = '\0';

#ifdef TEST
  printf("olc_replay: log size is %d chars.\n", buf.st_size);
#endif TEST

  switch (status) {
  case SUCCESS:
/*    XmTextSetString(w_replay_scrl, log); */
    XtSetArg(arg, XmNvalue, XmStringLtoRCreate(log, ""));
    XtSetValues(w_replay_scrl, &arg, 1);
    break;
  case NOT_CONNECTED:
    XmTextSetString(w_replay_scrl,
		    "No question, and therefore, no log to display.");
    break;
  case PERMISSION_DENIED:
    XmTextSetString(w_replay_scrl,
		    "You are not allowed to see this log.");
    break;
  case ERROR:
    XmTextSetString(w_replay_scrl,
		    "An error occurred while trying to retrieve this log.");
    break;
  default:
    XmTextSetString(w_replay_scrl,
		    "An error occurred while trying to retrieve this log.");
    status = handle_response(status, Request);
    status = ERROR;
    break;
  }
  free(log);
}

void olc_send (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{

}
  
void olc_done (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{

}
  
void olc_cancel (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{

}
  
void olc_savelog (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{

}
  
void olc_stock (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{

}
  
void olc_motd (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
  REQUEST Request;
  char file[NAME_LENGTH];

  if (! XtIsManaged(w_motd_dlg) )
    {
      fill_request(&Request);
      make_temp_name(file);
      x_get_motd(&Request,OLC,file);
      unlink(file);  
      XtSetSensitive(w_motd_btn, FALSE);
      XtManageChild(w_motd_dlg);
    }
}

void olc_update (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
  olc_replay();
  olc_status();
  olc_topic();
}
  
void olc_help (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
  Arg arg;
  struct stat buf;
  char *help;
  char *file = "olc.help";
  int fd;

  if (! XtIsManaged(w_help_dlg) )
    {
      stat(file, &buf);
      if ((help = malloc((1 + buf.st_size) * sizeof(char))) == (char *) NULL)
	fprintf(stderr, "olc_help: unable to malloc space for help.\n");
      if ((fd = open(file, O_RDONLY, 0)) < 0)
	fprintf(stderr, "olc_help: unable to open help file for read.\n");
      if ((read(fd, help, buf.st_size)) != buf.st_size)
	fprintf(stderr, "olc_help: unable to read help correctly.\n");
      close(fd);
      help[buf.st_size] = '\0';
      XtSetArg(arg, XmNmessageString, XmStringLtoRCreate(help, ""));
      XtSetValues(w_help_dlg, &arg, 1);
      free(help);
      XtSetSensitive(w_help_btn, FALSE);
      XtManageChild(w_help_dlg);
    }
}

void olc_quit (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
  if (! XtIsManaged(w_help_dlg) )
    {
      XtSetSensitive(w_quit_btn, FALSE);
      XtManageChild(w_quit_dlg);
    }
}



void dlg_ok (w, tag, callback_data)
     Widget w;
     int *tag;
     XmAnyCallbackStruct *callback_data;
{
  switch (*tag) {
  case MOTD_DLG:
    XtSetSensitive(w_motd_btn, TRUE);
    break;
  case HELP_DLG:
    XtSetSensitive(w_help_btn, TRUE);
    break;
  case QUIT_DLG:
    XtDestroyWidget(toplevel);
    exit(0);
    break;
  }
}

void dlg_cancel (w, tag, callback_data)
     Widget w;
     int *tag;
     XmAnyCallbackStruct *callback_data;
{
  switch (*tag) {
  case QUIT_DLG:
    XtSetSensitive(w_quit_btn, TRUE);
    break;
  }
}

