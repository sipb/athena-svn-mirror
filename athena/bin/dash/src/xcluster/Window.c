/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/xcluster/Window.c,v $
 * $Author: cfields $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#ifndef	lint
static char rcsid[] =
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/xcluster/Window.c,v 1.1 1992-04-29 13:55:50 cfields Exp $";
#endif	lint

#include "mit-copyright.h"
#include <stdio.h>
#include "Jets.h"
#include "Window.h"
#include <X11/Xutil.h>
#include <X11/Xatom.h>

static Boolean event_handler();
static void initialize(), prerealize(), realize(),
  querysize(), move(), resize(), destroy();

static Boolean got_wm_delete = 0;
static Atom wm_delete_window;

extern int DEBUG;

#define offset(field) XjOffset(WindowJet,field)

static XjResource resources[] = {
  { XjNx, XjCX, XjRInt, sizeof(int),
     offset(core.x), XjRString, XjDefaultValue },
  { XjNy, XjCY, XjRInt, sizeof(int),
     offset(core.y), XjRString, XjDefaultValue },
  { XjNwidth, XjCWidth, XjRInt, sizeof(int),
     offset(core.width), XjRString, XjDefaultValue },
  { XjNheight, XjCHeight, XjRInt, sizeof(int),
     offset(core.height), XjRString, XjDefaultValue },
  { XjNbackground, XjCBackground, XjRColor, sizeof(int),
      offset(window.background), XjRString, XjDefaultBackground },
  { XjNforeground, XjCForeground, XjRColor, sizeof(int),
      offset(window.foreground), XjRString, XjDefaultForeground },
  { XjNreverseVideo, XjCReverseVideo, XjRBoolean, sizeof(Boolean),
      offset(window.reverseVideo), XjRBoolean, (caddr_t)False },
  { XjNbackgroundPixmap, XjCBackgroundPixmap, XjRPixmap, sizeof(XjPixmap *),
      offset(window.pixmap), XjRString, NULL },
  { XjNborderColor, XjCBorderColor, XjRColor, sizeof(int),
      offset(window.borderColor), XjRColor, (caddr_t)-1 },
  { XjNborderWidth, XjCBorderWidth, XjRInt, sizeof(int),
     offset(window.borderWidth), XjRString, "1" },
  { XjNtitle, XjCTitle, XjRString, sizeof(char *),
     offset(window.title), XjRString, "No name" },
  { XjNgeometry, XjCGeometry, XjRString, sizeof(char *),
     offset(window.geometry), XjRString, (char *)NULL },
  { XjNdefGeometry, XjCGeometry, XjRString, sizeof(char *),
     offset(window.defGeometry), XjRString, (char *) NULL },
  { XjNminWidth, XjCMinWidth, XjRInt, sizeof(int),
     offset(window.minWidth), XjRInt, 0 },
  { XjNminHeight, XjCMinHeight, XjRInt, sizeof(int),
     offset(window.minHeight), XjRInt, 0 },
  { XjNoverrideRedirect, XjCOverrideRedirect, XjRBoolean, sizeof(Boolean),
     offset(window.overrideRedirect), XjRBoolean, (caddr_t)False },
  { XjNrootTransient, XjCRootTransient, XjRBoolean, sizeof(Boolean),
     offset(window.rootTransient), XjRBoolean, (caddr_t)False },
  { XjNcursorCode, XjCCursorCode, XjRInt, sizeof(int),
     offset(window.cursorCode), XjRString, XjDefaultValue },
  { XjNmapped, XjCMapped, XjRBoolean, sizeof(Boolean),
     offset(window.mapped), XjRBoolean, (caddr_t)True },
  { XjNiconic, XjCIconic, XjRBoolean, sizeof(Boolean),
     offset(window.iconic), XjRBoolean, (caddr_t)False },
  { XjNforceNWGravity, XjCForceNWGravity, XjRBoolean, sizeof(Boolean),
      offset(window.forceNWGravity), XjRBoolean, (caddr_t) False },
  { XjNdeleteProc, XjCDeleteProc, XjRCallback, sizeof(XjCallback *),
     offset(window.deleteProc), XjRString, NULL },
  { XjNclientMessageProc, XjCClientMessageProc, XjRCallback,
     sizeof(XjCallback *), offset(window.clientMessageProc), XjRString, NULL },
  { XjNmapNotifyProc, XjCMapNotifyProc, XjRCallback, sizeof(XjCallback *),
     offset(window.mapNotifyProc), XjRString, NULL },
  { XjNiconWindow, XjCIconWindow, XjRJet, sizeof(Jet),
     offset(window.iconWindow), XjRJet, NULL }
};

#undef offset

WindowClassRec windowClassRec = {
  {
    /* class name */	"Window",
    /* jet size   */	sizeof(WindowRec),
    /* initialize */	initialize,
    /* prerealize */    prerealize,
    /* realize */	realize,
    /* event */		event_handler,
    /* expose */	NULL,
    /* querySize */	querysize,
    /* move */		move,
    /* resize */	resize,
    /* destroy */	destroy,
    /* resources */	resources,
    /* number of 'em */	XjNumber(resources)
  }
};

JetClass windowJetClass = (JetClass)&windowClassRec;

static void querysize(me, size)
     WindowJet me;
     XjSize *size;
{
  if (DEBUG)
    printf ("QS(window)");

  if (me->core.child)
    XjQuerySize(me->core.child, size);
}

static void move(me, x, y)
     WindowJet me;
     int x, y;
{
  if (DEBUG)
    printf ("MV(window)");

  me->core.x = x;
  me->core.y = y;

  XMoveWindow(me->core.display, me->core.window, x, y
	      /*me->core.x, me->core.y*/);
}

static void resize(me, size)
     WindowJet me;
     XjSize *size;
{
  Jet child;

  if (DEBUG)
    printf ("RSZ(window)");

  me->core.width = (size->width) ? size->width : 1;
  me->core.height = (size->height) ? size->height : 1;

  /* child should be informed by getting a ConfigureNotify */
  XResizeWindow(me->core.display, me->core.window,
		me->core.width, me->core.height);

  for (child = me->core.child; child != NULL; child = child->core.sibling)
    XjResize(child, size);
}

static void initialize(me)
     WindowJet me;
{
  if (!got_wm_delete)
    {
      got_wm_delete = True;
      wm_delete_window = XInternAtom(me->core.display,
				     "WM_DELETE_WINDOW",
				     False);
    }
}

static void prerealize(me)
     WindowJet me;
{
  XjSize size;
  char defString[30];
  int mask;
  int ret_x, ret_y, ret_width, ret_height, ret_gravity;

  /*
   * Geometry... this is gross, but you've gotta do what you've gotta do...
   */
  me->core.borderWidth = me->window.borderWidth;

  if (me->core.x != -1)
    me->window.sizeHints.x = me->core.x;
  else
    me->window.sizeHints.x = 0;

  if (me->core.y != -1)
    me->window.sizeHints.y = me->core.y;
  else
    me->window.sizeHints.y = 0;

  if (me->core.child != NULL)
    XjQuerySize(me->core.child, &size);

  if (me->core.width != -1)
    me->window.sizeHints.width = me->core.width;
  else
    if (size.width != -1)
      me->window.sizeHints.width = size.width;
    else
      me->window.sizeHints.width = 100;

  if (me->core.height != -1)
    me->window.sizeHints.height = me->core.height;
  else
    if (size.height != -1)
      me->window.sizeHints.height = size.height;
    else
      me->window.sizeHints.height = 100;

  me->window.sizeHints.flags = PPosition | PSize;

  if (me->window.geometry != NULL)
    {
      /* I think this call is screwed - as far as I can tell, it
	 ignores sizeHints.{width,height} */
      /* so... */
      if (me->window.defGeometry != NULL  &&
	  strcmp(me->window.defGeometry, ""))
	mask = XWMGeometry(me->core.display,
			   DefaultScreen(me->core.display),
			   me->window.geometry,
			   me->window.defGeometry,
			   me->window.borderWidth,
			   &me->window.sizeHints,
			   &ret_x, &ret_y,
			   &ret_width, &ret_height,
			   &ret_gravity);
      else
	{
	  sprintf(defString, "%dx%d+%d+%d",
		  me->window.sizeHints.width,
		  me->window.sizeHints.height,
		  me->window.sizeHints.x,
		  me->window.sizeHints.y);

	  mask = XWMGeometry(me->core.display,
			     DefaultScreen(me->core.display),
			     me->window.geometry,
			     defString,
			     me->window.borderWidth,
			     &me->window.sizeHints,
			     &ret_x, &ret_y,
			     &ret_width, &ret_height,
			     &ret_gravity);
	}

      if (me->window.forceNWGravity)
	ret_gravity = NorthWestGravity;

      me->window.sizeHints.flags |= PWinGravity;

      if (mask & (XValue | YValue))
	me->window.sizeHints.flags |= USPosition;

      if (mask & (WidthValue | HeightValue))
	me->window.sizeHints.flags |= USSize;

      me->window.sizeHints.x = ret_x;
      me->window.sizeHints.y = ret_y;

/*      if (ret_width != 1) *//* this is not the right way, but I can't */
	me->window.sizeHints.width = ret_width;
/*      if (ret_height != 1) *//* figure the right way from the docs */
	me->window.sizeHints.height = ret_height;

      me->window.sizeHints.win_gravity = ret_gravity;
    }

  me->core.x = me->window.sizeHints.x;
  me->core.y = me->window.sizeHints.y;
  me->core.width = me->window.sizeHints.width;
  if(me->core.width <= 0)
    me->core.width = 32;
  me->core.height = me->window.sizeHints.height;
  if (me->core.height <= 0)
    me->core.height = 32;

  me->window.sizeHints.min_width = me->window.minWidth;
  me->window.sizeHints.min_height = me->window.minHeight;
  me->window.sizeHints.flags |= PMinSize;
}

static void realize(me)
     WindowJet me;
{
  Window parentwindow;
  Pixmap p = NULL;
  unsigned long valuemask;
  XGCValues values;
  GC gc;
/*  int background; */
  int attribsMask;
  XTextProperty name;
  XWMHints wmHints;
  XClassHint classHints;
  XSetWindowAttributes attribs;
  XjSize size;
  int temp;

/*  background = me->window.background; */
  parentwindow = me->core.window;

  if (me->window.reverseVideo)
    {
      temp = me->window.foreground;
      me->window.foreground = me->window.background;
      me->window.background = temp;
    }

  /*
   * Title...
   */
  name.encoding = XA_STRING;
  name.format = 8;
  name.value = (unsigned char *) me->window.title;
  name.nitems = strlen(me->window.title);

  /*
   * WM hints
   */
  wmHints.flags = StateHint;
  if (me->window.iconic)
    wmHints.initial_state = IconicState;
  else
    wmHints.initial_state = (me->window.mapped == 1) ?
      NormalState : WithdrawnState;

  if (me->window.iconWindow != NULL)
    {
      wmHints.flags |= IconWindowHint;
      wmHints.icon_window = me->window.iconWindow->core.window;

      if (((WindowJet)me->window.iconWindow)->
	  window.sizeHints.flags & USPosition)
	{
	  wmHints.flags |= IconPositionHint;
	  wmHints.icon_x = me->window.iconWindow->core.x;
	  wmHints.icon_y = me->window.iconWindow->core.y;
	}
    }

  /*
   * Class hints
   */
  classHints.res_name = programName;
  classHints.res_class = programClass;

  /*
   * Attributes
   */
  attribsMask = CWOverrideRedirect;
  attribs.override_redirect = me->window.overrideRedirect;

  attribs.background_pixel = me->window.background;
  attribsMask |= CWBackPixel;

  attribs.border_pixel = (me->window.borderColor == -1) ?
    me->window.foreground : me->window.borderColor;
  attribsMask |= CWBorderPixel;

  if (me->window.pixmap != NULL)
    { /* this is a mighty annoying pain in the rear */
      values.function = GXcopy;
      values.foreground = me->window.foreground;
      values.background = me->window.background;
      valuemask = GCForeground | GCBackground | GCFunction;

      gc = XCreateGC(me->core.display,
		     me->core.window,
		     valuemask,
		     &values);

      p = XCreatePixmap(me->core.display,
			parentwindow,
			me->window.pixmap->width,
			me->window.pixmap->height,
			DefaultDepth(me->core.display, /* wrong... sigh. */
				     DefaultScreen(me->core.display)));

      XCopyPlane(me->core.display,
		 me->window.pixmap->pixmap,
		 p,
		 gc,
		 0, 0,
		 me->window.pixmap->width,
		 me->window.pixmap->height,
		 0, 0, 1);

      XFreeGC(me->core.display, gc);

      attribs.background_pixmap = p;
      attribsMask |= CWBackPixmap;
      attribsMask &= ~CWBackPixel;
    }

  me->core.window = XCreateWindow(me->core.display,
				  parentwindow,
				  me->core.x, me->core.y,
				  me->core.width,
				  me->core.height,
				  me->window.borderWidth,
				  CopyFromParent,
				  InputOutput,
				  CopyFromParent,
				  attribsMask,
				  &attribs);

  if (me->window.pixmap != NULL)
    XFreePixmap(me->core.display, p);

  XSetWMProperties(me->core.display,
		   me->core.window,
		   &name, &name,
		   global_argv, global_argc,
		   &me->window.sizeHints,
		   &wmHints,
		   &classHints);

  if (me->window.rootTransient)
    XSetTransientForHint(me->core.display, 
			 me->core.window,
			 DefaultRootWindow(me->core.display));
 
  (void)XSetWMProtocols(me->core.display,
			me->core.window,
			&wm_delete_window,
			1);

  XjRegisterWindow(me->core.window, (Jet) me);
  XjSelectInput(me->core.display, me->core.window,
		ExposureMask | StructureNotifyMask | VisibilityChangeMask);
  if (me->window.mapped || me->window.iconic) /*trust this to be true?*/
    XMapWindow(me->core.display, me->core.window);

  if (me->window.cursorCode != -1)
    {
      me->window.cursor = XCreateFontCursor(me->core.display,
					    me->window.cursorCode);
      XDefineCursor(me->core.display, me->core.window, me->window.cursor);
    }
  else
    me->window.cursor = NULL;

  if (me->core.child != NULL)
    {
      if (me->core.child->core.x == -1)
	XjMove(me->core.child, 0, 0);

      size.width = me->core.width;
      size.height = me->core.height;
      XjResize(me->core.child, &size);
    }

  me->window.visibility = VisibilityFullyObscured;
}

static void destroy(me)
     WindowJet me;
{
  XjUnregisterWindow(me->core.window, (Jet) me);
  XDestroyWindow(me->core.display, me->core.window);

  if (me->window.cursor != NULL)
    XFreeCursor(me->core.display, me->window.cursor);
}

#define X1  event->xexpose.x
#define Y1  event->xexpose.y
#define X2  event->xexpose.x + event->xexpose.width
#define Y2  event->xexpose.y + event->xexpose.height
#define X3  child->core.x
#define Y3  child->core.y
#define X4  child->core.x + child->core.width
#define Y4  child->core.y + child->core.height
#define OVERLAP(x1,y1,x2,y2, x3,y3,x4,y4) \
  ( (MAX(x1,x3) <= MIN(x2,x4)) && (MAX(y1,y3) <= MIN(y2,y4)) )

static Boolean event_handler(me, event)
     WindowJet me;
     XEvent *event;
{
  WindowInfo info;
  Jet child;

  switch(event->type)
    {
    case GraphicsExpose:
    case Expose:
      if (DEBUG)
	printf("Window:event_handler  expose_count: %d\n",
	       event->xexpose.count);

      for (child = me->core.child; child != (Jet) NULL;
	   child = child->core.sibling)
	{
	  if (DEBUG)
	    {
	      printf("name: %s  need: %d  overlap: %d\n",
		     child->core.name, child->core.need_expose,
		     OVERLAP(X1,Y1,X2,Y2, X3,Y3,X4,Y4));
	      printf("exp: %d,%d %d,%d  jet: %d,%d %d,%d\n",
		     X1,Y1,X2,Y2, X3,Y3,X4,Y4);
	    }
	  if (!child->core.need_expose
	      && child->core.classRec->core_class.expose != NULL
	      && OVERLAP(X1,Y1,X2,Y2, X3,Y3,X4,Y4))
	    {
	      child->core.need_expose = True;
	    }
	}

      if (!event->xexpose.count)
	{
	  for (child = me->core.child; child != (Jet) NULL;
	       child = child->core.sibling)
	    if (child->core.need_expose
		&& child->core.classRec->core_class.expose != NULL)
	      {
		child->core.classRec->core_class.expose(child, event);
		child->core.need_expose = False;
	      }
	}
      break;

    case MapNotify:
      me->window.mapped = True;
      XjCallCallbacks((caddr_t) me, me->window.mapNotifyProc, event);
      break;

    case UnmapNotify:
      me->window.mapped = False;
      break;

    case VisibilityNotify:
      me->window.visibility = event->xvisibility.state;
      break;

    case ConfigureNotify:
      me->core.x = event->xconfigure.x;
      me->core.y = event->xconfigure.y;
      if (event->xconfigure.width != me->core.width ||
	  event->xconfigure.height != me->core.height)
	{
	  XjSize size;

	  size.width = event->xconfigure.width;
	  size.height = event->xconfigure.height;
	  XjResize((Jet) me, &size);
	}
      break;

    case ClientMessage:
      if (event->xclient.data.l[0] == wm_delete_window)
	{
	  XjCallCallbacks((caddr_t) me, me->window.deleteProc, event);
	  break;
	}
      else
	{
	  info.window = me;
	  info.event = event;
	  XjCallCallbacks((caddr_t) &info,
			  me->window.clientMessageProc, event);
	}
      break;

    default:
      return False;
    }
  return True;
}

void UnmapWindow(me)
     WindowJet me;
{
  XEvent event;

  XUnmapWindow(me->core.display, me->core.window);

  event.type = UnmapNotify;
  event.xunmap.event = DefaultRootWindow(me->core.display);
  event.xunmap.window = me->core.window;
  event.xunmap.from_configure = False;

  if (!XSendEvent(me->core.display,
		  DefaultRootWindow(me->core.display),
		  False,
		  SubstructureRedirectMask | SubstructureNotifyMask,
		  &event))
    XjWarning("window: SendEvent failed");
}

void MapWindow(me, raise)
     WindowJet me;
     Boolean raise;
{
  XWMHints wmHints;

  /* check for iconic and don't bother if so */
  wmHints.flags = StateHint;
  wmHints.initial_state = NormalState;

  if (me->window.iconWindow != NULL)
    {
      wmHints.flags |= IconWindowHint;
      wmHints.icon_window = me->window.iconWindow->core.window;
      if (((WindowJet)me->window.iconWindow)->
	  window.sizeHints.flags & USPosition)
	{
	  wmHints.flags |= IconPositionHint;
	  wmHints.icon_x = me->window.iconWindow->core.x;
	  wmHints.icon_y = me->window.iconWindow->core.y;
	}
    }

  XSetWMHints(me->core.display, me->core.window, &wmHints);

  if (!raise)
    XMapWindow(me->core.display, me->core.window);
  else
    XMapRaised(me->core.display, me->core.window);
}

Boolean WindowMapped(me)
     WindowJet me;
{
  return (Boolean)me->window.mapped;
}

int WindowVisibility(me)
     WindowJet me;
{
  return me->window.visibility;
}
