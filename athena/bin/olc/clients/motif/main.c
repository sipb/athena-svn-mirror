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
static char rcsid[]="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/main.c,v 1.4 1989-07-31 16:01:46 vanharen Exp $";
#endif

#include "xolc.h"

/*
 *  X- and Motif-type variables.
 */

static MrmHierarchy	Hierarchy;     /* MRM database hierarchy id */
static MrmCode		class;

static Display *display;
static Window root;
static int screen;

static char *vec[] = {"olc.uid"};	/* There is 1 uid file to fetch.  */

/*
 *  Callbacks that are attached to the buttons and widgets in the
 *   interface.
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
  widget_create()
;

static MrmCount reg_num = 14;		/* There are reg_num callbacks */
static MRMRegisterArg  reg_vec[] = {
  {"olc_new_ques", (caddr_t) olc_new_ques},
  {"olc_cont_ques", (caddr_t) olc_cont_ques},
  {"olc_stock", (caddr_t) olc_stock},
  {"olc_help", (caddr_t) olc_help},
  {"olc_quit", (caddr_t) olc_quit},
  {"olc_send", (caddr_t) olc_send},
  {"olc_done", (caddr_t) olc_done},
  {"olc_cancel", (caddr_t) olc_cancel},
  {"olc_savelog", (caddr_t) olc_savelog},
  {"olc_motd", (caddr_t) olc_motd},
  {"olc_update", (caddr_t) olc_update},
  {"dlg_ok", (caddr_t) dlg_ok},
  {"dlg_cancel", (caddr_t) dlg_cancel},
  {"widget_create", (caddr_t) widget_create},
};

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

int OLC=1;


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

  Arg arglist[1];
  XEvent event;
  XtAppContext my_app_context;

#ifdef HESIOD
  char **hp;                   /* return value of Hesiod resolver */
#endif

  program = rindex(*argv,'/');
  if(program == (char *) NULL)
     program = *argv;
  if(*program == '/')
     ++program;

#ifdef HESIOD
  if ((hp = hes_resolve(OLC_SERVICE,OLC_SERV_NAME)) == NULL)
    {
      fprintf(stderr,
              "Unable to get name of OLC server host from the Hesiod nameserver.\nThis means that you cannot use OLC at this time. Any problems \nyou may be experiencing with your workstation may be the result of this\nproblem. \n");
      exit(ERROR);
    }
  else
    (void) strcpy(DaemonHost, *hp);

#endif HESIOD


  uid = getuid();
  pwent = getpwuid(uid);
  (void) strcpy(User.username, pwent->pw_name);
  (void) strcpy(User.realname, pwent->pw_gecos);
  if (index(User.realname, ',') != 0)
    *index(User.realname, ',') = '\0';

  gethostname(hostname, LINE_LENGTH);
  host = gethostbyname(hostname);
  (void) strcpy(User.machine, (host ? host->h_name : hostname));
  User.uid = uid;

  expand_hostname(DaemonHost,INSTANCE,REALM);

#ifdef TEST
printf("main: starting for %s/%s (%d)  on %s\n",User.realname, User.username,
       User.uid, User.machine);
#endif TEST

  signal(SIGPIPE, SIG_IGN);
  

/*  Initialize Motif Resource Manager.	*/

  MrmInitialize();

/*
 *  First, try opening display.  If this fails, print a 'nice' error
 *  message and exit.
 */
  if (XOpenDisplay() == NULL)
    {
      fprintf(stderr, "%s: Unable to open X display.  Check to make sure that your\n\tDISPLAY environment variable is set.  Type:\n\n\t\tprintenv  DISPLAY\n\n\tto see if it is set.\n\n\tIf it is not, usually the problem can be fixed by\n\tsetting your DISPLAY to 'unix:0.0'.   Type:\n\n\t\tsetenv  DISPLAY  unix:0.0\n\n\tand try running this program again, or use the text-based\n\tinterface for OLC by typing:\n\n\t\tolc\n\n", program);
      exit(ERROR);
    }
/*
 *  If opening display was successful, then initialize toolkit, display,
 *  interface, etc.
 */
  toplevel = XtInitialize(NULL, "OLC", NULL, 0, &argc, argv);
  XtSetArg (arglist[0], XtNallowShellResize, TRUE);
  XtSetValues (toplevel, arglist, 1);
  display = XtDisplay(toplevel); 
  screen = DefaultScreen(display);
  root = RootWindow(display, screen);
  
#ifdef TEST
  printf("main: Opening hierarchy.  Registering and fetching widgets.\n");
#endif TEST
  if (MrmOpenHierarchy (1, vec, NULL, &Hierarchy) != MrmSUCCESS)
    {
      fprintf(stderr, "%s: Unable to OPEN heirarchy file that contains layout of interface.\n\tTry using the text-based interface instead.  Type:\n\n\t\tolc\n\n", program);
      exit(ERROR);
    }
      
  if (MrmRegisterNames (reg_vec, reg_num) != MrmSUCCESS)
    {
      fprintf(stderr, "%s: Unable to REGISTER names for layout of interface.\n\tTry using the text-based interface instead.  Type:\n\n\t\tolc\n\n", program);
      exit(ERROR);
    }

  if (MrmFetchWidget (Hierarchy, "main", toplevel, &main_form, 
		     &class) != MrmSUCCESS)
    {
      fprintf(stderr, "%s: Unable to FETCH layout of interface.\n\tTry using the text-based interface instead.  Type:\n\n\t\tolc\n\n", program);
      exit(ERROR);
    }

  XtManageChild(main_form);
/*
 * 
 */
  XtRealizeWidget(toplevel);

  olc_init();

/*
 * This program does not use XtMainLoop() because it also processes zephyr
 *  notifications.  There is no way to process events other than X events
 *  using XtMainLoop, but this is essentially what it does anyway.
 */

  while (1)
    {
      XtNextEvent(&event);
      XtDispatchEvent(&event);
    }
}


olc_init()
{
  int fd;			/* File descriptor for socket. */
  RESPONSE response;		/* Response code from daemon. */
  REQUEST Request;
  ERRCODE errcode=0;		/* Error code to return. */
  int n,first=0;
  char file[NAME_LENGTH];
  char topic[TOPIC_SIZE];
  Arg arg;
  
  fill_request(&Request);
  Request.request_type = OLC_STARTUP;
  
  fd = open_connection_to_daemon();
  send_request(fd, &Request);
  read_response(fd, &response);
  
#ifdef TEST
  printf("olc_init: requester %s, response: %d\n",
         Request.requester.username, response);
#endif TEST
  
  make_temp_name(file);
  x_get_motd(&Request,OLC,file);
  unlink(file);
  if (! XtIsManaged(w_motd_form) )
    XtManageChild(w_motd_form);

  switch(response)
    {
    case USER_NOT_FOUND:
#ifdef TEST
      printf("olc_init: %s not found\n", Request.requester.username);
#endif TEST
      if (! XtIsManaged(w_newq_btn) )
	XtManageChild(w_newq_btn);
      XtSetArg(arg, XmNleftWidget, (Widget) w_newq_btn);
      XtSetValues(w_stock_btn, &arg, 1);
      break;

    case CONNECTED:
    case SUCCESS:
      read_int_from_fd(fd, &n);
#ifdef TEST
      printf("olc_init: %s found\n", Request.requester.username);
#endif TEST
      if (! XtIsManaged(w_contq_btn) )
	XtManageChild(w_contq_btn);
      break;
   case PERMISSION_DENIED:
      printf("You are not allowed to use OLC.\n");
      exit(1);
    default:
      if(handle_response(response, &Request) == ERROR)
        exit(ERROR);
    }

  (void) close(fd);
  fflush(stdout);
}
