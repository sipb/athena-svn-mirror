/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/Button.c,v $
 * $Author: vanharen $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#if  (!defined(lint))  &&  (!defined(SABER))
static char *rcsid =
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/Button.c,v 1.4 1993-07-02 01:31:09 vanharen Exp $";
#endif

#include "mit-copyright.h"
#include <stdio.h>
#include "Jets.h"
#include "Button.h"

extern int DEBUG;

#define offset(field) XjOffset(ButtonJet,field)

static XjResource resources[] = {
  { XjNactivateProc, XjCActivateProc, XjRCallback, sizeof(XjCallback *),
      offset(button.activateProc), XjRString, NULL },
  { XjNdeactivateProc, XjCDeactivateProc, XjRCallback, sizeof(XjCallback *),
      offset(button.deactivateProc), XjRString, NULL },
  { XjNx, XjCX, XjRInt, sizeof(int),
      offset(core.x), XjRString, XjInheritValue },
  { XjNy, XjCY, XjRInt, sizeof(int),
      offset(core.y), XjRString, XjInheritValue },
  { XjNwidth, XjCWidth, XjRInt, sizeof(int),
      offset(core.width), XjRString, XjInheritValue },
  { XjNheight, XjCHeight, XjRInt, sizeof(int),
      offset(core.height), XjRString, XjInheritValue },
  { XjNborderWidth, XjCBorderWidth, XjRInt, sizeof(int),
      offset(button.borderWidth), XjRString, "1" },
  { XjNborderThickness, XjCBorderThickness, XjRInt, sizeof(int),
      offset(button.borderThickness), XjRString, "2" },
  { XjNpadding, XjCPadding, XjRInt, sizeof(int),
      offset(button.padding), XjRString, "5" },
  { XjNforeground, XjCForeground, XjRColor, sizeof(int),
      offset(button.foreground), XjRString, XjDefaultForeground },
  { XjNbackground, XjCBackground, XjRColor, sizeof(int),
      offset(button.background), XjRString, XjDefaultBackground },
  { XjNreverseVideo, XjCReverseVideo, XjRBoolean, sizeof(Boolean),
      offset(button.reverseVideo), XjRBoolean, (caddr_t) False },
  { XjNtoggle, XjCToggle, XjRBoolean, sizeof(Boolean),
      offset(button.toggle), XjRBoolean, (caddr_t) False },
  { XjNstate, XjCState, XjRBoolean, sizeof(Boolean),
      offset(button.state), XjRBoolean, (caddr_t) False },
  { XjNrepeatDelay, XjCInterval, XjRInt, sizeof(int),
      offset(button.repeatDelay), XjRString, "0" },
  { XjNinitialDelay, XjCInterval, XjRInt, sizeof(int),
      offset(button.initialDelay), XjRString, "0" },
  { XjNhighlightOnEnter, XjCHighlightOnEnter, XjRBoolean, sizeof(Boolean),
      offset(button.highlightOnEnter), XjRBoolean, (caddr_t) True },
};

#undef offset

static Boolean event_handler();
static void realize(), resize(), querySize(),
  move(), destroy(), wakeup();

ButtonClassRec buttonClassRec = {
  {
    /* class name */		"Button",
    /* jet size   */		sizeof(ButtonRec),
    /* classInitialize */	NULL,
    /* classInitialized? */	1,
    /* initialize */		NULL,
    /* prerealize */    	NULL,
    /* realize */		realize,
    /* event */			event_handler,
    /* expose */		NULL,
    /* querySize */     	querySize,
    /* move */			move,
    /* resize */        	resize,
    /* destroy */       	destroy,
    /* resources */		resources,
    /* number of 'em */		XjNumber(resources)
  }
};

JetClass buttonJetClass = (JetClass)&buttonClassRec;


/*
 * Things are currently broken screenwise.
 * It will be fun to fix later. :)
 */
static void realize(me)
     ButtonJet me;
{
  unsigned long valuemask;
  XGCValues values;

  if (me->button.reverseVideo)
    {
      int temp;

      temp = me->button.foreground;
      me->button.foreground = me->button.background;
      me->button.background = temp;
    }

  values.function = GXcopy;
  values.foreground = me->button.foreground;
  values.background = me->button.background;
  values.line_width = me->button.borderThickness;
  values.graphics_exposures = False;
  values.cap_style = CapProjecting;
  valuemask = ( GCForeground | GCBackground | GCLineWidth
	       | GCFunction | GCGraphicsExposures | GCCapStyle );

  me->button.foreground_gc = XjCreateGC(me->core.display,
					me->core.window,
					valuemask,
					&values);

  values.foreground = me->button.background;

  me->button.background_gc = XjCreateGC(me->core.display,
					me->core.window,
					valuemask,
					&values);

  values.foreground = me->button.foreground;
  values.line_width = me->button.borderWidth;

  me->button.gc = XjCreateGC(me->core.display,
			     me->core.window,
			     valuemask,
			     &values);

  values.function = GXxor;
  values.foreground = (me->button.foreground ^
		       me->button.background);

  me->button.invert_gc = XjCreateGC(me->core.display,
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

  me->button.inside = 0;
  me->button.selected = 0;
  me->button.pressed = 0;


  if (me->core.child)
    {
      XjVaGetValues(me->core.child,
		    XjNhighlightProc, &me->button.hilite,
		    XjNunHighlightProc, &me->button.unhilite,
		    NULL, NULL);
    }
}


static void destroy(me)
     ButtonJet me;
{
  XjFreeGC(me->core.display, me->button.gc);
  XjFreeGC(me->core.display, me->button.invert_gc);
  XjFreeGC(me->core.display, me->button.foreground_gc);
  XjFreeGC(me->core.display, me->button.background_gc);

  XjUnregisterWindow(me->core.window, (Jet) me);
}


static void querySize(me, size)
     ButtonJet me;
     XjSize *size;
{
  if (DEBUG)
    printf ("QS(button) '%s'\n", me->core.name);

  if (me->core.child != NULL)
    XjQuerySize(me->core.child, size);

  if (me->core.child == NULL || size->width == -1)
    size->width = size->height = 0;

  size->width += (me->button.borderWidth +
		  me->button.borderThickness + me->button.padding) * 2;
  size->height += (me->button.borderWidth +
		   me->button.borderThickness + me->button.padding) * 2;
}


static void move(me, x, y)
     ButtonJet me;
     int x, y;
{
  if (DEBUG)
    printf ("MV(button) '%s' x=%d,y=%d\n", me->core.name, x, y);

  me->core.x = x;
  me->core.y = y;

  if (me->core.child != NULL)
    XjMove(me->core.child,
	   me->core.x + me->button.borderWidth +
	   me->button.borderThickness + me->button.padding,
	   me->core.y + me->button.borderWidth +
	   me->button.borderThickness + me->button.padding);
}


static void resize(me, size)
     ButtonJet me;
     XjSize *size;
{
  XjSize childSize;

  if (DEBUG)
    printf ("RS(button) '%s' w=%d,h=%d\n", me->core.name,
	    size->width, size->height);

  me->core.width = size->width;
  me->core.height = size->height;

  if (me->core.child != NULL)
    {
      childSize.width = size->width -
	(me->button.borderWidth +
	 me->button.borderThickness + me->button.padding) * 2;
      childSize.height = size->height -
	(me->button.borderWidth +
	 me->button.borderThickness + me->button.padding) * 2;
      XjResize((Jet) me->core.child, &childSize);
    }
}


static void outline(me, foreground)
     ButtonJet me;
     Boolean foreground;
{
  if (me->button.highlightOnEnter)
    XDrawRectangle(me->core.display, me->core.window,
		   ((foreground) ?
		    me->button.foreground_gc : me->button.background_gc),
		   me->button.borderWidth + me->button.borderThickness / 2,
		   me->button.borderWidth + me->button.borderThickness / 2,
		   me->core.width - (2 * me->button.borderWidth) -
		   1 - me->button.borderThickness / 2,
		   me->core.height - (2 * me->button.borderWidth) -
		   1 - me->button.borderThickness / 2);
}


static void frame(me)
     ButtonJet me;
{
  if (me->button.borderWidth != 0)
    XDrawRectangle(me->core.display, me->core.window, me->button.gc,
		   me->button.borderWidth/2,
		   me->button.borderWidth/2,
		   me->core.width -
		   1 - me->button.borderWidth / 2,
		   me->core.height -
		   1 - me->button.borderWidth / 2);
}


/* make sure we don't mess with select() */
static void btn_select(me, flag)
     ButtonJet me;
     Boolean flag;
{
  if (flag != me->button.selected)
    {
      if (me->button.hilite)
	if (flag)
	  XjCallCallbacks((caddr_t) me->core.child, me->button.hilite, NULL);
      if (me->button.unhilite)
	if (!flag)
	  XjCallCallbacks((caddr_t) me->core.child, me->button.unhilite, NULL);

      if (!me->button.hilite)
	XFillRectangle(me->core.display, me->core.window,
		       me->button.invert_gc,
		       me->button.borderWidth + me->button.borderThickness,
		       me->button.borderWidth + me->button.borderThickness,
		       me->core.width - 2 * (me->button.borderWidth +
					     me->button.borderThickness),
		       me->core.height - 2 * (me->button.borderWidth +
					      me->button.borderThickness));
      me->button.selected = flag;
    }
}


static void btn_invert(me, flag)
     ButtonJet me;
     Boolean flag;
{
  if (flag)
    {
      if (me->button.hilite)
	XjCallCallbacks((caddr_t) me->core.child, me->button.hilite, NULL);
      else
	XFillRectangle(me->core.display, me->core.window,
		       me->button.invert_gc,
		       me->button.borderWidth + me->button.borderThickness,
		       me->button.borderWidth + me->button.borderThickness,
		       me->core.width - 2 * (me->button.borderWidth +
					     me->button.borderThickness),
		       me->core.height - 2 * (me->button.borderWidth +
					      me->button.borderThickness));
    }

  else
    if (me->button.unhilite)
      XjCallCallbacks((caddr_t) me->core.child, me->button.unhilite, NULL);
}


static void wakeup(me, id)
     ButtonJet me;
     int id;
{
  Window junkwin;
  int junk;
  unsigned int mask;

  XQueryPointer(XjDisplay(me), XjWindow(me),
		&junkwin, &junkwin, &junk, &junk, &junk, &junk, &mask);

  if (me->button.inside == True  &&
      (mask & Button1Mask ||
       mask & Button2Mask ||
       mask & Button3Mask ||
       mask & Button4Mask ||
       mask & Button5Mask))
    {
      XjCallCallbacks((caddr_t) me, me->button.activateProc, NULL);
      me->button.timerid = XjAddWakeup(wakeup, me, me->button.repeatDelay);
    }
}


static Boolean event_handler(me, event)
     ButtonJet me;
     XEvent *event;
{
  switch(event->type)
    {
    case GraphicsExpose:
    case Expose:
      if (event->xexpose.count != 0)
	break;

      XClearWindow(me->core.display, me->core.window);

      /* we can deal properly with a single child... */
      if (me->core.child != NULL)
	XjExpose(me->core.child, event);
#ifdef notdefined
	if (me->core.child->core.classRec->core_class.expose != NULL)
	  me->core.child->core.classRec->core_class.expose(me->core.child,
							   event);
#endif

      frame(me);
      if (me->button.toggle)
	{
	  if (me->button.inside)
	    {
	      outline(me, !me->button.state);
	      if (me->button.pressed)
		btn_invert(me, me->button.state);
	      else
		btn_invert(me, !me->button.state);
	    }
	  else
	    {
	      outline(me, me->button.state);
	      if (me->button.pressed)
		btn_invert(me, !me->button.state);
	      else
		btn_invert(me, me->button.state);
	    }
	}
      else if (me->button.inside)
	{
	  outline(me, True);
	  btn_invert(me, me->button.pressed);
	}
      break;

    case EnterNotify:
      if (me->button.inside == 1)
	break;
      me->button.inside = 1;
      if (me->button.toggle)
	{
	  outline(me, !me->button.state);
	  if (me->button.pressed)
	    btn_select(me, !me->button.state);
	}
      else
	{
	  outline(me, True);
	  btn_select(me, me->button.pressed);
	  if (me->button.pressed == True  &&
	      me->button.repeatDelay > 0)
	    {
	      XjCallCallbacks((caddr_t) me, me->button.activateProc, NULL);
	      me->button.timerid = XjAddWakeup(wakeup, me,
					       me->button.repeatDelay);
	    }
	}
      break;

    case LeaveNotify:
      if (me->button.inside == 0)
	break;
      me->button.inside = 0;
      if (me->button.toggle)
	{
	  outline(me, me->button.state);
	  btn_select(me, me->button.state);
	}
      else
	{
	  outline(me, False);
	  btn_select(me, False);
	}
      if (me->button.timerid)
	(void)XjRemoveWakeup(me->button.timerid);
      break;
 
    case ConfigureNotify:
      if (event->xconfigure.width != me->core.width ||
	  event->xconfigure.height != me->core.height)
	{
	  XjSize size;

	  size.width = event->xconfigure.width;
	  size.height = event->xconfigure.height;
	  XjResize((Jet) me, &size);

	  /*
	  me->core.width = event->xconfigure.width;
	  me->core.height = event->xconfigure.height;

	  if (me->core.child != NULL)
	    {
	      me->core.child->core.width = me->core.width;
	      me->core.child->core.height = me->core.height;
	    }
	    */
	}
      break;

    case ButtonPress:
    case ButtonRelease:
      if (me->button.toggle)
	{
	  if (event->type == ButtonPress)
	    btn_select(me, !me->button.state);
	  else
	    outline(me, me->button.state);
	}
      else
	btn_select(me, event->type == ButtonPress);

      if (event->type == ButtonPress  &&
          me->button.inside == True  &&
	  me->button.repeatDelay > 0)
	{
	  XjCallCallbacks((caddr_t) me,
			  me->button.activateProc,
			  (caddr_t) event);
	  me->button.timerid = XjAddWakeup(wakeup, me,
					   ((me->button.initialDelay > 0)
					    ? me->button.initialDelay
					    : me->button.repeatDelay));
	}

      if (event->type == ButtonRelease  &&
	  me->button.pressed == True   &&
          me->button.inside == True)
	{
	  if (me->button.toggle)
	    {
	      me->button.state = !(me->button.state);
	      if (me->button.state)
		XjCallCallbacks((caddr_t) me,
				me->button.activateProc,
				(caddr_t) event);
	      else
		XjCallCallbacks((caddr_t) me,
				me->button.deactivateProc,
				(caddr_t) event);
	    }
	  else
	    if (me->button.repeatDelay <= 0)
	      XjCallCallbacks((caddr_t) me,
			      me->button.activateProc,
			      (caddr_t) event);
	}

      if (event->type == ButtonRelease  &&
	  me->button.timerid)
	(void)XjRemoveWakeup(me->button.timerid);

      me->button.pressed = (event->type == ButtonPress);
      break;

    default:
      return False;
    }
  return True;
}


void SetToggleState(me, bool, callcallback)
     ButtonJet me;
     Boolean bool, callcallback;
{
  if (me->button.inside)
    outline(me, !bool);
  else
    outline(me, bool);

  if (me->button.state != bool)
    btn_select(me, bool);

  me->button.state = bool;

  if (callcallback)
    if (bool)
      XjCallCallbacks((caddr_t) me, me->button.activateProc, NULL);
    else
      XjCallCallbacks((caddr_t) me, me->button.deactivateProc, NULL);
}

Boolean GetToggleState(me)
     ButtonJet me;
{
  return (me->button.state);
}
