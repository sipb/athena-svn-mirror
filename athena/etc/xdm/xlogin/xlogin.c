/* $Header: /afs/dev.mit.edu/source/repository/athena/etc/xdm/xlogin/xlogin.c,v 1.33 1993-04-23 16:14:07 miki Exp $ */

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <utmp.h>
#include <fcntl.h>
#include <X11/Intrinsic.h>
#include <ctype.h>
#include "../wcl/WcCreate.h"
#include <X11/StringDefs.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Text.h>
#include <X11/Xaw/Form.h>
#ifdef SOLARIS
#include <X11/Xaw/SmeBSB.h>
#endif
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Shell.h>
#include <X11/Xmu/Drawing.h>
#include <X11/Xmu/Converters.h>
#include "owl.h"

/* Define the following if restarting the X server does not restore
 * auto-repeat properly
 */

#ifdef ultrix
#define BROKEN_AUTO_REP
#define DISABLE_AUTO_REP
#endif


#if defined(_IBMR2)
#include <time.h>
#include <sys/id.h>
#endif

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MOTD_FILENAME
#define MOTD_FILENAME "/afs/athena.mit.edu/system/config/motd/login.74"
#endif

#define OWL_AWAKE 0
#define OWL_SLEEPY 1

#define OWL_STATIC 0
#define OWL_BLINKINGCLOSED 1
#define OWL_BLINKINGOPEN 2
#define OWL_SLEEPING 3
#define OWL_WAKING 4

#define ACTIVATED 1
#define REACTIVATING 2

#define DAEMON 1	/* UID for scripts to run as */

gid_t def_grplist[] = { 101 };			/* default group list */


/*
 * Function declarations.
 */
extern void AriRegisterAthena ();
extern unsigned long random();
static void move_instructions(), screensave(), unsave(), start_reactivate();
static void blinkOwl(), initOwl(), adjustOwl(), catch_child(), setFontPath();
static void setAutoRepeat();
static int getAutoRepeat();
void focusACT(), unfocusACT(), runACT(), runCB(), focusCB(), resetCB();
void idleReset(), loginACT(), localErrorHandler(), setcorrectfocus();
void sigconsACT(), sigconsCB(), callbackACT(), attachandrunCB();
char *malloc(), *strdup();
extern void add_converter ();


/*
 * Definition of the Application resources structure.
 */

typedef struct _XLoginResources {
  int save_timeout;
  int move_timeout;
  int blink_timeout;
  int reactivate_timeout;
  int activate_timeout;
  int restart_timeout;
  int randomize;
  int detach_interval;
  String activate_prog;
  String reactivate_prog;
  String tty;
  String session;
  String fontpath;
  String srvdcheck;
  String loginName;
  Boolean blankAll;
  Boolean showMotd;
  String motdFile;
  String motd2File;
} XLoginResources;

/*
 * Command line options table.  Only resources are entered here...there is a
 * pass over the remaining options after XtParseCommand is let loose. 
 */

static XrmOptionDescRec options[] = {
  {"-save",	"*saveTimeout",		XrmoptionSepArg,	NULL},
  {"-move",	"*moveTimeout",		XrmoptionSepArg,	NULL},
  {"-blink",	"*blinkTimeout",	XrmoptionSepArg,	NULL},
  {"-reactivate","*reactivateProg",	XrmoptionSepArg,	NULL},
  {"-randomize","*randomize",		XrmoptionSepArg,	NULL},
  {"-detach",	"*detachInterval",	XrmoptionSepArg,	NULL},
  {"-idle",	"*reactivateTimeout",	XrmoptionSepArg,	NULL},
  {"-wait",	"*activateTimeout",	XrmoptionSepArg,	NULL},
  {"-restart",	"*restartTimeout",	XrmoptionSepArg,	NULL},
  {"-tty",	"*loginTty",		XrmoptionSepArg,	NULL},
  {"-session",	"*sessionScript",	XrmoptionSepArg,	NULL},
  {"-srvdcheck","*srvdCheck",		XrmoptionSepArg,	NULL},
  {"-fp",	"*fontPath",		XrmoptionSepArg,	NULL},
  {"-blankall", "*blankAll",		XrmoptionNoArg,   (caddr_t) "on"},
  {"-noblankall","*blankAll",		XrmoptionNoArg,   (caddr_t) "off"},
  {"-motdfile",	"*motdFile",		XrmoptionSepArg,	NULL},
  {"-motd2file","*motd2File",		XrmoptionSepArg,	NULL},
};

/*
 * The structure containing the resource information for the
 * Xlogin application resources.
 */

#define Offset(field) (XtOffset(XLoginResources *, field))

static XtResource my_resources[] = {
  {"saveTimeout", XtCInterval, XtRInt, sizeof(int),
     Offset(save_timeout), XtRImmediate, (caddr_t) 120},
  {"moveTimeout", XtCInterval, XtRInt, sizeof(int),
     Offset(move_timeout), XtRImmediate, (caddr_t) 20},
  {"blinkTimeout", XtCInterval, XtRInt, sizeof(int),
     Offset(blink_timeout), XtRImmediate, (caddr_t) 40},
  {"reactivateProg", XtCFile, XtRString, sizeof(String),
     Offset(reactivate_prog), XtRImmediate, "/etc/athena/reactivate"},
  {"randomize", XtCInterval, XtRInt, sizeof(int),
     Offset(randomize), XtRImmediate, (caddr_t) 60},
  {"detachInterval", XtCInterval, XtRInt, sizeof(int),
     Offset(detach_interval), XtRImmediate, (caddr_t) 12},
  {"activateTimeout", XtCInterval, XtRInt, sizeof(int),
     Offset(activate_timeout), XtRImmediate, (caddr_t) 30},
  {"restartTimeout", XtCInterval, XtRInt, sizeof(int),
     Offset(restart_timeout), XtRImmediate, (caddr_t) (60 * 60 * 12)},
  {"reactivateTimeout", XtCInterval, XtRInt, sizeof(int),
     Offset(reactivate_timeout), XtRImmediate, (caddr_t) 300},
  {"loginTty", XtCFile, XtRString, sizeof(String),
     Offset(tty), XtRImmediate, (caddr_t) "ttyv0"},
  {"sessionScript", XtCFile, XtRString, sizeof(String),
     Offset(session), XtRImmediate, (caddr_t) "/etc/athena/login/Xsession"},
  {"srvdcheck", XtCFile, XtRString, sizeof(String),
     Offset(srvdcheck), XtRImmediate, (caddr_t) "/srvd/.rvdinfo"},
#ifdef SOLARIS
  {"fontPath", XtCString, XtRString, sizeof(String),
     Offset(fontpath), XtRImmediate, (caddr_t) "/usr/openwin/lib/fonts/" },
#else
  {"fontPath", XtCString, XtRString, sizeof(String),
     Offset(fontpath), XtRImmediate, (caddr_t) "/usr/athena/lib/X11/fonts/misc/,/usr/athena/lib/X11/fonts/75dpi/,/usr/athena/lib/X11/fonts/100dpi/" },
#endif
  {"loginName", XtCString, XtRString, sizeof(String),
     Offset(loginName), XtRImmediate, (caddr_t) "" },
  {"blankAllScreens", XtCBoolean, XtRBoolean, sizeof(Boolean),
     Offset(blankAll), XtRImmediate, (caddr_t) True},
  {"showMotd", XtCBoolean, XtRBoolean, sizeof(Boolean),
     Offset(showMotd), XtRImmediate, (caddr_t) True},
  {"motdFile", XtCString, XtRString, sizeof(String),
     Offset(motdFile), XtRImmediate, (caddr_t) MOTD_FILENAME },
  {"motd2File", XtCString, XtRString, sizeof(String),
     Offset(motd2File), XtRImmediate, (caddr_t) "" },
};

#undef Offset

XtActionsRec actions[] = {
    { "setfocus", focusACT },
    { "unsetfocus", unfocusACT },
    { "run", runACT },
    { "idleReset", idleReset },
    { "login", loginACT },
    { "reset", resetCB },
    { "setCorrectFocus", setcorrectfocus },
    { "signalConsoleACT", sigconsACT },
    { "callbackACT", callbackACT },
};




#ifndef CONSOLEPID
#define CONSOLEPID "/etc/athena/console.pid"
#endif
#ifndef UTMPF
#define UTMPF "/etc/utmp"
#endif

/*
 * Globals.
 */
XtIntervalId curr_timerid = 0, blink_timerid = 0, react_timerid = 0;
Widget appShell;
Widget saver, ins;
Widget savershell[10];
int num_screens;
XLoginResources resources;
GC owlGC, isGC;
Display *dpy;
Window owlWindow, isWindow;
int owlNumBitmaps, isNumBitmaps;
unsigned int owlWidth, owlHeight, isWidth, isHeight;
int owlState, owlDelta, owlTimeout;
Pixmap owlBitmaps[20], isBitmaps[20];
struct timeval starttime;
int activation_state, activation_pid, activate_count = 0;
int attach_state, attach_pid;
int attachhelp_state, attachhelp_pid, quota_pid;
int exiting = FALSE;
extern char *defaultpath;

/*
 * Local Globals
 */
static int autorep;


/******************************************************************************
*   MAIN function
******************************************************************************/

void
main(argc, argv)
     int argc;
     char* argv[];
{   
  XtAppContext app;
  Widget hitanykey, namew;
  Display *dpy1;
  char hname[64], *c;
  Arg args[1];
  int i;
  unsigned acc = 0;
#ifdef SOLARIS
   static char buf[1024];
#endif
  signal(SIGCHLD, catch_child);

  /* Have to find this argument before initializing the toolkit.
   * We set both XAPPLRESDIR and XENVIRONMENT.  The effect is that
   * the -config argument names a directory that will have the file
   * Xlogin which contains the resources, and may optionally have
   * a Xlogin.local file containing additional resources which will
   * override those in the regular file.
   */
  for (i = 1; i < argc; i++)
    if (!strcmp(argv[i], "-config") && (i+1 < argc)) {
	setenv("XAPPLRESDIR", argv[i+1], 1);
	sprintf(hname, "%s/Xlogin.local", argv[i+1]);
	setenv("XENVIRONMENT", hname, 1);
	break;
    }

  /*
   *  Intialize Toolkit creating the application shell, and get
   *  application resources.
   */
  appShell = XtInitialize ("xlogin", "Xlogin",
			   options, XtNumber(options),
			   &argc, argv);
  add_converter ();
  app = XtWidgetToApplicationContext(appShell);
  XtAppSetErrorHandler(app, localErrorHandler);
  dpy = XtDisplay(appShell);
  XtAppAddActions(app, actions, XtNumber(actions));

  XtGetApplicationResources(appShell, (caddr_t) &resources, 
			    my_resources, XtNumber(my_resources),
			    NULL, (Cardinal) 0);

  WcRegisterCallback(app, "UnsetFocus", unfocusACT, NULL);
  WcRegisterCallback(app, "runCB", runCB, NULL);
  WcRegisterCallback(app, "setfocusCB", focusCB, NULL);
  WcRegisterCallback(app, "resetCB", resetCB, NULL);
  WcRegisterCallback(app, "signalConsoleCB", sigconsCB, NULL);
  WcRegisterCallback(app, "idleResetCB", idleReset, NULL);
  WcRegisterCallback(app, "attachAndRunCB", attachandrunCB, NULL);

  /*
   *  Register all Athena widget classes
   */
  AriRegisterAthena ( app );

  /* clear console */
  sigconsCB(NULL, "clear", NULL);

  /* Turn off keyboard autorepeat - triggers bugs when ^P used to 
   * shutdown for a console login
   */
#ifdef BROKEN_AUTO_REP
  autorep = AutoRepeatModeOn;
#else
  autorep = getAutoRepeat();
#endif
  setAutoRepeat(AutoRepeatModeOff);

  /*
   *  Create widget tree below toplevel shell using Xrm database
   */
  WcWidgetCreation ( appShell );

  /*
   *  Realize the widget tree, finish up initializing,
   *  and enter the main application loop
   */
  XtRealizeWidget ( appShell );

  initOwl( appShell );		/* widget tree MUST be realized... */
  adjustOwl( appShell );

  /* Put the hostname in the label of the host widget */
  gethostname(hname, sizeof(hname));
  XtSetArg(args[0], XtNlabel, hname);
  namew = WcFullNameToWidget(appShell, "*login*host");
  XtSetValues(namew, args, 1);
  namew = WcFullNameToWidget(appShell, "*instructions*host");
  XtSetValues(namew, args, 1);
  /* Also seed random number generator with hostname */
  c = hname;
  while (*c)
    acc = (acc << 1) ^ *c++;
  srandom(acc);
  resources.reactivate_timeout += random() % resources.randomize;
  saver = WcFullNameToWidget(appShell, "*savershell");
  ins = WcFullNameToWidget(appShell, "*instructions");
  hitanykey = WcFullNameToWidget(appShell, "*hitanykey");

#define MASK	KeyReleaseMask | ButtonReleaseMask
  XtAddEventHandler(saver, MASK,
		    FALSE, unsave, TRUE);
  XtAddEventHandler(hitanykey, MASK,
		    FALSE, unsave, TRUE);

  curr_timerid = XtAddTimeOut(resources.save_timeout * 1000,
			      screensave, NULL);
  blink_timerid = XtAddTimeOut(1000, blinkOwl, NULL);
  gettimeofday(&starttime, NULL);
  resetCB(namew, NULL, NULL);
  if (access(resources.srvdcheck, F_OK) != 0)
    start_reactivate(NULL, NULL);
  else
    activation_state = ACTIVATED;

  /* Make another connection to the X server so that there won't be a
   * period where there are no connections to the server, causing it
   * to reset (as when a config_console is done and console is the
   * only X program running). I.e., we don't close this connection,
   * and the X socket is inherited by Xsession.
   */
  dpy1 = XOpenDisplay(DisplayString(dpy));
  
  if ((i = XConnectionNumber(dpy1)) >= 0) {
	  fcntl(i, F_SETFD, 0);
  }

  setenv("PATH", defaultpath, 1);
#ifdef SOLARIS
   sprintf(buf,"hosttype=%s","sun4");
   putenv(buf);
#endif
  /* create shells to blank out all other screens, if any... */
  num_screens = 1;		/* cover ourselves by setting number of */
				/* screens to one, and */
  savershell[0] = saver;	/* fill in the saver shell for now... */

  if (resources.blankAll  &&  ScreenCount(dpy) > 1)
    {
      int this_screen = XScreenNumberOfScreen(XtScreen(saver));
      char *orig_dpy, *ptr;
      Cardinal zero = (Cardinal) 0;

      num_screens = MIN(ScreenCount(dpy), 10);
      orig_dpy = XtNewString(DisplayString(dpy));

      if (orig_dpy != NULL)	/* if the alloc worked, continue... */
	{			/* we are counting on displaystring to */
				/* always contain a period...  the */
				/* displaystring is always canonicalized */
				/* for us...  isn't that nice of them? */
	  if ((ptr = rindex(orig_dpy, '.')) != NULL)
	    {
	      ptr++;

	      for (i=0; i < ScreenCount(dpy) && i < 10; i++)
		{		/* only does screens 0 thru 9... if you */
				/* have more screens than that, you */
				/* lose. */
		  if (i == this_screen)
		    savershell[i] = saver;
		  else
		    {
		      Widget root;

		      *ptr = (char) i + '0';
		      dpy1 = XtOpenDisplay(app, orig_dpy, "xlogin", "Xlogin",
					   NULL, 0, &zero, NULL);
		      root = XtAppCreateShell("xlogin", "Xlogin",
					      applicationShellWidgetClass,
					      dpy1, NULL, 0);
		      savershell[i] = XtCreatePopupShell("savershell",
						 transientShellWidgetClass,
						 root, NULL, 0);
		      XtAddEventHandler(savershell[i], MASK,
					FALSE, unsave, TRUE);
		    }
		}
	    }
	}
      XtFree(orig_dpy);
    }


  /* tell display manager we're ready, just like X server handshake */
  if (signal(SIGUSR1, SIG_IGN) == SIG_IGN)
    kill(getppid(), SIGUSR1);

  XtMainLoop ( );
}

static Dimension x_max = 0, y_max = 0;

static void
move_instructions(data, timerid)
     XtPointer  data;
     XtIntervalId  *timerid;
{
  Position x, y;
  Window wins[2];

  if (!x_max)			/* get sizes, if we haven't done so already */
    {
      Arg args[2];

      XtSetArg(args[0], XtNwidth, &x_max);
      XtSetArg(args[1], XtNheight, &y_max);
      XtGetValues(ins, args, 2);

      if (WidthOfScreen(XtScreen(ins)) < x_max + 1)
	x_max = 1;
      else
	x_max = WidthOfScreen(XtScreen(ins)) - x_max;
      if (HeightOfScreen(XtScreen(ins)) < y_max + 1)
	y_max = 1;
      else
	y_max = HeightOfScreen(XtScreen(ins)) - y_max;

    }

  x = random() % x_max;
  y = random() % y_max;
  XtMoveWidget(ins, x, y);
  if (activation_state != REACTIVATING) {
      XRaiseWindow(XtDisplay(ins), XtWindow(ins));
      wins[0] = XtWindow(ins);
      wins[1] = XtWindow(saver);
      XRestackWindows(XtDisplay(ins), wins, 2);
  }

  curr_timerid = XtAddTimeOut(resources.move_timeout * 1000,
			      move_instructions, NULL);
}

static void
start_reactivate(data, timerid)
     XtPointer  data;
     XtIntervalId  *timerid;
{
    int in_use = 0;
    int file;
    struct utmp utmp;
    struct timeval now;

#ifdef SOLARIS
    gettimeofday(&now);
#else
    gettimeofday(&now, NULL);
#endif
    if (now.tv_sec - starttime.tv_sec > resources.restart_timeout) {
	fprintf(stderr, "Restarting X Server\n");
	exit(0);
    }

    do_motd();

    if ((file = open(UTMPF, O_RDONLY, 0)) >= 0) {
	while (read(file, (char *) &utmp, sizeof(utmp)) > 0) {
	    if (utmp.ut_name[0] != 0
#if defined(_AIX)
		&& utmp.ut_type == USER_PROCESS
#endif
		) {
		in_use = 1;
		break;
	    }
	}
	close(file);
    }

    if (in_use ||
	activation_state == REACTIVATING) {
	react_timerid = XtAddTimeOut(resources.reactivate_timeout * 1000,
				     start_reactivate, NULL);
	return;
    }

    /* clear console */
    sigconsCB(NULL, "clear", NULL);

    activation_state = REACTIVATING;
    activation_pid = fork();
    switch (activation_pid) {
    case 0:
 	if (activate_count % resources.detach_interval == 0)
 	  execl(resources.reactivate_prog, resources.reactivate_prog,
 		"-detach", 0);
 	else
 	  execl(resources.reactivate_prog, resources.reactivate_prog, 0);
	fprintf(stderr, "XLogin: unable to exec reactivate program \"%s\"\n",
		resources.reactivate_prog);
	_exit(1);
    case -1:
	fprintf(stderr, "XLogin: unable to fork for reactivatation\n");
	activation_state = ACTIVATED;
	break;
    default:
	break;
    }

    activate_count++;
    react_timerid = XtAddTimeOut(resources.reactivate_timeout * 1000,
				 start_reactivate, NULL);
}


void
idleReset()
{
    if (curr_timerid)
      XtRemoveTimeOut(curr_timerid);
    curr_timerid = XtAddTimeOut(resources.save_timeout * 1000,
			      screensave, NULL);
}


static void stop_activate(data, timerid)
     XtPointer  data;
     XtIntervalId  *timerid;
{
    if (activation_state == ACTIVATED) return;

    kill(activation_pid, SIGKILL);
    fprintf(stderr, "Workstation activation failed to finish normally.\n");
    activation_state = ACTIVATED;
}


static void
screensave(data, timerid)
     XtPointer  data;
     XtIntervalId  *timerid;
{
  static int first_time = TRUE;
  Pixmap pixmap;
  Cursor cursor;
  XColor c;
  int i;

  XtPopdown(WcFullNameToWidget(appShell, "*getSessionShell"));
  XtPopdown(WcFullNameToWidget(appShell, "*warningShell"));
  XtPopdown(WcFullNameToWidget(appShell, "*queryShell"));

  for (i=0; i < num_screens; i++)
    XtPopup(savershell[i], XtGrabNone);

  do_motd();
  XtPopup(ins, XtGrabNone);
  XRaiseWindow(XtDisplay(ins), XtWindow(ins));
  unfocusACT(appShell, NULL, NULL, NULL);
  if (first_time)
    {
      /*
       *  Contortions to "get rid of" cursor on screensaver windows.
       */
      c.pixel = BlackPixel(dpy, DefaultScreen(dpy));
      XQueryColor(dpy, DefaultColormap(dpy, DefaultScreen(dpy)), &c);

      pixmap = XCreateBitmapFromData(dpy, XtWindow(appShell), "", 1, 1);

      cursor = XCreatePixmapCursor(dpy, pixmap, pixmap, &c, &c,
				   (unsigned int) 0, (unsigned int) 0);
      XFreePixmap(dpy, pixmap);
      for (i=0; i < num_screens; i++)
	XDefineCursor(dpy, XtWindow(savershell[i]), cursor);
      XDefineCursor(dpy, XtWindow(ins), cursor);
      XFreeCursor(dpy, cursor);

      first_time = FALSE;
    }

  if (blink_timerid != 0)	/* don't blink while screensaved... */
    XtRemoveTimeOut(blink_timerid);
  blink_timerid = 0;

  /* don't let the real screensaver kick in */
  XSetScreenSaver(dpy, 0, -1, DefaultBlanking, DefaultExposures);
  curr_timerid = XtAddTimeOut(resources.move_timeout * 1000,
			      move_instructions, NULL);
  react_timerid = XtAddTimeOut(resources.reactivate_timeout * 1000,
			       start_reactivate, NULL);
}


/* Check the motd file and update the contents of the widget if necessary */

do_motd()
{
    static Widget motdtext = NULL;
    static time_t modtime = 0, modtime2 = 0;
    struct stat stbuf, stbuf2;
    Arg args[1];
    char buf[10000], *temp, *s, *d;
    int fid, len, do_g_motd, do_l_motd;

    if (!motdtext) {
	motdtext = WcFullNameToWidget(appShell, "*motd");

	/* Initialize motdtext to NULL in case it never gets set.
	   This happens in the case of a bad stat on the motd
	   file, or showMotd false. */
	buf[0] = '\0';
	XtSetArg(args[0], XtNlabel, buf);
	XtSetValues(motdtext, args, 1);
    }

    if (resources.showMotd) {
        do_g_motd = (resources.motdFile != NULL && *resources.motdFile &&
		   !stat(resources.motdFile, &stbuf) && 
		   stbuf.st_mtime != modtime);
	if (do_g_motd)
	    modtime = stbuf.st_mtime;

	do_l_motd = (resources.motd2File != NULL && *resources.motd2File &&
		       !stat(resources.motd2File, &stbuf2) &&
		       stbuf2.st_mtime != modtime2);
	if (do_l_motd)
	        modtime2 = stbuf2.st_mtime;

	if (do_g_motd || do_l_motd) {
	    /* read the new motd */
	    len = 0;
	    if (resources.motdFile != NULL && *resources.motdFile &&
		(fid = open(resources.motdFile, O_RDONLY)) >= 0) {
		len = read(fid, buf, sizeof(buf));
		close(fid);
	    }
	    if (resources.motd2File != NULL && *resources.motd2File &&
		(fid = open(resources.motd2File, O_RDONLY)) >= 0) {
		len += read(fid, &(buf[len]), sizeof(buf) - len);
		close(fid);
	    }
	    buf[len] = 0;

	    /* de-tabbify the motd (label widgets don't do tabs) */
	    for (s = buf; *s; s++)
	      if (*s == '\t') len += 7;
	    d = temp = malloc(len+1);
	    len = 0;
	    for (s = buf; *s; s++) {
		switch (*s) {
		case '\t':
		    *d++ = ' ';
		    len++;
		    while (len++ % 8 != 0)
		      *d++ = ' ';
		    len--;
		    break;
		case '\n':
		    len = 0;
		    *d++ = *s;
		    break;
		default:
		    *d++ = *s;
		    len++;
		}
	    }
	    *d = 0;

	    /* now set the text */
	    XtSetArg(args[0], XtNlabel, temp);
	    XtSetValues(motdtext, args, 1);
	    free(temp);

	    /* force move_instructions() to recompute size */
	    x_max = 0;
	}
    }
}


static void
unsave(w, popdown, event, bool)
     Widget w;
     int popdown;
     XEvent *event;
     Boolean *bool;
{
  /* hide console */
  sigconsCB(NULL, "hide", NULL);

  if (popdown)
    {
      int i;

      XtPopdown(ins);
      for (i=0; i < num_screens; i++)
	XtPopdown(savershell[i]);
    }

  /* enable the real screensaver */
  XSetScreenSaver(dpy, -1, -1, DefaultBlanking, DefaultExposures);
  resetCB(w, NULL, NULL);

  if (curr_timerid != 0)
    XtRemoveTimeOut(curr_timerid);
  curr_timerid = XtAddTimeOut(resources.save_timeout * 1000,
			      screensave, NULL);
  blink_timerid = XtAddTimeOut(random() % (10 * 1000),
			       blinkOwl, NULL);
  if (react_timerid != 0)
    XtRemoveTimeOut(react_timerid);
  if (activation_state == REACTIVATING)
    react_timerid = XtAddTimeOut(resources.activate_timeout * 1000,
				 stop_activate, NULL);
}


void loginACT(w, event, p, n)
Widget w;
XEvent *event;
String *p;
Cardinal *n;
{
    Arg args[2];
    char *login, *passwd, *script;
    int mode = 1;
    Pixmap bm1, bm2, bm3, bm4, bm5;
    XawTextBlock tb;
    extern char *dologin();
    XEvent e;

    if (curr_timerid)
      XtRemoveTimeOut(curr_timerid);

    XtSetArg(args[0], XtNstring, &login);
    XtGetValues(WcFullNameToWidget(appShell, "*name_input"), args, 1);
    XtSetArg(args[0], XtNstring, &passwd);
    XtGetValues(WcFullNameToWidget(appShell, "*pword_input"), args, 1);


    XtSetArg(args[0], XtNleftBitmap, &bm1);
    XtGetValues(WcFullNameToWidget(appShell, "*lmenuEntry1"), args, 1);
    XtSetArg(args[0], XtNleftBitmap, &bm2);
    XtGetValues(WcFullNameToWidget(appShell, "*lmenuEntry2"), args, 1);
    XtSetArg(args[0], XtNleftBitmap, &bm3);
    XtGetValues(WcFullNameToWidget(appShell, "*lmenuEntry3"), args, 1);
    XtSetArg(args[0], XtNleftBitmap, &bm4);
    XtGetValues(WcFullNameToWidget(appShell, "*lmenuEntry4"), args, 1);
    XtSetArg(args[0], XtNleftBitmap, &bm5);
    XtGetValues(WcFullNameToWidget(appShell, "*lmenuEntry5"), args, 1);

    /* determine which option was selected by seeing which 4 of the 5 match */
    if (bm1 == bm2 && bm1 == bm3 && bm1 == bm4)
      mode = 5;
    if (bm1 == bm2 && bm1 == bm3 && bm1 == bm5)
      mode = 4;
    if (bm1 == bm2 && bm1 == bm4 && bm1 == bm5)
      mode = 3;
    if (bm1 == bm3 && bm1 == bm4 && bm1 == bm5)
      mode = 2;
    if (bm2 == bm3 && bm2 == bm4 && bm2 == bm5)
      mode = 1;

    XtSetArg(args[0], XtNstring, &script);
    XtGetValues(WcFullNameToWidget(appShell, "*getsession*value"), args, 1);
    unfocusACT(appShell, NULL, NULL, NULL);
    XtUnmapWidget(appShell);
    /* To clear the cut buffer in case someone types ^U while typing
     * their password. */
    XDeleteProperty(dpy, DefaultRootWindow(dpy), XA_CUT_BUFFER0);
    XDeleteProperty(dpy, DefaultRootWindow(dpy), XA_CUT_BUFFER1);
    XFlush(dpy);

    /* wait for activation to finish.  We play games with signals here
     * because we are not waiting within the XtMainloop for it to handle
     * the timers.
     */
    if (activation_state != ACTIVATED) {
#if (defined POSIX || defined sun)
	void (*oldsig)();
#else
	int (*oldsig)();
#endif

	fprintf(stderr, "Waiting for workstation to finish activating...");
	fflush(stderr);
	oldsig = signal(SIGALRM, stop_activate);
	alarm(resources.activate_timeout);
	while (activation_state != ACTIVATED)
	  sigpause(0);
	signal(SIGALRM, oldsig);
	fprintf(stderr, "done.\n");
    }

    if (access(resources.srvdcheck, F_OK) != 0)
      tb.ptr = "Workstation failed to activate successfully.  Please notify Athena operations.";
    else {
	setAutoRepeat(autorep);
 	setFontPath();
	XWarpPointer(dpy, None, RootWindow(dpy, DefaultScreen(dpy)),
		     0, 0, 0, 0, 300, 300);
 	XFlush(dpy);
	tb.ptr = dologin(login, passwd, mode, script, resources.tty,
			 resources.session, DisplayString(dpy));
	XWarpPointer(dpy, None, RootWindow(dpy, DefaultScreen(dpy)),
		     0, 0, 0, 0, WidthOfScreen(DefaultScreenOfDisplay(dpy))/2,
		     HeightOfScreen(DefaultScreenOfDisplay(dpy))/2);
	setAutoRepeat(AutoRepeatModeOff);
    }
    XtMapWidget(appShell);
    XtPopup(WcFullNameToWidget(appShell, "*warningShell"), XtGrabExclusive);
    tb.firstPos = 0;
    tb.length = strlen(tb.ptr);
    tb.format = FMT8BIT;
    XawTextReplace(WcFullNameToWidget(appShell, "*warning*value"),
		   0, 65536, &tb);
    XtCallActionProc(WcFullNameToWidget(appShell, "*warning*value"),
		     "form-paragraph", &e, NULL, 0);
    focusCB(appShell, "*warning*value", NULL);
    curr_timerid = XtAddTimeOut(resources.save_timeout * 1000,
				screensave, NULL);
}


/* login failed: Set the exit flag, then return the message the usual way. */

char *lose(msg)
char *msg;
{
    exiting = TRUE;
    return(msg);
}


void focusACT(w, event, p, n)
Widget w;
XEvent *event;
String *p;
Cardinal *n;
{
    Widget target;

#if defined(_AIX) && defined(_IBMR2)
    static int done_once = 0;

    /* This crock works around the an invalid argument error on the
     * XSetInputFocus() call below the very first time it is called,
     * only when running on the RIOS.  We still don't know just what
     * causes it.
     */
    if (done_once == 0) {
	done_once++;
	XSync(dpy, FALSE);
	sleep(1);
	XSync(dpy, FALSE);
    }
#endif

    target = WcFullNameToWidget(appShell, p[0]);
    XSetInputFocus(dpy, XtWindow(target), RevertToPointerRoot, CurrentTime);
}


void focusCB(w, s, unused)
Widget w;
char *s;
caddr_t unused;
{
    Cardinal one = 1;

    focusACT(w, NULL, &s, &one);
}


void unfocusACT(w, event, p, n)
Widget w;
XEvent *event;
String *p;
Cardinal *n;
{
    int rvt;
    Window win;

    XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
    XGetInputFocus(dpy, &win, &rvt);
}


void setcorrectfocus(w, event, p, n)
Widget w;
XEvent *event;
String *p;
Cardinal *n;
{
    Arg args[2];
    char *win;
    Cardinal i;
    Boolean bool;

    XtSetArg(args[0], XtNdisplayCaret, &bool);
    XtGetValues(WcFullNameToWidget(appShell, "*name_input"), args, 1);
    if (bool)
      win = "*name_input";
    else
      win = "*pword_input";
    i = 1;
    focusACT(w, event, &win, &i);
}


void runACT(w, event, p, n)
Widget w;
XEvent *event;
char **p;
Cardinal *n;
{
    char **argv;
    int i;

    unfocusACT(w, event, p, n);
    argv = (char **)malloc(sizeof(char *) * (*n + 3));
    argv[0] = "sh";
    argv[1] = "-c";
    for (i = 0; i < *n; i++)
      argv[i+2] = p[i];
    argv[i+2] = NULL;

    /* wait for activation to finish */
    if (activation_state != ACTIVATED)
      fprintf(stderr, "Waiting for workstation to finish activating...\n");
    while (activation_state != ACTIVATED)
      sigpause(0);
    if (access(resources.srvdcheck, F_OK) != 0) {
	fprintf(stderr, "Workstation failed to activate successfully.  Please notify Athena operations.");
	return;
    }
    sigconsCB(NULL, "hide", NULL);
    setFontPath();
    setAutoRepeat(autorep);
    XFlush(dpy);
    XtCloseDisplay(dpy);

    unsetenv("XAPPLRESDIR");
    unsetenv("XENVIRONMENT");

    setenv("PATH", defaultpath, 1);
    setenv("USER", "daemon", 1);
    setenv("SHELL", "/bin/sh", 1);
    setenv("DISPLAY", ":0", 1);
    setgroups(sizeof(def_grplist)/sizeof(gid_t), def_grplist);

#if defined(_AIX) && defined(_IBMR2)
    setuidx(ID_LOGIN, DAEMON);
#endif
    setgid(def_grplist[0]);
    if (setuid(DAEMON)) {
	fprintf(stderr, "Unable to set user id.\n");
	return;
    }
    execv("/bin/sh", argv);
    fprintf(stderr, "XLogin: unable to exec /bin/sh\n");
    _exit(3);
}


void runCB(w, s, unused)
Widget w;
char *s;
caddr_t unused;
{
    Cardinal i = 1;

    runACT(w, NULL, &s, &i);
}


void attachandrunCB(w, s, unused)
Widget w;
char *s;
caddr_t unused;
{
    char *cmd, locker[256];
    Cardinal i = 1;

    cmd = index(s, ',');
    if (cmd == NULL) {
	fprintf(stderr,
		"Xlogin warning: need two arguments in AttachAndRun(%s)\n",
		s);
	return;
    }
    strncpy(locker, s, cmd - s);
    locker[cmd - s] = 0;
    cmd++;


    attach_state = -1;
    switch (attach_pid = fork()) {
    case 0:
	execlp("attach", "attach", "-n", "-h", "-q", locker, NULL);
	fprintf(stderr, "Xlogin warning: unable to attach locker %s\n", locker);
	_exit(1);
    case -1:
	fprintf(stderr, "Xlogin: unable to fork to attach locker\n");
	break;
    default:
	while (attach_state == -1)
	  sigpause(0);
	if (attach_state != 0) {
	    fprintf(stderr, "Unable to attach locker %s, aborting...\n",
		    locker);
	    return;
	}
    }
    
    runACT(w, NULL, &cmd, &i);
}


void sigconsACT(w, event, p, n)
Widget w;
XEvent *event;
char **p;
Cardinal *n;
{
    int sig, pid;
    FILE *f;
    char buf[BUFSIZ];

    if (!strcmp(p[0], "clear"))
      sig = SIGFPE;
    else if (!strcmp(p[0], "hide"))
      sig = SIGUSR2;
    else if (!strcmp(p[0], "show"))
      sig = SIGUSR1;
    else if (!strcmp(p[0], "config"))
      sig = SIGHUP;
    else
      sig = atoi(p[0]);

    f = fopen(CONSOLEPID, "r");
    if (f) {
	fgets(buf, sizeof(buf), f);
	pid = atoi(buf);
	if (pid)
	  kill(pid, sig);
	fclose(f);
    }
}


void sigconsCB(w, s, unused)
Widget w;
char *s;
caddr_t unused;
{
    Cardinal i = 1;

    sigconsACT(w, NULL, &s, &i);
}


void callbackACT(w, event, p, n)
Widget w;
XEvent *event;
char **p;
Cardinal *n;
{
    w = WcFullNameToWidget(appShell, p[0]);
    XtCallCallbacks(w, "callback", p[1]);
}


void resetCB(w, s, unused)
Widget w;
char *s;
caddr_t unused;
{
    XawTextBlock tb;
    Widget name_input;

    if (exiting == TRUE)
      exit(0);
    focusCB(appShell, "*name_input", NULL);
    WcSetValueCB(appShell, "*lmenuEntry1.leftBitmap: check", NULL);
    WcSetValueCB(appShell, "*lmenuEntry2.leftBitmap: white", NULL);
    WcSetValueCB(appShell, "*lmenuEntry3.leftBitmap: white", NULL);
    WcSetValueCB(appShell, "*lmenuEntry4.leftBitmap: white", NULL);
    WcSetValueCB(appShell, "*lmenuEntry5.leftBitmap: white", NULL);
    WcSetValueCB(appShell, "*selection.label:  ", NULL);
    WcSetValueCB(appShell, "*name_input.displayCaret: TRUE", NULL);
    WcSetValueCB(appShell, "*name_input.borderColor: black", NULL);
    WcSetValueCB(appShell, "*pword_input.borderColor: white", NULL);

    tb.firstPos = tb.length = 0;
    tb.ptr = "";
    tb.format = FMT8BIT;
    XawTextReplace(WcFullNameToWidget(appShell, "*pword_input"), 0, 65536, &tb);
    XawTextReplace(WcFullNameToWidget(appShell, "*getsession*value"),
		   0, 65536, &tb);

    if (resources.loginName != NULL  &&  strlen(resources.loginName) != 0)
      {
	if (strlen(resources.loginName) > 8)
	  resources.loginName[8] = '\0';
	tb.ptr = resources.loginName;
	tb.length = strlen(tb.ptr);
      }
    name_input = WcFullNameToWidget(appShell, "*name_input");
    XawTextReplace(name_input, 0, 65536, &tb);
    XawTextSetInsertionPoint(name_input, (XawTextPosition) tb.length);

    if (curr_timerid)
      XtRemoveTimeOut(curr_timerid);
    curr_timerid = XtAddTimeOut(resources.save_timeout * 1000,
			      screensave, NULL);
}


setvalue(w, done, unused)
Widget w;
int *done;
{
    *done = 1;
}


prompt_user(msg, abort_proc)
char *msg;
void (*abort_proc)();
{
    XawTextBlock tb;
    XEvent e;
    static void (*oldcallback)() = NULL;
    static int done;

    XtPopup(WcFullNameToWidget(appShell, "*queryShell"), XtGrabExclusive);
    tb.firstPos = 0;
    tb.ptr = msg;
    tb.length = strlen(msg);
    tb.format = FMT8BIT;
    XawTextReplace(WcFullNameToWidget(appShell, "*query*value"),
		   0, 65536, &tb);
    XtCallActionProc(WcFullNameToWidget(appShell, "*query*value"),
		     "form-paragraph", &e, NULL, 0);
    focusCB(appShell, "*query*value", NULL);
    if (oldcallback)
      XtRemoveCallback(WcFullNameToWidget(appShell, "*query*giveup"),
		       XtNcallback, oldcallback, NULL);
    else
      XtAddCallback(WcFullNameToWidget(appShell, "*query*cont"),
		    XtNcallback, setvalue, &done);
    XtAddCallback(WcFullNameToWidget(appShell, "*query*giveup"),
		  XtNcallback, abort_proc, NULL);
    oldcallback = abort_proc;
    curr_timerid = XtAddTimeOut(resources.save_timeout * 1000,
				abort_proc, NULL);

    /* repeat main_loop here so we can check status & return */
    done = 0;
    while (!done) {
	XtAppNextEvent(_XtDefaultAppContext(), &e);
	XtDispatchEvent(&e);
    }

    XtRemoveTimeOut(curr_timerid);
    curr_timerid = 0;
    XtPopdown(WcFullNameToWidget(appShell, "*queryShell"));
    XFlush(dpy);
}



#define updateOwl()	XCopyPlane(dpy, owlBitmaps[owlCurBitmap], \
				   owlWindow, owlGC, 0, 0, \
				   owlWidth, owlHeight, 0, 0, 1)
#define updateIs()	XCopyPlane(dpy, isBitmaps[isCurBitmap], \
				   isWindow, isGC, 0, 0, \
				   isWidth, isHeight, 0, 0, 1)

static void
blinkOwl(data, intervalid)
     XtPointer data;
     XtIntervalId *intervalid;
{
  static int owlCurBitmap, isCurBitmap;
  owlTimeout = 0;

  if (owlNumBitmaps == 0) return;

  switch(owlDelta)
    {
    case OWL_BLINKINGCLOSED:	/* your eyelids are getting heavy... */
      owlCurBitmap++;
      isCurBitmap++;
      updateOwl();
      updateIs();
      if (owlCurBitmap == owlNumBitmaps - 1)
	owlDelta = OWL_BLINKINGOPEN;
      break;

    case OWL_BLINKINGOPEN:	/* you will awake, feeling refreshed... */
      owlCurBitmap--;
      isCurBitmap--;
      updateOwl();
      updateIs();
      if (owlCurBitmap == ((owlState == OWL_SLEEPY) * (owlNumBitmaps) / 2))
	{
	  owlTimeout = random() % (10 * 1000);
	  owlDelta = OWL_BLINKINGCLOSED;
	}
      break;

    case OWL_SLEEPING:		/* transition to sleeping state */
      owlCurBitmap++;
      isCurBitmap++;
      updateOwl();
      updateIs();
      if (owlCurBitmap == ((owlState == OWL_SLEEPY) * (owlNumBitmaps) / 2))
	{
	  owlState = OWL_SLEEPY;
	  owlDelta = OWL_STATIC;
	}
      break;

    case OWL_WAKING:		/* transition to waking state */
      owlCurBitmap--;
      isCurBitmap--;
      updateOwl();
      updateIs();
      if (owlCurBitmap == 0)
	{
	  owlState = OWL_AWAKE;
	  owlDelta = OWL_STATIC;
	}
      break;

    case OWL_STATIC:
      break;
    }

  blink_timerid = XtAddTimeOut((owlTimeout
				? owlTimeout : resources.blink_timeout),
			       blinkOwl, NULL);
}

static void initOwl(search)
     Widget search;
{
  Widget owl, is;
  Arg args[3];
  int n, done;
  char *filenames, *ptr;
  XGCValues values;
  XtGCMask valuemask;

  owl = WcFullNameToWidget(search, "*eyes");

  if (owl != NULL)
    {
      owlWindow = XtWindow(owl);
      if (owlWindow != None)
	{
	  n = 0;
	  done = 0;
	  XtSetArg(args[n], XtNlabel, &filenames); n++;
	  XtSetArg(args[n], XtNforeground, &values.foreground); n++;
	  XtSetArg(args[n], XtNbackground, &values.background); n++;
	  XtGetValues(owl, args, n);

	  values.function = GXcopy;
	  valuemask = GCForeground | GCBackground | GCFunction;

	  owlNumBitmaps = 0;
	  ptr = filenames;
	  while (ptr != NULL && !done)
	    {
	      while (*ptr != '\0' && !isspace(*ptr))
		ptr++;

	      if (*ptr == '\0')
		done = 1;
	      else
		*ptr = '\0';

	      owlBitmaps[owlNumBitmaps] = XmuLocateBitmapFile(XtScreen(owl),
							      filenames,
							      NULL, 0,
							      &owlWidth,
							      &owlHeight,
							      NULL, NULL);
	      if (owlBitmaps[owlNumBitmaps] == None)
		return; /* abort */
#ifdef notdef
	      if (BitmapSuccess != XReadBitmapFile(dpy, owlWindow,
						   filenames,
						   &owlWidth, &owlHeight,
						   &owlBitmaps[owlNumBitmaps],
						   &scratch, &scratch))
		return; /* abort */
#endif
	      owlNumBitmaps++;
	      if (!done)
		{
		  *ptr = ' ';
		  while (isspace(*ptr))
		    ptr++;
		}
	      filenames = ptr;
	    }

	  owlGC = XtGetGC(owl, valuemask, &values);
	  owlState = OWL_AWAKE;
	  owlDelta = OWL_BLINKINGCLOSED;
	}
    }


  is = WcFullNameToWidget(search, "*logo2");

  if (is != NULL)
    {
      isWindow = XtWindow(is);
      if (isWindow != None)
	{
	  n = 0;
	  done = 0;
	  XtSetArg(args[n], XtNlabel, &filenames); n++;
	  XtSetArg(args[n], XtNforeground, &values.foreground); n++;
	  XtSetArg(args[n], XtNbackground, &values.background); n++;
	  XtGetValues(is, args, n);

	  values.function = GXcopy;
	  valuemask = GCForeground | GCBackground | GCFunction;

	  isNumBitmaps = 0;
	  ptr = filenames;
	  while (ptr != NULL && !done)
	    {
	      while (*ptr != '\0' && !isspace(*ptr))
		ptr++;

	      if (*ptr == '\0')
		done = 1;
	      else
		*ptr = '\0';

	      isBitmaps[isNumBitmaps] = XmuLocateBitmapFile(XtScreen(is),
							      filenames,
							      NULL, 0,
							      &isWidth,
							      &isHeight,
							      NULL, NULL);
	      if (isBitmaps[isNumBitmaps] == None)
		return; /* abort */
#ifdef notdef
	      if (BitmapSuccess != XReadBitmapFile(dpy, isWindow,
						   filenames,
						   &isWidth, &isHeight,
						   &isBitmaps[isNumBitmaps],
						   &scratch, &scratch))
		return; /* abort */
#endif
	      isNumBitmaps++;
	      if (!done)
		{
		  *ptr = ' ';
		  while (isspace(*ptr))
		    ptr++;
		}
	      filenames = ptr;
	    }

	  isGC = XtGetGC(is, valuemask, &values);
	}
    }
  if (isNumBitmaps != owlNumBitmaps)
    fprintf(stderr, "number of owl bitmaps differs from number of IS bitmaps (%d != %d)\n", owlNumBitmaps, isNumBitmaps);
}

static short conditions[] =
{
  /*51,   82,  114,  144,  176,  206,  238,  269,*/299,  331,  362,  393,
   552,  582,  616,  646,  677,  708,  739,  770,  799,  830,  862,  893,  924,
  1083, 1113, 1147, 1177, 1208, 1239, 1270, 1301, 1331, 1363, 1394, 1425,
  1584, 1615, 1648, 1679, 1710, 1740, 1772, 1802, 1832, 1864, 1895, 1926,
  2085, 2116, 2149, 2179, 2211, 2241, 2270, 2302, 2332, 2362, 2394, 2424, 2456,
  2615, 2646, 2679, 2710, 2742, 2772, 2803, 2834, 2864, 2895, 2926, 2957,
  3116, 3147, 3180, 3211, 3243, 3273, 3305, 3335, 3366, 3397, 3427, 3459,
  3617, 3647, 3682, 3711, 3742, 3774, 3804, 3836, 3866, 3897, 3928, 3959, 3990
};

static Boolean conditionsMet()
{
  time_t t;
  struct tm *now;
  short test;
  int i;

  t = time(0);
  now = localtime(&t);
  test = (now->tm_year - 92) * 512 + (now->tm_mon + 1) * 32 + now->tm_mday;

  i = 0;
  while ((i < sizeof(conditions)) && (test > conditions[i]))
    i++;

  if ((test == conditions[i]) && (now->tm_hour >= 18))
    return True;

  return False;
}

static void adjustOwl(search)
     Widget search;
{
  Widget version, logo, eyes, Slogo, Sversion;
  XtWidgetGeometry logoGeom, eyesGeom, versionGeom;
  Pixmap owlPix;
  Arg args[2];
  int newx;

  if (conditionsMet())
    {
      /* Look up important widgets */
      logo = WcFullNameToWidget(search, "*login*logo");
      eyes = WcFullNameToWidget(search, "*eyes");
      version = WcFullNameToWidget(search, "*login*version");

      Slogo = WcFullNameToWidget(search, "*hitanykey*logo");
      Sversion = WcFullNameToWidget(search, "*hitanykey*version");

      /* Plug in the owl bitmap */
      owlPix = XCreatePixmapFromBitmapData(dpy, DefaultRootWindow(dpy),
					   owl_bits, owl_width, owl_height,
					   1, 0, 1);
      XtSetArg(args[0], XtNbitmap, owlPix);
      XtSetValues(logo, args, 1);

      XtSetValues(Slogo, args, 1);

      /* Adjust eyes, version */
      XtQueryGeometry(logo, NULL, &logoGeom);
      XtQueryGeometry(eyes, NULL, &eyesGeom);
      XtQueryGeometry(version, NULL, &versionGeom);

      XtMoveWidget(eyes, (logoGeom.width -
			  ((eyesGeom.x - logoGeom.x) + eyesGeom.width)) +
		         logoGeom.x,
		   eyesGeom.y);

      newx = (logoGeom.width - ((versionGeom.x - logoGeom.x) +
				versionGeom.width)) + logoGeom.x;
      XtMoveWidget(version, newx, versionGeom.y);

      /* Depends on the fact that both logos have the same geometry. */
      XtSetArg(args[0], XtNhorizDistance, newx);
      XtSetValues(Sversion, args, 1);
    }
}

/* Called from within the toolkit */
void localErrorHandler(s)
String s;
{
    fprintf(stderr, "XLogin X error: %s\n", s);
    cleanup(NULL);
    exit(1);
}


static void catch_child()
{
    int pid;
#ifndef SOLARIS
    union wait status;
#else
    int status;
#endif
    char *number();

    /* Necessary on the rios- it sets the signal handler to SIG_DFL */
    /* during the execution of a signal handler */
    signal(SIGCHLD,catch_child);
#ifdef SOLARIS
    pid = waitpid(-1, &status, WNOHANG);
#else
    pid = wait3(&status, WNOHANG, 0);
#endif
    if (pid == activation_pid) {
	switch (activation_state) {
	case REACTIVATING:
	    if (pid == activation_pid)
	      activation_state = ACTIVATED;
	    break;
	case ACTIVATED:
	default:
	    fprintf(stderr, "XLogin: child %d exited\n", pid);
	}
    } else if (pid == attach_pid) {
#ifdef SOLARIS
	attach_state = WEXITSTATUS(status);
#else
	attach_state = status.w_retcode;
#endif
    } else if (pid == attachhelp_pid) {
#ifdef SOLARIS
	attachhelp_state =  WEXITSTATUS(status);
#else
	attachhelp_state =  status.w_retcode;
#endif
    } else if (pid == quota_pid) {
	quota_pid = 0;
    } else
#ifdef SOLARIS
      fprintf(stderr, "XLogin: child %d exited with status %d\n",
	      pid, WEXITSTATUS(status));
#else
      fprintf(stderr, "XLogin: child %d exited with status %d\n",
	      pid, status.w_retcode);
#endif
}


char *strdup(string)
char *string;
{
    register char *cp;

    if (!(cp = malloc(strlen(string) + 1)))
      return(NULL);
    return(strcpy(cp,string));
}


static void setFontPath()
{
    static int ndirs = 0;
    static char **dirlist;
    register char *cp;
    register int i=0;
    char *dirs;

    if (!ndirs) {
 	dirs = cp = strdup(resources.fontpath);
 	if (cp == NULL)
	  localErrorHandler("Out of memory");

 	ndirs = 1;
 	while (cp = index(cp, ',')) { ndirs++; cp++; }

 	cp = dirs;
 	dirlist = (char **)malloc(ndirs*sizeof(char *));
 	if (dirlist == NULL)
	  localErrorHandler("Out of memory");

 	dirlist[i++] = cp;
 	while (cp = index(cp, ',')) {
 	    *cp++ = '\0';
 	    dirlist[i++] = cp;
 	}
    }

    XSetFontPath(dpy, dirlist, ndirs);
}

static void setAutoRepeat(mode)
int mode;
{
#ifdef DISABLE_AUTO_REP
    XKeyboardControl cntrl;
    cntrl.auto_repeat_mode = mode;
    XChangeKeyboardControl(dpy,KBAutoRepeatMode,&cntrl);
#endif
    return;
}

#ifndef BROKEN_AUTO_REP
static int getAutoRepeat()
{
#ifdef DISABLE_AUTO_REP
    XKeyboardState st;
    XGetKeyboardControl(dpy, &st);
    return st.global_auto_repeat;
#else
    return 0;
#endif
}
#endif 

