/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains the main routine of the user program, "xolc".
 *
 *	Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *      $Id: main.c,v 1.21 1999-01-22 23:12:21 ghudson Exp $
 */

#ifndef SABER
#ifndef lint
static char rcsid[]="$Id: main.c,v 1.21 1999-01-22 23:12:21 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>

#include <pwd.h>
#include <netdb.h>
#include <sys/param.h>

#include "xolc.h"
#include "data.h"

/*
 *  OLC-type variables.
 */

static XrmOptionDescRec options[] = {
{"-scale",      "*load.minScale",       XrmoptionSepArg,           NULL},
};

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
     char **argv;
{  
  Arg args[10];
  int n = 0;
  char *config;

  program = strrchr(*argv,'/');
  if(program == (char *) NULL)
     program = *argv;
  if(*program == '/')
     ++program;

  config = getenv("OLXX_CONFIG");
  if (! config)
    config = OLC_CONFIG_PATH;
 
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
  xolc = XtInitialize("xolc" , "Xolc", NULL, 0, &argc, argv);
  add_converter();

  ++argv, --argc;
  while (argc > 0 && argv[0][0] == '-') {
    if (!strcmp (argv[0], "-name") && (argc>1)) {
      program = argv[1];
      ++argv, --argc;
    }
    else if (!strcmp (argv[0], "-config") && (argc>1)) {
      config = argv[1];
      ++argv, --argc;
    }
    else if (!strcmp (argv[0], "-server") && (argc>1)) {
      /*
       * this is a kludge, but the other interface is already
       * there
       */
      set_env_var ("OLCD_HOST", argv[1]);
      ++argv, --argc;
    }
    else if (!strcmp (argv[0], "-port") && (argc>1)) {
      set_env_var ("OLCD_PORT", argv[1]);
      ++argv, --argc;
    }
    else {
      fprintf (stderr, "%s: flag unknown or its argument is missing: %s\n",
	       program, argv[0]);
      exit (1);
    }
    ++argv;
    --argc;
  }

  if (incarnate(program, config) == FATAL) {
    /* Fatal problem.  Messages indicating causes were already displayed... */
    exit(1);
  }

  n=0;
  XtSetArg(args[n], XmNallowShellResize, TRUE); n++;
  XtSetValues(xolc, args, n);

  MuInitialize(xolc);

  MakeInterface();
  MakeNewqForm();
  MakeContqForm();
  MakeMotdForm();
  MakeDialogs();

/*
 * 
 */
  XtRealizeWidget(xolc);
  olc_init();
  XtUnmapWidget(w_newq_form);
  XtUnmapWidget(w_contq_form);
  XtManageChild(main_form);

#ifdef LOG_USAGE
  log_startup("xolc");
#endif

/*
 * Go into XtMainLoop.  This does not exit.  Rather, the user exits the
 *  program by clicking on "quit" or through some other action.
 */

  XtMainLoop();
}


/* Set an environment variable.
 * Arguments:	var: a string containing the name of the variable.
 *		value: a string containing the new value.
 * Returns:	nothing.
 * Non-local returns: on some platforms, exits with code 1 if malloc fails.
 */

void
set_env_var(const char *var, const char *value)
{
#ifdef PUTENV
  char *buf = malloc(strlen(var)+strlen(value)+2);
  if (buf == NULL)
    {
      fprintf(stderr, "Out of memory, can't expand environment.\n");
      exit(1);
    }
  sprintf(buf, "%s=%s", var, value);
  putenv(buf);
#else
  setenv (var, value, 1);
#endif
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

 try_again:
  fill_request(&Request);
  Request.request_type = OLC_STARTUP;
  
  status = open_connection_to_daemon(&Request, &fd);
  if(status)
    {
      if (handle_response(status, &Request) == FAILURE)
	goto try_again;
      exit(ERROR);
    }

  status = send_request(fd, &Request);
  if (status)
    {
      if ((handle_response(status, &Request)) == FAILURE) {
	close(fd);
	goto try_again;
      }
      exit(ERROR);
    }
  read_response(fd, &response);
  
  switch(response)
    {
    case USER_NOT_FOUND:
      XtManageChild(w_newq_btn);
      XtSetArg(arg, XmNleftWidget, (Widget) w_newq_btn);
      XtSetValues(w_stock_btn, &arg, 1);
      XtManageChild(w_stock_btn);
      break;

    case CONNECTED:
    case SUCCESS:
      read_int_from_fd(fd, &n);
      t_set_default_instance(&Request);
      has_question = TRUE;
      XtManageChild(w_contq_btn);
      XtSetArg(arg, XmNleftWidget, (Widget) w_contq_btn);
      XtSetValues(w_stock_btn, &arg, 1);
      XtManageChild(w_stock_btn);
      break;

    case PERMISSION_DENIED:
      MuErrorSync("You are not allowed to use OLC.\nPlease contact a consultant at 253-4435.");
      exit(1);

    default:
      status = handle_response(response, &Request);
      if (status == FAILURE)
	{
	  close(fd);
	  goto try_again;
	}
      if (status == ERROR)
        exit(ERROR);
    }

  make_temp_name(file);
  switch(x_get_motd(&Request,0,file,FALSE))     /* NB: 2nd arg is irrelevant */
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
