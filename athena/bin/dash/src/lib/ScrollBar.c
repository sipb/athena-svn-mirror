/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/ScrollBar.c,v $
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
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/ScrollBar.c,v 1.1 1991-09-03 11:09:13 vanharen Exp $";
#endif	lint

#include "mit-copyright.h"
#include <stdio.h>
#include "Jets.h"
#include "ScrollBar.h"
#include <X11/cursorfont.h>

#define offset(field) XjOffset(ScrollBarJet,field)

static XjResource resources[] = {
  { XjNchangeProc, XjCChangeProc, XjRCallback, sizeof(XjCallback *),
      offset(scrollBar.changeProc), XjRString, NULL },
  { XjNx, XjCX, XjRInt, sizeof(int),
      offset(core.x), XjRString, XjInheritValue },
  { XjNy, XjCY, XjRInt, sizeof(int),
      offset(core.y), XjRString, XjInheritValue },
  { XjNwidth, XjCWidth, XjRInt, sizeof(int),
      offset(core.width), XjRString, "15" },
  { XjNheight, XjCHeight, XjRInt, sizeof(int),
      offset(core.height), XjRString, "100" },
  { XjNborderWidth, XjCBorderWidth, XjRInt, sizeof(int),
      offset(scrollBar.borderWidth), XjRString, "1" },
  { XjNborderThickness, XjCBorderThickness, XjRInt, sizeof(int),
      offset(scrollBar.borderThickness), XjRString, "1" },
  { XjNborderColor, XjCBorderColor, XjRColor, sizeof(int),
      offset(scrollBar.borderColor), XjRColor, (caddr_t)-1 },
  { XjNpadding, XjCPadding, XjRInt, sizeof(int),
      offset(scrollBar.padding), XjRString, "5" },
  { XjNreverseVideo, XjCReverseVideo, XjRBoolean, sizeof(Boolean),
      offset(scrollBar.reverseVideo), XjRBoolean, (caddr_t) False },
  { XjNforeground, XjCForeground, XjRColor, sizeof(int),
      offset(scrollBar.foreground), XjRString, XjDefaultForeground },
  { XjNbackground, XjCBackground, XjRColor, sizeof(int),
      offset(scrollBar.background), XjRString, XjDefaultBackground },
  { XjNthumb, XjCThumb, XjRPixmap, sizeof(XjPixmap *),
      offset(scrollBar.thumb), XjRString, NULL },
  { XjNverticalCursorCode, XjCCursorCode, XjRInt, sizeof(int),
     offset(scrollBar.vertCursorCode), XjRInt,
      (caddr_t) XC_sb_v_double_arrow },
  { XjNhorizontalCursorCode, XjCCursorCode, XjRInt, sizeof(int),
     offset(scrollBar.horizCursorCode), XjRInt,
      (caddr_t) XC_sb_h_double_arrow },
  { XjNupCursorCode, XjCCursorCode, XjRInt, sizeof(int),
     offset(scrollBar.upCursorCode), XjRInt, (caddr_t) XC_sb_up_arrow },
  { XjNdownCursorCode, XjCCursorCode, XjRInt, sizeof(int),
     offset(scrollBar.downCursorCode), XjRInt, (caddr_t) XC_sb_down_arrow },
  { XjNleftCursorCode, XjCCursorCode, XjRInt, sizeof(int),
     offset(scrollBar.leftCursorCode), XjRInt, (caddr_t) XC_sb_left_arrow },
  { XjNrightCursorCode, XjCCursorCode, XjRInt, sizeof(int),
     offset(scrollBar.rightCursorCode), XjRInt, (caddr_t) XC_sb_right_arrow },
  { XjNorientation, XjCOrientation, XjROrientation, sizeof(int),
     offset(scrollBar.orientation), XjRString, XjVertical },
  { XjNshowArrows, XjCShowArrows, XjRBoolean, sizeof(Boolean),
      offset(scrollBar.showArrows), XjRBoolean, (caddr_t) False },
};

#undef offset

static Boolean event_handler();
static void realize(), resize(), querySize(),
  move(), destroy(), initialize();

ScrollBarClassRec scrollBarClassRec = {
  {
    /* class name */	"ScrollBar",
    /* jet size   */	sizeof(ScrollBarRec),
    /* initialize */	initialize,
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

JetClass scrollBarJetClass = (JetClass)&scrollBarClassRec;

static void calcPos(me)
     ScrollBarJet me;
{
  int range, pixrange, visible, pixvisible;

  range = me->scrollBar.maximum - me->scrollBar.minimum + 1;
  visible = MIN(me->scrollBar.visible, range);

  /*
     want to scale range into pixrange, but it may not be the
     transformation we really expect due to the minimum size
     constraint on the scrollbar thumb. So, the mapping we
     want is:
     		(a.) normalize
		(b.) 0 -> 0
		(c.) (range - 1) - visible + 1 -> height - pixvisible
   */

  if (me->scrollBar.orientation == Vertical)
    {
      pixrange = me->core.height - 2 * me->scrollBar.borderWidth;
      pixvisible = me->scrollBar.thumbHeight;
      me->scrollBar.thumbY = me->core.y + me->scrollBar.borderWidth;

      if (range - visible != 0)
	me->scrollBar.thumbY +=
	  (me->scrollBar.current - me->scrollBar.minimum) *
	    (pixrange - pixvisible) / (range - visible);
    }
  else
    {
      pixrange = me->core.width - 2 * me->scrollBar.borderWidth;
      pixvisible = me->scrollBar.thumbWidth;
      me->scrollBar.thumbX = me->core.x + me->scrollBar.borderWidth;

      if (range - visible != 0)
	me->scrollBar.thumbX +=
	  (me->scrollBar.current - me->scrollBar.minimum) *
	    (pixrange - pixvisible) / (range - visible);
    }
}

static int calcCurrent(me, button, thumbX, thumbY)
     ScrollBarJet me;
     unsigned int button;
     int thumbX, thumbY;
{
  int range, pixrange, visible, pixvisible;
  int current, increment;

  range = me->scrollBar.maximum - me->scrollBar.minimum + 1;
  visible = MIN(me->scrollBar.visible, range);

  if (me->scrollBar.orientation == Vertical)
    {
      pixrange = me->core.height - 2 * me->scrollBar.borderWidth;
      pixvisible = me->scrollBar.thumbHeight;
    }
  else
    {
      pixrange = me->core.width - 2 * me->scrollBar.borderWidth;
      pixvisible = me->scrollBar.thumbWidth;
    }
  if (pixrange < 1)
    pixrange = 1;

  switch(button)
    {
    case Button1:
    case Button3:
      current = me->scrollBar.current;
      if (me->scrollBar.orientation == Vertical)
	increment = 1 + visible *
	  (thumbY - me->core.y - me->scrollBar.borderWidth)
	    / pixrange;
      else
	increment = 1 + visible *
	  (thumbX - me->core.x - me->scrollBar.borderWidth)
	    / pixrange;

      if (button == Button1)
	current += increment;
      else
	current -= increment;
      break;

    case Button2:
      current = me->scrollBar.minimum;
      if (pixrange - pixvisible != 0)
	{
	  if (me->scrollBar.orientation == Vertical)
	    increment = (thumbY - me->core.y - me->scrollBar.borderWidth) *
	      (range - visible) / (pixrange - pixvisible);
	  else
	    increment = (thumbX - me->core.x - me->scrollBar.borderWidth) *
	      (range - visible) / (pixrange - pixvisible);
	  current += increment;
	}
      break;
    }

  return current;
}

static void initSizes(me)
     ScrollBarJet me;
{
  int range, pixrange, visible;
  int size;

  /* range of possible values */
  range = me->scrollBar.maximum - me->scrollBar.minimum + 1;
  if (me->scrollBar.orientation == Vertical)
    pixrange = me->core.height - 2 * me->scrollBar.borderWidth;
  else
    pixrange = me->core.width - 2 * me->scrollBar.borderWidth;

  /* how much of range can be seen */
  visible = MIN(me->scrollBar.visible, range);

  size = MAX(pixrange * visible / range, 5);

  if (me->scrollBar.orientation == Vertical)
    {
      me->scrollBar.thumbHeight = size;
      me->scrollBar.thumbWidth = (me->core.width
				  - 2 * me->scrollBar.borderWidth);
      me->scrollBar.thumbX = me->core.x + me->scrollBar.borderWidth;
    }
  else
    {
      me->scrollBar.thumbWidth = size;
      me->scrollBar.thumbHeight = (me->core.height
				  - 2 * me->scrollBar.borderWidth);
      me->scrollBar.thumbY = me->core.y + me->scrollBar.borderWidth;
    }
}

static void initialize(me)
     ScrollBarJet me;
{
  me->scrollBar.realized = 0;

  me->scrollBar.inside = 0;
  me->scrollBar.selected = 0;
  me->scrollBar.pressed = 0;
  me->scrollBar.minimum = 0;
  me->scrollBar.maximum = 10;
  me->scrollBar.current = 5;
  me->scrollBar.visible = 2;
}

/*
 * Things are currently broken screenwise.
 * It will be fun to fix later. :)
 */
static void realize(me)
     ScrollBarJet me;
{
  unsigned long valuemask;
  XGCValues values;
  int swap;

  if (me->scrollBar.reverseVideo)
    {
      swap = me->scrollBar.foreground;
      me->scrollBar.foreground = me->scrollBar.background;
      me->scrollBar.background = swap;
    }

  values.function = GXcopy;

  if (me->scrollBar.borderThickness != 0)
    {
      values.foreground = (me->scrollBar.borderColor == -1) ?
	me->scrollBar.foreground : me->scrollBar.borderColor;
      values.line_width = me->scrollBar.borderThickness;
      valuemask = GCForeground | GCLineWidth | GCFunction;

      me->scrollBar.line_gc = XCreateGC(me->core.display,
					me->core.window,
					valuemask,
					&values);
    }

  values.foreground = (me->scrollBar.borderColor == -1) ?
    me->scrollBar.foreground : me->scrollBar.borderColor;
  values.background = me->scrollBar.foreground;
  values.line_width = 0;
  valuemask = GCForeground | GCBackground | GCLineWidth | GCFunction;

  me->scrollBar.bg_gc = XCreateGC(me->core.display,
				  me->core.window,
				  valuemask,
				  &values);

  values.foreground = me->scrollBar.background;

  me->scrollBar.fg_gc = XCreateGC(me->core.display,
				  me->core.window,
				  valuemask,
				  &values);

  valuemask = GCForeground | GCBackground | GCFunction;

  me->scrollBar.thumb_gc = XCreateGC(me->core.display,
				     me->core.window,
				     valuemask,
				     &values);

  if (me->scrollBar.thumb != NULL)
    { /* this is a mighty annoying pain in the rear */
      /* destroy p... check also Window.c */
      Pixmap p;

      p = XCreatePixmap(me->core.display,
			me->core.window,
			me->scrollBar.thumb->width,
			me->scrollBar.thumb->height,
			DefaultDepth(me->core.display, /* wrong... sigh. */
				     DefaultScreen(me->core.display)));

      XCopyPlane(me->core.display,
		 me->scrollBar.thumb->pixmap,
		 p,
		 me->scrollBar.thumb_gc,
		 0, 0,
		 me->scrollBar.thumb->width,
		 me->scrollBar.thumb->height,
		 0, 0, 1);

      values.fill_style = FillTiled;
      values.tile = p;
      valuemask = GCTile | GCFillStyle;

      XChangeGC(me->core.display,
		me->scrollBar.thumb_gc,
		valuemask,
		&values);
    }

  if (me->scrollBar.upCursorCode != -1)
    me->scrollBar.upCursor = XCreateFontCursor(me->core.display,
					       me->scrollBar.upCursorCode);
  else
    me->scrollBar.upCursor = NULL;

  if (me->scrollBar.downCursorCode != -1)
    me->scrollBar.downCursor = XCreateFontCursor(me->core.display,
						 me->scrollBar.downCursorCode);
  else
    me->scrollBar.downCursor = NULL;

  if (me->scrollBar.leftCursorCode != -1)
    me->scrollBar.leftCursor = XCreateFontCursor(me->core.display,
						 me->scrollBar.leftCursorCode);
  else
    me->scrollBar.leftCursor = NULL;

  if (me->scrollBar.rightCursorCode != -1)
    me->scrollBar.rightCursor = XCreateFontCursor(me->core.display,
						 me->scrollBar.rightCursorCode);
  else
    me->scrollBar.rightCursor = NULL;

  if (me->scrollBar.vertCursorCode != -1)
    me->scrollBar.vertCursor = XCreateFontCursor(me->core.display,
						 me->scrollBar.vertCursorCode);
  else
    me->scrollBar.vertCursor = NULL;

  if (me->scrollBar.horizCursorCode != -1)
    me->scrollBar.horizCursor = XCreateFontCursor(me->core.display,
						me->scrollBar.horizCursorCode);
  else
    me->scrollBar.horizCursor = NULL;


  XDefineCursor(me->core.display, me->core.window,
		(me->scrollBar.orientation == Vertical)
		? me->scrollBar.vertCursor
		: me->scrollBar.horizCursor);

  /*
   * Usurp events for this window
   */
  XjRegisterWindow(me->core.window, me);
  XjSelectInput(me->core.display, me->core.window,
		ExposureMask | StructureNotifyMask |
		ButtonPressMask | ButtonReleaseMask | Button2MotionMask);

  me->scrollBar.realized = 1;

  initSizes(me);
  calcPos(me);
}

static void destroy(me)
     ScrollBarJet me;
{
  XFreeGC(me->core.display, me->scrollBar.thumb_gc);
  if (me->scrollBar.borderThickness != 0)
    XFreeGC(me->core.display, me->scrollBar.line_gc);
  if (me->scrollBar.vertCursor != NULL)
    XFreeCursor(me->core.display, me->scrollBar.vertCursor);
  if (me->scrollBar.horizCursor != NULL)
    XFreeCursor(me->core.display, me->scrollBar.horizCursor);
  if (me->scrollBar.upCursor != NULL)
    XFreeCursor(me->core.display, me->scrollBar.upCursor);
  if (me->scrollBar.downCursor != NULL)
    XFreeCursor(me->core.display, me->scrollBar.downCursor);
  if (me->scrollBar.rightCursor != NULL)
    XFreeCursor(me->core.display, me->scrollBar.rightCursor);
  if (me->scrollBar.leftCursor != NULL)
    XFreeCursor(me->core.display, me->scrollBar.leftCursor);

  XjUnregisterWindow(me->core.window, me);
}

static void querySize(me, size)
     ScrollBarJet me;
     XjSize *size;
{
  size->width = me->core.width;
  size->height = me->core.height;
}

static void move(me, x, y)
     ScrollBarJet me;
     int x, y;
{
  me->core.x = x;
  me->core.y = y;

  if (me->core.child != NULL)
    XjMove(me->core.child,
	   me->core.x + me->scrollBar.borderWidth +
	   me->scrollBar.borderThickness + me->scrollBar.padding,
	   me->core.y + me->scrollBar.borderWidth +
	   me->scrollBar.borderThickness + me->scrollBar.padding);
}

static void resize(me, size)
     ScrollBarJet me;
     XjSize *size;
{
  me->core.width = size->width;
  me->core.height = size->height;

  if (me->scrollBar.realized)
    {
      initSizes(me);
      calcPos(me);
    }
}

/*
 * clearAround clears the area around the scrollbar's thumb if the thumb
 * is "warped" somewhere with SetScrollBar.
 */
static void clearAround(me)
     ScrollBarJet me;
{
  int x1, x2, y1, y2, w1, w2, h1, h2;

  if (me->scrollBar.orientation == Vertical)
    {
      x1 = x2 = me->scrollBar.thumbX;
      y1 = me->core.y;
      y2 = me->scrollBar.thumbY + me->scrollBar.thumbHeight;
      w1 = w2 = me->scrollBar.thumbWidth;
      h1 = me->scrollBar.thumbY - me->core.y;
      h2 = me->core.height - me->scrollBar.thumbY + me->scrollBar.thumbHeight;
    }
  else
    {
      x1 = me->core.x;
      x2 = me->scrollBar.thumbX + me->scrollBar.thumbWidth;
      y1 = y2 = me->scrollBar.thumbY;
      w1 = me->scrollBar.thumbX - me->core.x;
      w2 = me->core.width - me->scrollBar.thumbX + me->scrollBar.thumbWidth;
      h1 = h2 = me->scrollBar.thumbHeight;
    }

  XClearArea(me->core.display, me->core.window, x1, y1, w1, h1, False);

  XClearArea(me->core.display, me->core.window, x2, y2, w2, h2, False);
}

/*
 * drawThumb draws the thumb at its current position (whenever there is
 * an expose or if the thumb moves.
 */
static void drawThumb(me)
     ScrollBarJet me;
{
  if (me->scrollBar.borderThickness != 0)
    {
      XDrawRectangle(me->core.display,
		     me->core.window,
		     me->scrollBar.line_gc,
		     me->scrollBar.thumbX + me->scrollBar.borderThickness/2,
		     me->scrollBar.thumbY + me->scrollBar.borderThickness/2,
		     me->scrollBar.thumbWidth -
		     1 - me->scrollBar.borderThickness / 2,
		     me->scrollBar.thumbHeight -
		     1 - me->scrollBar.borderThickness / 2);
    }

  XFillRectangle(me->core.display,
		 me->core.window,
		 me->scrollBar.thumb_gc,
		 me->scrollBar.thumbX + me->scrollBar.borderThickness,
		 me->scrollBar.thumbY + me->scrollBar.borderThickness,
		 me->scrollBar.thumbWidth -
		 (2 * me->scrollBar.borderThickness),
		 me->scrollBar.thumbHeight -
		 (2 * me->scrollBar.borderThickness));
}


static void drawArrows(me)
     ScrollBarJet me;
{
  int w, h, x1, y1, x2, y2;
  int lx1, lx2, ly1, ly2;

  if (me->scrollBar.orientation == Vertical)
    {
      lx2 = w = me->core.width;
      ly1 = ly2 = h = me->scrollBar.arrowSize;
      lx1 = x1 = y1 = 0;
    }
  else
    {
      w = me->core.width;
      h = me->scrollBar.arrowSize;
      x1 = y1 = 0;
    }

  XFillRectangle(me->core.display, me->core.window, me->scrollBar.fg_gc,
		 x1, y1, w, h);

  XDrawLine(me->core.display, me->core.window, me->scrollBar.bg_gc,
	    lx1, ly1, lx2, ly2);

  XFillRectangle(me->core.display, me->core.window, me->scrollBar.fg_gc,
		 0, me->core.height - me->core.width,
		 me->core.width, me->core.width);

  XDrawLine(me->core.display,
	    me->core.window,
	    me->scrollBar.bg_gc,
	    0, me->core.height - me->core.width,
	    me->core.width, me->core.height - me->core.width);
}


static void drawScrollbar(me)
     ScrollBarJet me;
{
  drawThumb(me);
  if (me->scrollBar.showArrows && me->scrollBar.arrowSize != 0)
    drawArrows(me);
}


static void clear(me)
     ScrollBarJet me;
{
  XClearWindow(me->core.display, me->core.window);
}

static void moveThumb(me, current) /* needs orientation! */
     ScrollBarJet me;
     int current;
{
  int oldPos, newPos;
  int range, visible;

  range = me->scrollBar.maximum - me->scrollBar.minimum + 1;
  visible = MIN(me->scrollBar.visible, range);

  if (current < me->scrollBar.minimum)
    current = me->scrollBar.minimum;
  else
    if (current > (me->scrollBar.maximum - visible + 1))
      current = me->scrollBar.maximum - visible + 1;

  if (current == me->scrollBar.current)
    return;

  oldPos = (me->scrollBar.orientation == Vertical)
    ? me->scrollBar.thumbY
      : me->scrollBar.thumbX;

  me->scrollBar.current = current;
  calcPos(me);
  drawScrollbar(me);

  if (me->scrollBar.orientation == Vertical)
    {
      if (oldPos < me->scrollBar.thumbY)
	XClearArea(me->core.display,
		   me->core.window,
		   me->scrollBar.thumbX,
		   oldPos,
		   me->scrollBar.thumbWidth,
		   me->scrollBar.thumbY - oldPos,
		   False);
      else
	XClearArea(me->core.display,
		   me->core.window,
		   me->scrollBar.thumbX,
		   me->scrollBar.thumbY + me->scrollBar.thumbHeight,
		   me->scrollBar.thumbWidth,
		   oldPos - me->scrollBar.thumbY,
		   False);
    }
  else
    {
      if (oldPos < me->scrollBar.thumbX)
	XClearArea(me->core.display,
		   me->core.window,
		   oldPos,
		   me->scrollBar.thumbY,
		   me->scrollBar.thumbX - oldPos,
		   me->scrollBar.thumbHeight,
		   False);
      else
	XClearArea(me->core.display,
		   me->core.window,
		   me->scrollBar.thumbX + me->scrollBar.thumbWidth,
		   me->scrollBar.thumbY,
		   oldPos - me->scrollBar.thumbX,
		   me->scrollBar.thumbHeight,
		   False);
    }

  XjCallCallbacks(me, me->scrollBar.changeProc, NULL);
}
/*==========*/
static Boolean event_handler(me, event)
     ScrollBarJet me;
     XEvent *event;
{
  XEvent tmp;
  int range, pixrange, visible;
  int current;

  switch(event->type)
    {
    case GraphicsExpose:
    case Expose:
      if (event->xexpose.count != 0)
	break;

      drawScrollbar(me);
      break;

    case ConfigureNotify:
      if (event->xconfigure.width != me->core.width ||
	  event->xconfigure.height != me->core.height)
	{
	  XjSize size;

	  size.width = event->xconfigure.width;
	  size.height = event->xconfigure.height;
	  XjResize(me, &size);
	  clear(me);
	  drawScrollbar(me);
	}
      break;

    case ButtonPress:
      switch(event->xbutton.button)
	{
	case Button1:
	  XDefineCursor(me->core.display, me->core.window,
			(me->scrollBar.orientation == Vertical)
			? me->scrollBar.upCursor
			: me->scrollBar.leftCursor);
	  break;

	case Button2:
	  current = calcCurrent(me, Button2,
				event->xbutton.x, event->xbutton.y);
	  XDefineCursor(me->core.display, me->core.window,
			(me->scrollBar.orientation == Vertical)
			? me->scrollBar.leftCursor
			: me->scrollBar.upCursor);
	  moveThumb(me, current);
	  break;

	case Button3:
	  XDefineCursor(me->core.display, me->core.window,
			(me->scrollBar.orientation == Vertical)
			? me->scrollBar.downCursor
			: me->scrollBar.rightCursor);
	  break;
	}
      break;

    case MotionNotify:
      if (XPending(me->core.display))
	{
	  XPeekEvent(me->core.display, &tmp);
	  if (tmp.type == MotionNotify && tmp.xany.window == me->core.window)
	    break; /* ignore this event */
	}

      current = calcCurrent(me, Button2, event->xmotion.x, event->xmotion.y);

      moveThumb(me, current);
      break;

    case ButtonRelease:
      current = calcCurrent(me, event->xbutton.button,
			    event->xbutton.x, event->xbutton.y);
      moveThumb(me, current);
      XDefineCursor(me->core.display, me->core.window,
		    (me->scrollBar.orientation == Vertical)
		    ? me->scrollBar.vertCursor
		    : me->scrollBar.horizCursor);
      break;

    default:
      return False;
    }
  return True;
}

void SetScrollBar(me, min, max, visible, value)
     ScrollBarJet me;
     int min, max, visible, value;
{
  me->scrollBar.minimum = min;
  me->scrollBar.maximum = max;
  me->scrollBar.visible = visible;
  me->scrollBar.current = value;

  if (me->scrollBar.realized)
    {
      initSizes(me);
      calcPos(me);
      clearAround(me);
      drawScrollbar(me);
    }
}

int GetScrollBarValue(me)
     ScrollBarJet me;
{
  return me->scrollBar.current;
}
