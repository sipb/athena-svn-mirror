/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/List.c,v $
 * $Author: probe $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#if  (!defined(lint))  &&  (!defined(SABER))
static char *rcsid =
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/List.c,v 1.1 1993-10-12 05:49:22 probe Exp $";
#endif

#include "mit-copyright.h"
#include "Jets.h"
#include "List.h"
#include "xselect.h"

#define offset(field) XjOffset(ListJet,field)

static XjResource resources[] = {
  { XjNx, XjCX, XjRInt, sizeof(int),
     offset(core.x), XjRString, XjInheritValue },
  { XjNy, XjCY, XjRInt, sizeof(int),
     offset(core.y), XjRString, XjInheritValue },
  { XjNwidth, XjCWidth, XjRInt, sizeof(int),
     offset(core.width), XjRString, XjInheritValue },
  { XjNheight, XjCHeight, XjRInt, sizeof(int),
     offset(core.height), XjRString, XjInheritValue },
  { XjNforeground, XjCForeground, XjRColor, sizeof(int),
      offset(list.foreground), XjRString, XjDefaultForeground },
  { XjNbackground, XjCBackground, XjRColor, sizeof(int),
      offset(list.background), XjRString, XjDefaultBackground },
  { XjNreverseVideo, XjCReverseVideo, XjRBoolean, sizeof(Boolean),
      offset(list.reverseVideo), XjRBoolean, (caddr_t) False },
  { XjNresizeProc, XjCResizeProc, XjRCallback, sizeof(XjCallback *),
     offset(list.resizeProc), XjRString, NULL },
  { XjNscrollProc, XjCScrollProc, XjRCallback, sizeof(XjCallback *),
      offset(list.scrollProc), XjRString, NULL },
  { XjNinternalBorder, XjCBorderWidth, XjRInt, sizeof(int),
     offset(list.internalBorder), XjRString, "2" },
  { XjNmultiClickTime, XjCMultiClickTime, XjRInt, sizeof(int),
      offset(list.multiClickTime), XjRString, "250" },
  { XjNscrollDelay, XjCScrollDelay, XjRInt, sizeof(int),
      offset(list.scrollDelay1), XjRString, "100" },
  { XjNscrollDelay2, XjCScrollDelay, XjRInt, sizeof(int),
      offset(list.scrollDelay2), XjRString, "50" },
  { XjNlistMode, XjCListMode, XjRListMode, sizeof(int),
      offset(list.listMode), XjRString, XjSingleSelect },
  { XjNautoSelect, XjCAutoSelect, XjRBoolean, sizeof(Boolean),
      offset(list.autoSelect), XjRBoolean, (caddr_t) False },
  { XjNalwaysOne, XjCAlwaysOne, XjRBoolean, sizeof(Boolean),
      offset(list.alwaysOne), XjRBoolean, (caddr_t) False },
};

#undef offset

static void class_init(), realize(), querySize(), move(), destroy(), resize();
static Boolean event_handler();
static int StrToListMode();

ListClassRec listClassRec = {
  {
    /* class name */		"List",
    /* jet size   */		sizeof(ListRec),
    /* classInitialize */	class_init,
    /* classInitialized? */	0,
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

JetClass listJetClass = (JetClass)&listClassRec;


/*
 * String to ListMode conversion
 */
static int StrToListMode(display, window, where, resource, type, address)
     Display *display;
     Window window;
     caddr_t where;
     XjResource *resource;
     char *type;
     caddr_t address;
{
  int retval = -1;
  int mode = SingleSelect;

  if (strcasecmp(address, XjSingleSelect) == 0)
    {
      mode = SingleSelect;
      retval = 0;
    }

  if (strcasecmp(address, XjMultiSelect) == 0)
    {
      mode = MultiSelect;
      retval = 0;
    }

  if (retval)
    {
      char errtext[100];
      sprintf(errtext, "bad list mode value `%s', using `SingleSelect'",
	      address);
      XjWarning(errtext);
    }

  *((int *)((char *)where + resource->resource_offset)) = mode;
  return retval;
}


static void class_init(me)
     ListJet me;
{
  XjRegisterCallback(StrToListMode, XjRListMode);
}


static void realize(me)
     ListJet me;
{
  Jet child;
  XjSize child_size;
  int x, y;
  unsigned long valuemask;
  XGCValues values;

  /*
   * Usurp events for this window
   */
  XjRegisterWindow(me->core.window, (Jet) me);
  XjSelectInput(me->core.display, me->core.window,
		ExposureMask | EnterWindowMask | LeaveWindowMask |
		StructureNotifyMask | ButtonPressMask |
		ButtonReleaseMask | ButtonMotionMask);

  x = me->core.x + me->list.internalBorder;
  y = me->core.y + me->list.internalBorder;
  for (child = me->core.child; child != NULL; child = child->core.sibling)
    {
      XjMove(child, x, y);
      XjQuerySize(child, &child_size);
      y += child_size.height + me->list.internalBorder;
    }

  if (me->list.reverseVideo)
    {
      int temp;

      temp = me->list.foreground;
      me->list.foreground = me->list.background;
      me->list.background = temp;
    }

  values.function = GXxor;
  values.foreground = (me->list.foreground ^ me->list.background);
  values.background = me->list.background;
  values.graphics_exposures = False;
  valuemask = ( GCForeground | GCBackground
	       | GCFunction | GCGraphicsExposures );

  me->list.invert_gc = XjCreateGC(me->core.display,
				  me->core.window,
				  valuemask,
				  &values);

  values.function = GXcopy;
  values.foreground = me->list.background;
  values.line_width = me->list.internalBorder;
  values.cap_style = CapProjecting;
  valuemask = ( GCForeground | GCLineWidth
	       | GCFunction | GCGraphicsExposures | GCCapStyle );

  me->list.background_gc = XjCreateGC(me->core.display,
				      me->core.window,
				      valuemask,
				      &values);

  values.foreground = me->list.foreground;

  me->list.foreground_gc = XjCreateGC(me->core.display,
				      me->core.window,
				      valuemask,
				      &values);
  me->list.focus = me->core.child;
  if (me->list.alwaysOne)
    me->list.selected = me->core.child;
  else
    me->list.selected = NULL;
}

static void destroy(me)
     ListJet me;
{
  XjFreeGC(me->core.display, me->list.invert_gc);
  XjFreeGC(me->core.display, me->list.background_gc);
  XjFreeGC(me->core.display, me->list.foreground_gc);

  XjUnregisterWindow(me->core.window, (Jet) me);
}

static void querySize(me, size)
     ListJet me;
     XjSize *size;
{
  Jet child;
  XjSize child_size;

  size->width = size->height = me->list.internalBorder;

  for (child = me->core.child; child != NULL; child = child->core.sibling)
    {
      XjQuerySize(child, &child_size);
      size->width += child_size.width + me->list.internalBorder;
      size->height += child_size.height + me->list.internalBorder;
    }
}

static void move(me, new_x, new_y)
     ListJet me;
     int new_x, new_y;
{
  int x, y;
  Jet child;
   
  me->core.x = new_x;
  me->core.y = new_y;

  x = me->core.x + me->list.internalBorder;
  y = me->core.y + me->list.internalBorder;
  for (child = me->core.child; child != NULL; child = child->core.sibling)
    {
      XjMove(child, x, y);
      y += child->core.height + me->list.internalBorder;
    }
}

static void resize(me, size)
     ListJet me;
     XjSize *size;
{
  Jet child;
  XjSize child_size;

  me->core.width = size->width;
  me->core.height = size->height;
  for (child = me->core.child; child != NULL; child = child->core.sibling)
    {
      XjQuerySize(child, &child_size);
      child_size.width = me->core.width - 2 * me->list.internalBorder;
      XjResize(child, child_size);
    }
}


Jet where(me, x, y, line)
     ListJet me;
     int x, y;
     int *line;
{
  Jet child = NULL;
  int count=1;

  for (child = me->core.child; child != NULL; child = child->core.sibling)
    {
      if (y >= child->core.y - me->list.internalBorder
	  &&  y <= child->core.y + child->core.height
	  + me->list.internalBorder)
	break;
      count++;
    }

  *line = count;
  return child;
}

static Boolean event_handler(me, event)
     ListJet me;
     XEvent *event;
{
  int tmp;
  char *oldStart, *oldEnd;
  int length;
  Jet jet, child;
  XjSize child_size;
  Jet newSelect = NULL;

  switch(event->type)
    {
    case EnterNotify:
      jet = me->list.focus;
      tmp = me->list.internalBorder/2;
      XDrawRectangle(me->core.display, me->core.window,
		     me->list.foreground_gc,
		     jet->core.x - tmp,
		     jet->core.y - tmp,
		     jet->core.width + tmp + 1,
		     jet->core.height + tmp + 1);
      break;

    case LeaveNotify:
      jet = me->list.focus;
      tmp = me->list.internalBorder/2;
      XDrawRectangle(me->core.display, me->core.window,
		     me->list.background_gc,
		     jet->core.x - tmp,
		     jet->core.y - tmp,
		     jet->core.width + tmp + 1,
		     jet->core.height + tmp + 1);
      break;

    case GraphicsExpose:
    case Expose:
      if (event->xexpose.count != 0)
	break;

      XClearWindow(me->core.display, me->core.window);

      for (child = me->core.child; child != NULL; child = child->core.sibling)
	XjExpose(child, event);
      if (me->list.selected != NULL)
	{
	  Jet jet = me->list.selected;
	  XFillRectangle(me->core.display, me->core.window,
			 me->list.invert_gc,
			 jet->core.x, jet->core.y,
			 jet->core.width, jet->core.height);
	}
      break;

    case ButtonRelease:
      if (me->list.buttonDown == False)
	break;
      me->list.buttonDown = False;

      if (event->xbutton.button == Button1)
	me->list.lastUpTime = event->xbutton.time;
      break;

    case ButtonPress:
      if (me->list.buttonDown == True)
	break;

      /* MUST compute clickTimes before calling "where" below, as the */
      /* return values from it depend on the value of clickTime... */

      if (event->xbutton.button == Button1)
	{
	  if (me->list.clickTimes != 0
	      && event->xbutton.time - me->list.lastUpTime
	      < me->list.multiClickTime)
	    me->list.clickTimes++;
	  else
	    me->list.clickTimes = 1;
	}

      /* deal with button3 being first one pushed... */
      if (event->xbutton.button == Button3
	  &&  me->list.clickTimes == 0)
	me->list.clickTimes = 1;

      me->list.buttonDown = True;
      me->list.whichButton = event->xbutton.button;
      jet = where(me, event->xbutton.x, event->xbutton.y, &tmp);

      printf("s_s=%d  sel=%d  jet=%d  alw=%d\n",
	     !me->list.listMode,
	     me->list.selected,
	     jet, me->list.alwaysOne);

      /*  Erase old selection, IF:
       *   - we are in singleSelect mode
       *   - there is an old selection
       *   - new selection does not equal the old one
       *   - we are NOT is alwaysOne mode AND selection is non-null
       */
      if (me->list.listMode == SingleSelect
	  &&  me->list.selected != NULL
	  &&  me->list.selected != jet
	  &&  me->list.alwaysOne
	  &&  jet != NULL)
	{
	  Jet jet = me->list.selected;
	  XFillRectangle(me->core.display, me->core.window,
			 me->list.invert_gc,
			 jet->core.x, jet->core.y,
			 jet->core.width, jet->core.height);
	}

      /*  Invert new selection, IF:
       *   - selected jet is non-null
       *   - OR we are in singleSelect mode
       *     AND we are in alwaysOne mode
       *     AND selected jet is NOT the same as old selection
       */
      if (jet != NULL
	  &&  (me->list.listMode == SingleSelect
	       &&  me->list.alwaysOne
	       &&  me->list.selected != jet))
	{
	  XFillRectangle(me->core.display, me->core.window,
			 me->list.invert_gc,
			 jet->core.x, jet->core.y,
			 jet->core.width, jet->core.height);
	}

      if (jet != NULL)
	{
	  if (me->list.internalBorder)
	    {
	      int width = me->list.internalBorder/2;

	      if (me->list.focus)
		{
		  Jet jet = me->list.focus;

		  XDrawRectangle(me->core.display, me->core.window,
				 me->list.background_gc,
				 jet->core.x - width,
				 jet->core.y - width,
				 jet->core.width + width + 1,
				 jet->core.height + width + 1);
		}

	      XDrawRectangle(me->core.display, me->core.window,
			     me->list.foreground_gc,
			     jet->core.x - width,
			     jet->core.y - width,
			     jet->core.width + width + 1,
			     jet->core.height + width + 1);
	    }

	  me->list.focus = jet;
	}

      /*
       * Figure out what the current selection is, if any.
       */
      switch (me->list.listMode)
	{
	case SingleSelect:
	  if (me->list.alwaysOne)
	    {
	      if (jet != NULL)
		newSelect = jet;
	      else
		newSelect = me->list.selected;
	    }
	  else if (me->list.selected != jet)
	    newSelect = jet;

	  me->list.selected = newSelect;
	  break;

	default:
	  break;
	}

      break;
/*
      
      XFillRectangle(me->core.display, me->core.window,
      me->list.invert_gc,
      jet->core.x, jet->core.y,
      jet->core.width, jet->core.height);

      switch(me->list.whichButton)
	{
	case Button1:
	  me->list.startSelect = me->list.startPivot =
	    find_boundary(me, &w, line, START);
	  me->list.endSelect = me->list.endPivot =
	    find_boundary(me, &w, line, END);
	  break;

	case Button3:
	  if ((int)w <
	      ((int)me->list.realStart +
	       (int)me->list.realEnd)
	      / 2)
	    {
	      me->list.startEnd = 0;
	      me->list.startSelect = find_boundary(me, &w, line, START);
	      me->list.endSelect = me->list.realEnd;
	    }
	  else
	    {
	      me->list.startEnd = 1;
	      me->list.endSelect = find_boundary(me, &w, line, END);
	      me->list.startSelect = me->list.realStart;
	    }
	  break;
	}

      showSelect(me, oldStart, oldEnd);
*/


#ifdef notdoneyet
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
#endif /* notdoneyet */

    default:
      return False;
    }
  return True;
}



#ifdef textdisplaycode
==============================================================================
#define START True
#define END False

  { XjNhighlightForeground, XjCForeground, XjRString, sizeof(int),
      offset(textDisplay.hl_fg_name), XjRString, "" },
  { XjNhighlightBackground, XjCBackground, XjRString, sizeof(int),
      offset(textDisplay.hl_bg_name), XjRString, "" },
  { XjNreverseVideo, XjCReverseVideo, XjRBoolean, sizeof(Boolean),
      offset(textDisplay.reverseVideo), XjRBoolean, (caddr_t)False },

static void initialize(), expose(), realize(), querySize(), move(),
  destroy(), resize(), appendLines(), drawChars();
static Boolean event_handler();

TextDisplayClassRec textDisplayClassRec = {
  {
    /* class name */		"TextDisplay",
    /* jet size   */		sizeof(TextDisplayRec),
    /* classInitialize */	NULL,
    /* classInitialized? */	1,
    /* initialize */		initialize,
    /* prerealize */    	NULL,
    /* realize */		realize,
    /* event */			event_handler,
    /* expose */		expose,
    /* querySize */     	querySize,
    /* move */			move,
    /* resize */        	resize,
    /* destroy */       	destroy,
    /* resources */		resources,
    /* number of 'em */		XjNumber(resources)
  }
};


static void initialize(me)
     TextDisplayJet me;
{
  me->textDisplay.realized = 0;

  me->textDisplay.charWidth = me->textDisplay.font->max_bounds.width;
  me->textDisplay.charHeight = (me->textDisplay.font->ascent
				+ me->textDisplay.font->descent);

  me->textDisplay.lineStartsSize = 1000;
  me->textDisplay.lineStarts = (char **)XjMalloc(1000 * sizeof(char *));
  me->textDisplay.startSelect = 0;
  me->textDisplay.endSelect = 0;
  me->textDisplay.realStart = 0;
  me->textDisplay.realEnd = 0;
  me->textDisplay.selection = NULL;
  me->textDisplay.clickTimes = 0;
  me->textDisplay.buttonDown = False;

  /*
   * Initialize the first lines for safety...  trust me...
   * (it's in case there's no text in the jet and someone tries to do a
   *  multi-click to select some text...  it also keeps "appendLines"
   *  from breaking...)
   */
  me->textDisplay.lineStarts[0] = "";

  set_character_class(me->textDisplay.charClass);
  xselInitAtoms(me->core.display);
}

/*
 * Things are currently broken screenwise.
 * It will be fun to fix later. :)
 */
static void realize(me)
     TextDisplayJet me;
{
  unsigned long valuemask, valuemask2;
  unsigned long pixel;
  XGCValues values;

  if (me->textDisplay.reverseVideo)
    {
      int swap = me->textDisplay.foreground;
      me->textDisplay.foreground = me->textDisplay.background;
      me->textDisplay.background = swap;
    }

  if (strcmp(me->textDisplay.hl_fg_name, "")
      &&  !StrToXPixel(XjDisplay(me), me->textDisplay.hl_fg_name, &pixel))
    me->textDisplay.hl_foreground = pixel;
  else
    me->textDisplay.hl_foreground = me->textDisplay.background;

  if (strcmp(me->textDisplay.hl_bg_name, "")
      &&  !StrToXPixel(XjDisplay(me), me->textDisplay.hl_bg_name, &pixel))
    me->textDisplay.hl_background = pixel;
  else
    me->textDisplay.hl_background = me->textDisplay.foreground;

  values.function = GXcopy;
  values.font = me->textDisplay.font->fid;
  values.foreground = me->textDisplay.foreground;
  values.background = me->textDisplay.background;
  valuemask = GCForeground | GCBackground | GCFont | GCFunction;
  me->textDisplay.gc = XjCreateGC(me->core.display,
				  me->core.window,
				  valuemask,
				  &values);

  values.foreground = values.background;
  valuemask2 = GCForeground | GCFunction;
  me->textDisplay.gc_clear = XjCreateGC(me->core.display,
					me->core.window,
					valuemask2,
					&values);


  values.foreground = ((me->textDisplay.foreground
			== me->textDisplay.hl_foreground)
		       ? me->textDisplay.background
		       : me->textDisplay.hl_foreground);
  values.background = ((me->textDisplay.background
			== me->textDisplay.hl_background)
		       ? me->textDisplay.foreground
		       : me->textDisplay.hl_background);
  valuemask = GCForeground | GCBackground | GCFont | GCFunction;
  me->textDisplay.selectgc = XjCreateGC(me->core.display,
					me->core.window,
					valuemask,
					&values);

  values.foreground = values.background;
  valuemask2 = GCForeground | GCFunction;
  me->textDisplay.gc_fill = XjCreateGC(me->core.display,
				       me->core.window,
				       valuemask2,
				       &values);

  me->textDisplay.visLines = (me->core.height -
			      2 * me->textDisplay.internalBorder) /
				me->textDisplay.charHeight;

  /*
   * Usurp events for this window
   */
  XjRegisterWindow(me->core.window, (Jet) me);
  XjSelectInput(me->core.display, me->core.window,
		ButtonPressMask | ButtonReleaseMask | ButtonMotionMask);

  me->textDisplay.realized = 1;
}

static void resize(me, size)
     TextDisplayJet me;
     XjSize *size;
{
  me->core.width = size->width;
  me->core.height = size->height;
  me->textDisplay.visLines = (me->core.height -
			      2 * me->textDisplay.internalBorder) /
				me->textDisplay.charHeight;
  me->textDisplay.columns = (me->core.width -
			     2 * me->textDisplay.internalBorder) /
			       me->textDisplay.charWidth;
  if (me->textDisplay.realized)
    appendLines(me, me->textDisplay.text, 0);
}

static void drawLine(me, line, y)
     TextDisplayJet me;
     int line, y;
{
  int c = 0, length;
  char *ptr, *end, *last;

  ptr = me->textDisplay.lineStarts[line];
  last = me->textDisplay.lineStarts[line + 1];

  while (ptr != last)
    {
      end = ptr;

      while (end < last && *end != '\t')
	end++;

      length = end - ptr;

      if (length != 0)
	{
	  XDrawString(me->core.display, me->core.window,
		      me->textDisplay.gc,
		      me->core.x + me->textDisplay.internalBorder +
		      c * me->textDisplay.charWidth,
		      y + me->textDisplay.font->ascent,
		      ptr, length - (ptr[length - 1] == '\n'));
	  c += length;
	}

      if (end < last && *end == '\t') /* could be reduced, but... */
	{
	  c = 8 * ((c / 8) + 1);
	  end++;
	  length++;
	  while (end < last && *end == '\t')
	    {
	      end++;
	      length++;
	      c += 8;
	    }
	}
	    
      ptr += length;
    }
}

static void drawText(me, start, num, y)
     TextDisplayJet me;
     int start;
     int num, y;
{
  char *first, *last;

  first = me->textDisplay.lineStarts[start];
  last = me->textDisplay.lineStarts[MIN(start + num,
					   me->textDisplay.numLines)];

  
  if (me->textDisplay.startSelect == me->textDisplay.endSelect ||
      last <= me->textDisplay.startSelect ||
      first >= me->textDisplay.endSelect)
    {
      while (num != 0 && start < me->textDisplay.numLines)
	{
	  drawLine(me, start, y);
	  start++;
	  num--;
	  y += me->textDisplay.charHeight;
	}
/*
      drawChars(me, me->textDisplay.gc, first, last);
*/
      return;
    }

  /*
   * Since we're here, there must be some selected text drawn.
   */
  if (first <= me->textDisplay.startSelect) /* last > startSelect */
    {
      drawChars(me, me->textDisplay.gc,
		first, me->textDisplay.startSelect);
      first = me->textDisplay.startSelect;
    }

  drawChars(me, me->textDisplay.selectgc,
	    first, MIN(me->textDisplay.endSelect, last));

  if (last > me->textDisplay.endSelect)
    drawChars(me, me->textDisplay.gc,
	      me->textDisplay.endSelect, last);
}

static void drawSome(me, gc, y, startOfLine, start, last)
     TextDisplayJet me;
     GC gc;
     int y;
     char *startOfLine, *start, *last;
{
  int c = 0, length;
  char *ptr, *end;

/*  fprintf(stdout, "%d\n", gc == me->textDisplay.selectgc); */
  /*
   * At the end of this while, c is the column number to start
   * drawing at, and ptr is start.
   */
  ptr = startOfLine;
  while (ptr != start)
    {
      end = ptr;

      while (end < start && *end != '\t')
	end++;

      length = end - ptr;
      c += length;

      if (end < start && *end == '\t') /* could be reduced, but... */
	{
	  c = 8 * ((c / 8) + 1);
	  end++;
	  length++;
	  while (end < start && *end == '\t')
	    {
	      end++;
	      length++;
	      c += 8;
	    }
	}
	    
      ptr += length;
    }

  while (ptr != last)
    {
      end = ptr;

      while (end < last && *end != '\t')
	end++;

      length = end - ptr;

      if (length != 0)
	{
	  XDrawImageString(me->core.display, me->core.window,
			   gc,
			   me->core.x + me->textDisplay.internalBorder +
			   c * me->textDisplay.charWidth,
			   y + me->textDisplay.font->ascent,
			   ptr, length - (ptr[length - 1] == '\n'));
/*	  fprintf(stdout, "%.*s",  length - (ptr[length - 1] == '\n'),
		  ptr);
	  fflush(stdout); */
	  c += length;

	  /* this is gross... */
	  if (ptr[length-1] == '\n')
	    {
	      int tmp = (c-1) * me->textDisplay.charWidth +
		me->textDisplay.internalBorder;

	      XFillRectangle(me->core.display, me->core.window,
			     ((gc == me->textDisplay.selectgc)
			      ? me->textDisplay.gc_fill
			      : me->textDisplay.gc_clear),
			     me->core.x + tmp,
			     y,
			     MAX(0, (me->textDisplay.columns + 1 - c)
				 * me->textDisplay.charWidth),
			     (me->textDisplay.font->ascent +
			      me->textDisplay.font->descent));
	    }
	}

      if (end < last && *end == '\t') /* could be reduced, but... */
	{
	  XDrawImageString(me->core.display, me->core.window,
			   gc,
			   me->core.x + me->textDisplay.internalBorder +
			   c * me->textDisplay.charWidth,
			   y + me->textDisplay.font->ascent,
			   "        ", 8 - (c%8));
/*	  fprintf(stdout, "%.*s", 8-(c%8), "        ");
	  fflush(stdout); */
	  c = 8 * ((c / 8) + 1);
	  end++;
	  length++;
	  while (end < last && *end == '\t')
	    {
	      XDrawImageString(me->core.display, me->core.window,
			       gc,
			       me->core.x + me->textDisplay.internalBorder +
			       c * me->textDisplay.charWidth,
			       y + me->textDisplay.font->ascent,
			       "        ", 8);
	      end++;
	      length++;
	      c += 8;
/*	      fprintf(stdout, "        ");
	      fflush(stdout); */
	    }
	}
	    
      ptr += length;
    }
}

static void drawChars(me, gc, start, end)
     TextDisplayJet me;
     GC gc;
     char *start, *end;
{
  int y;
  int line;

  for (line = me->textDisplay.topLine;
       line < me->textDisplay.topLine + me->textDisplay.visLines &&
       line < me->textDisplay.numLines;
       line++)
    if (me->textDisplay.lineStarts[line + 1] > start)
      break;

  if (me->textDisplay.lineStarts[line] > start)
    start = me->textDisplay.lineStarts[line];

  if (start >= end)
    return;

  if (line >= me->textDisplay.topLine + me->textDisplay.visLines ||
      line >= me->textDisplay.numLines)
    return;

  y = me->core.y + me->textDisplay.internalBorder +
    (line - me->textDisplay.topLine) * me->textDisplay.charHeight;

  for (;line < me->textDisplay.topLine + me->textDisplay.visLines &&
       line < me->textDisplay.numLines; line++)
    {
      drawSome(me, gc, y, me->textDisplay.lineStarts[line],
	       start,
	       MIN(me->textDisplay.lineStarts[line + 1], end));
      y += me->textDisplay.charHeight;
      start = me->textDisplay.lineStarts[line + 1];
      if (start > end)
	break;
    }
}

static void drawCharacters(me, start, end)
     TextDisplayJet me;
     char *start, *end;
{
  if (start < me->textDisplay.startSelect)
    {
      if (end <= me->textDisplay.startSelect)
	{
	  drawChars(me, me->textDisplay.gc, start, end);
	  return;
	}

      drawChars(me, me->textDisplay.gc,
		start,
		me->textDisplay.startSelect);
      start = me->textDisplay.startSelect;
    }

  if (start < me->textDisplay.endSelect)
    {
      if (end <= me->textDisplay.endSelect)
	{
	  drawChars(me, me->textDisplay.selectgc, start, end);
	  return;
	}

      drawChars(me, me->textDisplay.selectgc,
		start,
		me->textDisplay.endSelect);
      start = me->textDisplay.endSelect;
    }

  drawChars(me, me->textDisplay.gc, start, end);
}

/*
 * This draws only the regions of text whose selected
 * state has changed.
 */
static void showSelect(me, selStart, selEnd)
     TextDisplayJet me;
     char *selStart, *selEnd;
{
  int i, j;
  char *k, *s[4];

  s[0] = selStart;
  s[1] = selEnd;
  s[2] = me->textDisplay.startSelect;
  s[3] = me->textDisplay.endSelect;

  /* sort the array into order. */
  for (i = 0; i < 3; i++)
    for (j = i + 1; j < 4; j++)
      if (s[i] > s[j])
	{
	  k = s[i];
	  s[i] = s[j];
	  s[j] = k;
	}

  if (s[0] != s[1])
    drawCharacters(me, s[0], s[1]);

  if (s[2] != s[3])
    drawCharacters(me, s[2], s[3]);
}


static int timerid = -1;

void auto_scroll(me, id)
     TextDisplayJet me;
     int id;			/* ARGSUSED */
{
  Window junkwin;
  int x, y, junk;
  unsigned int junkmask;
  XEvent event;

  timerid = -1;

  XQueryPointer(XjDisplay(me), XjWindow(me),
		&junkwin, &junkwin, &junk, &junk, &x, &y, &junkmask);

  event.type = MotionNotify;	/* Fake out the event_handler into */
  event.xbutton.x = x;		/* thinking that the mouse has moved. */
  event.xbutton.y = y;		/* It will then deal with selecting and */
  event_handler(me, &event);	/* scrolling more, as needed. */
}


char *where(me, x, y, line)
     TextDisplayJet me;
     int x, y;
     int *line;
{
  char *ptr, *end;
  int col, tmp, vert, c = 0;
  int clickTimes = me->textDisplay.clickTimes % 5;
#define LINE *line
  
  if (me->textDisplay.numLines == 0)
    {
      LINE = 0;
      return me->textDisplay.lineStarts[0];
    }

  vert = (y - me->core.y - me->textDisplay.internalBorder);
  LINE = ((y - me->core.y - me->textDisplay.internalBorder) /
	  me->textDisplay.charHeight) + me->textDisplay.topLine;
  col = ((x - me->core.x - me->textDisplay.internalBorder) /
	 me->textDisplay.charWidth);

  /*
   * Deal with autoscrolling...  first check if we want to scroll up...
   */
  if (vert < -20  &&  me->textDisplay.topLine > 0
      &&  timerid == -1
      &&  (me->textDisplay.scrollDelay1 >= 0 ||
	   me->textDisplay.scrollDelay2 >= 0))
    {
      SetLine(me, me->textDisplay.topLine - 1);
      XjCallCallbacks(me, me->textDisplay.scrollProc, NULL);

      timerid = XjAddWakeup(auto_scroll, me,
			    (vert < -40  &&
			     me->textDisplay.scrollDelay2 >= 0)
			    ? me->textDisplay.scrollDelay2
			    : me->textDisplay.scrollDelay1);
    }

  /* bounds checking */
  if (LINE < me->textDisplay.topLine)
    LINE = me->textDisplay.topLine;
#ifdef notdef
  if (LINE < 0)			/* is this really necessary??? */
    LINE = 0;
#endif
  if (vert < -40)
    return me->textDisplay.lineStarts[LINE];

  /*
   * ...now check if we're past the last line, and the last line is
   *  *not* at the bottom of the jet...  if this is the case, then we
   *  want to return the end of the text...
   */
  if (LINE > me->textDisplay.numLines
      &&  me->textDisplay.visLines > me->textDisplay.numLines)
    {
      LINE = me->textDisplay.numLines;
      return me->textDisplay.lineStarts[me->textDisplay.numLines];
    }

  /*
   * ...then check if we want to scroll down...
   */
  tmp = me->textDisplay.topLine + me->textDisplay.visLines;
  if (LINE >= tmp)
    LINE = tmp - 1;

  if (vert - me->core.height > 20  &&  tmp < me->textDisplay.numLines
      &&  timerid == -1
      &&  (me->textDisplay.scrollDelay1 >= 0 ||
	   me->textDisplay.scrollDelay2 >= 0))
    {
      SetLine(me, me->textDisplay.topLine + 1);	/* scroll down... */
      XjCallCallbacks(me, me->textDisplay.scrollProc, NULL);
      LINE += 1;

      timerid = XjAddWakeup(auto_scroll, me,
			    (vert - me->core.height > 40  &&
			     me->textDisplay.scrollDelay2 >= 0)
			    ? me->textDisplay.scrollDelay2
			    : me->textDisplay.scrollDelay1);
    }

  /* bounds checking */
#ifdef notdef
  if (LINE == me->textDisplay.numLines - 1)
    {
#endif
      if (vert - me->core.height > 40)
	return me->textDisplay.lineStarts[LINE + 1];
#ifdef notdef
    }
#endif
  if (LINE >= me->textDisplay.numLines)
    LINE = me->textDisplay.numLines - 1;


  ptr = me->textDisplay.lineStarts[LINE];
  end = me->textDisplay.lineStarts[LINE + 1];

  while (c < col && ptr < end)
    {
      while ((c < col) && *ptr != '\t' && ptr < end)
	{
	  ptr++;
	  c++;
	}

      if ((c < col) && ptr < end && *ptr == '\t')
	{
	  c = 8 * ((c / 8) + 1);
	  ptr++;

	  while ((c < col) && *ptr == '\t' && ptr < end)
	    {
	      c += 8;
	      ptr++;
	    }
	}
    }

  if (clickTimes == 1)
    return ptr;

#ifdef notdef
  if (me->textDisplay.columns <= col)	/* to deal with dragging off the */
    return ptr;				/* right edge */	
#endif
  if (ptr == end)
    ptr = end - 1;

  return ptr;

#undef LINE
}

char *find_boundary(me, ptr, line, find_start)
     TextDisplayJet me;
     char **ptr;
     int line;
     Boolean find_start;
{
  int clickTimes = me->textDisplay.clickTimes % 5;

  if (me->textDisplay.numLines == 0)
    return *ptr;

  if (clickTimes == 1)		/* single-clicks - select by char */
    return *ptr;

  if (clickTimes == 2)		/* double-clicks - select by "word" */
    {
      char *tmp;
      
      if (find_start)
	{
	  tmp = *ptr - 1;
	  while(tmp >= me->textDisplay.lineStarts[0] &&
		charClass[*tmp] == charClass[**ptr])
	    tmp--;
	  tmp++;
	}
      else
	{
	  tmp = *ptr;
	  while(tmp <= me->textDisplay.lineStarts[me->textDisplay.numLines] &&
		charClass[*tmp] == charClass[**ptr])
	    tmp++;
	}
      return tmp;
    }

  if (clickTimes == 3)		/* triple-clicks - select by "line" */
    return me->textDisplay.lineStarts[(find_start)
				      ? line
				      : MIN(me->textDisplay.numLines,
					    line + 1)];

  if (clickTimes == 4)		/* quad-clicks - select by "paragraph" */
    {
      char *tmp;
      int blankline, x;

      if (find_start)
	{
	  /*
	   *  scan backward to the beginning of the "paragraph"
	   */
	  for (x = line;
	       x >= 0;
	       x--)
	    {
	      blankline = True;
	      if (x < me->textDisplay.numLines)
		for (tmp = me->textDisplay.lineStarts[x];
		     blankline && tmp <= (me->textDisplay.lineStarts[x+1] - 1);
		     tmp++)
		  if (*tmp != ' '  && *tmp != '\t' &&
		      *tmp != '\n' && *tmp != '\0')
		    blankline = False;

	      if (blankline)
		return me->textDisplay.lineStarts[(x == line) ? x : x+1];
	    }
	  return me->textDisplay.lineStarts[0];
	}

      else
	{
	  /*
	   *  scan forward to the end of the "paragraph"
	   */
	  for (x = line;
	       x < me->textDisplay.numLines;
	       x++)
	    {
	      blankline = True;
	      for (tmp = me->textDisplay.lineStarts[x];
		   blankline && tmp <= (me->textDisplay.lineStarts[x+1] - 1);
		   tmp++)
		if (*tmp != ' '  && *tmp != '\t' &&
		    *tmp != '\n' && *tmp != '\0')
		  blankline = False;

	      if (blankline)
		return me->textDisplay.lineStarts[(x == line) ? x+1 : x];
	    }
	  return me->textDisplay.lineStarts[x];
	}
    }

				/* quint-clicks - select all text */
  return me->textDisplay.lineStarts[(find_start)
				    ? 0 : me->textDisplay.numLines];
}


static void expose(me, event)
     Jet me;
     XEvent *event;		/* ARGSUSED */
{
  drawText((TextDisplayJet) me,
	   ((TextDisplayJet) me)->textDisplay.topLine,
	   ((TextDisplayJet) me)->textDisplay.visLines,
	   me->core.y + ((TextDisplayJet) me)->textDisplay.internalBorder);
}

static void appendLines(me, text, num)
     TextDisplayJet me;
     char *text;
     int num;
{
  int c, col;
  char *top = me->textDisplay.lineStarts[me->textDisplay.topLine];

  if (me->textDisplay.columns < 1 ||
      text == NULL)
    return;

  while (*text != '\0')
    {
      /* The +2 is a useful hack. */
      if (num + 2 > me->textDisplay.lineStartsSize)
	{
	  me->textDisplay.lineStarts =
	    (char **)XjRealloc(me->textDisplay.lineStarts,
			       (1000 + me->textDisplay.lineStartsSize)
			       * sizeof(char *));
	  me->textDisplay.lineStartsSize += 1000;
	}

      me->textDisplay.lineStarts[num] = text;
      if (text == top)
	me->textDisplay.topLine = num;

      col = 0;
      for (c = 0; col < me->textDisplay.columns; c++)
	switch(text[c])
	  {
	  case '\0':
	    c--;
	  case '\n':
	    col = me->textDisplay.columns;
	    break;
	  case '\t':
	    col = 8 * ((col / 8) + 1);
	    break;
	  default:
	    col++;
	    break;
	  }
      text += c;
      if (/* text > me->textDisplay.text && not necessary */
	  text[0] == '\n' && text[-1] != '\n') /* then wrapped */
	text++; /* stupid to <cr> here */

      num++;
    }

  me->textDisplay.lineStarts[num] = text;
  me->textDisplay.numLines = num;
  if (text == top)
    me->textDisplay.topLine = num;

  if (me->textDisplay.realized)
    XjCallCallbacks(me, me->textDisplay.resizeProc, NULL);
}

void showNewLines(me, start)
     TextDisplayJet me;
     int start;
{
  if (start >= me->textDisplay.topLine &&
      start <= (me->textDisplay.topLine + me->textDisplay.visLines - 1))
    drawText(me,
	     start,
	     me->textDisplay.topLine + me->textDisplay.visLines - start,
	     me->core.y + me->textDisplay.internalBorder +
	     (start - me->textDisplay.topLine) *
	     me->textDisplay.charHeight);
}

void AddText(me)
     TextDisplayJet me;
{
  int l;

  l = me->textDisplay.numLines;
  if (l > 0)  l--;
  appendLines(me, me->textDisplay.lineStarts[l], l);
  if (me->textDisplay.realized)
    showNewLines(me, l);
}

void MoveText(me, text, offset)
     TextDisplayJet me;
     char *text;
     int offset;
{
  me->textDisplay.startSelect = (char *)
    MAX((int) text, (int) me->textDisplay.startSelect - offset);
  me->textDisplay.endSelect = (char *)
    MAX((int) text, (int) me->textDisplay.endSelect - offset);
  me->textDisplay.realStart = (char *)
    MAX((int) text, (int) me->textDisplay.realStart - offset);
  me->textDisplay.realEnd = (char *)
    MAX((int) text, (int) me->textDisplay.realEnd - offset);

  me->textDisplay.text = text;
  me->textDisplay.topLine = 0;

  appendLines(me, text, 0);
  if (me->textDisplay.realized)
    XClearArea(me->core.display, me->core.window,
	       me->core.x, me->core.y,
	       me->core.width, me->core.height, True);
}

void SetText(me, text)
     TextDisplayJet me;
     char *text;
{
  me->textDisplay.startSelect = text;
  me->textDisplay.endSelect = text;
  me->textDisplay.realStart = text;
  me->textDisplay.realEnd = text;

  MoveText(me, text, 0);
}

int TopLine(me)
     TextDisplayJet me;
{
  return me->textDisplay.topLine;
}

int VisibleLines(me)
     TextDisplayJet me;
{
  return me->textDisplay.visLines;
}

int CountLines(me)
     TextDisplayJet me;
{
  return me->textDisplay.numLines;
}

#define abs(x) (((x) < 0) ? (-(x)) : (x))

void SetLine(me, value)
     TextDisplayJet me;
     int value;
{
  int diff, charHeight;

  if (value == me->textDisplay.topLine)
    return;

  charHeight = me->textDisplay.charHeight;

  diff = value - me->textDisplay.topLine;
  if (diff > 0 &&
      diff < me->textDisplay.visLines)
    { /* scroll */
      XCopyArea(me->core.display,
		me->core.window,
		me->core.window,
		me->textDisplay.gc,
		me->core.x,
		me->core.y + me->textDisplay.internalBorder +
		diff * charHeight,
		me->core.width,
		(me->textDisplay.visLines - diff) * charHeight,
		me->core.x,
		me->core.y + me->textDisplay.internalBorder);

      XClearArea(me->core.display,
		 me->core.window,
		 me->core.x,
		 me->core.y + me->textDisplay.internalBorder +
		 (me->textDisplay.visLines - diff) * charHeight,
		 me->core.width,
		 diff * charHeight,
		 False);

      me->textDisplay.topLine = value;

      drawText(me,
	       value + me->textDisplay.visLines - diff,
	       diff,
	       me->core.y + me->textDisplay.internalBorder +
	       (me->textDisplay.visLines - diff) * charHeight);
    }
  else
    if (diff < 0 &&
	(-diff) < me->textDisplay.visLines)
      {
	/* scroll */
	diff = -diff;
	
	XCopyArea(me->core.display,
		  me->core.window,
		  me->core.window,
		  me->textDisplay.gc,
		  me->core.x,
		  me->core.y + me->textDisplay.internalBorder,
		  me->core.width,
		  (me->textDisplay.visLines - diff) * charHeight,
		  me->core.x,
		  me->core.y + me->textDisplay.internalBorder +
		  diff * charHeight);

	XClearArea(me->core.display,
		   me->core.window,
		   me->core.x,
		   me->core.y + me->textDisplay.internalBorder,
		   me->core.width,
		   diff * charHeight,
		   False);
      
	me->textDisplay.topLine = value;

	drawText(me,
		 me->textDisplay.topLine,
		 diff,
		 me->core.y + me->textDisplay.internalBorder);
      }
  else
    { /* completely new screen... */
      XClearArea(me->core.display,
		 me->core.window,
		 me->core.x,
		 me->core.y,
		 me->core.width,
		 me->core.height,
		 False);

      me->textDisplay.topLine = value;

      drawText(me,
	       me->textDisplay.topLine,
	       me->textDisplay.visLines,
	       me->core.y + me->textDisplay.internalBorder);
    }
}

static Boolean event_handler(me, event)
     TextDisplayJet me;
     XEvent *event;
{
  switch(event->type)
    {
    case ButtonRelease:
      if (me->textDisplay.buttonDown == False)
	break;
      me->textDisplay.buttonDown = False;

      if (event->xbutton.button == Button1)
	me->textDisplay.lastUpTime = event->xbutton.time;

      me->textDisplay.realStart = me->textDisplay.startSelect;
      me->textDisplay.realEnd = me->textDisplay.endSelect;

      if (me->textDisplay.startSelect == me->textDisplay.endSelect)
	break;

      if (xselGetOwnership(me->core.display, /* bug! */
			   me->core.window, 
			   event->xbutton.time))
	{
	  length = (int)(me->textDisplay.realEnd -
			 me->textDisplay.realStart);
	  if (me->textDisplay.selection != NULL)
	    XjFree(me->textDisplay.selection);
	  me->textDisplay.selection = (char *)XjMalloc(length + 1);
	  bcopy(me->textDisplay.realStart,
		me->textDisplay.selection, length);
	  me->textDisplay.selection[length] = '\0';
	}
      break;

    case MotionNotify:
      /* sigh... Xqvss can't be trusted, due to kernel bug */
      if (me->textDisplay.buttonDown == False)
	break;

      w = where(me, event->xbutton.x, event->xbutton.y, &line);

      switch(me->textDisplay.whichButton)
	{
	case Button1:
	  me->textDisplay.startSelect = me->textDisplay.startPivot;
	  me->textDisplay.endSelect = me->textDisplay.endPivot;

	  if (w >= me->textDisplay.endPivot)
	    me->textDisplay.endSelect = find_boundary(me, &w, line, END);
	  else if (w < me->textDisplay.startPivot)
	    me->textDisplay.startSelect = find_boundary(me, &w, line, START);
	  break;

	case Button3:
	  if (w > me->textDisplay.realEnd)
	    {
	      me->textDisplay.startEnd = 1;
	      me->textDisplay.endSelect = find_boundary(me, &w, line, END);
	      me->textDisplay.startSelect = me->textDisplay.realStart;
	    }
	  else if (w < me->textDisplay.realStart)
	    {
	      me->textDisplay.startEnd = 0;
	      me->textDisplay.startSelect = find_boundary(me, &w, line, START);
	      me->textDisplay.endSelect = me->textDisplay.realEnd;
	    }
	  else if (me->textDisplay.startEnd == 0)
	    me->textDisplay.startSelect = find_boundary(me, &w, line, START);
	  else
	    me->textDisplay.endSelect = find_boundary(me, &w, line, END);
	  break;
	}
      showSelect(me, oldStart, oldEnd);
      break;

    case SelectionRequest:
      xselProcessSelection(me->core.display, me->core.window, event,
			   me->textDisplay.selection);
      break;

    case SelectionClear:
      xselOwnershipLost(event->xselectionclear.time);
      me->textDisplay.endSelect = me->textDisplay.startSelect;
      showSelect(me, oldStart, oldEnd);
      if (me->textDisplay.selection != NULL)
	{
	  XjFree(me->textDisplay.selection);
	  me->textDisplay.selection = NULL;
	}
      break;

    default:
      return False;
    }

  return True;
}
#endif
