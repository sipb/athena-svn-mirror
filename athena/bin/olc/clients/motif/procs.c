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
static char rcsid[]="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/procs.c,v 1.5 1989-12-01 16:27:06 vanharen Exp $";
#endif

#include "xolc.h"
#include "data.h"
#include "buttons.h"

char current_topic[TOPIC_SIZE] = "unknown";

/*
 *  Procedures
 *
 */

void Help(w, tag, callback_data)
     Widget w;
     int *tag;
     XmAnyCallbackStruct *callback_data;
{
  char message[BUF_SIZE];
  char help_filename[NAME_LENGTH * 2];

  (void) strcpy(help_filename, HELP_DIR);
  (void) strcat(help_filename, "/");

  switch( (int) tag)
    {
    case NEWQ_BTN: 
      (void) strcat(help_filename, HELP_NEWQ_BTN);
      MuHelpFile(help_filename);
/*      MuHelp("You have asked for help on the new question button.\nUse it to ask a new question in OLC."); */
      break;
    case CONTQ_BTN:
      MuHelp("You have asked for help on the continue question button.\nUse it to continue your question in OLC.");
      break;
    default:
      sprintf(message, "button: %d\nwidget id: %d", tag, w);
      MuHelp(message);
      break;
    }
/*
  MuHelp("Congratulations!  You have gotten 'help' to work!  I have been unable to do so,\nand so, have not put any useful information into this message.  Have a nice day.");
*/
}

void olc_new_ques (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
  REQUEST Request;
  char file[NAME_LENGTH];
  int status;

  if (fill_request(&Request) != SUCCESS)
    {
      MuError("olc_new_ques: unable to fill request struct");
      return;
    }

  XtSetSensitive(w_newq_btn, FALSE);
  MuSetWaitCursor(toplevel);
/*  MakeNewqForm(); */

  current_topic[0] = '\0';
  
  make_temp_name(file);
  
 list_try_again:
  status = x_list_topics(&Request, file);
  
  if (status != SUCCESS)
    {
      unlink(file);
      if (MuGetBoolean("Unable to get list of topics.\nWould you like to try again?\n\nIf not, you will exit this program.", "Yes", "No",
		       NULL, TRUE, Mu_Popup_Center))
	goto list_try_again;
    }
  else
    {
      if ( XtIsManaged(w_motd_form) )
	XtUnmanageChild(w_motd_form);
      XtManageChild(w_newq_form);
      ask_screen = TRUE;
    }
  unlink(file);
  MuSetStandardCursor(toplevel);
}


void olc_clear_newq (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
  XmTextSetString(w_newq_scrl, "");
}

void olc_send_newq (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
  REQUEST Request;
  Widget wl[2];

  if (fill_request(&Request) != SUCCESS)
    {
      MuError("olc_send_newq: unable to fill request struct");
      return;
    }

#ifdef TEST
  XtSetSensitive(w_send_newq_btn, FALSE);
#endif TEST
  MuSetWaitCursor(toplevel);

  if ((current_topic[0] == '\0') &&
      (strlen(XmTextGetString(w_newq_scrl)) == 0))
    {
      MuErrorSync("You must select a topic for your question from the list in the\ntop half of this window.  Simply click on the line that most\nclosely matches the topic of your question.\n\nYou must also type in the text of your question in the area in\nthe bottom half of this window.  Move the mouse into that\narea and type your question.");
      MuSetStandardCursor(toplevel);
#ifdef TEST
      XtSetSensitive(w_send_newq_btn, TRUE);
#endif TEST
      return;
    }
  
  if (current_topic[0] == '\0')
    {
      MuErrorSync("You must select a topic for your question from the list in the\ntop half of this window.  Simply click on the line that most\nclosely matches the topic of your question.");
      MuSetStandardCursor(toplevel);
#ifdef TEST
      XtSetSensitive(w_send_newq_btn, TRUE);
#endif TEST
      return;
    }
  
  if (strlen(XmTextGetString(w_newq_scrl)) == 0)
    {
      MuErrorSync("You must type in the text of your question in the area in the\nbottom half of this window.  Move the mouse into that area\nand type your question.");
      MuSetStandardCursor(toplevel);
#ifdef TEST
      XtSetSensitive(w_send_newq_btn, TRUE);
#endif TEST
      return;
    }

  if (has_question == TRUE)
    {
      MuWarning("It appears that you already have a question entered in OLC.\nContinuing with that question...");
/*      MakeContqForm(); */
      olc_status();
/*      olc_topic(); */
      olc_replay();
      if ( XtIsManaged(w_newq_form) )
	XtUnmanageChild(w_newq_form);
      XtManageChild(w_contq_form);
      MuSetStandardCursor(toplevel);
      return;
    }

  if (x_ask(&Request, current_topic, XmTextGetString(w_newq_scrl))
      != SUCCESS)
    {
      MuError("An error occurred when trying to enter your question.\n\nEither try again or call a consultant at 253-4435.");
      MuSetStandardCursor(toplevel);
      return;
    }

  has_question = TRUE;
/*  MakeContqForm(); */
  olc_status();
/*  olc_topic(); */
  olc_replay();
  if ( XtIsManaged(w_newq_form) )
    XtUnmanageChild(w_newq_form);
  XtManageChild(w_contq_form);
  MuSetStandardCursor(toplevel);
  replay_screen = TRUE;
}


void olc_topic_select (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmListCallbackStruct *callback_data;
{
  int item;

  item = callback_data->item_position - 1;
  if (TopicTable[item].topic[0] == '#')
    {
      MuError("Not a valid topic line.  Please select another entry.");
      current_topic[0] = '\0';
      XmListDeselectAllItems(w); 
    }
  else
    strcpy(current_topic, TopicTable[item].topic);
}

void olc_cont_ques (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
  XtSetSensitive(w_contq_btn, FALSE);
  MuSetWaitCursor(toplevel);
/*  MakeContqForm(); */
  olc_status();
/*  olc_topic(); */
  olc_replay();
  if ( XtIsManaged(w_motd_form) )
    XtUnmanageChild(w_motd_form);
  XtManageChild(w_contq_form);
  replay_screen = TRUE;
  MuSetStandardCursor(toplevel);
}

olc_topic()
{
  REQUEST Request;
  int status;
  char topic[TOPIC_SIZE];
  Arg arg;

  if (fill_request(&Request) != SUCCESS)
    {
      MuError("olc_topic: unable to fill request struct");
      return;
    }

  status = OGetTopic(&Request,topic);
  switch(status) {
  case SUCCESS:
    strcpy(current_topic, topic);
    XtSetArg(arg, XmNlabelString, MotifString(topic));
    XtSetValues(w_topic_lbl, &arg, 1);
    break;
  case PERMISSION_DENIED:
    XtSetArg(arg, XmNlabelString, MotifString("unauth"));
    XtSetValues(w_topic_lbl, &arg, 1);
    break;
  case FAILURE:
    XtSetArg(arg, XmNlabelString, MotifString("unknown"));
    XtSetValues(w_topic_lbl, &arg, 1);
    break;
  default:
    XtSetArg(arg, XmNlabelString, MotifString("unknown"));
    XtSetValues(w_topic_lbl, &arg, 1);
    status = handle_response(status, &Request);
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

  if (fill_request(&Request) != SUCCESS)
    {
      MuError("olc_status: unable to fill request struct");
      return;
    }

  status = OWho(&Request, &list);
  switch (status) {
  case SUCCESS:
    if(list.connected.uid >= 0) {
      XtSetArg(arg, XmNlabelString,
	       MotifString(sprintf(connect,
				   "You are connected to %s %s.",
				   list.connected.title,
				   list.connected.realname)));
      XtSetValues(w_connect_lbl, &arg, 1);
    }
    else {
      XtSetArg(arg, XmNlabelString,
	       MotifString("You are not connected to a consultant."));
      XtSetValues(w_connect_lbl, &arg, 1);
    }
    strcpy(current_topic, list.topic);
    XtSetArg(arg, XmNlabelString, MotifString(list.topic));
    XtSetValues(w_topic_lbl, &arg, 1);
    break;
  default:
    XtSetArg(arg, XmNlabelString, MotifString("Status unknown."));
    XtSetValues(w_connect_lbl, &arg, 1);
    XtSetArg(arg, XmNlabelString, MotifString("unknown"));
    XtSetValues(w_topic_lbl, &arg, 1);
    status = handle_response(status, &Request);
    break;
  }
}

olc_replay()
{
  REQUEST Request;
  char file[NAME_LENGTH];
  int status;
  struct stat statbuf;
  char *log;
  int fd;

  if (fill_request(&Request) != SUCCESS)
    {
      MuError("olc_replay: unable to fill request struct");
      return;
    }

  make_temp_name(file);
/*
 *  Do a show or something here so that the "user has read reply" will show
 *   up in the log.  Currently, the OLC library does not support this
 *   function.  Will have to wait for new library to be built.
 *
 *  status = OGetMessage(&Request, file, NULL, OLC_SHOW);
 */
  status = OReplayLog(&Request,file);

  switch (status)
    {
    case SUCCESS:

      if (stat(file, &statbuf))
	{
	  MuError("replay: unable to stat replay file.");
	  return;
	}
	  
      if ((log = malloc((1 + statbuf.st_size) * sizeof(char)))
	  == (char *) NULL)
	{
	  fprintf(stderr, "olc_replay: unable to malloc space for log.");
	  return;
	}
      
      if ((fd = open(file, O_RDONLY, 0)) < 0)
	{
	  MuError("replay: unable to open log file for read.");
	  return;
	}

      if ((read(fd, log, statbuf.st_size)) != statbuf.st_size)
	{
	  MuError("replay: unable to read log correctly.");
	}

      log[statbuf.st_size] = '\0';
      XmTextSetString(w_replay_scrl, log);
      close(fd);
      free(log);
      
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
      status = handle_response(status, &Request);
      break;
    }
  unlink(file);
}

void olc_done (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
  REQUEST Request;
  int status;
  Widget wl[2];

  wl[0] = toplevel;
  wl[1] = w_send_form;

#ifdef TEST
  XtSetSensitive(w_done_btn, FALSE);
#endif TEST
  MuSetCursors(wl, 2, XC_watch);
  if (fill_request(&Request) != SUCCESS)
    {
      MuError("done: Unable to fill request struct.");
    }
  else {
    status = x_done(&Request);
  }
  MuSetCursors(wl, 2, XC_top_left_arrow);
#ifdef TEST
  XtSetSensitive(w_done_btn, TRUE);
#endif TEST
}
  
void olc_cancel (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
  REQUEST Request;
  int status;
  Widget wl[2];

  wl[0] = toplevel;
  wl[1] = w_send_form;

#ifdef TEST
  XtSetSensitive(w_cancel_btn, FALSE);
#endif TEST
  MuSetCursors(wl, 2, XC_watch);
  if (fill_request(&Request) != SUCCESS)
    {
      MuError("cancel: Unable to fill request struct.");
    }
  else {
    status = x_cancel(&Request);
  }
  MuSetCursors(wl, 2, XC_top_left_arrow);
#ifdef TEST
  XtSetSensitive(w_cancel_btn, TRUE);
#endif TEST
}
  
void olc_savelog (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
  char *homedir;
  char file[BUF_SIZE];
  REQUEST Request;
  int status;
  Widget wl[2];

  wl[0] = toplevel;
  wl[1] = w_send_form;

  homedir = getenv("HOME");
  sprintf(file, "%s/%s.%s", homedir, "OLC.log", current_topic);
  if (MuGetString("Please enter the name of a file to save the log in:\n",
		  file, BUF_SIZE, NULL, Mu_Popup_Center))
    {
      if (fill_request(&Request) != SUCCESS)
	{
	  MuError("olc_savelog: unable to fill request struct");
	  return;
	}

      MuSetCursors(wl, 2, XC_watch);
      status = OReplayLog(&Request,file);
      MuSetCursors(wl, 2, XC_top_left_arrow);

      switch (status)
	{
	case SUCCESS:
	  break;

	case NOT_CONNECTED:
	  MuError("You do not have a question.  Therefore, no log to save.");
	  break;

	case PERMISSION_DENIED:
	  MuError("You are not allowed to see this log.");
	  break;
	  
	case ERROR:
	  MuError("An error occurred while trying to retrieve this log.");
	  break;

	default:
	  status = handle_response(status, &Request);
	  break;
	}
    }
}
  
void olc_stock (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
  char message[BUF_SIZE];

  sprintf(message, "Stock answers are not yet implemented, sorry.\nTry typing:\n\n           olc_answers\n\nat your Unix prompt instead.\n\n%s!", happy_message());
  MuHelp(message);
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
#ifdef TEST
      XtSetSensitive(w_motd_btn, FALSE);
#endif TEST
      
      if (fill_request(&Request) != SUCCESS)
	{
	  MuError("olc_motd: unable to fill request struct");
	  return;
	}
      
      make_temp_name(file);
  
    motd_try_again:
      switch(x_get_motd(&Request,OLC,file,TRUE))
	{
	case FAILURE:
	  goto motd_try_again;
	case ERROR:
	  exit(ERROR);
	deafult:
	  break;
	}

      unlink(file);  
      XtManageChild(w_motd_dlg);
      MuSetStandardCursor(w_motd_dlg);
    }
}

void olc_update (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
  Widget wl[2];

  wl[0] = toplevel;
  wl[1] = w_send_form;

#ifdef TEST
  XtSetSensitive(w_update_btn, FALSE);
#endif TEST
  MuSetCursors(wl, 2, XC_watch);
  olc_status();
/*  olc_topic(); */
  olc_replay();
  MuSetCursors(wl, 2, XC_top_left_arrow);
#ifdef TEST
  XtSetSensitive(w_update_btn, TRUE);
#endif TEST
}
  
void olc_help (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
  Arg arg;
  struct stat statbuf;
  char *help;
  char help_filename[NAME_LENGTH * 2];
  int fd;
  Widget wl[2];

  wl[0] = toplevel;
  wl[1] = w_send_form;

  (void) strcpy(help_filename, HELP_DIR);
  (void) strcat(help_filename, "/");

  if (replay_screen)
    (void) strcat(help_filename, HELP_REPLAY);
  else if (ask_screen)
    (void) strcat(help_filename, HELP_ASK);
  else if (init_screen)
    {
      if (has_question)
	(void) strcat(help_filename, HELP_INIT_CONTQ);
      else
	(void) strcat(help_filename, HELP_INIT_NEWQ);
    }
  else
    (void) strcat(help_filename, HELP_UNKNOWN);
  
  if (! XtIsManaged(w_help_dlg) )
    {
#ifdef TEST
      XtSetSensitive(w_help_btn, FALSE);
#endif TEST
      MuSetCursors(wl, 2, XC_watch);

      if (stat(help_filename, &statbuf))
	{
	  MuError("help: unable to stat help file.");
#ifdef TEST
	  XtSetSensitive(w_help_btn, TRUE);
#endif TEST
	  MuSetCursors(wl, 2, XC_top_left_arrow);
	  return;
	}

      if ((help = malloc((1 + statbuf.st_size) * sizeof(char)))
	  == (char *) NULL)
	{
	  fprintf(stderr, "help: unable to malloc space for help.\n");
	  MuError("help: unable to malloc space for help.");
#ifdef TEST
	  XtSetSensitive(w_help_btn, TRUE);
#endif TEST
	  MuSetCursors(wl, 2, XC_top_left_arrow);
	  return;
	}

      if ((fd = open(help_filename, O_RDONLY, 0)) < 0)
	{
	  fprintf(stderr, "help: unable to open help file for read.\n");
	  MuError("help: unable to open help file for read.");
#ifdef TEST
	  XtSetSensitive(w_help_btn, TRUE);
#endif TEST	
	  MuSetCursors(wl, 2, XC_top_left_arrow);
	  return;
	}

      if ((read(fd, help, statbuf.st_size)) != statbuf.st_size)
	{
	  fprintf(stderr, "help: unable to read help correctly.\n");
	  MuError("help: unable to read help correctly.");
#ifdef TEST
	  XtSetSensitive(w_help_btn, TRUE);
#endif TEST
	  MuSetCursors(wl, 2, XC_top_left_arrow);
	  return;
	}

      help[statbuf.st_size] = '\0';
      XtSetArg(arg, XmNmessageString, MotifString(help));
      XtSetValues(w_help_dlg, &arg, 1);
      XtManageChild(w_help_dlg);
      MuSetStandardCursor(w_help_dlg);
      close(fd);
      free(help);
#ifdef TEST
      XtSetSensitive(w_help_btn, TRUE);
#endif TEST
      MuSetCursors(wl, 2, XC_top_left_arrow);
    }
}

void olc_quit (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
#ifdef TEST
  XtSetSensitive(w_quit_btn, FALSE);
#endif TEST
  if (has_question)
    {
      if (MuGetBoolean(QUIT_MESSAGE, "Yes", "No",
		       NULL, FALSE, Mu_Popup_Center))
	exit(0);
    }
  else
    {
      if (MuGetBoolean("You have not yet entered a question.\n\nAre you sure you want to quit OLC?", "Yes", "No", NULL, FALSE, Mu_Popup_Center))
	exit(0);
    }
#ifdef TEST
  XtSetSensitive(w_quit_btn, TRUE);
#endif TEST
}


void dlg_ok (w, tag, callback_data)
     Widget w;
     int *tag;
     XmAnyCallbackStruct *callback_data;
{
  switch (*tag)
    {
    case MOTD_BTN:
#ifdef TEST
      XtSetSensitive(w_motd_btn, TRUE);
#endif TEST
      break;
    case HELP_BTN:
#ifdef TEST
      XtSetSensitive(w_help_btn, TRUE);
#endif TEST
      break;
    }
}

void dlg_cancel (w, tag, callback_data)
     Widget w;
     int *tag;
     XmAnyCallbackStruct *callback_data;
{
/*  switch (*tag) {
  case 18:
    XtSetSensitive(w_quit_btn, TRUE);
    break;
  } */
}


void olc_send (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
  XtSetSensitive(w_send_btn, FALSE);
  CalcXandY(Mu_Popup_Bottom, w_send_form);
  XtManageChild(w_send_form);
}
  
void olc_clear_msg (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
  XmTextSetString(w_send_scrl, "");
}

void olc_send_msg (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
  REQUEST Request;
  int status;
  Widget wl[2];

  wl[0] = toplevel;
  wl[1] = w_send_form;

  MuSetCursors(wl, 2, XC_watch);

  if(fill_request(&Request) != SUCCESS)
    {
      MuError("olc_send_msg: Unable to fill request struct.");
      MuSetCursors(wl, 2, XC_top_left_arrow);
      return;
    }

  status = x_reply(&Request, XmTextGetString(w_send_scrl));
  MuSetCursors(wl, 2, XC_top_left_arrow);

  if (status == SUCCESS)
    XmTextSetString(w_send_scrl, "");   
}

void olc_close_msg (w, tag, callback_data)
     Widget w;
     caddr_t *tag;
     XmAnyCallbackStruct *callback_data;
{
  XtUnmanageChild(w_send_form);
  XtSetSensitive(w_send_btn, TRUE);
}
  
char * parse_text (string, columns)
     char *string;
     int columns;
{
  int m,n;
  char *cur_ptr;
  char *prev_ptr;
  char *lastline_ptr;
  char *spc_ptr;
  char *newline_ptr;

  char stringbuf[BUF_SIZE];

  
/*  for (m=0; m=strlen(string), m++)
    {
*/
  cur_ptr = string;
  prev_ptr = string;
  lastline_ptr = string;

/*
  while( (cur_ptr = index(prev_ptr, ' ')) !=0)
    {
      if ((cur_ptr - lastline_ptr) < columns)
	{
	  prev_ptr = ++cur_ptr;
	}
      else
	{
	  lastline_ptr = prev_ptr;
	  prev_ptr--;
	  *prev_ptr = '\n';
	  prev_ptr++;
	}
    }
*/


  
/*  while (strlen(strncpy(stringbuf, cur_ptr, columns)) == columns)*/
  while (strlen(cur_ptr) > columns)
    {
      strncpy(stringbuf, cur_ptr, columns);
      stringbuf[columns] = '\0';
      spc_ptr = rindex(stringbuf, ' ');
      newline_ptr = rindex(stringbuf, '\n');
      if (newline_ptr < spc_ptr)
	cur_ptr = ++newline_ptr;
      else
	{
	  *spc_ptr = '\n';
	  cur_ptr = ++spc_ptr;
	}
    }

  return(string);
}
