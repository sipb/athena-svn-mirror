/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/console/console.c,v $
 * $Author: cfields $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#if  (!defined(lint))  &&  (!defined(SABER))
static char rcsid[] =
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/console/console.c,v 1.10 1995-08-14 21:30:28 cfields Exp $";
#endif

#include "mit-copyright.h"
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <X11/Xj/Jets.h>
#include <X11/Xj/Window.h>
#include <X11/Xj/Button.h>
#include <X11/Xj/Label.h>
#include <X11/Xj/Form.h>
#include <X11/Xj/ScrollBar.h>
#include <X11/Xj/TextDisplay.h>
#include <X11/Xj/Arrow.h>
#include <X11/Xresource.h>

#ifndef CONSOLEDEFAULTS
#define CONSOLEDEFAULTS "/etc/athena/login/Console"
#endif

#define XENV "XENVIRONMENT"
#define CONSOLEFILE "/usr/tmp/console.log"

extern int DEBUG;

static XrmOptionDescRec opTable[] = {
{"+rv",         "*reverseVideo", XrmoptionNoArg,        (caddr_t) "off"},
{"-background", "*background",  XrmoptionSepArg,        (caddr_t) NULL},
{"-bd",         "*window.borderColor", XrmoptionSepArg,	(caddr_t) NULL},
{"-bg",         "*background",  XrmoptionSepArg,        (caddr_t) NULL},
{"-bordercolor","*borderColor", XrmoptionSepArg,        (caddr_t) NULL},
{"-borderwidth","*window.borderWidth", XrmoptionSepArg, (caddr_t) NULL},
{"-bw",         "*window.borderWidth", XrmoptionSepArg, (caddr_t) NULL},
{"-display",    ".display",     XrmoptionSepArg,        (caddr_t) NULL},
{"-fg",         "*foreground",  XrmoptionSepArg,        (caddr_t) NULL},
{"-fn",         "*font",        XrmoptionSepArg,        (caddr_t) NULL},
{"-font",       "*font",        XrmoptionSepArg,        (caddr_t) NULL},
{"-foreground", "*foreground",  XrmoptionSepArg,        (caddr_t) NULL},
{"-geometry",   "*window.geometry", XrmoptionSepArg,    (caddr_t) NULL},
{"-icongeometry", "*iconWindow.geometry", XrmoptionSepArg, (caddr_t) NULL},
{"-name",       ".name",        XrmoptionSepArg,        (caddr_t) NULL},
{"-reverse",    "*reverseVideo", XrmoptionNoArg,        (caddr_t) "on"},
{"-rv",         "*reverseVideo", XrmoptionNoArg,        (caddr_t) "on"},
{"-xrm",        NULL,           XrmoptionResArg,        (caddr_t) NULL},
{"-appdefs",	".appDefs",	XrmoptionSepArg,	(caddr_t) NULL},
{"-f",		".appDefs",	XrmoptionSepArg,	(caddr_t) NULL},
{"-blink",      ".frequency",   XrmoptionSepArg,        (caddr_t) NULL},
{"-unmap",      ".autoUnmap",   XrmoptionSepArg,        (caddr_t) NULL},
{"-map",        "*window.mapped", XrmoptionNoArg, 	(caddr_t) "True"},
{"+map",        "*window.mapped", XrmoptionNoArg, 	(caddr_t) "False"},
{"-input",	".input",	XrmoptionSepArg,	(caddr_t) NULL},
{"-inputfd",	".inputfd",	XrmoptionSepArg,	(caddr_t) NULL},
{"-nostdin",	".nostdin",	XrmoptionNoArg,		(caddr_t) "True"},
{"-iconic",	"*window.iconic", XrmoptionNoArg,	(caddr_t) "True"},
{"+iconic",	"*window.iconic", XrmoptionNoArg,	(caddr_t) "False"},
{"-notimestamp",".timestamp",	XrmoptionNoArg,		(caddr_t) "False"},
{"-timestamp",	".timestamp",	XrmoptionNoArg,		(caddr_t) "True"},
{"-hidelabel",	"*hideLabel.label", XrmoptionSepArg,	(caddr_t) NULL},
{"-title",	"*title.label",	XrmoptionSepArg,	(caddr_t) NULL},
{"-hideproc",	"*hideButton.activateProc", XrmoptionSepArg, (caddr_t) NULL},
{"-cc",		"*textDisplay.charClass", XrmoptionSepArg, (caddr_t) NULL},
{"-titlebar",	"*window.title",XrmoptionSepArg,	(caddr_t) NULL},
{"-nosession",	"*showCommand",	XrmoptionNoArg,		(caddr_t) "false"},
{"-global",	"*global",	XrmoptionNoArg,		(caddr_t) "true"},
{"-autoscroll",	".autoscroll",	XrmoptionNoArg,		(caddr_t) "True"},
{"-noautoscroll",".autoscroll",	XrmoptionNoArg,		(caddr_t) "False"},
{"-file",	".file",	XrmoptionSepArg,	(caddr_t) ""},
{"-debug",	".debug",	XrmoptionNoArg,		(caddr_t) "true"},
};

typedef struct _MyResources
{
  int frequency, autoUnmap;
  char *input;
  int inputfd;
  Boolean timestamp, autoscroll, nostdin;
  char *file;
  Boolean debug;
} MyResources;

typedef struct _MyResources *MyResourcesPtr;

MyResources parms;

#define offset(field) XjOffset(MyResourcesPtr,field)

static XjResource appResources[] =
{
  { "frequency", "Frequency", XjRInt, sizeof(int),
      offset(frequency), XjRInt, (caddr_t)1000 },
  { "autoUnmap", "AutoUnmap", XjRInt, sizeof(int),
      offset(autoUnmap), XjRInt, (caddr_t)0 },
  { "input", "Input", XjRString, sizeof(char *),
      offset(input), XjRString, (caddr_t)"" },
  { "inputfd", "Inputfd", XjRInt, sizeof(int),
      offset(inputfd), XjRInt, (caddr_t)0 },
  { "nostdin", "Nostdin", XjRBoolean, sizeof(Boolean),
      offset(nostdin), XjRBoolean, (caddr_t) False },
  { "timestamp", "Timestamp", XjRBoolean, sizeof(Boolean),
      offset(timestamp), XjRBoolean, (caddr_t) True },
  { "autoscroll", "Autoscroll", XjRBoolean, sizeof(Boolean),
      offset(autoscroll), XjRBoolean, (caddr_t) True },
  { "file", "File", XjRString, sizeof(char *),
      offset(file), XjRString, (caddr_t)CONSOLEFILE },
  { "debug", "Debug", XjRBoolean, sizeof(Boolean),
      offset(debug), XjRBoolean, (caddr_t) False },
};

#undef offset

#define INPUTSIZE 1024
char inputbuf[INPUTSIZE];

#define BUFSIZE 16384 /* must be at least two times as big as INPUTSIZE */
char *text;
int length;


#define HUP 0
#define FPE 1
#define USR1 2
#define USR2 3
#define NUMSIGS 4
int sigflags[NUMSIGS];
int val = 0;
Jet root;
WindowJet iconWindow;
LabelJet icon;
WindowJet win;
ScrollBarJet sj;
TextDisplayJet tj;
int inverted = False, blinking = False, timerid;
int unmaptimerid;


static int loadFile(filename, info, max)
     char *filename, *info;
     int max;
{
    int fd, size;
    struct stat buf;
    int num;
    char errtext[100];
    char *errfmt = "error %d %s `%s'";

    if (-1 == (fd = open(filename, O_RDONLY, 0)))
      {
	sprintf(errtext, errfmt, errno, "opening", filename);
	XjWarning(errtext);
	return 0;
      }

    if (-1 == fstat(fd, &buf)) /* could only return EIO */
      {
	sprintf(errtext, errfmt, errno, "in fstat for", filename);
	XjWarning(errtext);
	close(fd);
	return 0;
      }

    size = (int)buf.st_size;

    if (-1 == (num = read(fd, info, MIN(size, max - 1))))
      {
	sprintf(errtext, errfmt, errno, "reading", filename);
	XjWarning(errtext);
	close(fd);
	return 0;
      }
    close(fd);

    info[num] = '\0';
    return num;
}


static void saveFile(filename, info, length)
     char *filename, *info;
     int length;
{
  int fd;
  char errtext[100];
  char *errfmt = "error %d %s `%s'";

  if (-1 == (fd = open(filename, O_TRUNC | O_CREAT | O_WRONLY, 0644)))
    {
      sprintf(errtext, errfmt, errno, "opening", filename);
      XjWarning(errtext);
      return;
    }

  if (length != write(fd, info, length))
    {
      sprintf(errtext, errfmt, errno, "writing", filename);
      XjWarning(errtext);
      close(fd); /* I doubt it. */
      return;
    }

  close(fd);
}


int hide(fromJet, what, data)
     Jet fromJet;
     char *what;
     caddr_t data;
     /*ARGSUSED*/
{
  UnmapWindow(win);
  return 0;
}


void resetunmaptimer()
{
  if (parms.autoUnmap != 0)
    {
      if (unmaptimerid != 0)
	(void)XjRemoveWakeup(unmaptimerid);
      unmaptimerid = XjAddWakeup(hide, 0, 1000 * parms.autoUnmap);
    }
}


int scroll(fromJet, increment, data)
     Jet fromJet;
     int increment;
     caddr_t data;
     /*ARGSUSED*/
{
  int cl, vl;

  resetunmaptimer();
  cl = CountLines(tj);
  vl = VisibleLines(tj);
  val = GetScrollBarValue(sj);
  if ((increment > 0  &&  val < cl - vl)
      ||  (increment < 0   &&  val > 0))
    {
      val += increment;
      SetScrollBar(sj, 0, MAX(0, cl - 1), vl, val);
      SetLine(tj, val);
    }
  return 0;
}


int newvalue(fromJet, nothing, data)
     Jet fromJet;
     int nothing;
     caddr_t data;
     /*ARGSUSED*/
{
  resetunmaptimer();
  SetLine(tj, val = GetScrollBarValue((ScrollBarJet) fromJet));
  return 0;
}


int textscroll(fromJet, nothing, data)
     Jet fromJet;
     int nothing;
     caddr_t data;
     /*ARGSUSED*/
{
  int cl, vl;

  resetunmaptimer();
  cl = CountLines(tj);
  vl = VisibleLines(tj);
  val = TopLine(tj);
  SetScrollBar(sj, 0, MAX(0, cl - 1), vl, val);
  return 0;
}


int textresize(fromJet, nothing, data)
     Jet fromJet;
     int nothing;
     caddr_t data;
     /*ARGSUSED*/
{
  int cl, vl;

  resetunmaptimer();
  cl = CountLines(tj);
  vl = VisibleLines(tj);

  if ((val != 0) && (val > cl - vl))
    {
      if (parms.autoscroll)
	{
	  val = MAX(0, cl - vl);

	  SetLine(tj, val);
	}
    }

  SetScrollBar(sj, 0, MAX(0, cl - 1), vl, val);
  return 0;
}


int delete(fromJet, what, data)
     Jet fromJet;
     char *what;
     caddr_t data;
     /*ARGSUSED*/
{
  XjDestroyJet(fromJet);
  XCloseDisplay(root->core.display);
  XjExit(0);
  return 0;				/* For linting... */
}


void blinkoff()
{
  if (blinking)
    {
      if (inverted)
	{
	  inverted = !inverted;
	  setIcon(icon, Center, inverted);
	}

      (void)XjRemoveWakeup(timerid);
      blinking = False;
    }
}


int mapnotify(fromJet, what, data)
     Jet fromJet;
     char *what;
     caddr_t data;
     /*ARGSUSED*/
{
  blinkoff();
  return 0;
}


void OurMapWindow(who)
     WindowJet who;
{
  blinkoff();
  resetunmaptimer();
  MapWindow(who, True);
}


int blink(info, id)
     int info, id;
     /*ARGSUSED*/
{
  inverted = !inverted;
  setIcon(icon, Center, inverted);
  timerid = XjAddWakeup(blink, 0, MAX(100, parms.frequency));
  return 0;
}


int appendToBuffer(what, howmuch)
     char *what;
     int howmuch;
{
  int moved = 0;

  if (howmuch == 0)		/* if nothing to copy, return */
    return 0;

  /*
   * If there's not enough room to copy in whatever was asked for, then
   * chop off the beginning of the text, using bcopy...
   */
  if (BUFSIZE - length <= howmuch)
    {
      char *c;

      c = text + MIN(INPUTSIZE, length);
      while ((c < text + length) && *c != '\0' && *c != '\n')
	c++;
      if (*c != '\n')		/* if this is a looooong line, screw it. */
	c = text + MIN(INPUTSIZE, length);
      else
	c++;

      moved = c - text;
      length -= moved;

      bcopy(c, text, length);	/* depends on intelligent bcopy */
    }

  bcopy(what, text + length, howmuch);
  length += howmuch;
  text[length] = '\0';		/* Null terminate the string... */
				/* WARNING:  This can overflow!  Why? */
  return moved;
}


int input(fd, pfd)
     int fd;
     int *pfd;
     /*ARGSUSED*/
{
  int redo = 0;
  int cl, vl, l;
  char ctrl[2];
  int i = 0, j;
  static int eol = 1;

  ctrl[0] = '^';
  l = read(*pfd, inputbuf, INPUTSIZE);

  if (l < 1)
    {
      XjReadCallback((XjCallbackProc)NULL, *pfd, (caddr_t) pfd);
      return 0;
      /* close(0); */
    }

  if (l != 0)
    {
      while (i < l)
	{
	  if (eol && parms.timestamp)
	    {
	      struct timeval now;
	      time_t timet;
	      char *timestr;

	      gettimeofday(&now, NULL);
	      timet = (time_t) now.tv_sec;
	      timestr = (char *) ctime(&timet);
	      redo += appendToBuffer(&timestr[11], 5);
	      redo += appendToBuffer(" ", 1);
	    }

	  eol = 0;
	  j = i;
	  while (j < l && !eol)
	    {
	      if (iscntrl(inputbuf[j]))
		{
		  redo += appendToBuffer(&inputbuf[i], j - i);
		  i = j + 1;

		  switch(inputbuf[j])
		    {
		    case 7: /* bell compression option... */
		      XBell(root->core.display, 0);
		    case 0:
		    case 13:
		      break;
		    case '\n':
		      eol = 1;
		    case '\t':
		      redo += appendToBuffer(&inputbuf[j], 1);
		      break;
		    default:
		      ctrl[1] = inputbuf[j] | 64;
		      redo += appendToBuffer(ctrl, 2);
		      break;
		    }
		}
	      j++;
	    }
	  redo += appendToBuffer(&inputbuf[i], j - i);
	  i = j;
	}
      if (!redo)
	AddText(tj);		/* ??? */
      else
	MoveText(tj, text, redo);

      cl = CountLines(tj);
      vl = VisibleLines(tj);

      if (parms.autoscroll)
	{
	  val = MAX(0, cl - vl);

	  SetScrollBar(sj, 0, MAX(cl - 1, 0), vl, val);
	  SetLine(tj, val);
	}
      
      if (!WindowMapped((WindowJet)win))
	{
	  if (!WindowMapped(iconWindow))
	    OurMapWindow((WindowJet)win); /* we're hidden, not iconified */
	  else
	    {
	      if (!blinking)
		{
		  blinking = True;
		  blink(0, 0);
		}
	    }
	}
      else
	{
	  resetunmaptimer();
	  if (WindowVisibility(win) != VisibilityUnobscured)
	    XRaiseWindow(win->core.display, win->core.window); /* ICCCM ok */
	}
    }
  return 0;
}

#if defined(AIX_ARCH)
void sighandler(sig)		/* broken header files under aix... */
     int sig;
#else
void sighandler(sig, code, scp)
     int sig, code;
     struct sigcontext *scp;
     /*ARGSUSED*/
#endif
{
  switch(sig)
    {
    case SIGHUP:
      sigflags[HUP] = 1;
      break;
    case SIGFPE:
      sigflags[FPE] = 1;
      break;
    case SIGUSR1:
      sigflags[USR1] = 1;
      break;
    case SIGUSR2:
      sigflags[USR2] = 1;
      break;
    }
  return;
}


XjCallbackRec callbacks[] =
{
  { "newvalue", newvalue },
  { "textresize", textresize },
  { "textscroll", textscroll },
  { "delete", delete },
  { "hide", hide },
  { "mapnotify", mapnotify },
  { "scroll", scroll },
};


static int checkSignals()
{
  int sigs;
  int cl, vl;
  int ret_code=0;

  do
    {
      sigs = 0;

      if (sigflags[FPE])
	{
	  sigs++;
	  sigflags[FPE] = 0;
	  text[0] = '\0';
	  length = 0;
	  saveFile(parms.file, text, length);

	  blinkoff();
	  if (WindowMapped(win))
	    UnmapWindow(win);

	  SetText(tj, text);

	  cl = CountLines(tj);
	  vl = VisibleLines(tj);

	  val = MAX(0, cl - vl);

	  SetScrollBar(sj, 0, MAX(cl - 1, 0), vl, val);
	  SetLine(tj, val);
	  XFlush(tj->core.display);
	}

      if (sigflags[USR1])
	{
	  sigs++;
	  sigflags[USR1] = 0;
	  if (!WindowMapped((WindowJet)win))
	    {
	      OurMapWindow((WindowJet)win);
	      XFlush(win->core.display);
	    }
	  else
	    if (WindowVisibility(win) != VisibilityUnobscured)
	      {
		XRaiseWindow(win->core.display, win->core.window); /* ICCCM ok */
		XFlush(win->core.display);
	      }
	}

      if (sigflags[USR2])
	{
	  sigs++;
	  sigflags[USR2] = 0;
	  if (WindowMapped(win))
	    {
	      UnmapWindow(win);
	      XFlush(win->core.display);
	    }
	}

      if (sigflags[HUP])
	{
	  sigs++;
	  sigflags[HUP] = 0;
	  saveFile(parms.file, text, length);
	  XCloseDisplay(root->core.display);
	  XjExit(0);
	}
      ret_code += sigs;
    }
  while(sigs);
  return ret_code;
}


fatal(display)
     Display *display;
     /*ARGSUSED*/
{
  XjExit(1);
}


main(argc, argv)
     int argc;
     char **argv;
{
  Jet w, b, f;
  int cl, vl, i;
  int zero = 0;
  int auxinput = -1;
  int size=0;
  struct stat buf;
#ifndef ultrix
  struct sigaction act;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
#endif


  for (i = 0; i < NUMSIGS; i++)
    sigflags[i] = 0;

#ifndef ultrix /* s.b. ifdef POSIX but need to test on ultrix */
  act.sa_handler= (void (*)()) sighandler;
  (void) sigaction(SIGHUP, &act, NULL);
  (void) sigaction(SIGFPE, &act, NULL);
  (void) sigaction(SIGUSR1, &act, NULL);
  (void) sigaction(SIGUSR2, &act, NULL);
#else
  (void) signal(SIGHUP, sighandler);
  (void) signal(SIGFPE, sighandler);
  (void) signal(SIGUSR1, sighandler);
  (void) signal(SIGUSR2, sighandler);
#endif

#if defined(HAS_PUTENV)
/*
 *  setenv() doesn't exist on some systems...  it's putenv instead.
 */
  if ((char *) getenv(XENV) == NULL)
    {
      char foo[1024];

      sprintf(foo, "%s=%s", XENV, CONSOLEDEFAULTS);
      putenv(foo);
    }
#else
  setenv(XENV, CONSOLEDEFAULTS, 0);
#endif

  root = XjCreateRoot(&argc, argv, "Console", NULL,
		      opTable, XjNumber(opTable));

  (void) XSetIOErrorHandler(fatal);

  XjLoadFromResources(NULL,
		      NULL,
		      programClass,
		      programName,
		      appResources,
		      XjNumber(appResources),
		      (caddr_t) &parms);

  DEBUG = parms.debug;

  XjRegisterCallbacks(callbacks, XjNumber(callbacks));

  iconWindow = (WindowJet) XjVaCreateJet("iconWindow",
					 windowJetClass, root, NULL, NULL);
  icon = (LabelJet) XjVaCreateJet("icon",
				  labelJetClass, iconWindow, NULL, NULL);
  XjRealizeJet(root);

  win = (WindowJet) XjVaCreateJet("window", windowJetClass, root,
				  XjNiconWindow, iconWindow,
				  NULL, NULL);
  f = XjVaCreateJet("form", formJetClass, win, NULL, NULL);

  /*
   * Button
   */
  w = XjVaCreateJet("hide", windowJetClass, f, NULL, NULL);
  b = XjVaCreateJet("hideButton", buttonJetClass, w, NULL, NULL);
  (void) XjVaCreateJet("hideLabel", labelJetClass, b, NULL, NULL);

  /*
   * Arrow buttons
   */
  w = XjVaCreateJet("scrollup", windowJetClass, f, NULL, NULL);
  b = XjVaCreateJet("scrollupButton", buttonJetClass, w, NULL, NULL);
  (void) XjVaCreateJet("scrollupArrow", arrowJetClass, b, NULL, NULL);

  w = XjVaCreateJet("scrolldown", windowJetClass, f, NULL, NULL);
  b = XjVaCreateJet("scrolldownButton", buttonJetClass, w, NULL, NULL);
  (void) XjVaCreateJet("scrolldownArrow", arrowJetClass, b, NULL, NULL);

  /*
   * Label
   */
  (void) XjVaCreateJet("title", labelJetClass, f, NULL, NULL);

  /*
   * Scrollbar
   */
  w = XjVaCreateJet("scrollBarWindow", windowJetClass, f, NULL, NULL);
  sj = (ScrollBarJet) XjVaCreateJet("scrollBar",
				    scrollBarJetClass, w, NULL, NULL);

  /*
   * TextDisplay
   */
  w = XjVaCreateJet("textDisplayWindow", windowJetClass, f, NULL, NULL);
  tj = (TextDisplayJet) XjVaCreateJet("textDisplay",
				      textDisplayJetClass, w, NULL, NULL);


  XjRealizeJet((Jet) XjParent(f));


  if (! stat(parms.file, &buf))	/* If there was an error stat'ing the */
    size = (int)buf.st_size;	/* file, we will find out soon enough */
				/* when we call loadFile below.  No need */
				/* to display an error msg here. */

  size = (size > BUFSIZE) ? size : BUFSIZE;
  text = XjMalloc(size);
  text[0] = '\0';
  length = 0;
  length = loadFile(parms.file, text, size);

  if (parms.input && *parms.input)
    {
      auxinput = open(parms.input, O_RDONLY, 0);
      if (auxinput == -1)
	{
#define CANT_OPEN "console: can't open input\n"
	  appendToBuffer(CANT_OPEN, sizeof(CANT_OPEN)-1);
	}
    }
  SetText(tj, text);

  cl = CountLines(tj);
  vl = VisibleLines(tj);

  if (parms.autoscroll)
    {
      val = MAX(0, cl - vl);

      SetScrollBar(sj, 0, MAX(cl - 1, 0), vl, val);
      SetLine(tj, val);
    }

  if (parms.nostdin == False)
    XjReadCallback((XjCallbackProc)input, zero, (caddr_t) &zero);

  if (auxinput != -1)
    XjReadCallback((XjCallbackProc)input, auxinput, (caddr_t) &auxinput);

  if (parms.inputfd != 0)
    XjReadCallback((XjCallbackProc)input, parms.inputfd,
		   (caddr_t) &parms.inputfd);

  XjSetSignalChecker(checkSignals);

  XjEventLoop(root);
}
