/*
 * xscreensaver-button.c: Provide a simple button interface to xscreensaver
 * David Maze <dmaze@mit.edu>
 * MIT Athena Something-or-another
 */

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SmeBSB.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MENU_NAME "menu"

#ifndef BINDIR
#define BINDIR ""
#endif

Widget create_menu(Widget toplevel);
static Pixmap get_pixmap(Widget toplevel);
void stop_ss();
void die();
void split_and_exec(char *cmd, pid_t *pid);

void callback_activate(Widget w, XtPointer client_data, XtPointer call_data);
void callback_prefs(Widget w, XtPointer client_data, XtPointer call_data);
void callback_quit(Widget w, XtPointer client_data, XtPointer call_data);
void action_activate(Widget w, XEvent *event,
                     String *params, Cardinal *num_params);

void sig_chld();
void sig_int();
void sig_term();

typedef struct _AppData
{
  char *xscreensaver;
  char *activate_command;
  char *prefs_command;
  int xroger_width;
  int xroger_height;
  Boolean redirect;
  char *log_file;
} AppData;

#define XtNxscreensaver "xscreensaver"
#define XtNactivateCommand "activateCommand"
#define XtNprefsCommand "prefsCommand"
#define XtNxrogerWidth "xrogerWidth"
#define XtNxrogerHeight "xrogerHeight"
#define XtNredirect "redirect"
#define XtNlogFile "logFile"
#define XtCCommand "Command"
#define XtCRedirect "Redirect"
#define XtCLogFile "LogFile"

static XtResource resources[] =
{
  {
    XtNxscreensaver,
    XtCCommand,
    XtRString,
    sizeof(String),
    XtOffsetOf(AppData, xscreensaver),
    XtRString,
    BINDIR "xss -no-splash -no-start-locked"
  },
  {
    XtNactivateCommand,
    XtCCommand,
    XtRString,
    sizeof(String),
    XtOffsetOf(AppData, activate_command),
    XtRString,
    BINDIR "xss-command -lock"
  },
  {
    XtNprefsCommand,
    XtCCommand,
    XtRString,
    sizeof(String),
    XtOffsetOf(AppData, prefs_command),
    XtRString,
    BINDIR "xss-command -prefs"
  },
  {
    XtNxrogerWidth,
    XtCWidth,
    XtRInt,
    sizeof(int),
    XtOffsetOf(AppData, xroger_width),
    XtRImmediate,
    (XtPointer)48
  },
  {
    XtNxrogerHeight,
    XtCHeight,
    XtRInt,
    sizeof(int),
    XtOffsetOf(AppData, xroger_height),
    XtRImmediate,
    (XtPointer)48
  },
  {
    XtNredirect,
    XtCRedirect,
    XtRBoolean,
    sizeof(Boolean),
    XtOffsetOf(AppData, redirect),
    XtRImmediate,
    (XtPointer)False
  },
  {
    XtNlogFile,
    XtCLogFile,
    XtRString,
    sizeof(String),
    XtOffsetOf(AppData, log_file),
    XtRString,
    ".xss-log"
  },
};

static XrmOptionDescRec options[] =
{
  { "-xss",        "*xscreensaver",    XrmoptionSepArg, NULL             },
  { "-activate",   "*activateCommand", XrmoptionSepArg, NULL             },
  { "-prefs",      "*prefsCommand",    XrmoptionSepArg, NULL             },
  { "-logoWidth",  "*xrogerWidth",     XrmoptionSepArg, NULL             },
  { "-logoHeight", "*xrogerHeight",    XrmoptionSepArg, NULL             },
  { "-redirect",   "*redirect",        XrmoptionNoArg,  (XtPointer)True  },
  { "-noredirect", "*redirect",        XrmoptionNoArg,  (XtPointer)False },
  { "-logFile",    "*logFile",         XrmoptionSepArg, NULL             },
};

static XtActionsRec actions[] =
{
  { "activate", action_activate },
};

static char *fallbacks[] =
{
  "*translations: #override <Btn1Down>,<Btn1Up>: activate()",
  NULL
};

static AppData app_data;
static Widget toplevel;
static XtAppContext context;
static pid_t child;
static Pixmap pixmap;
static int fdRedirect;
static char *progname;

int main(int argc, char **argv)
{
  Widget button;
  struct sigaction sa;

  progname = strrchr(argv[0], '/');
  progname = (progname) ? progname + 1 : argv[0];

  /* Catch some signals. */
  sa.sa_handler = sig_chld;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGCHLD, &sa, NULL);
  
  sa.sa_handler = sig_int;
  sigaction(SIGINT, &sa, NULL);
  
  sa.sa_handler = sig_term;
  sigaction(SIGTERM, &sa, NULL);

  toplevel = XtVaAppInitialize(&context, "XScreenSaverButton",
			       options, XtNumber(options),
			       &argc, argv, fallbacks, NULL);

  XtAppAddActions(context, actions, XtNumber(actions));
  XtVaGetApplicationResources(toplevel, &app_data,
			      resources, XtNumber(resources), NULL);

  /* Open the log file, if necessary. */
  if (app_data.redirect)
  {
    fdRedirect = open(app_data.log_file, O_WRONLY | O_APPEND | O_CREAT, 0600);
    if (fdRedirect < 0)
    {
      app_data.redirect = False;
      perror(progname);
      fprintf(stderr, "%s: stdout/stderr redirect disabled\n", progname);
    }
  }
  
  split_and_exec(app_data.xscreensaver, &child);

  pixmap = get_pixmap(toplevel);
  button = XtVaCreateManagedWidget("xss", menuButtonWidgetClass, toplevel,
				   XtNmenuName, (XtArgVal)MENU_NAME,
				   XtNbitmap, (XtArgVal)pixmap,
				   NULL);

  create_menu(toplevel);

  XtRealizeWidget(toplevel);
  XtAppMainLoop(context);
  die();
}

static Pixmap get_pixmap(Widget toplevel)
{
  GC draw_gc, erase_gc;
  Pixmap pixmap;
  Display *dpy = XtDisplay(toplevel);
  XGCValues values;
  XtGCMask mask = GCForeground | GCBackground;

  pixmap = XCreatePixmap(dpy,
			 RootWindowOfScreen(XtScreen(toplevel)),
			 app_data.xroger_width, app_data.xroger_height, 1);
  
  values.foreground = 1;
  values.background = 0;
  draw_gc = XCreateGC(dpy, pixmap, mask, &values);
  
  values.foreground = 0;
  values.background = 1;
  erase_gc = XCreateGC(dpy, pixmap, mask, &values);

  XFillRectangle(dpy, pixmap, erase_gc, 0, 0,
		 app_data.xroger_width, app_data.xroger_height);
  skull(dpy, pixmap, draw_gc, erase_gc, 0, 0,
	app_data.xroger_width, app_data.xroger_height);
  
  XFreeGC(dpy, draw_gc);
  XFreeGC(dpy, erase_gc);
  
  return pixmap;
}

void stop_ss()
{
  kill(child, SIGTERM);
}

void die()
{
  Display *dpy = XtDisplay(toplevel);
  
  stop_ss();
  XtUnrealizeWidget(toplevel);
  XtDestroyWidget(toplevel);
  XtDestroyApplicationContext(context);
  XFreePixmap(dpy, pixmap);
  exit(0);
}

Widget create_menu(Widget toplevel)
{
  Widget menu, item;

  menu = XtVaCreatePopupShell(MENU_NAME, simpleMenuWidgetClass, toplevel,
			      NULL);

  item = XtVaCreateManagedWidget("activate", smeBSBObjectClass, menu,
				 XtNlabel, "Activate screensaver",
				 NULL);
  XtAddCallback(item, "callback", callback_activate, NULL);

  item = XtVaCreateManagedWidget("prefs", smeBSBObjectClass, menu,
				 XtNlabel, "Demo / Preferences",
				 NULL);
  XtAddCallback(item, "callback", callback_prefs, NULL);

  item = XtVaCreateManagedWidget("exit", smeBSBObjectClass, menu,
				 XtNlabel, "Exit",
				 NULL);
  XtAddCallback(item, "callback", callback_quit, NULL);

  return menu;
}

void callback_activate(Widget w, XtPointer client_data, XtPointer call_data)
{
  split_and_exec(app_data.activate_command, NULL);
}

void callback_prefs(Widget w, XtPointer client_data, XtPointer call_data)
{
  split_and_exec(app_data.prefs_command, NULL);
}

void callback_quit(Widget w, XtPointer client_data, XtPointer call_data)
{
  die();
}

void sig_chld()
{
  pid_t pid = waitpid(-1, NULL, WNOHANG);
  if (pid == child)
  {
    exit(0);
  }
}

void sig_int()
{
  stop_ss();
}

void sig_term()
{
  stop_ss();
}

void action_activate(Widget w, XEvent *event,
                     String *params, Cardinal *num_params)
{
  split_and_exec(app_data.activate_command, NULL);
}

/* What this does: takes cmd, breaks it up into a series of words, fork()s,
 * and exec()s.  If pid is non-NULL, saves the pid of the child in *pid.
 * die()s if the fork() fails. */
void split_and_exec(char *cmd, pid_t *pid)
{
  char *newcmd;
  char **words;
  int i, ntokens = 1;
  pid_t other;
  sigset_t mask, omask;
  
  for (newcmd = cmd; *newcmd; newcmd++)
    if (*newcmd == ' ')
      ntokens++;

  newcmd = malloc(strlen(cmd) + 1);
  strcpy(newcmd, cmd);
  words = malloc(ntokens * sizeof(char *));
  
  i = 0;
  words[i++] = strtok(newcmd, " ");
  while (words[i++] = strtok(NULL, " "));

  /* Block SIGCHLD until we've stored into *pid, if requested. */
  if (pid)
  {
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, &omask);
  }

  /* Create the child process. */
  other = fork();
  if (other == -1)
  {
    fprintf(stderr, "%s: fork() failed\n", progname);
    die();
  }

  /* Store into *pid if requested. */
  if (pid)
  {
    *pid = other;
    sigprocmask(SIG_SETMASK, &omask, NULL);
  }

  if (other == 0) /* we're the child */
  {
    if (app_data.redirect)
    {
      /* Set up stdout and stderr on the child to point to the redirected
       * file descriptor. */
      dup2(fdRedirect, 1);
      dup2(fdRedirect, 2);
      close(fdRedirect);
    }
    execvp(words[0], words);
    perror(progname);
    fprintf(stderr, "%s: execvp(%s) failed\n", progname, cmd);
    _exit(0);
  }

  free(words);
  free(newcmd);
}
