/* $Header: /afs/dev.mit.edu/source/repository/athena/etc/xdm/xlogin/xlogin.c,v 1.1 1990-10-22 18:00:19 mar Exp $ */

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <ctype.h>
#include <WcCreate.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Label.h>
#include <X11/Xlib.h>

#define OWL_AWAKE 0
#define OWL_SLEEPY 1

#define OWL_STATIC 0
#define OWL_BLINKINGCLOSED 1
#define OWL_BLINKINGOPEN 2
#define OWL_SLEEPING 3
#define OWL_WAKING 4

/*
 * Function declarations.
 */
extern void AriRegisterAthena ();
static void move_instructions(), screensave(), unsave();
static void blinkOwl(), initOwl();

/*
 * Definition of the Application resources structure.
 */

typedef struct _XLoginResources {
  int save_timeout;
  int move_timeout;
  int blink_timeout;
} XLoginResources;

/*
 * Command line options table.  Only resources are entered here...there is a
 * pass over the remaining options after XtParseCommand is let loose. 
 */

static XrmOptionDescRec options[] = {
  {"-save",	"*saveTimeout",		XrmoptionSepArg,	NULL},
  {"-move",	"*moveTimeout",		XrmoptionSepArg,	NULL},
  {"-blink",	"*blinkTimeout",	XrmoptionSepArg,	NULL},
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
};

#undef Offset

/*
 * Globals.
 */
XtIntervalId curr_timerid = NULL, blink_timerid = NULL;
Widget appShell;
Widget saver, ins;
XLoginResources resources;
GC owlGC;
Display *dpy;
Window owlWindow;
int owlNumBitmaps;
unsigned int owlWidth, owlHeight;
int owlState, owlDelta, owlTimeout;
Pixmap owlBitmaps[20];

/******************************************************************************
*   MAIN function
******************************************************************************/

void
main(argc, argv)
     int argc;
     char* argv[];
{   
  XtAppContext app;
  Widget hitanykey;

  srandom(time(0));

  /*
   *  Intialize Toolkit creating the application shell, and get
   *  application resources.
   */
  appShell = XtInitialize ("xlogin", "Xlogin",
			   options, XtNumber(options),
			   &argc, argv);
  app = XtWidgetToApplicationContext(appShell);
  dpy = XtDisplay(appShell);

  XtGetApplicationResources(appShell, (caddr_t) &resources, 
			    my_resources, XtNumber(my_resources),
			    NULL, (Cardinal) 0);

  /*
   *  Register all Athena widget classes
   */
  AriRegisterAthena ( app );

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

  XtMainLoop ( );
}

static void
move_instructions(data, timerid)
     XtPointer  data;
     XtIntervalId  *timerid;
{
  static int x_max = 0, y_max = 0;

  if (!x_max)			/* get sizes, if we haven't done so already */
    {
      Arg args[2];

      XtSetArg(args[0], XtNwidth, &x_max);
      XtSetArg(args[1], XtNheight, &y_max);
      XtGetValues(ins, args, 2);

      x_max -= WidthOfScreen(XtScreen(ins));
      y_max -= HeightOfScreen(XtScreen(ins));
    }

  XtMoveWidget(ins, random() % x_max, random() % y_max);

  curr_timerid = XtAddTimeOut(resources.move_timeout * 1000,
			      move_instructions, NULL);
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

  XtPopup(saver, XtGrabNone);
  XtPopup(ins, XtGrabNone);
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
      XDefineCursor(dpy, XtWindow(saver), cursor);
      XDefineCursor(dpy, XtWindow(ins), cursor);
      XFreeCursor(dpy, cursor);

      first_time = FALSE;
    }

  if (blink_timerid != NULL)	/* don't blink while screensaved... */
    XtRemoveTimeOut(blink_timerid);

  curr_timerid = XtAddTimeOut(resources.move_timeout * 1000,
			      move_instructions, NULL);
}

static void
unsave(w, popdown, event, bool)
     Widget w;
     int popdown;
     XEvent *event;
     Boolean *bool;
{
  if (popdown)
    {
      XtPopdown(ins);
      XtPopdown(saver);
    }

  if (curr_timerid != NULL)
    XtRemoveTimeOut(curr_timerid);

  curr_timerid = XtAddTimeOut(resources.save_timeout * 1000,
			      screensave, NULL);
  blink_timerid = XtAddTimeOut(random() % (10 * 1000),
			       blinkOwl, NULL);
}



#define updateOwl()	XCopyPlane(dpy, owlBitmaps[owlCurBitmap], \
				   owlWindow, owlGC, 0, 0, \
				   owlWidth, owlHeight, 0, 0, 1)

static void
blinkOwl(data, intervalid)
     XtPointer data;
     XtIntervalId *intervalid;
{
  static int owlCurBitmap;
  owlTimeout = 0;

  switch(owlDelta)
    {
    case OWL_BLINKINGCLOSED:	/* your eyelids are getting heavy... */
      owlCurBitmap++;
      updateOwl();
      if (owlCurBitmap == owlNumBitmaps - 1)
	owlDelta = OWL_BLINKINGOPEN;
      break;

    case OWL_BLINKINGOPEN:	/* you will awake, feeling refreshed... */
      owlCurBitmap--;
      updateOwl();
      if (owlCurBitmap == ((owlState == OWL_SLEEPY) * (owlNumBitmaps) / 2))
	{
	  owlTimeout = random() % (10 * 1000);
	  owlDelta = OWL_BLINKINGCLOSED;
	}
      break;

    case OWL_SLEEPING:		/* transition to sleeping state */
      owlCurBitmap++;
      updateOwl();
      if (owlCurBitmap == ((owlState == OWL_SLEEPY) * (owlNumBitmaps) / 2))
	{
	  owlState = OWL_SLEEPY;
	  owlDelta = OWL_STATIC;
	}
      break;

    case OWL_WAKING:		/* transition to waking state */
      owlCurBitmap--;
      updateOwl();
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
  Widget owl;
  Arg args[3];
  int n = 0, done = 0, scratch;
  char *filenames, *ptr;
  XGCValues values;
  XtGCMask valuemask;

  owl = WcFullNameToWidget(search, "*eyes");

  if (owl != NULL)
    {
      owlWindow = XtWindow(owl);
      if (owlWindow != NULL)
	{
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

	      if (BitmapSuccess != XReadBitmapFile(dpy, owlWindow,
						   filenames,
						   &owlWidth, &owlHeight,
						   &owlBitmaps[owlNumBitmaps],
						   &scratch, &scratch))
		return; /* abort */

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
}
