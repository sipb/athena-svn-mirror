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

#ifndef	lint
static char rcsid[] =
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/Button.c,v 1.1 1991-09-02 21:35:26 vanharen Exp $";
#endif	lint

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
      offset(button.reverseVideo), XjRBoolean, (caddr_t)False },
  { XjNtoggle, XjCToggle, XjRBoolean, sizeof(Boolean),
      offset(button.toggle), XjRBoolean, (caddr_t)False },
  { XjNstate, XjCState, XjRBoolean, sizeof(Boolean),
      offset(button.state), XjRBoolean, (caddr_t)False },
};

#undef offset

static Boolean event_handler();
static void realize(), resize(), querySize(),
  move(), destroy();

ButtonClassRec buttonClassRec = {
  {
    /* class name */	"Button",
    /* jet size   */	sizeof(ButtonRec),
    /* initialize */	NULL,
    /* prerealize */    NULL,
    /* realize */	realize,
    /* event */		event_handler,
    /* expose */	NULL,
    /* querySize */     querySize,
    /* move */		move,
    /* resize */        resize,
    /* destroy */       destroy,
    /* resources */	resources,
    /* number of 'em */	XjNumber(resources)
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
  valuemask = GCForeground | GCBackground | GCLineWidth | GCFunction;

  me->button.foreground_gc = XCreateGC(me->core.display,
				       me->core.window,
				       valuemask,
				       &values);

  values.foreground = me->button.background;

  me->button.background_gc = XCreateGC(me->core.display,
				       me->core.window,
				       valuemask,
				       &values);

  values.foreground = me->button.foreground;
  values.line_width = me->button.borderWidth;

  me->button.gc = XCreateGC(me->core.display,
			    me->core.window,
			    valuemask,
			    &values);

  values.function = GXxor;
  values.foreground = (me->button.foreground ^
		       me->button.background);

  me->button.invert_gc = XCreateGC(me->core.display,
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
}

static void destroy(me)
     ButtonJet me;
{
  XFreeGC(me->core.display, me->button.gc);
  XFreeGC(me->core.display, me->button.invert_gc);
  XFreeGC(me->core.display, me->button.foreground_gc);
  XFreeGC(me->core.display, me->button.background_gc);

  XjUnregisterWindow(me->core.window, (Jet) me);
}

static void querySize(me, size)
     ButtonJet me;
     XjSize *size;
{
  if (DEBUG)
    printf ("QS(button)");

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
    printf ("MV(button)");

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
    printf ("RSZ(button)");

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
  XDrawRectangle(me->core.display, me->core.window,
		 (foreground) ?
		   me->button.foreground_gc : me->button.background_gc,
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
      XFillRectangle(me->core.display, me->core.window,
		     me->button.invert_gc,
		     me->button.borderWidth + me->button.borderThickness,
		     me->button.borderWidth + me->button.borderThickness,
		     me->core.width - 2 * (me->button.borderWidth +
					   me->button.borderThickness),
		     me->core.height - 2 * (me->button.borderWidth +
					    me->button.borderThickness));
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
	if (me->core.child->core.classRec->core_class.expose != NULL)
	  me->core.child->core.classRec->core_class.expose(me->core.child,
							   event);

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

      if (event->type == ButtonRelease  &&
	  me->button.pressed == True   &&
          me->button.inside == True)
	{
	  if (me->button.toggle)
	    {
	      me->button.state = !(me->button.state);
	      if (me->button.state)
		XjCallCallbacks((caddr_t) me, me->button.activateProc, event);
	      else
		XjCallCallbacks((caddr_t) me, me->button.deactivateProc,
				event);
	    }
	  else
	    XjCallCallbacks((caddr_t) me, me->button.activateProc, event);
	}

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
