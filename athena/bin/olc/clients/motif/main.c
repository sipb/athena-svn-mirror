/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains the main routine of the user program, "olc".
 *
 *      Win Treese
 *      Dan Morgan
 *      Bill Saphir
 *      MIT Project Athena
 *
 *      Ken Raeburn
 *      MIT Information Systems
 *
 *      Steve Dyer
 *      IBM/MIT Project Athena
 *      converted to use Hesiod in place of clustertab
 *
 *      Tom Coppeto
 *	Chris VanHaren
 *      MIT Project Athena
 *
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/main.c,v $
 *      $Author: vanharen $
 */

#ifndef lint
static char rcsid[]="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/main.c,v 1.6 1989-10-11 16:10:03 vanharen Exp $";
#endif

#include "xolc.h"
#include "data.h"

/*
 *  X- and Motif-type variables.
 */

Display *display;
Window root;
int screen;

/*
 *  OLC-type variables.
 */

PERSON User;                            /* Structure describing user. */
char DaemonHost[LINE_LENGTH];           /* Name of the daemon's machine. */
char *program;

char REALM[40];
char INSTANCE[40];

char *LOCAL_REALM = "ATHENA.MIT.EDU";
char *LOCAL_REALMS[] =
{
  "MIT.EDU",
  "DU.MIT.EDU",
  "SIPB.MIT.EDU",
  "PIKA.MIT.EDU",
  "CARLA.MIT.EDU",
  "ZBT.MIT.EDU",
  "",
};

int has_question=FALSE;
int init_screen=FALSE;
int ask_screen=FALSE;
int replay_screen=FALSE;
int OLCR=0, OLC=0;
int XOLCR=0, XOLC=0;

/*
 *  Function:	main() is the startup for OLC.  It initializes the X display,
 *			then contacts the daemon to display the motd and
 *			determine the state of the user.
 *  Arguments:	argc:  Number of args in command line.  Used by XtInitialize.
 *		argv:  Array of words in command line.  Used by XtInitialize.
 *  Returns:    ERROR if unable to open display, else nothing.
 *  Notes:
 *	
 */

main(argc, argv)
     int argc;
     char *argv[];
{  
  struct passwd *getpwuid();   /* Get a password entry based on the user ID.*/
  struct passwd *pwent;        /* Password entry. */
  char hostname[LINE_LENGTH];  /* Name of local machine. */
  struct hostent *host;
  int uid;                     /* User ID number. */
  char *tty;                   /* Terminal path name. */
  char *prompt;

  Arg args[10];
  int n = 0;
  XEvent event;
  XtAppContext my_app_context;

  unsigned int icon_h, icon_w;
  int *xh, *yh;
  Pixmap pixmap;

#ifdef HESIOD
  char **hp;                   /* return value of Hesiod resolver */
#endif

/*
 * All client specific stuff should be initialized here, if they wish
 * to play after dinner.
 */

  program = rindex(*argv,'/');
  if(program == (char *) NULL)
     program = *argv;
  if(*program == '/')
     ++program;

  if(string_eq(program,"xolc"))
    {
      OLC = 1;
      XOLC = 1;
    }
  else
    {
      OLCR = 1;
      XOLCR = 1;
    }

#ifdef HESIOD
  if ((hp = hes_resolve(OLC_SERVICE,OLC_SERV_NAME)) == NULL)
    {
      fprintf(stderr, "\n%s:\tUnable to get name of OLC server host from the Hesiod nameserver.\n\tThis means that you cannot use OLC at this time. Any problems \n\tyou may be experiencing with your workstation may be the result\n\tof this problem.  Try again in a few minutes, or call a consultant\n\tfor help at 253-4435.\n\n", program);
      exit(ERROR);
    }
  else
    (void) strcpy(DaemonHost, *hp);

#endif HESIOD


  uid = getuid();
  if ((pwent = getpwuid(uid)) == NULL)
    {
      fprintf(stderr, "\n%s:\tUnable to find you in the password file.  Any problems that you\n\tmay have been having may be related to the fact that you are not\n\tin the password file.  Try logging out and back in again, or\n\tcall a consultant for help at 253-4435.\n\n", program);
      exit(ERROR);
    }
  (void) strcpy(User.username, pwent->pw_name);
  (void) strcpy(User.realname, pwent->pw_gecos);
  if (index(User.realname, ',') != 0)
    *index(User.realname, ',') = '\0';

  gethostname(hostname, LINE_LENGTH);
  host = gethostbyname(hostname);
  (void) strcpy(User.machine, (host ? host->h_name : hostname));
  User.uid = uid;

  expand_hostname(DaemonHost,INSTANCE,REALM);

/*
 *  First, try opening display.  If this fails, print a 'nice' error
 *  message and exit.
 */
  if (XOpenDisplay(NULL) == NULL)
    {
      fprintf(stderr, "\n%s:\tUnable to open X display.  Check to make sure that your DISPLAY\n\tenvironment variable is set.  Type:\n\n\t\tprintenv  DISPLAY\n\n\tto see if it is set.\n\n\tIf it is not, usually the problem can be fixed by setting your\n\tDISPLAY to 'unix:0.0'.   Type:\n\n\t\tsetenv  DISPLAY  unix:0.0\n\n\tand try running this program again.  Any problems you may have\n\tbeen having while trying to run X programs may be due to problem.\n\tTry running them again after setting your DISPLAY variable.  If\n\tyou are still having problems, you may want to use the text\n\tinterface for OLC instead.  Type:\n\n\t\tolc\n\n", program);
      exit(ERROR);
    }
/*
 *  If opening display was successful, then initialize toolkit, display,
 *  interface, etc.
 */
  setenv("XAPPLRESDIR", APP_DEF_DIR, 1);
  toplevel = XtInitialize(NULL, "Xolc", NULL, 0, &argc, argv);

  display = XtDisplay(toplevel); 
  screen = DefaultScreen(display);
  root = RootWindow(display, screen);

  XReadBitmapFile(display, root, ICON_FILENAME,
		  &icon_h, &icon_w,
		  &pixmap, &xh, &yh);

  n=0;
  XtSetArg(args[n], XmNallowShellResize, TRUE); n++;
/*  XtSetArg(args[n], XmNwidth, 570); n++;
 *  XtSetArg(args[n], XmNheight, 355); n++;
 */
  XtSetArg(args[n], XmNminWidth, 530); n++;
  XtSetArg(args[n], XmNminHeight, 355); n++;
  XtSetArg(args[n], XmNiconPixmap, pixmap); n++;
  XtSetValues(toplevel, args, n);

  MuInitialize(toplevel);

  
  MakeInterface();
  MakeContqForm();
  MakeNewqForm();
  MakeMotdForm();
  MakeDialogs();

/*
 * 
 */
  XtRealizeWidget(toplevel);

  olc_init();

/*
 * Go into XtMainLoop.  This does not exit.  Rather, the user exits the
 *  program by clicking on "quit" or through some other action.
 */

  XtMainLoop();
}


/*
 * Function:    olc_init() completes the initialization process for
 *                      the user program.
 * Arguments:   None.
 * Returns:     Nothing.
 * Notes:
 *      First, find out if the user has a current question.  If not,
 *      prompt for topic and question.  Next, send an OLC_STARTUP
 *      request to the daemon, either starting a new conversation or
 *      continuing an old one.  A message notifying the user of the
 *      status of the question is then printed.  Finally, we return
 *      to the OLC main loop.
 */

olc_init()
{
  int fd;			/* File descriptor for socket. */
  RESPONSE response;		/* Response code from daemon. */
  REQUEST Request;
  ERRCODE errcode=0;		/* Error code to return. */
  int n,first=0;
  char file[NAME_LENGTH];
  char topic[TOPIC_SIZE];
  int status;
  Arg arg;

  init_screen = TRUE;

 init_try_again:
  fill_request(&Request);
  Request.request_type = OLC_STARTUP;
  
  fd = open_connection_to_daemon();
  status = send_request(fd, &Request);
  if (status)
    {
      if ((handle_response(status, &Request)) == FAILURE)
	{
	  close(fd);
	  goto init_try_again;
	}
      else
	exit(ERROR);
    }
  
  read_response(fd, &response);
  
  make_temp_name(file);

  switch(x_get_motd(&Request,OLC,file,0))
    {
    case FAILURE:
      unlink(file);
      goto init_try_again;
    case ERROR:
      exit(ERROR);
    default:
      break;
    }
  unlink(file);

  switch(response)
    {
    case USER_NOT_FOUND:
      XtSetArg(arg, XmNleftWidget, (Widget) w_newq_btn);
      XtSetValues(w_stock_btn, &arg, 1);
      if (! XtIsManaged(w_newq_btn) )
	XtManageChild(w_newq_btn);
      break;

    case CONNECTED:
    case SUCCESS:
      read_int_from_fd(fd, &n);
      t_set_default_instance(&Request);
      has_question = TRUE;
      if (! XtIsManaged(w_contq_btn) )
	XtManageChild(w_contq_btn);
      break;

    case PERMISSION_DENIED:
      MuErrorSync("You are not allowed to use OLC.\nPlease contact a consultant at 253-4435.");
      exit(1);
    default:
      if(handle_response(response, &Request) == ERROR)
        exit(ERROR);
    }

  (void) close(fd);
  fflush(stdout);
}
