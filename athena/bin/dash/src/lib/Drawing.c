/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/Drawing.c,v $
 * $Author: vanharen $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#if  (!defined(lint))  &&  (!defined(SABER))
static char rcsid[] =
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/Drawing.c,v 1.4 1993-07-02 01:24:13 vanharen Exp $";
#endif

#include "mit-copyright.h"
#include <stdio.h>
#include "Jets.h"
#include "Drawing.h"

extern int DEBUG;

#define offset(field) XjOffset(DrawingJet,field)

static XjResource resources[] = {
  { XjNexposeProc, XjCExposeProc, XjRCallback, sizeof(XjCallback *),
      offset(drawing.exposeProc), XjRString, NULL },
  { XjNrealizeProc, XjCRealizeProc, XjRCallback, sizeof(XjCallback *),
      offset(drawing.realizeProc), XjRString, NULL },
  { XjNresizeProc, XjCResizeProc, XjRCallback, sizeof(XjCallback *),
      offset(drawing.resizeProc), XjRString, NULL },
  { XjNdestroyProc, XjCDestroyProc, XjRCallback, sizeof(XjCallback *),
      offset(drawing.destroyProc), XjRString, NULL },
  { XjNeventProc, XjCEventProc, XjRCallback, sizeof(XjCallback *),
      offset(drawing.eventProc), XjRString, NULL },
  { XjNx, XjCX, XjRInt, sizeof(int),
      offset(core.x), XjRString, XjInheritValue },
  { XjNy, XjCY, XjRInt, sizeof(int),
      offset(core.y), XjRString, XjInheritValue },
  { XjNwidth, XjCWidth, XjRInt, sizeof(int),
      offset(core.width), XjRString, XjInheritValue },
  { XjNheight, XjCHeight, XjRInt, sizeof(int),
      offset(core.height), XjRString, XjInheritValue },
  { XjNlineWidth, XjCLineWidth, XjRInt, sizeof(int),
      offset(drawing.lineWidth), XjRString, "0" },
  { XjNforeground, XjCForeground, XjRColor, sizeof(int),
      offset(drawing.foreground), XjRString, XjDefaultForeground },
  { XjNbackground, XjCBackground, XjRColor, sizeof(int),
      offset(drawing.background), XjRString, XjDefaultBackground },
  { XjNreverseVideo, XjCReverseVideo, XjRBoolean, sizeof(Boolean),
      offset(drawing.reverseVideo), XjRBoolean, (caddr_t)False },
  { XjNfont, XjCFont, XjRFontStruct, sizeof(XFontStruct *),
      offset(drawing.font), XjRString, XjDefaultFont }
};

#undef offset

static Boolean event_handler();
static void realize(), querySize(), destroy();

DrawingClassRec drawingClassRec = {
  {
    /* class name */		"Drawing",
    /* jet size   */		sizeof(DrawingRec),
    /* classInitialize */	NULL,
    /* classInitialized? */	1,
    /* initialize */		NULL,
    /* prerealize */    	NULL,
    /* realize */		realize,
    /* event */			event_handler,
    /* expose */		NULL,
    /* querySize */     	querySize,
    /* move */			NULL,
    /* resize */        	NULL,
    /* destroy */       	destroy,
    /* resources */		resources,
    /* number of 'em */		XjNumber(resources)
  }
};

JetClass drawingJetClass = (JetClass)&drawingClassRec;

/*
 * Things are currently broken screenwise.
 * It will be fun to fix later. :)
 */
static void realize(me)
     DrawingJet me;
{
  unsigned long valuemask;
  XGCValues values;

  if (me->drawing.reverseVideo)
    {
      int temp;

      temp = me->drawing.foreground;
      me->drawing.foreground = me->drawing.background;
      me->drawing.background = temp;
    }

  values.function = GXcopy;
  values.foreground = me->drawing.foreground;
  values.background = me->drawing.background;
  values.line_width = me->drawing.lineWidth;
  values.font = me->drawing.font->fid;
  values.graphics_exposures = False;
  valuemask = ( GCForeground | GCBackground | GCLineWidth | GCFunction
	       | GCFont | GCGraphicsExposures );

  me->drawing.foreground_gc = XjCreateGC(me->core.display,
					 me->core.window,
					 valuemask,
					 &values);

  values.foreground = me->drawing.background;

  me->drawing.background_gc = XjCreateGC(me->core.display,
					 me->core.window,
					 valuemask,
					 &values);

  values.function = GXxor;
  values.foreground = (me->drawing.foreground ^
		       me->drawing.background);

  me->drawing.invert_gc = XjCreateGC(me->core.display,
				     me->core.window,
				     valuemask,
				     &values);
  /*
   * Usurp events for this window
   */
  XjRegisterWindow(me->core.window, (Jet) me);
  XjSelectInput(me->core.display, me->core.window,
		ExposureMask | EnterWindowMask | LeaveWindowMask |
		StructureNotifyMask | ButtonPressMask | ButtonReleaseMask);
  me->core.width = me->core.parent->core.width;
  me->core.height = me->core.parent->core.height;
  XjCallCallbacks((caddr_t) me, me->drawing.resizeProc, NULL);
}

static void destroy(me)
     DrawingJet me;
{
  XjFreeGC(me->core.display, me->drawing.invert_gc);
  XjFreeGC(me->core.display, me->drawing.foreground_gc);
  XjFreeGC(me->core.display, me->drawing.background_gc);

  XjUnregisterWindow(me->core.window, (Jet) me);
}

static void querySize(me, size)
     DrawingJet me;
     XjSize *size;
{
  size->width = me->core.width;
  size->height = me->core.height;
}

static Boolean event_handler(me, event)
     DrawingJet me;
     XEvent *event;
{
  switch(event->type)
    {
    case GraphicsExpose:
    case Expose:
      if (event->xexpose.count != 0)
	break;
      XjCallCallbacks((caddr_t) me, me->drawing.exposeProc, event);
      break;

    case ConfigureNotify:
      if (me->core.width != event->xconfigure.width  ||
	  me->core.height != event->xconfigure.height)
	{
	  me->core.width = event->xconfigure.width;
	  me->core.height = event->xconfigure.height;
	  XjCallCallbacks((caddr_t) me, me->drawing.resizeProc, event);
	}
      break;

    case ButtonPress:
    case ButtonRelease:
      XjCallCallbacks((caddr_t) me, me->drawing.eventProc, event);
      break;

    default:
      return False;
    }
  return True;
}
