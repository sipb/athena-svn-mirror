/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains the create procedures for the widgets used in the
 * X-based interface.
 *
 *      Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *      $Id: procs.c,v 1.30 1999-03-06 16:47:45 ghudson Exp $
 */

#ifndef lint
static char rcsid[]="$Id: procs.c,v 1.30 1999-03-06 16:47:45 ghudson Exp $";
#endif

#include <mit-copyright.h>
#include "config.h"

#include <signal.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <Xm/List.h>
#include <Xm/Text.h>
#include <Xm/ScrollBar.h>
#include <Xm/SelectioB.h>

#include <nl_requests.h>

#include "xolc.h"
#include "data.h"
#include "buttons.h"

char current_topic[TOPIC_SIZE] = "unknown";
int sa_pid = 0;

#ifndef HAVE_STRERROR
extern const char *const sys_errlist[];
#define strerror(x) (sys_errlist[x])
#endif

#if defined(__STDC__)
# define P_(s) s
#else
# define P_(s) ()
#endif

static RETSIGTYPE reaper P_((int sig));
static RETSIGTYPE view_ready P_((int sig));

#undef P_

/*
 *  Procedures
 *
 */

static RETSIGTYPE
reaper(sig)
{
  int foo;
  int pid;
  Arg args[1];

#ifdef HAVE_WAITPID
  pid = waitpid(-1, &foo, WNOHANG);
#else /* don't HAVE_WAITPID */
  pid = wait3(&foo, WNOHANG, 0);
#endif /* don't HAVE_WAITPID */
  if (pid <= 0)
    return;
  if (pid == sa_pid) {
    sa_pid = 0;
    XtSetArg(args[0],XmNsensitive,TRUE);
    XtSetValues(w_stock_btn, args, 1);
  }
  return;
}

static RETSIGTYPE
view_ready(sig)
{
#ifdef HAVE_SIGACTION
  struct sigaction act;

  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler= SIG_IGN;
  sigaction(SIGUSR1, &act, NULL);
#else /* don't HAVE_SIGACTION */
  signal(SIGUSR1, SIG_IGN);
#endif /* don't HAVE_SIGACTION */
  STANDARD_CURSOR;
  return;
}  

void
olc_new_ques (w, tag, callback_data)
     Widget w;
     XtPointer tag;
     XtPointer callback_data;
{
  REQUEST Request;
  char file[MAXPATHLEN];
  ERRCODE status;

  if (fill_request(&Request) != SUCCESS)
    {
      MuError("olc_new_ques: unable to fill request struct");
      return;
    }

  XtSetSensitive(w_newq_btn, FALSE);
  WAIT_CURSOR;
  XtMapWidget(w_newq_form);
  XtUnmapWidget(w_motd_form);
  ask_screen = TRUE;

  current_topic[0] = '\0';
  
  make_temp_name(file);
  
  status = x_list_topics(&Request, file);
  unlink(file);
  
  if (status != SUCCESS)
    exit(ERROR);
  
  STANDARD_CURSOR;
#ifdef LOG_USAGE
  log_view("question_start");
#endif
}


void
olc_clear_newq (w, tag, callback_data)
     Widget w;
     XtPointer tag;
     XtPointer callback_data;
{
  XmTextSetString(w_newq_scrl, "");
}

void
olc_send_newq (w, tag, callback_data)
     Widget w;
     XtPointer tag;
     XtPointer callback_data;
{
  REQUEST Request;
  char buf[BUFSIZ];
  char *q_text;

  if (fill_request(&Request) != SUCCESS)
    {
      MuError("olc_send_newq: unable to fill request struct");
      return;
    }

  WAIT_CURSOR;

  memset(buf, 0, BUFSIZ);
  q_text = XmTextGetString(w_newq_scrl);
  if (current_topic[0] == '\0')
      strcpy(buf,"You must select a topic for your question from the list in the\ntop half of this window.  Simply click on the line that most\nclosely matches the topic of your question.\n\n");
  if (*q_text == '\0')
      strcat(buf,"You must type in the text of your question in the area in\nthe bottom half of this window.  Move the mouse into that\narea and type your question.");

  if (buf[0] != '\0') {
    MuErrorSync(buf);
    STANDARD_CURSOR;
    return;
  }

  if (has_question == TRUE)
    {
      MuWarning("It appears that you already have a question entered in OLC.\nContinuing with that question...");
      olc_status();
      olc_replay();
      XtMapWidget(w_contq_form);
      XtUnmapWidget(w_newq_form);
      STANDARD_CURSOR;
      return;
    }

  if (x_ask(&Request, current_topic, q_text) != SUCCESS)
    {
      /* MuError is a macro, so don't try putting #if directives
       * inside its argument.
       */
#ifdef CONSULT_PHONE_NUMBER
      MuError("An error occurred when trying to enter your question.\n\n"
	      "Either try again or call a consultant at "
	      CONSULT_PHONE_NUMBER ".");
#else /* no CONSULT_PHONE_NUMBER */
      MuError("An error occurred when trying to enter your question.\n\n"
	      "Either try again or contact a consultant.");
#endif /* no CONSULT_PHONE_NUMBER */
      STANDARD_CURSOR;
      return;
    }

  has_question = TRUE;
  olc_status();
  olc_replay();
  XtMapWidget(w_contq_form);
  XtUnmapWidget(w_newq_form);
  STANDARD_CURSOR;
  replay_screen = TRUE;
#ifdef LOG_USAGE
  log_view("question_ask");
#endif
}


void
olc_topic_select (w, tag, callback_data)
     Widget w;
     XtPointer tag;
     XtPointer callback_data;
{
  XmListCallbackStruct *list = (XmListCallbackStruct *) callback_data;
  int item;

  item = list->item_position - 1;
  if (TopicTable[item].topic[0] == '#')
    {
      MuError("Not a valid topic line.  Please select another entry.");
      current_topic[0] = '\0';
      XmListDeselectAllItems(w); 
    }
  else
    {
      strcpy(current_topic, TopicTable[item].topic);
      _XmGrabTheFocus(w_newq_scrl, NULL);
    }
}

void
olc_cont_ques (w, tag, callback_data)
     Widget w;
     XtPointer tag;
     XtPointer callback_data;
{
  XtSetSensitive(w_contq_btn, FALSE);
  WAIT_CURSOR;
  olc_status();
  olc_replay();
  XtMapWidget(w_contq_form);
  XtUnmapWidget(w_motd_form);
  replay_screen = TRUE;
  STANDARD_CURSOR;
#ifdef LOG_USAGE
  log_view("question_continue");
#endif
}

void
olc_status()
{
  REQUEST Request;
  ERRCODE status;
  LIST list;
  Arg args[1];
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
      sprintf(connect, "You are connected to %s %s.", list.connected.title,
	      list.connected.realname);
      XtSetArg(args[0], XmNlabelString, MotifString(connect));
      XtSetValues(w_connect_lbl, args, 1);
    }
    else {
      XtSetArg(args[0], XmNlabelString,
	       MotifString("You are not connected to a consultant."));
      XtSetValues(w_connect_lbl, args, 1);
    }
    strcpy(current_topic, list.topic);
    XtSetArg(args[0], XmNlabelString, MotifString(list.topic));
    XtSetValues(w_topic_lbl, args, 1);
    break;
  default:
    XtSetArg(args[0], XmNlabelString, MotifString("Status unknown."));
    XtSetValues(w_connect_lbl, args, 1);
    XtSetArg(args[0], XmNlabelString, MotifString("unknown"));
    XtSetValues(w_topic_lbl, args, 1);
    status = handle_response(status, &Request);
    break;
  }
}

void
olc_replay()
{
  static char *log;
  static int loglen = 0;
  static init = 0;

  REQUEST Request;
  ERRCODE status;
  int fd;
  int actlen;
  Arg Args[2];
  Widget sb,w;
  int sb_value, sb_slider_size, sb_inc, sb_pinc, sb_max;

/* Mark message as read */

  WAIT_CURSOR;
  if(fill_request(&Request) != SUCCESS)
    return;

  status = OShowMessageIntoFile(&Request,"/dev/null");

  if (open_connection_to_nl_daemon(&fd) != SUCCESS) {
    MuError("Could not contact OLC Daemon\n");
    return;
  }
  status = nl_get_log(fd,&log,&loglen,User.username,User.instance,&actlen);
  if (status) {
    switch (status) {
    case ERR_NO_SUCH_Q:
      XmTextSetString(w_replay_scrl,
		    "No question, and therefore, no log to display.");
      break;
    case ERR_NO_ACL:
      XmTextSetString(w_replay_scrl,
		      "Sorry, charlie, but you're not on the acl.\n");
      break;
    default:
    /** need to do some set_com_err_hook stuff here **/
      com_err("olc_replay", status, "received while retrieving log to replay");
    }
    return;
  }

  XmTextSetString(w_replay_scrl, log);

/* Scroll down to end of the log */
  if (init == 0)
    init = 1;
  else {
/*    pos = actlen-1;
      XtSetArg(Args[0],XmNcursorPosition, pos);
      XtSetValues(w_replay_scrl,Args,1);
*/
    w = XtParent(w_replay_scrl);
    XtSetArg(Args[0],XmNverticalScrollBar,&sb);
    XtGetValues(w,Args,1);
    XmScrollBarGetValues(sb,&sb_value,&sb_slider_size,&sb_inc,&sb_pinc);
    XtSetArg(Args[0],XmNmaximum,&sb_max);
    XtGetValues(sb,Args,1);
    sb_value = sb_max - sb_slider_size;
    XmScrollBarSetValues(sb,sb_value,sb_slider_size,sb_inc,sb_pinc, TRUE);
  }
  STANDARD_CURSOR;
}

void
olc_done (w, tag, callback_data)
     Widget w;
     XtPointer tag;
     XtPointer callback_data;
{
  REQUEST Request;

  WAIT_CURSOR;
  if (fill_request(&Request) != SUCCESS) {
    MuError("done: Unable to fill request struct.");
    return;
  }

  x_done(&Request);
  STANDARD_CURSOR;

}
  
void
olc_cancel (w, tag, callback_data)
     Widget w;
     XtPointer tag;
     XtPointer callback_data;
{
  REQUEST Request;
  ERRCODE status;

  WAIT_CURSOR;
  if (fill_request(&Request) != SUCCESS)
    {
      MuError("cancel: Unable to fill request struct.");
    }
  else {
    status = x_cancel(&Request);
  }
  STANDARD_CURSOR;
}
  
void
olc_savelog (w, tag, callback_data)
     Widget w;
     XtPointer tag;
     XtPointer callback_data;
{
  char *homedir;
  char file[BUF_SIZE];
  Widget W;
  Arg A[1];

  XtSetArg(A[0],XmNsensitive, False);
  XtSetValues(w_savelog_btn,A,1);
  
  W = XmSelectionBoxGetChild(w_save_dlg,XmDIALOG_TEXT);
  homedir = (char *) getenv("HOME");
  sprintf(file, "%s/%s.%s", homedir, "OLC.log", current_topic);
  XmTextSetString(W,file);
  XtManageChild(w_save_dlg);
  _XmGrabTheFocus(W, NULL);
#ifdef LOG_USAGE
  log_view("log_save");
#endif
}

void
save_cbk (w, tag, callback_data)
     Widget w;
     XtPointer tag;
     XtPointer callback_data;
{
  XmSelectionBoxCallbackStruct *s_box =
    (XmSelectionBoxCallbackStruct *) callback_data;
  Arg A[1];
  ERRCODE status;
  int fd;
  int out_fd;
  char *filename;
  char buf[BUFSIZ];
  char *log;
  int loglen, actlen;

  XtSetArg(A[0],XmNsensitive, TRUE);
  XtSetValues(w_savelog_btn,A,1);
  XtUnmanageChild(w_save_dlg);

  switch (s_box->reason) {
  case XmCR_CANCEL:
    return;
  case XmCR_OK:
    WAIT_CURSOR;
    filename = XmTextGetString(XmSelectionBoxGetChild(w_save_dlg,
						      XmDIALOG_TEXT));
    out_fd = open(filename,O_WRONLY|O_CREAT|O_TRUNC,0600);
    if (out_fd < 0) {
      sprintf(buf,"Error opening %s: %s", filename, strerror(errno));
      MuError(buf);
      XtFree(filename);
      return;
    }
    
    XtFree(filename);
    if (open_connection_to_nl_daemon(&fd) != SUCCESS) {
      MuError("Could not contact OLC Daemon\n");
      return;
    }
    log = NULL;
    loglen = 0;
    
    status = nl_get_log(fd,&log,&loglen,User.username,User.instance,&actlen);
    if (status) {
      switch (status) {
      case ERR_NO_SUCH_Q:
	XmTextSetString(w_replay_scrl,
			"No question, and therefore, no log to display.");
	break;
      case ERR_NO_ACL:
	XmTextSetString(w_replay_scrl,
			"Sorry, charlie, but you're not on the acl.\n");
	break;
      default:
	com_err("olc_replay", status,"received while retrieving log to save");
      }
    }
    write(out_fd,log,loglen);
    close(out_fd);
    STANDARD_CURSOR;
  }
  return;
}

void
olc_stock (w, tag, callback_data)
     Widget w;
     XtPointer tag;
     XtPointer callback_data;
{
  Arg args[1];
  char pidascii[7];
#ifdef HAVE_SIGACTION
  struct sigaction action;
#endif /* don't HAVE_SIGACTION */

  WAIT_CURSOR;
  XtSetArg(args[0],XmNsensitive,FALSE);
  XtSetValues(w_stock_btn, args, 1);
  
  sprintf(pidascii,"%d",getpid());
  sa_pid = fork();
  if (sa_pid == -1) {
    MuError("Error in fork; cannot start stock answer browser");
    return;
  }
  if (sa_pid == 0) {
    if (execl(SA_LOC,SA_ARGV0,"-signal",pidascii,0) == -1) {
      perror("Error in execl; cannot start stock answer browser");
      STANDARD_CURSOR;
      _exit(1);
    }
  }
#ifdef HAVE_SIGACTION
  action.sa_flags = 0;
  sigemptyset(&action.sa_mask);

  action.sa_handler = reaper;
  sigaction(SIGCHLD, &action, NULL);
  
  action.sa_handler = view_ready;
  sigaction(SIGUSR1, &action, NULL);
#else /* don't HAVE_SIGACTION */
  signal(SIGCHLD,reaper);
  signal(SIGUSR1,view_ready);
#endif /* don't HAVE_SIGACTION */

#ifdef LOG_USAGE
  log_view("browser_start");
#endif
}
  
void
olc_motd (w, tag, callback_data)
     Widget w;
     XtPointer tag;
     XtPointer callback_data;
{
  Arg args[1];

  REQUEST Request;
  char file[MAXPATHLEN];

  XtSetArg(args[0], XmNsensitive, FALSE);
  XtSetValues(w_motd_btn, args, 1);

  WAIT_CURSOR;
  if (fill_request(&Request) != SUCCESS) {
    MuError("olc_motd: unable to fill request struct");
    return;
  }
  make_temp_name(file);
  
  switch(x_get_motd(&Request,0,file,TRUE))      /* NB: 2nd arg is irrelevant */
    {
    case FAILURE:
    case ERROR:
      exit(ERROR);
    default:
      break;
    }
    
/*
  XtSetArg(args[0], XmNsensitive, TRUE);
  XtSetValues(w_motd_btn, args, 1);
*/
  unlink(file);  
  XtManageChild(w_motd_dlg);
  STANDARD_CURSOR;
}

void
olc_update (w, tag, callback_data)
     Widget w;
     XtPointer tag;
     XtPointer callback_data;
{

  WAIT_CURSOR;
  olc_status();
  olc_replay();
  STANDARD_CURSOR;
}
  
void
olc_help (w, tag, callback_data)
     Widget w;
     XtPointer tag;
     XtPointer callback_data;
{
  Arg args[1];
  char *help;
  char help_filename[MAXPATHLEN];
  struct stat statbuf;
  int fd;

  XtSetArg(args[0], XmNsensitive, FALSE);
  XtSetValues(w_help_btn, args, 1);

  (void) strcpy(help_filename, HELP_PATH);

  if (replay_screen)
    (void) strcat(help_filename, HELP_REPLAY);
  else if (ask_screen)
    (void) strcat(help_filename, HELP_ASK);
  else
    {
      if (has_question)
	(void) strcat(help_filename, HELP_INIT_CONTQ);
      else
	(void) strcat(help_filename, HELP_INIT_NEWQ);
    }

  
  WAIT_CURSOR;
  
  if (stat(help_filename, &statbuf)) {
    MuError("help: unable to stat help file.");
    STANDARD_CURSOR;
    return;
  }
  
  help = malloc(1 + statbuf.st_size);
  if (help == NULL)
    {
      fprintf(stderr, "help: unable to malloc space for help.\n");
      MuError("help: unable to malloc space for help.");
      STANDARD_CURSOR;
      return;
    }
  
  fd = open(help_filename, O_RDONLY, 0);
  if (fd < 0)
    {
      free(help);
      fprintf(stderr, "help: unable to open help file for read.\n");
      MuError("help: unable to open help file for read.");
      STANDARD_CURSOR;
      return;
    }
  
  if ((read(fd, help, statbuf.st_size)) != statbuf.st_size)
    {
      close(fd);
      free(help);
      fprintf(stderr, "help: unable to read help correctly.\n");
      MuError("help: unable to read help correctly.");
      STANDARD_CURSOR;
      return;
    }
  
  help[statbuf.st_size] = '\0';
  XtSetArg(args[0], XmNmessageString, MotifString(help));
  XtSetValues(w_help_dlg, args, 1);
  XtManageChild(w_help_dlg);
  close(fd);
  free(help);
  STANDARD_CURSOR;
}

void
olc_quit (w, tag, callback_data)
     Widget w;
     XtPointer tag;
     XtPointer callback_data;
{
  if (has_question) {
    if (MuGetBoolean(QUIT_MESSAGE, "Yes", "No", NULL, FALSE))
      exit(0);
  }
  else {
    if (MuGetBoolean("You have not yet entered a question.\n\nAre you sure you want to quit OLC?",
                     "Yes", "No", NULL, FALSE))
      exit(0);
    }
}


void
dlg_ok (w, tag, callback_data)
     Widget w;
     XtPointer tag;
     XtPointer callback_data;
{
  Arg args[1];

  /* We're passing in a (small) integer value as the pointer callback
   * arg.  This is not guaranteed to work (pointers != integers), but
   * it almost always does in practice, and is less ugly than any of
   * the alternatives.  --bert 07feb97
   */
  switch ((int) tag)
    {
    case MOTD_BTN:
      XtSetArg(args[0], XmNsensitive, TRUE);
      XtSetValues(w_motd_btn, args, 1);
      break;
    case HELP_BTN:
      XtSetArg(args[0], XmNsensitive, TRUE);
      XtSetValues(w_help_btn, args, 1);
      break;
    }
}

void
olc_send (w, tag, callback_data)
     Widget w;
     XtPointer tag;
     XtPointer callback_data;
{
  XtSetSensitive(w_send_btn, FALSE);
  XtManageChild(w_send_form);
  _XmGrabTheFocus(w_send_scrl, NULL);
}
  
void
olc_clear_msg (w, tag, callback_data)
     Widget w;
     XtPointer tag;
     XtPointer callback_data;
{
  XmTextSetString(w_send_scrl, "");
}

void
olc_send_msg (w, tag, callback_data)
     Widget w;
     XtPointer tag;
     XtPointer callback_data;
{
  REQUEST Request;
  ERRCODE status;

  WAIT_CURSOR;

  if(fill_request(&Request) != SUCCESS) {
    MuError("olc_send_msg: Unable to fill request struct.");
    STANDARD_CURSOR;
    return;
  }

  status = x_reply(&Request, XmTextGetString(w_send_scrl));
  STANDARD_CURSOR;

  if (status == SUCCESS)
    XmTextSetString(w_send_scrl, "");   
}

void
olc_close_msg (w, tag, callback_data)
     Widget w;
     XtPointer tag;
     XtPointer callback_data;
{
  XtUnmanageChild(w_send_form);
  XtSetSensitive(w_send_btn, TRUE);
}
