/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/console/console.c,v $
 * $Author: vanharen $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#ifndef	lint
static char rcsid[] =
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/console/console.c,v 1.1 1991-09-03 11:11:19 vanharen Exp $";
#endif	lint

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
#include "Jets.h"
#include "Window.h"
#include "Button.h"
#include "Label.h"
#include "Form.h"
#include "ScrollBar.h"
#include "TextDisplay.h"
#include "Icon.h"
#include <X11/Xresource.h>

#ifndef CONSOLEDEFAULTS
#define CONSOLEDEFAULTS "/etc/athena/login/Console"
#endif

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
{"-iconic",	"*window.iconic", XrmoptionNoArg,	(caddr_t) "True"},
{"+iconic",	"*window.iconic", XrmoptionNoArg,	(caddr_t) "False"},
{"-notimestamp",".timestamp",	XrmoptionNoArg,		(caddr_t) "False"},
};

typedef struct _MyResources {
  int frequency, autoUnmap;
  char *input;
  Boolean timestamp;
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
  { "timestamp", "Timestamp", XjRBoolean, sizeof(Boolean),
      offset(timestamp), XjRBoolean, (caddr_t) True },
};

#undef offset

#define CONSOLEFILE "/usr/tmp/console.log"

#define BUFSIZE 16384 /* must be at least twice 1024 */
char *text;
int length;

#define INPUTSIZE 1024
char inputbuf[INPUTSIZE];

#define HUP 0
#define FPE 1
#define USR1 2
#define USR2 3
#define NUMSIGS 4
int sigflags[NUMSIGS];

#if defined(ultrix) || defined(_AIX) || defined(sun)  || defined(_AUX_SOURCE)
/*
 *  setenv() doesn't exist on some systems...  it's putenv instead.
 *  So, we write our own setenv routine, and use it instead.
 *
 *  Arguments:  (same as setenv in BSD Unix)
 *      char *name;     name of environment variable to set or change.
 *      char *value;    value to set or change environment variable to.
 *      int overwrite;  if non-zero, force variable to value.
 *                      if zero, and variable does not exist, add variable
 *                              to environment.
 *                      else, do nothing.
 *
 *  Returns:   (same as setenv in bsd Unix)
 *      -1              if unsuccessful (unable to malloc enough space for
 *                              an expanded environment).
 *      0               otherwise.
 *
 */
static int setenv(name, value, overwrite)
     char *name, *value;
     int overwrite;
{
  char *string;

  if ((overwrite) ||
      ( (char *) getenv(name) == NULL))
    {
      string = (char *) malloc((strlen(name) + strlen(value) + 2)
                               * sizeof(char));
					/* add 1 for the null and 1 for "=" */
      if (string == NULL)
        return(-1);
      strcpy(string, name);
      strcat(string, "=");
      strcat(string, value);
      if (! putenv(string) )
        return(-1);
    }
  return(0);
}
#endif


void resetunmaptimer();
int val = 0;
Jet root, tj, sj, w, icon, iconWindow;
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

    if (-1 == (fd = open(filename, O_RDONLY, 0)))
      {
	sprintf(errtext, "error %d opening %s", errno, filename);
	XjWarning(errtext);
	return 0;
      }

    if (-1 == fstat(fd, &buf)) /* could only return EIO */
      {
	sprintf(errtext, "error %d in fstat for %s", errno, filename);
	XjWarning(errtext);
	close(fd);
	return 0;
      }

    size = (int)buf.st_size;

    if (-1 == (num = read(fd, info, MIN(size, max - 1))))
      {
	sprintf(errtext, "error %d reading %s", errno, filename);
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

  if (-1 == (fd = open(filename, O_TRUNC | O_CREAT | O_WRONLY, 0644)))
    {
      sprintf(errtext, "error %d opening %s", errno, filename);
      XjWarning(errtext);
      return;
    }

  if (length != write(fd, info, length))
    {
      sprintf(errtext, "error %d writing %s", errno, filename);
      XjWarning(errtext);
      close(fd); /* I doubt it. */
      return;
    }

  close(fd);
}

int newvalue(fromJet, nothing, data)
     Jet fromJet;
     int nothing;
     caddr_t data;
{
  resetunmaptimer();
  SetLine(tj, val = GetScrollBarValue(fromJet));
  return 0;
}

int textresize(fromJet, nothing, data)
     Jet fromJet;
     int nothing;
     caddr_t data;
{
  int cl, vl;

  resetunmaptimer();
  cl = CountLines(tj);
  vl = VisibleLines(tj);

  if ((val != 0) && (val > cl - vl))
    {
      val = MAX(0, cl - vl);
      SetLine(tj, val);
    }

  SetScrollBar(sj, 0, MAX(0, cl - 1), vl, val);
  return 0;
}

int delete(fromJet, what, data)
     Jet fromJet;
     char *what;
     caddr_t data;
{
  XjDestroyJet(fromJet);
  XCloseDisplay(root->core.display);
  exit(0);
  return 0;				/* For linting... */
}

int hide(fromJet, what, data)
     Jet fromJet;
     char *what;
     caddr_t data;
{
  UnmapWindow(w);
  return 0;
}

char buffer[512];

void blinkoff()
{
  if (blinking)
    {
      if (inverted)
	{
	  inverted = !inverted;
	  SetIcon(icon, inverted);
	}

      (void)XjRemoveWakeup(timerid);
      blinking = False;
    }
}

int mapnotify(fromJet, what, data)
     Jet fromJet;
     char *what;
     caddr_t data;
{
  blinkoff();
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

void OurMapWindow(who)
     WindowJet who;
{
  blinkoff();
  resetunmaptimer();
  MapWindow(who, True);
}

void blink(info)
     int info;
{
  inverted = !inverted;
  SetIcon(icon, inverted);
  timerid = XjAddWakeup(blink, 0, MAX(100, parms.frequency));
}

int appendToBuffer(what, howmuch)
     char *what;
     int howmuch;
{
  int moved = 0;

  if (howmuch == 0)
    return 0;

  if (BUFSIZE - length < howmuch)
    {
      char *c;

      c = text + MIN(INPUTSIZE, length);
      while ((c < text + length) && *c != '\0' && *c != '\n')
	c++;
      if (*c != '\n') /* if this is a looooong line, screw it. */
	c = text + MIN(INPUTSIZE, length);
      else
	c++;

      length -= (c - text);
      moved = c - text;
      /* depends on intelligent bcopy */
      bcopy(c, text, length);
    }

  bcopy(what, text + length, howmuch);
  length += howmuch;
  text[length] = '\0';

  return moved;
}

input(fd, pfd)
     int fd;
     int *pfd;
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
      XjReadCallback((XjCallbackProc)NULL, *pfd, pfd);
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
	      char *timestr;

	      gettimeofday(&now, NULL);
	      timestr = (char *) ctime(&now.tv_sec);
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
	AddText(tj);
      else
	MoveText(tj, text, redo);

      cl = CountLines(tj);
      vl = VisibleLines(tj);

      val = MAX(0, cl - vl);

      SetScrollBar(sj, 0, MAX(0, cl - 1), vl, val);
      SetLine(tj, val);

      if (!WindowMapped((WindowJet)w))
	{
	  if (!WindowMapped(iconWindow))
	    OurMapWindow((WindowJet)w); /* we're hidden, not iconified */
	  else
	    {
	      if (!blinking)
		{
		  blinking = True;
		  blink(0);
		}
	    }
	}
      else
	{
	  resetunmaptimer();
	  if (WindowVisibility(w) != VisibilityUnobscured)
	    XRaiseWindow(w->core.display, w->core.window); /* ICCCM ok */
	}
    }
}

void sighandler(sig, code, scp)
     int sig, code;
     struct sigcontext *scp;
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
  { "delete", delete },
  { "hide", hide },
  { "mapnotify", mapnotify }
};

static void checkSignals()
{
  int sigs;
  int cl, vl;

  do
    {
      sigs = 0;

      if (sigflags[FPE])
	{
	  sigs++;
	  sigflags[FPE] = 0;
	  text[0] = '\0';
	  length = 0;
	  saveFile(CONSOLEFILE, text, length);

	  blinkoff();
	  if (WindowMapped(w))
	    UnmapWindow(w);

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
	  if (!WindowMapped((WindowJet)w))
	    {
	      OurMapWindow((WindowJet)w);
	      XFlush(w->core.display);
	    }
	  else
	    if (WindowVisibility(w) != VisibilityUnobscured)
	      {
		XRaiseWindow(w->core.display, w->core.window); /* ICCCM ok */
		XFlush(w->core.display);
	      }
	}

      if (sigflags[USR2])
	{
	  sigs++;
	  sigflags[USR2] = 0;
	  if (WindowMapped(w))
	    {
	      UnmapWindow(w);
	      XFlush(w->core.display);
	    }
	}

      if (sigflags[HUP])
	{
	  sigs++;
	  sigflags[HUP] = 0;
	  saveFile(CONSOLEFILE, text, length);
	  XCloseDisplay(root->core.display);
	  exit(0);
	}
    }
  while(sigs);
}

fatal(display)
     Display *display;
{
  exit(1);
}

main(argc, argv)
int argc;
char **argv;
{
  Display *display;
  Jet w0, b0, l0, w1, w2, f, l1;
  int cl, vl, i;
  int zero = 0;
  int auxinput = -1;

  for (i = 0; i < NUMSIGS; i++)
    sigflags[i] = 0;

  (void)signal(SIGHUP, sighandler);
  (void)signal(SIGFPE, sighandler);
  (void)signal(SIGUSR1, sighandler);
  (void)signal(SIGUSR2, sighandler);

  setenv("XENVIRONMENT", CONSOLEDEFAULTS, 0);

  root = XjCreateRoot(&argc, argv, "Console", NULL,
		      opTable, XjNumber(opTable));

  (void)XSetIOErrorHandler(fatal);

  XjLoadFromResources(NULL,
		      NULL,
		      programName,
		      programClass,
		      appResources,
		      XjNumber(appResources),
		      &parms);

  display = root->core.display;

  XjRegisterCallbacks(callbacks, XjNumber(callbacks));

  iconWindow = XjVaCreateJet("iconWindow", windowJetClass, root, NULL, NULL);
  icon = XjVaCreateJet("icon", iconJetClass, iconWindow, NULL, NULL);
  XjRealizeJet(root);

  w = XjVaCreateJet("window", windowJetClass, root,
		    XjNiconWindow, iconWindow,
		    NULL, NULL);
  f = XjVaCreateJet("form", formJetClass, w, NULL, NULL);

  /*
   * Button
   */
  w0 = XjVaCreateJet("hide", windowJetClass, f, NULL, NULL);
  b0 = XjVaCreateJet("hideButton", buttonJetClass, w0, NULL, NULL);
  l0 = XjVaCreateJet("hideLabel", labelJetClass, b0, NULL, NULL);

  /*
   * Label
   */
  l1 = XjVaCreateJet("title", labelJetClass, f, NULL, NULL);

  w1 = XjVaCreateJet("scrollBarWindow", windowJetClass, f, NULL, NULL);
  sj = XjVaCreateJet("scrollBar", scrollBarJetClass, w1, NULL, NULL);
  w2 = XjVaCreateJet("textDisplayWindow", windowJetClass, f, NULL, NULL);
  tj = XjVaCreateJet("textDisplay", textDisplayJetClass, w2, NULL, NULL);

  XjRealizeJet(w);

  text = XjMalloc(BUFSIZE);
  text[0] = '\0';
  length = 0;
  length = loadFile(CONSOLEFILE, text, BUFSIZE);

  if (parms.input && *parms.input) {
      auxinput = open(parms.input, O_RDONLY, 0);
      if (auxinput == -1) {
#define CANT_OPEN "console: can't open input\n"
	  appendToBuffer(CANT_OPEN, sizeof(CANT_OPEN)-1);
      }
  }
  SetText(tj, text);

  cl = CountLines(tj);
  vl = VisibleLines(tj);

  val = MAX(0, cl - vl);

  SetScrollBar(sj, 0, MAX(cl - 1, 0), vl, val);
  SetLine(tj, val);

      
  XjReadCallback((XjCallbackProc)input, zero, &zero);
  if (auxinput != -1)
      XjReadCallback((XjCallbackProc)input, auxinput, &auxinput);

  XjSetSignalChecker(checkSignals);

  XjEventLoop(root);
}
