/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/AClock.c,v $
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
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/AClock.c,v 1.2 1991-09-04 10:09:55 vanharen Exp $";
#endif	lint

#include "mit-copyright.h"
#include <stdio.h>
#if defined(ultrix) || defined(_AIX)  ||  defined(_AUX_SOURCE)
#include <time.h>
#endif
#include <sys/time.h>
#include <math.h>
#include "Jets.h"
#include "AClock.h"

#ifdef SHAPE
#include <X11/extensions/shape.h>
#endif

#include "sintable.h"

extern int DEBUG;

#define offset(field) XjOffset(AClockJet,field)

static XjResource resources[] = {
  { XjNx, XjCX, XjRInt, sizeof(int),
     offset(core.x), XjRString, XjInheritValue },
  { XjNy, XjCY, XjRInt, sizeof(int),
     offset(core.y), XjRString, XjInheritValue },
  { XjNwidth, XjCWidth, XjRInt, sizeof(int),
     offset(core.width), XjRString, XjInheritValue },
  { XjNheight, XjCHeight, XjRInt, sizeof(int),
     offset(core.height), XjRString, XjInheritValue },
  { XjNpadding, XjCPadding, XjRInt, sizeof(int),
      offset(aClock.padding), XjRString, "7" },  
  { XjNminorTick, XjCTick, XjRInt, sizeof(int),
      offset(aClock.minorTick), XjRString, "1" },
  { XjNmajorTick, XjCTick, XjRInt, sizeof(int),
      offset(aClock.majorTick), XjRString, "5" },
  { XjNminorStart, XjCTick, XjRInt, sizeof(int),
      offset(aClock.minorStart), XjRString, "97" },
  { XjNminorEnd, XjCTick, XjRInt, sizeof(int),
      offset(aClock.minorEnd), XjRString, "99" },
  { XjNmajorStart, XjCTick, XjRInt, sizeof(int),
      offset(aClock.majorStart), XjRString, "90" },
  { XjNmajorEnd, XjCTick, XjRInt, sizeof(int),
      offset(aClock.majorEnd), XjRString, "100" },
  { XjNsecondStart, XjCHand, XjRInt, sizeof(int),
      offset(aClock.secondStart), XjRString, "73" },
  { XjNsecondEnd, XjCHand, XjRInt, sizeof(int),
      offset(aClock.secondEnd), XjRString, "88" },
  { XjNsecondArc, XjCArc, XjRInt, sizeof(int),
      offset(aClock.secondArc), XjRString, "6" },
  { XjNminuteStart, XjCHand, XjRInt, sizeof(int),
      offset(aClock.minuteStart), XjRString, "71" },
  { XjNminuteEnd, XjCHand, XjRInt, sizeof(int),
      offset(aClock.minuteEnd), XjRString, "-10" },
  { XjNminuteArc, XjCArc, XjRInt, sizeof(int),
      offset(aClock.minuteArc), XjRString, "100" },
  { XjNhourStart, XjCHand, XjRInt, sizeof(int),
      offset(aClock.hourStart), XjRString, "45" },
  { XjNhourEnd, XjCHand, XjRInt, sizeof(int),
      offset(aClock.hourEnd), XjRString, "-10" },
  { XjNhourArc, XjCArc, XjRInt, sizeof(int),
      offset(aClock.hourArc), XjRString, "100" },
  { XjNforeground, XjCForeground, XjRColor, sizeof(int),
      offset(aClock.foreground), XjRString, XjDefaultForeground },
  { XjNbackground, XjCBackground, XjRColor, sizeof(int),
      offset(aClock.background), XjRString, XjDefaultBackground },
  { XjNhands, XjCForeground, XjRColor, sizeof(int),
      offset(aClock.hands), XjRString, XjDefaultForeground },
  { XjNhighlight, XjCForeground, XjRColor, sizeof(int),
      offset(aClock.highlight), XjRString, XjDefaultForeground },
  { XjNreverseVideo, XjCReverseVideo, XjRBoolean, sizeof(Boolean),
      offset(aClock.reverseVideo), XjRBoolean, (caddr_t) False },
  { XjNupdate, XjCInterval, XjRInt, sizeof(int),
      offset(aClock.update), XjRString, "1" },
  { XjNkeepRound, XjCKeepRound, XjRBoolean, sizeof(Boolean),
      offset(aClock.keepRound), XjRBoolean, (caddr_t) False },
#ifdef SHAPE
  { XjNround, XjCRound, XjRBoolean, sizeof(Boolean),
      offset(aClock.round), XjRBoolean, (caddr_t) False },
#endif
};

#undef offset

static void wakeup(), expose(), realize(), querySize(), move(),
  resize(), initialize(), destroy();

AClockClassRec aClockClassRec = {
  {
    /* class name */	"AnalogClock",
    /* jet size   */	sizeof(AClockRec),
    /* initialize */	initialize,
    /* prerealize */    NULL,
    /* realize */	realize,
    /* event */		NULL,
    /* expose */	expose,
    /* querySize */     querySize,
    /* move */		move,
    /* resize */        resize,
    /* destroy */       destroy,
    /* resources */	resources,
    /* number of 'em */	XjNumber(resources)
  }
};

JetClass aClockJetClass = (JetClass)&aClockClassRec;


static void initialize(me)
     AClockJet me;
{
#ifdef SHAPE
  int shape_event_base, shape_error_base;

  if (me->aClock.round && !XShapeQueryExtension (XjDisplay(me), 
						 &shape_event_base, 
						 &shape_error_base))
    me->aClock.round = False;

  me->aClock.mask = 0;
#endif

  me->aClock.realized = 0;
}

static int intsin(angle)
     int angle;
{
  int quadr; /* used to be quad; conflicts with Ultrix. grrrr. */

  if (angle >= 0)
    {
      quadr = (angle%CIRCLE) / ANGLES;
      angle = angle%ANGLES;
    }
  else
    {
      quadr = (3 - ((-angle)%CIRCLE) / ANGLES);
      angle = ANGLES - ((-angle)%ANGLES);
    }

  switch(quadr)
    {
    case 0:
      return sinTable[angle];
    case 1:
      return sinTable[(ANGLES - 1) - angle];
    case 2:
      return -sinTable[angle];
    case 3:
      return -sinTable[(ANGLES - 1 ) - angle];
    default:
      fprintf(stdout, "sin broken (%d)\n", angle);
      exit(-1);
    }
  return 0;		  /* just to make saber happy... look up 2 lines... */
}

#define intcos(angle) intsin(angle + ANGLES)

/*
 * Things are currently broken screenwise.
 * It will be fun to fix later. :)
 */
static void realize(me)
     AClockJet me;
{
  unsigned long valuemask;
  XGCValues values;
  Pixmap tmp;

  me->aClock.realized = 1;

  if (me->aClock.reverseVideo)
    {
      int temp;

      temp = me->aClock.foreground;
      if (me->aClock.hands == me->aClock.foreground)
	me->aClock.hands = me->aClock.background;
      if (me->aClock.highlight == me->aClock.foreground)
	me->aClock.highlight = me->aClock.background;
      me->aClock.foreground = me->aClock.background;
      me->aClock.background = temp;
    }

  valuemask = ( GCForeground | GCBackground
	       | GCFunction | GCGraphicsExposures );
  values.graphics_exposures = False;
  values.function = GXcopy;
  values.background = me->aClock.background;

#ifdef SHAPE
  tmp = XCreatePixmap (XjDisplay(me), XjWindow(me), 1, 1, 1);
  values.foreground = 0;
  me->aClock.eraseGC = XCreateGC(XjDisplay(me), tmp, valuemask, &values);
  values.foreground = 1;
  me->aClock.setGC = XCreateGC(XjDisplay(me), tmp, valuemask, &values);
  XFreePixmap(XjDisplay(me), tmp);
#endif

  values.foreground = me->aClock.foreground;
  me->aClock.handsGC = me->aClock.highlightGC = me->aClock.foregroundGC
			= XCreateGC(me->core.display,
				    me->core.window,
				    valuemask,
				    &values);

  if (me->aClock.hands != me->aClock.foreground)
    {
      values.foreground = me->aClock.hands;
      me->aClock.handsGC = XCreateGC(me->core.display,
				     me->core.window,
				     valuemask,
				     &values);
    }

  if (me->aClock.highlight != me->aClock.foreground)
    {
      values.foreground = me->aClock.highlight;
      me->aClock.highlightGC = XCreateGC(me->core.display,
					 me->core.window,
					 valuemask,
					 &values);
    }

  values.foreground = me->aClock.background;
  me->aClock.backgroundGC = XCreateGC(me->core.display,
				      me->core.window,
				      valuemask,
				      &values);

  me->aClock.h = me->aClock.m = me->aClock.s = -1;
  me->aClock.timerid = XjAddWakeup(wakeup, me, 1000 * me->aClock.update);

#ifdef SHAPE
  if (me->aClock.round)
    {
      XjSize size;

      size.width = me->core.width;
      size.height = me->core.height;
      resize(me, &size);
    }
#endif /* SHAPE */
}

static void querySize(me, size)
     AClockJet me;
     XjSize *size;
{
  size->width = 50;
  size->height = 50;
}

static void move(me, x, y)
     AClockJet me;
     int x, y;
{
  if (DEBUG)
    printf ("MV(aClock)x=%d,y=%d\n", x, y);

  me->core.x = x;
  me->core.y = y;
  me->aClock.centerx = me->core.width / 2 + me->core.x;
  me->aClock.centery = me->core.height / 2 + me->core.y;
}

static void resize(me, size)
     AClockJet me;
     XjSize *size;
{
  int width, height;

  if (DEBUG)
    printf ("RSZ(aClock)w=%d,h=%d\n", size->width, size->height);

  if (me->aClock.keepRound)
    width = height = MIN(size->width, size->height);
  else
    {
      width = size->width;
      height = size->height;
    }

  me->aClock.centerx = size->width / 2 + me->core.x;
  me->aClock.centery = size->height / 2 + me->core.y;
  me->aClock.xradius = width / 2 + me->core.x - me->aClock.padding;
  me->aClock.yradius = height / 2 + me->core.y - me->aClock.padding;

  if (me->aClock.realized == 1)
    {
      XFillRectangle(me->core.display, me->core.window,
		     me->aClock.backgroundGC,
		     me->core.x, me->core.y,
		     me->core.width, me->core.height);
#ifdef SHAPE
      if (me->aClock.round)
	{
	  Jet parent;
	  int x, y, bw = 0;

	  /*
	   * allocate a pixmap to draw shapes in
	   */
	  if (me->aClock.mask)
	    XFreePixmap(XjDisplay(me), me->aClock.mask);

	  me->aClock.mask = XCreatePixmap (XjDisplay(me), XjWindow(me),
					   size->width, size->height, 1);

	  /* erase the pixmap */
	  XFillRectangle (XjDisplay(me), me->aClock.mask, me->aClock.eraseGC,
			  0, 0, size->width, size->height);

	  /*
	   * draw the bounding shape.  Doing this first
	   * eliminates extra exposure events.
	   */

	  x = me->aClock.centerx - width/2 - me->core.x;
	  y = me->aClock.centery - height/2 - me->core.y;

	  XFillArc (XjDisplay(me), me->aClock.mask,
		    me->aClock.setGC, x, y, width, height,
		    0, 360 * 64);
	  XShapeCombineMask (XjDisplay(me), XjWindow(me), ShapeBounding,
			     0, 0, me->aClock.mask, ShapeSet);

	  /*
	   * Find the highest enclosing widget (a window) to get borderwidth.
	   */
	  for (parent = (Jet) me; XjParent(parent) && !bw;
	       parent = XjParent(parent))
	    if (!strcmp(parent->core.classRec->core_class.className, "Window"))
	      bw = parent->core.borderWidth;

	  /* erase the pixmap */
	  XFillRectangle (XjDisplay (me), me->aClock.mask, me->aClock.eraseGC,
			  0, 0, size->width, size->height);

	  /*
	   * draw the clip shape
	   */

	  XFillArc (XjDisplay(me), me->aClock.mask,
		    me->aClock.setGC, x + bw, y + bw,
		    width - 2 * bw, height - 2 * bw,
		    0, 360 * 64);
	  XShapeCombineMask(XjDisplay(me), XjWindow(me), ShapeClip, 
			    0, 0, me->aClock.mask, ShapeSet);
	}
#endif /* SHAPE */
    }

  me->core.width = size->width;		/* These sizes must be set AFTER */
  me->core.height = size->height;	/* the FillRectangle above... */
}



static void drawHand(me, drawable, gc_fill, gc_hilite, angle, start, end, arc)
     AClockJet me;
     Drawable drawable;
     GC gc_fill, gc_hilite;
     int angle;
     int start, end, arc;
{
  int offset = arc * CIRCLE / (360 * 2);

  XPoint p[4];

  p[0].x = (end * me->aClock.xradius / 100) * intsin(angle - offset) /
    SCALE + me->aClock.centerx;
  p[0].y = (-(end * me->aClock.yradius / 100) * intcos(angle - offset) /
    SCALE) + me->aClock.centery;
  p[1].x = (start * me->aClock.xradius / 100) * intsin(angle) /
    SCALE + me->aClock.centerx;
  p[1].y = (-(start * me->aClock.yradius / 100) * intcos(angle) /
    SCALE) + me->aClock.centery;
  p[2].x = (end * me->aClock.xradius / 100) * intsin(angle + offset) /
    SCALE + me->aClock.centerx;
  p[2].y = (-(end * me->aClock.yradius / 100) * intcos(angle + offset) /
    SCALE) + me->aClock.centery;
  p[3] = p[0];

  XFillPolygon(me->core.display, drawable,
	       gc_fill, p, 3, Convex, CoordModeOrigin);

  XDrawLines(me->core.display, drawable,
	     gc_hilite, p, 4, CoordModeOrigin);
}


static void drawSecond(me, drawable, gc_fill, gc_hilite, angle)
     AClockJet me;
     Drawable drawable;
     GC gc_fill, gc_hilite;
     int angle;
{
  int offset = me->aClock.secondArc * CIRCLE / (360 * 2);
  int ssx = me->aClock.secondStart * me->aClock.xradius / 100;
  int ssy = me->aClock.secondStart * me->aClock.yradius / 100;
  int sex = me->aClock.secondEnd * me->aClock.xradius / 100;
  int sey = me->aClock.secondEnd * me->aClock.yradius / 100;
  int mx = (ssx + sex) / 2;
  int my = (ssy + sey) / 2;

  XPoint p[5];

  p[0].x = intsin(angle) * ssx / SCALE + me->aClock.centerx;
  p[0].y = -(intcos(angle) * ssy / SCALE) + me->aClock.centery;
  p[2].x = intsin(angle) * sex / SCALE + me->aClock.centerx;
  p[2].y = -(intcos(angle) * sey / SCALE) + me->aClock.centery;

  p[1].x = intsin(angle - offset) * mx / SCALE + me->aClock.centerx;
  p[1].y = -(intcos(angle - offset) * my / SCALE) + me->aClock.centery;
  p[3].x = intsin(angle + offset) * mx / SCALE + me->aClock.centerx;
  p[3].y = -(intcos(angle + offset) * my / SCALE) + me->aClock.centery;
  p[4] = p[0];

  XFillPolygon(me->core.display, drawable,
	       gc_fill, p, 4, Convex, CoordModeOrigin);

  XDrawLines(me->core.display, drawable,
	     gc_hilite, p, 5, CoordModeOrigin);
}


static int update(me, expose)
     AClockJet me;
     int expose;
{
  struct timeval now;
  struct tm *t;
  int oh = me->aClock.h, om = me->aClock.m, os = me->aClock.s;
  int h, m, s;

  gettimeofday(&now, NULL);
  t = localtime(&now.tv_sec);

  h = t->tm_hour%12;
  m = t->tm_min;
  s = t->tm_sec;

  if ((s != os || expose) && (me->aClock.update < 31))
    {
      if (s != os)
	drawSecond(me, me->core.window,
		   me->aClock.backgroundGC, me->aClock.backgroundGC,
		   os * CIRCLE / 60);

      drawSecond(me, me->core.window,
		 me->aClock.handsGC, me->aClock.highlightGC,
		 s * CIRCLE / 60);
    }

  if (m != om || expose)
    {
      if (m != om)
	{
	  drawHand(me, me->core.window,
		   me->aClock.backgroundGC, me->aClock.backgroundGC,
		   om * CIRCLE / 60,
		   me->aClock.minuteStart, me->aClock.minuteEnd,
		   me->aClock.minuteArc);

	  drawHand(me, me->core.window,
		   me->aClock.backgroundGC, me->aClock.backgroundGC,
		   (60 * oh + om) * CIRCLE / 720,
		   me->aClock.hourStart, me->aClock.hourEnd,
		   me->aClock.hourArc);
	}

      drawHand(me, me->core.window,
	       me->aClock.handsGC, me->aClock.highlightGC,
	       m * CIRCLE / 60,
	       me->aClock.minuteStart, me->aClock.minuteEnd,
	       me->aClock.minuteArc);

      drawHand(me, me->core.window,
	       me->aClock.handsGC, me->aClock.highlightGC,
	       (60 * h + m) * CIRCLE / 720,
	       me->aClock.hourStart, me->aClock.hourEnd,
	       me->aClock.hourArc);
    }

  me->aClock.h = h; me->aClock.m = m; me->aClock.s = s;  

  if (me->aClock.update > 1)
    return(1000 * me->aClock.update);

  return(1000 - (now.tv_usec / 1000));
}

static void drawTicks(me)
     AClockJet me;
{
  int ticks;
  int start, end;
  int x1, y1, x2, y2;

  for (ticks = 0; ticks < 60; ticks++)
    {

      if (me->aClock.majorTick != 0
	  && ticks % me->aClock.majorTick == 0)
	{
	  start = me->aClock.majorStart;
	  end = me->aClock.majorEnd;
	}
      else if (me->aClock.minorTick != 0
	       && ticks % me->aClock.minorTick == 0)
	{
	  start = me->aClock.minorStart;
	  end = me->aClock.minorEnd;
	}
      else continue;

      x1 = me->aClock.centerx +
	(me->aClock.xradius * start) / 100
	  * intsin(ticks * CIRCLE / 60) / SCALE;

      y1 = me->aClock.centery +
	(me->aClock.yradius * start) / 100
	  * -(intcos(ticks * CIRCLE / 60)) / SCALE;

      x2 = me->aClock.centerx +
	(me->aClock.xradius * end) / 100
	  * intsin(ticks * CIRCLE / 60) / SCALE;

      y2 = me->aClock.centery +
	(me->aClock.yradius * end) / 100
	  * -(intcos(ticks * CIRCLE / 60)) / SCALE;

      XDrawLine(me->core.display,
		me->core.window,
		me->aClock.foregroundGC,
		x1, y1, x2, y2);
    }
}

static void wakeup(me)
     AClockJet me;
{
  me->aClock.timerid = XjAddWakeup(wakeup, me, update(me, 0));
}

static void expose(me, event)
     AClockJet me;
     XEvent *event;
{
  drawTicks(me);
  (void) update(me, 1);
}

static void destroy(me)
     AClockJet me;
{
  XFreeGC(me->core.display, me->aClock.backgroundGC);
  XFreeGC(me->core.display, me->aClock.foregroundGC);

  (void)XjRemoveWakeup(me->aClock.timerid);
}
