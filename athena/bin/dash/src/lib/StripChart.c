/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/StripChart.c,v $
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
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/StripChart.c,v 1.2 1991-09-04 10:13:51 vanharen Exp $";
#endif	lint

#include "mit-copyright.h"
#include <stdio.h>
#include "Jets.h"
#include "StripChart.h"

#define offset(field) XjOffset(StripChartJet,field)

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
      offset(stripChart.foreground), XjRString, XjDefaultForeground },
  { XjNbackground, XjCBackground, XjRColor, sizeof(int),
      offset(stripChart.background), XjRString, XjDefaultBackground },
  { XjNscaleColor, XjCForeground, XjRColor, sizeof(int),
      offset(stripChart.scaleColor), XjRColor, (caddr_t)-1 },
  { XjNdotScale, XjCDotScale, XjRBoolean, sizeof(Boolean),
      offset(stripChart.dotScale), XjRBoolean, (caddr_t)True },
  { XjNreverseVideo, XjCReverseVideo, XjRBoolean, sizeof(Boolean),
      offset(stripChart.reverseVideo), XjRBoolean, (caddr_t)False },
  { XjNdataProc, XjCDataProc, XjRCallback, sizeof(XjCallback *),
      offset(stripChart.dataProc), XjRString, NULL },
  { XjNscale, XjCScale, XjRInt, sizeof(int),
      offset(stripChart.scale), XjRInt, (caddr_t)100 },
  { XjNscaleInc, XjCScaleInc, XjRInt, sizeof(int),
      offset(stripChart.scaleInc), XjRInt, (caddr_t)100 },
  { XjNinterval, XjCInterval, XjRInt, sizeof(int),
      offset(stripChart.interval), XjRInt, (caddr_t)5000 },
  { XjNscrollDistance, XjCScrollDistance, XjRInt, sizeof(int),
      offset(stripChart.scrollDist), XjRInt, (caddr_t)1 }
};

#undef offset

static void initialize(), expose(), realize(), querySize(), move(),
  destroy(), resize(), wakeup();

StripChartClassRec stripChartClassRec = {
  {
    /* class name */	"StripChart",
    /* jet size   */	sizeof(StripChartRec),
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

JetClass stripChartJetClass = (JetClass)&stripChartClassRec;

static void initialize(me)
     StripChartJet me;
{
  me->stripChart.data = NULL;
  me->stripChart.parity = 0;
}

/*
 * Things are currently broken screenwise.
 * It will be fun to fix later. :)
 */
static void realize(me)
     StripChartJet me;
{
  unsigned long valuemask;
  XGCValues values;
  int screen;
  StripChartInit stripInit;

  screen = DefaultScreen(me->core.display);

  if (me->stripChart.scaleColor == -1)
    me->stripChart.scaleColor = me->stripChart.foreground;

  values.function = GXcopy;
  values.foreground = me->stripChart.reverseVideo ?
    me->stripChart.background : me->stripChart.foreground;
  values.graphics_exposures = False;
  valuemask = GCForeground | GCFunction | GCGraphicsExposures;

  me->stripChart.gc = XCreateGC(me->core.display,
				me->core.window,
				valuemask,
				&values);

  if (me->stripChart.scaleColor == me->stripChart.foreground ||
      me->stripChart.reverseVideo)
    me->stripChart.scalegc = me->stripChart.gc;
  else
    {
      values.foreground = me->stripChart.scaleColor;
      me->stripChart.scalegc = XCreateGC(me->core.display,
					 me->core.window,
					 valuemask,
					 &values);
    }

  me->stripChart.data = (int *)XjMalloc(sizeof(int) * me->core.width);
  me->stripChart.x = 0;

  stripInit.interval = me->stripChart.interval;
  stripInit.j = me;
  fprintf(stdout, "strip: %d\n", stripInit.interval);
  XjCallCallbacks(&stripInit, me->stripChart.dataProc, NULL);
}

static void destroy(me)
     StripChartJet me;
{
  XFreeGC(me->core.display, me->stripChart.gc);
  XjFree(me->stripChart.data);
  /* missing... */
}

static void querySize(me, size)
     StripChartJet me;
     XjSize *size;
{
  size->width = me->core.width;
  size->height = me->core.height;
}

static void move(me, x, y)
     StripChartJet me;
     int x, y;
{
  me->core.x = x;
  me->core.y = y;
}

static void moveData(me, distance)
     StripChartJet me;
     int distance;
{
  int i, max = 0;

  for (i = 0; i < me->stripChart.x - distance; i++)
    {
      me->stripChart.data[i] = me->stripChart.data[i + distance];
      max = MAX(max, me->stripChart.data[i]);
    }

  me->stripChart.x -= distance;
  while (max < me->stripChart.scale - me->stripChart.scaleInc)
    me->stripChart.scale -= me->stripChart.scaleInc;

  if (distance & 1)
    me->stripChart.parity = !me->stripChart.parity;
}

static void drawScale(me, where)
     StripChartJet me;
     int where;
{
  int y, i;

  i = me->core.height * me->stripChart.scaleInc / me->stripChart.scale;

  if (!me->stripChart.dotScale ||
      ((where & 1) == me->stripChart.parity))
    for (y = me->core.height - i; y > 0; y -= i)
      XDrawPoint(me->core.display, me->core.window,
		 me->stripChart.scalegc,
		 me->core.x + where, me->core.y + y);
}

static void expose(me, event)
     StripChartJet me;
     XEvent *event;
{
  int i;

  for (i = 0; i < me->stripChart.x; i++)
    {
      drawScale(me, i);
      XDrawLine(me->core.display,
		me->core.window,
		me->stripChart.gc,
		me->core.x + i,
		me->core.y + me->core.height - 1,
		me->core.x + i,
		me->core.y + me->core.height - 1 -
		(me->core.height * me->stripChart.data[i] /
	                           me->stripChart.scale));
    }

  for (i = me->stripChart.x; i < me->core.width; i++)
    drawScale(me, i);
}

static void resize(me, size)
     StripChartJet me;
     XjSize *size;
{
  int change = 0;

  if (me->stripChart.data != NULL)
    {
      if (me->core.width != size->width)
	{
	  change++;
	  
	  if (me->stripChart.x >= size->width)
	    moveData(me, me->stripChart.x - size->width + 1);
	  
	  me->stripChart.data = 
	    (int *)XjRealloc(me->stripChart.data, size->width * sizeof(int));
	  me->core.width = size->width;
	}

      if (me->core.height != size->height)
	{
	  change++;
	  me->core.height = size->height;
	}

      if (change)
	XClearArea(me->core.display, me->core.window,
		   me->core.x, me->core.y,
		   me->core.width, me->core.height, True);
    }
  else
    {
      me->core.width = size->width;
      me->core.height = size->height;
    }
}

void XjStripChartData(me, data)
     StripChartJet me;
     int data;
{
  int oldScale;
  Boolean redraw = False;

  me->stripChart.data[me->stripChart.x++] = data;

  oldScale = me->stripChart.scale;
  if (me->stripChart.x > me->core.width)
    {
      moveData(me, me->stripChart.scrollDist);
      if (me->stripChart.scale != oldScale)
	redraw = True;
      else
	{
	  XCopyArea(me->core.display, me->core.window, me->core.window,
		    me->stripChart.gc,
		    me->core.x + me->stripChart.scrollDist,
		    me->core.y,
		    me->core.width - me->stripChart.scrollDist,
		    me->core.height,
		    me->core.x, me->core.y);

	  XClearArea(me->core.display, me->core.window,
		     me->core.x + me->core.width - me->stripChart.scrollDist,
		     me->core.y,
		     me->stripChart.scrollDist,
		     me->core.height, False);
	}
    }

  if (data > me->stripChart.scale || redraw)
    {
      if (!redraw) /* data > me->stripChart.scale */
	me->stripChart.scale += me->stripChart.scaleInc;

      XClearArea(me->core.display, me->core.window,
		 me->core.x, me->core.y,
		 me->core.width, me->core.height, False);
      expose(me, (XEvent *) NULL);
    }
  else
    {
      drawScale(me, me->stripChart.x - 1);
      XDrawLine(me->core.display,
		me->core.window,
		me->stripChart.gc,
		me->core.x + me->stripChart.x - 1,
		me->core.y + me->core.height - 1,
		me->core.x + me->stripChart.x - 1,
		me->core.y + me->core.height - 1 -
	            (me->core.height * data / me->stripChart.scale));
    }
}
