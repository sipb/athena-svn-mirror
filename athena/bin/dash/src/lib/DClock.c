/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/DClock.c,v $
 * $Author: danw $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#if  (!defined(lint))  &&  (!defined(SABER))
static char *rcsid =
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/DClock.c,v 1.12 1998-07-27 16:07:08 danw Exp $";
#endif

#include "mit-copyright.h"
#include <stdio.h>
#include <X11/Xos.h>		/* needed for <time.h> or <sys/time.h> */
#include <ctype.h>
#include "Jets.h"
#include "DClock.h"


static char *wday[] =
{
  "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday",
};

static char *month[] =
{
  "January", "February", "March", "April", "May", "June", "July",
  "August", "September", "October", "November", "December",
};

#define DEF_FMT "%.3w %.3n %2D %02e:%02m:%02s %C%Y"
#define offset(field) XjOffset(DClockJet,field)

static XjResource resources[] = {
  { XjNx, XjCX, XjRInt, sizeof(int),
      offset(core.x), XjRString, XjInheritValue },
  { XjNy, XjCY, XjRInt, sizeof(int),
      offset(core.y), XjRString, XjInheritValue },
  { XjNwidth, XjCWidth, XjRInt, sizeof(int),
      offset(core.width), XjRString, XjInheritValue },
  { XjNheight, XjCHeight, XjRInt, sizeof(int),
      offset(core.height), XjRString, XjInheritValue },
  { XjNcenterY, XjCCenter, XjRBoolean, sizeof(Boolean),
      offset(dClock.centerY), XjRBoolean, (caddr_t) False },
  { XjNjustify, XjCJustify, XjRJustify, sizeof(int),
      offset(dClock.justify), XjRString, XjCenterJustify },
  { XjNpadding, XjCPadding, XjRInt, sizeof(int),
      offset(dClock.padding), XjRString, "0" },  
  { XjNforeground, XjCForeground, XjRColor, sizeof(int),
      offset(dClock.foreground), XjRString, XjDefaultForeground },
  { XjNbackground, XjCBackground, XjRColor, sizeof(int),
      offset(dClock.background), XjRString, XjDefaultBackground },
  { XjNreverseVideo, XjCReverseVideo, XjRBoolean, sizeof(Boolean),
      offset(dClock.reverseVideo), XjRBoolean, (caddr_t) False },
  { XjNfont, XjCFont, XjRFontStruct, sizeof(XFontStruct *),
      offset(dClock.font), XjRString, XjDefaultFont },
  { XjNformat, XjCFormat, XjRString, sizeof(char *),
      offset(dClock.format[0]), XjRString, DEF_FMT },
  { XjNformat2, XjCFormat, XjRString, sizeof(char *),
      offset(dClock.format[1]), XjRString, DEF_FMT },
  { XjNupdate, XjCInterval, XjRInt, sizeof(int),
      offset(dClock.update), XjRString, "1" },
  { XjNblinkColons, XjCBlink, XjRBoolean, sizeof(Boolean),
      offset(dClock.blink_colons), XjRBoolean, (caddr_t) False },
  { XjNtimeOffset, XjCTimeOffset, XjRInt, sizeof(int),
      offset(dClock.timeOffset), XjRString, "0" },
};

#undef offset

static void wakeup(), expose(), realize(), querySize(),
  move(), resize(), destroy();
static char *get_label();
static Boolean event_handler();

DClockClassRec dClockClassRec = {
  {
    /* class name */		"DigitalClock",
    /* jet size   */		sizeof(DClockRec),
    /* classInitialize */	NULL,
    /* classInitialized? */	1,
    /* initialize */		NULL,
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

JetClass dClockJetClass = (JetClass)&dClockClassRec;


/*
 * Things are currently broken screenwise.
 * It will be fun to fix later. :)
 */
static void realize(me)
     DClockJet me;
{
  unsigned long valuemask;
  XGCValues values;
  int w, len;

  me->dClock.current_fmt = w = 1;
  len = strlen(get_label(me)) + 15;
  me->dClock.current_fmt = w = 0;
  len = MAX(len, strlen(get_label(me)) + 15);

/*
 *  Add in some extra.  9 is the difference between the longest
 *  {month,day}name and the shortest of each. Add in some more for extra
 *  security.
 *
 *  This will actually give us a rectangle that will be too big, but
 *  better too big than too small.
 */

  w = len * (me->dClock.font->max_bounds.rbearing
	     - me->dClock.font->min_bounds.lbearing);
  me->dClock.pmap_ht = (me->dClock.font->max_bounds.ascent
			+ me->dClock.font->max_bounds.descent);

  me->dClock.pmap = XjCreatePixmap(me->core.display, me->core.window,
				   w, me->dClock.pmap_ht,
				   DefaultDepth(me->core.display,
					     DefaultScreen(me->core.display)));

  me->dClock.colons_on = False;

  if (me->dClock.reverseVideo)
    {
      int temp;

      temp = me->dClock.foreground;
      me->dClock.foreground = me->dClock.background;
      me->dClock.background = temp;
    }

  values.foreground = me->dClock.foreground;
  values.background = me->dClock.background;
  values.font = me->dClock.font->fid;
  values.graphics_exposures = False;
  valuemask = ( GCForeground | GCBackground | GCFont
	       | GCGraphicsExposures );

  me->dClock.gc = XjCreateGC(me->core.display,
			     me->core.window,
			     valuemask,
			     &values);
  XCopyGC(me->core.display, DefaultGC(me->core.display,
				      DefaultScreen(me->core.display)),
	  GCFunction, me->dClock.gc);

  values.foreground = me->dClock.background;
  values.function = GXcopy;
  values.graphics_exposures = False;
  valuemask = GCForeground | GCFunction | GCGraphicsExposures;

  me->dClock.gc_bkgnd = XjCreateGC(me->core.display,
				   me->core.window,
				   valuemask,
				   &values);

  me->dClock.timerid = XjAddWakeup(wakeup, me, 1000 * me->dClock.update);

  XjRegisterWindow(me->core.window, (Jet) me);
  XjSelectInput(me->core.display, me->core.window, ButtonPressMask);
}


static void querySize(me, size)
     DClockJet me;
     XjSize *size;
{
  char *label;

  label = get_label(me);
  size->width = XTextWidth(me->dClock.font, label, strlen(label));

  size->height = me->dClock.font->ascent + me->dClock.font->descent;
}


static void move(me, x, y)
     DClockJet me;
     int x, y;
{
  me->core.x = x;
  me->core.y = y;
}


static void resize(me, size)
     DClockJet me;
     XjSize *size;
{
  if (me->core.width == size->width
      && me->core.height == size->height)
    return;

  if (me->dClock.gc_bkgnd != NULL)
    XFillRectangle(me->core.display, me->core.window,
		   me->dClock.gc_bkgnd,
		   me->core.x, me->core.y,
		   me->core.width, me->core.height);

  me->core.width = size->width;
  me->core.height = size->height;
}


static void destroy(me)
     DClockJet me;
{
  XjFreeGC(me->core.display, me->dClock.gc);
  XjFreeGC(me->core.display, me->dClock.gc_bkgnd);

  (void)XjRemoveWakeup(me->dClock.timerid);
}


static int draw(me)
     DClockJet me;
{
/*   struct timeval now; */
  char *label;
  int len, w, max_w;
  int x, y;
  static int old_w = 0;
  int start = 0;

  label = get_label(me);
  len = strlen(label);

  w = XTextWidth(me->dClock.font, label, len);
  max_w = MAX(w, old_w);

  switch(me->dClock.justify)
    {
    case Left:
      start = 0;
      x = me->core.x + me->dClock.padding;
      break;

    case Right:
      start = MAX(0, old_w - w);
      x = me->core.x + (me->core.width - max_w - me->dClock.padding);
      break;

    default:				/* Center, default */
      start = (MAX(0, old_w - w))/2;
      x = me->core.x + (me->core.width - max_w) / 2;
      break;
    }

  y = me->core.y;
  if (me->dClock.centerY)
    y += (me->core.height -
	  (me->dClock.font->ascent + me->dClock.font->descent)) / 2;

#if 1
/*
 *  Here we do some software double-buffering of the clock so it doesn't
 *  flicker.  If you don't like it - tough.  Or use the part after the
 *  #else below.
 */
  XFillRectangle(me->core.display, me->dClock.pmap, me->dClock.gc_bkgnd,
		 0, 0, max_w, me->dClock.pmap_ht);

  if (me->dClock.colons_on)
    {
      char tmp[1024];
      int new_x = start;
      int width, len2 = 0;
      char *ptr, *ptr2;

      strcpy(tmp, label);
      for (ptr = ptr2 = tmp; (ptr2 = strchr(ptr, ':')) != 0; /*empty*/)
	{
	  len2 = (int) (ptr2 - ptr);
	  *ptr2++ = '\0';
	  width = XTextWidth(me->dClock.font, ptr, len2);
	  XDrawString(me->core.display, me->dClock.pmap,
		      me->dClock.gc,
		      new_x, me->dClock.font->ascent,
		      ptr, len2);
	  new_x += (XTextWidth(me->dClock.font, ":", 1) + width);
	  ptr = ptr2;
	}
      XDrawString(me->core.display, me->dClock.pmap,
		  me->dClock.gc,
		  new_x, me->dClock.font->ascent,
		  ptr, strlen(ptr));

      me->dClock.colons_on = False;
    }
  else
    {
      XDrawString(me->core.display, me->dClock.pmap,
		  me->dClock.gc,
		  start, me->dClock.font->ascent,
		  label, len);

      if (me->dClock.blink_colons)
	me->dClock.colons_on = True;
    }

  XCopyArea(me->core.display, me->dClock.pmap, me->core.window,
	    me->dClock.gc,
	    0, 0, max_w, me->dClock.pmap_ht, x, y);
#else
  XDrawImageString(me->core.display, me->core.window,
		   me->dClock.gc,
		   x, y + me->dClock.font->ascent,
		   label, len);
#endif
  
  old_w = w;

/*  if (me->dClock.update > 1) */
    return(1000 * me->dClock.update);

/*  gettimeofday(&now, NULL);
  return(1000 - (now.tv_usec / 1000));
*/
}

static void wakeup(me, id)
     DClockJet me;
     int id;
{
  me->dClock.timerid = XjAddWakeup(wakeup, me, draw(me));
}

static void expose(me, event)
     DClockJet me;
     XEvent *event;
{
  (void) draw(me);
}


static char *get_label(me)
     DClockJet me;
{
  static char outbuf[BUFSIZ];
  struct tm *tp;
  struct timeval tv;
  struct timezone tz;
  char *in, *out, *start, fmbuf[10];

  gettimeofday(&tv, &tz);
  tv.tv_sec += me->dClock.timeOffset;

  tp = localtime((time_t *) &tv.tv_sec);

  in = me->dClock.format[me->dClock.current_fmt];
  for (out = outbuf; *in; in++)
    {
      if (*in != '%')
	*out++ = *in;
      else
	{
	  start = in++;
	  while (!isalpha(*in))
	    in++;
	  sprintf(fmbuf, "%.*s%c", in - start, start,
		  strchr("MDCYehmso", *in) ? 'd' : 's');
	  switch (*in)
	    {
	    case 'w': /* 'w' = weekday name (Monday - Sunday) */
	      sprintf(out, fmbuf, wday[tp->tm_wday]);
	      break;
	    case 'n': /* 'n' = monthname (January - December) */
	    case 't': /* backward compatibility */
	      sprintf(out, fmbuf, month[tp->tm_mon]);
	      break;
	    case 'M': /* 'M' = Month number (1 - 12) */
	      sprintf(out, fmbuf, tp->tm_mon + 1);
	      break;
	    case 'D': /* 'D' = Day of month (1 - {28,30,31}) */
	      sprintf(out, fmbuf, tp->tm_mday);
	      break;
	    case 'C': /* 'C' = Century (all but last 2 dig of year - ex: 19) */
	      sprintf(out, fmbuf, (tp->tm_year + 1900) / 100);
	      break;
	    case 'Y': /* 'Y' = Year number (no century - ex: 91) */
	      sprintf(out, fmbuf, tp->tm_year % 100);
	      break;
	    case 'e': /* 'e' = european time (24 hour time) */
	      sprintf(out, fmbuf, tp->tm_hour);
	      break;
	    case 'h': /* 'h' = hour number (1 - 12) */
	      sprintf(out, fmbuf, (tp->tm_hour > 12) ? (tp->tm_hour - 12) :
		      ((tp->tm_hour == 0) ? 12 : tp->tm_hour));
	      break;
	    case 'm': /* 'm' = minute (0 - 60) */
	      sprintf(out, fmbuf, tp->tm_min);
	      break;
	    case 's': /* 's' = seconds (0 - 60) */
	      sprintf(out, fmbuf, tp->tm_sec);
	      break;
	    case 'A': /* 'A' = AMPM ("AM" or "PM") */
	      sprintf(out, fmbuf, (tp->tm_hour > 11) ? "PM" : "AM");
	      break;
	    case 'a': /* 'a' = ampm ("am" or "pm") */
	      sprintf(out, fmbuf, (tp->tm_hour > 11) ? "pm" : "am");
	      break;
	    case 'z': /* 'z' = timeZone (ex: EDT, EST, CDT, CST, etc.) */
#if defined(SOLARIS) || defined(sgi)
	      sprintf(out, fmbuf, tzname[tp->tm_isdst]);
#else
	      sprintf(out, fmbuf, tp->tm_zone);
#endif
	      break;
	    case 'o': /* 'o' = day Of year (1 - 366) */
	      sprintf(out, fmbuf, tp->tm_yday + 1);
	      break;
	    default:
	      sprintf(out, fmbuf, "?");
	      break;
	    }
	  out += strlen(out);
	}
    }

  *out = '\0';
  return(outbuf);
}

static Boolean event_handler(me, event)
     DClockJet me;
     XEvent *event;
{
  switch(event->type)
    {
    case ButtonPress:
      {
	int x, y, w, h, len;
	char *label;

	label = get_label(me);
	len = strlen(label);
	w = XTextWidth(me->dClock.font, label, len);
	h = me->dClock.pmap_ht;
	switch(me->dClock.justify)
	  {
	  case Left:
	    x = me->core.x + me->dClock.padding;
	    break;
	  case Right:
	    x = me->core.x + (me->core.width - w - me->dClock.padding);
	    break;
	  default:			/* Center, default */
	    x = me->core.x + (me->core.width - w) / 2;
	    break;
	  }
	y = me->core.y;
	if (me->dClock.centerY)
	  y += (me->core.height -
		(me->dClock.font->ascent + me->dClock.font->descent)) / 2;

	if (event->xbutton.x > x
	    && event->xbutton.x < x+w
	    && event->xbutton.y > y
	    && event->xbutton.y < y+h)
	  {
	    me->dClock.current_fmt = !me->dClock.current_fmt;
	    (void) draw(me);
	    return True;
	  }
	else
	  return False;
	break;
      }


    default:
      return False;
    }
}
