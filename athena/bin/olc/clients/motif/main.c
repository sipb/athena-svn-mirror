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
 *      $Author: lwvanels $
 */

#ifndef SABER
#ifndef lint
static char rcsid[]="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/main.c,v 1.10 1991-03-24 14:30:37 lwvanels Exp $";
#endif
#endif

#include <pwd.h>
#include <netdb.h>
#include <sys/param.h>

#include "xolc.h"
#include "data.h"

/*
 *  OLC-type variables.
 */

PERSON User;                            /* Structure describing user. */
char DaemonHost[MAXHOSTNAMELEN];           /* Name of the daemon's machine. */
char *program;

#ifdef KERBEROS
char REALM[REALM_SZ];
char INSTANCE[INST_SZ];

extern char *LOCAL_REALM;
extern char *LOCAL_REALMS[];
#endif

int select_timeout = 300;

int has_question=FALSE;
int init_screen=FALSE;
int ask_screen=FALSE;
int replay_screen=FALSE;
int OLCR=0, OLC=0;

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
  char hostname[MAXHOSTNAMELEN];  /* Name of local machine. */
  struct hostent *host;

  Arg args[10];
  int n = 0;

#ifdef HESIOD
  char **hp;                   /* return value of Hesiod resolver */
#endif

  program = rindex(*argv,'/');
  if(program == (char *) NULL)
     program = *argv;
  if(*program == '/')
     ++program;

  if(string_eq(program,"xolc"))
    {
      OLC = 1;
    }
  else
    {
      OLCR = 1;
    }


  ++argv, --argc;
  while (argc > 0 && argv[0][0] == '-') {
    if (!strcmp (argv[0], "-server")) {
      /*
       * this is a kludge, but the other interface is already
       * there
       */
      (void) setenv ("OLCD_HOST", argv[1], 1);
      ++argv, --argc;
    }
    else if (!strcmp (argv[0], "-port")) {
      (void) setenv ("OLCD_PORT", argv[1], 1);
      ++argv, --argc;
    }
    else {
      fprintf (stderr, "%s: unknown control argument %s\n",
	       program, argv[0]);
      exit (1);
    }
    ++argv;
    --argc;
  }

/*
 *  First, try opening display.  If this fails, print a 'nice' error
 *  message and exit.
 */
  if (XOpenDisplay(NULL) == NULL)
    {
      fprintf(stderr, "\n\tUnable to open X display.  Check to make sure that your DISPLAY\n");
      fprintf(stderr, "\tenvironment variable is set.  Type:\n\n");
      fprintf(stderr, "\t\tprintenv  DISPLAY\n\n\tto see if it is set.\n\n");
      fprintf(stderr, "\tIf it is not, usually the problem can be fixed by setting your");
      fprintf(stderr, "\n\tDISPLAY to 'unix:0.0'.   Type:\n\n\t\tsetenv  DISPLAY  unix:0.0\n\n");
      fprintf(stderr, "\tand try running this program again.  Any problems you may have\n");
      fprintf(stderr, "\tbeen having while trying to run X programs may be due to problem.\n");
      fprintf(stderr, "\tTry running them again after setting your DISPLAY variable.  If\n");
      fprintf(stderr, "\tyou are still having problems, you may want to use the text\n");
      fprintf(stderr, "\tinterface for OLC instead.  Type:\n\n\t\tolc\n\n");
      exit(ERROR);
    }
/*
 *  If opening display was successful, then initialize toolkit, display,
 *  interface, etc.
 */
  toplevel = XtInitialize(NULL, "Xolc", NULL, 0, &argc, argv);

  n=0;
  XtSetArg(args[n], XmNallowShellResize, TRUE); n++;
  XtSetValues(toplevel, args, n);

  MuInitialize(toplevel);

  MakeInterface();
  MakeContqForm();
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
  int n=0;
  char file[MAXPATHLEN];
  int status;
  Arg arg;

  init_screen = TRUE;

  OInitialize();

  fill_request(&Request);
  Request.request_type = OLC_STARTUP;
  
  status = open_connection_to_daemon(&Request, &fd);
  if(status)
    {
      handle_response(status, &Request);
      exit(ERROR);
    }

  status = send_request(fd, &Request);
  if (status)
    {
      if ((handle_response(status, &Request)) == FAILURE)
	close(fd);
      exit(ERROR);
    }
  read_response(fd, &response);
  
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

  make_temp_name(file);
  switch(x_get_motd(&Request,OLC,file,0))
    {
    case FAILURE:
    case ERROR:
      unlink(file);
      exit(ERROR);
    }
  unlink(file);
  (void) close(fd);
  fflush(stdout);
}
