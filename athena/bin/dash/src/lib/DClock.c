/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/DClock.c,v $
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
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/DClock.c,v 1.3 1991-12-17 10:26:54 vanharen Exp $";
#endif

#include "mit-copyright.h"
#include <stdio.h>
#if defined(ultrix) || defined(_AIX)  ||  defined(_AUX_SOURCE)
#include <time.h>
#endif
#include <sys/time.h>
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

#define DEF_FMT "%.3w %.3n %2D %02e:%02m:%02s 19%Y"
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
};

#undef offset

static void wakeup(), expose(), initialize(), realize(), querySize(),
  move(), /*resize(),*/  destroy(), parse_formats();
static char *get_label();
static Boolean event_handler();

DClockClassRec dClockClassRec = {
  {
    /* class name */	"DigitalClock",
    /* jet size   */	sizeof(DClockRec),
    /* initialize */	initialize,
    /* prerealize */    NULL,
    /* realize */	realize,
    /* event */		event_handler,
    /* expose */	expose,
    /* querySize */     querySize,
    /* move */		move,
    /* resize */        NULL /*resize*/,
    /* destroy */       destroy,
    /* resources */	resources,
    /* number of 'em */	XjNumber(resources)
  }
};

JetClass dClockJetClass = (JetClass)&dClockClassRec;


static void initialize(me)
     DClockJet me;
{
  parse_formats(me);		/* Parse the format string... */
}

  
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
  len = strlen(get_label(me)) + 9 + me->dClock.num_fmts[w];
  me->dClock.current_fmt = w = 0;
  len = MAX(len, strlen(get_label(me)) + 9 + me->dClock.num_fmts[w]);

/*
 *  Add in some extra.  9 is the difference between the longest
 *  {month,day}name and the shortest of each. Add in num_fmts for extra
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

/*
  XjRegisterWindow(me->core.window, (Jet) me);
  XjSelectInput(me->core.display, me->core.window, ButtonPressMask);
*/
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
  struct timeval now;
  char *label;
  int len, w, max_w;
  int x=0, y;
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

    case Center:
      start = (MAX(0, old_w - w))/2;
      x = me->core.x + (me->core.width - max_w) / 2;
      break;

    case Right:
      start = MAX(0, old_w - w);
      x = me->core.x + (me->core.width - max_w - me->dClock.padding);
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
      for (ptr = ptr2 = tmp; (ptr2 = index(ptr, ':')) != 0; /*empty*/)
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
  int i;
  struct timeval tv;
  struct timezone tz;
  caddr_t f[MAX_FMTS];

  gettimeofday(&tv, &tz);
  tp = localtime(&tv.tv_sec);	/* don't cast to time_t... (ultrix lossage) */

  for (i=0; i < MAX_FMTS; i++)
    {
      switch(me->dClock.fmt_type[me->dClock.current_fmt][i])
	{
	  /*
	   * 'w' = weekday name (Monday - Sunday)
	   * 'n' = monthname (January - December)
	   * 'M' = Month number (1 - 12)
	   * 'D' = Day of month (1 - {28,30,31})
	   * 'Y' = Year number (no century - ex: 91)
	   * 'e' = european time (24 hour time)
	   * 'h' = hour number (1 - 12)
	   * 'm' = minute (0 - 60)
	   * 's' = seconds (0 - 60)
	   * 'A' = AMPM ("AM" or "PM")
	   * 'a' = ampm ("am" or "pm")
	   * 'z' = timeZone (ex: EDT, EST, CDT, CST, etc.)
	   * 'o' = day Of year (1 - 366)
	   */

	case '!':
	  f[i] = (caddr_t) 0;
	  break;
	case 'w':
	  f[i] = (caddr_t) wday[tp->tm_wday];
	  break;
	case 't':		/* 't' for backwards-compatibility */
	case 'n':
	  f[i] = (caddr_t) month[tp->tm_mon];
	  break;
	case 'M':
	  f[i] = (caddr_t) (tp->tm_mon + 1);
	  break;
	case 'D':
	  f[i] = (caddr_t) tp->tm_mday;
	  break;
	case 'Y':
	  f[i] = (caddr_t) tp->tm_year;
	  break;
	case 'e':
	  f[i] = (caddr_t) tp->tm_hour;
	  break;
	case 'h':
	  f[i] = (caddr_t) ((tp->tm_hour > 12)
			    ? (tp->tm_hour - 12)
			    : ((tp->tm_hour == 0)
			       ? 12
			       : tp->tm_hour));
	  break;
	case 'm':
	  f[i] = (caddr_t) tp->tm_min;
	  break;
	case 's':
	  f[i] = (caddr_t) tp->tm_sec;
	  break;
	case 'A':
	  f[i] = (caddr_t) ((tp->tm_hour) > 11 ? "PM" : "AM");
	  break;
	case 'a':
	  f[i] = (caddr_t) ((tp->tm_hour) > 11 ? "pm" : "am");
	  break;
	case 'z':
#if defined (_AIX) || defined(_AUX_SOURCE)
 	  f[i] = (caddr_t) tzname[tp->tm_isdst];
#else
#ifdef ultrix
	  f[i] = (caddr_t) timezone(tz.tz_minuteswest, tp->tm_isdst);
#else
	  f[i] = (caddr_t) tp->tm_zone;
#endif
#endif
	  break;
	case 'o':
	  f[i] = (caddr_t) (tp->tm_yday + 1);
	  break;
	default:
	  f[i] = (caddr_t) "?";
	  break;
	}
    }

  vsprintf(outbuf, me->dClock.fmts[me->dClock.current_fmt], f);
  return(outbuf);
}


static void parse_formats(me)
     DClockJet me;
{
  char *s_ptr, *f_ptr;
  int i, j;

  for (i = 0; i < 2; i++)
    {
      me->dClock.num_fmts[i] = 0;
      s_ptr = me->dClock.format[i];
      f_ptr = me->dClock.fmts[i];

      while (*s_ptr != '\0')
	{
	  if (*s_ptr != '%')
	    *f_ptr++ = *s_ptr++;
	  else
	    {
	      if (*(s_ptr+1) == '%')
		{
		  *f_ptr++ = '%';  *f_ptr++ = '%';
		  s_ptr += 2;
		}
	      else
		{
		  while(!isalpha(*s_ptr))
		    *f_ptr++ = *s_ptr++;

		  switch(me->dClock.fmt_type[i][me->dClock.num_fmts[i]]
			 = *s_ptr++)
		    /* YES, I really do want an assignment here... */
		    {
		      /*
		       * 'w' = weekday name (Monday - Sunday)
		       * 'n' = monthname (January - December)
		       * 'M' = Month number (1 - 12)
		       * 'D' = Day of month (1 - {28,30,31})
		       * 'Y' = Year number (no century - ex: 91)
		       * 'e' = european time (24 hour time)
		       * 'h' = hour number (1 - 12)
		       * 'm' = minute (0 - 60)
		       * 's' = seconds (0 - 60)
		       * 'A' = AMPM ("AM" or "PM")
		       * 'a' = ampm ("am" or "pm")
		       * 'z' = timeZone (ex: EDT, EST, CDT, CST, etc.)
		       * 'o' = day Of year (1 - 366)
		       */

		    case 'w':
		    case 'n':
		    case 't':	/* 't' for backwards-compatibility */
		    case 'A':
		    case 'a':
		    case 'z':
		      *f_ptr++ = 's';
		      (me->dClock.num_fmts[i])++;
		      break;

		    case 'M':
		    case 'D':
		    case 'Y':
		    case 'e':
		    case 'h':
		    case 'm':
		    case 's':
		    case 'o':
		      *f_ptr++ = 'd';
		      (me->dClock.num_fmts[i])++;
		      break;

		    default:
		      *f_ptr++ = 's';
		      (me->dClock.num_fmts[i])++;
		      break;
		    }
		}
	    }
	}
      for (j = me->dClock.num_fmts[i]; j < MAX_FMTS; j++)
	{
	  me->dClock.fmt_type[i][j] = '!';
	  *f_ptr++ = '%'; *f_ptr++ = 'c';
	}
      *f_ptr = '\0';
    }
}

static Boolean event_handler(me, event)
     DClockJet me;
     XEvent *event;
{
  switch(event->type)
    {
    case ButtonPress:
      me->dClock.current_fmt = !me->dClock.current_fmt;
      (void) draw(me);
      break;

    default:
      return False;
    }
  return True;
}
