/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/Arrow.c,v $
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
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/Arrow.c,v 1.1 1993-07-02 01:34:33 vanharen Exp $";
#endif

#include "mit-copyright.h"
#include <stdio.h>
#include "Jets.h"
#include "Arrow.h"

extern int DEBUG;

#define offset(field) XjOffset(ArrowJet,field)

static XjResource resources[] = {
  { XjNx, XjCX, XjRInt, sizeof(int),
      offset(core.x), XjRString, XjInheritValue },
  { XjNy, XjCY, XjRInt, sizeof(int),
      offset(core.y), XjRString, XjInheritValue },
  { XjNwidth, XjCWidth, XjRInt, sizeof(int),
      offset(core.width), XjRString, "15" },
  { XjNheight, XjCHeight, XjRInt, sizeof(int),
      offset(core.height), XjRString, "15" },
  { XjNforeground, XjCForeground, XjRColor, sizeof(int),
      offset(arrow.foreground), XjRString, XjDefaultForeground },
  { XjNbackground, XjCBackground, XjRColor, sizeof(int),
      offset(arrow.background), XjRString, XjDefaultBackground },
  { XjNfillColor, XjCForeground, XjRColor, sizeof(int),
      offset(arrow.fillColor), XjRString, XjDefaultForeground },
  { XjNreverseVideo, XjCReverseVideo, XjRBoolean, sizeof(Boolean),
      offset(arrow.reverseVideo), XjRBoolean, (caddr_t) False },
  { XjNpadding, XjCPadding, XjRInt, sizeof(int),
      offset(arrow.padding), XjRString, "2" },
  { XjNdirection, XjCDirection, XjRDirection, sizeof(int),
      offset(arrow.direction), XjRString, XjNorth },
  { XjNhighlightProc, XjCHighlightProc, XjRCallback, sizeof(XjCallback *),
      offset(arrow.highlightProc), XjRString, "fillArrow(1)" },
  { XjNunHighlightProc, XjCHighlightProc, XjRCallback, sizeof(XjCallback *),
      offset(arrow.unHighlightProc), XjRString, "fillArrow(0)" },
};

#undef offset

static void classInitialize(), expose(), realize(),
  querySize(), move(), destroy(), resize(), computePoints();

int fillArrow();

ArrowClassRec arrowClassRec = {
  {
    /* class name */		"Arrow",
    /* jet size   */		sizeof(ArrowRec),
    /* classInitialize */	classInitialize,
    /* classInitialized? */	0,
    /* initialize */		NULL,
    /* prerealize */    	NULL,
    /* realize */		realize,
    /* event */			NULL,
    /* expose */		expose,
    /* querySize */     	querySize,
    /* move */			move,
    /* resize */        	resize,
    /* destroy */       	destroy,
    /* resources */		resources,
    /* number of 'em */		XjNumber(resources)
  }
};

JetClass arrowJetClass = (JetClass)&arrowClassRec;


static void classInitialize(me)
     ArrowJet me;
{
  XjRegisterCallback(fillArrow, "fillArrow");
}


/*
 * Things are currently broken screenwise.
 * It will be fun to fix later. :)
 */
static void realize(me)
     ArrowJet me;
{
  unsigned long valuemask;
  XGCValues values;

  if (me->arrow.reverseVideo)
    {
      int temp;

      temp = me->arrow.foreground;
      me->arrow.foreground = me->arrow.background;
      me->arrow.background = temp;
      if (me->arrow.fillColor == me->arrow.background)
	me->arrow.fillColor = me->arrow.foreground;
    }

  values.foreground = me->arrow.foreground;
  values.background = me->arrow.background;
  values.function = GXcopy;
  values.graphics_exposures = False;
  valuemask = ( GCForeground | GCBackground |
	       GCFunction | GCGraphicsExposures );

  me->arrow.gc_reverse = XjCreateGC(me->core.display,
				    me->core.window,
				    valuemask,
				    &values);

  values.foreground = me->arrow.background;
  values.background = me->arrow.foreground;
  valuemask = ( GCForeground | GCBackground |
	       GCFunction | GCGraphicsExposures );

  me->arrow.gc = XjCreateGC(me->core.display,
			    me->core.window,
			    valuemask,
			    &values);

  values.foreground = me->arrow.fillColor;
  valuemask = ( GCForeground | GCBackground |
	       GCFunction | GCGraphicsExposures );

  me->arrow.gc_fill = XjCreateGC(me->core.display,
				 me->core.window,
				 valuemask,
				 &values);
}

static void destroy(me)
     ArrowJet me;
{
  XjFreeGC(me->core.display, me->arrow.gc);
  XjFreeGC(me->core.display, me->arrow.gc_reverse);
  XjFreeGC(me->core.display, me->arrow.gc_fill);
}

static void querySize(me, size)
     ArrowJet me;
     XjSize *size;
{
  if (DEBUG)
    printf ("QS(arrow) '%s' w=%d,h=%d\n", me->core.name, me->core.width,
	    me->core.height);

  size->width = me->core.width;
  size->height = me->core.height;
}

static void move(me, x, y)
     ArrowJet me;
     int x, y;
{
  if (DEBUG)
    printf ("MV(arrow) '%s' x=%d,y=%d\n", me->core.name, x, y);

  me->core.x = x;
  me->core.y = y;
  computePoints(me);
}

static void resize(me, size)
     Jet me;
     XjSize *size;
{
  if (DEBUG)
    printf ("RS(arrow) '%s' w=%d,d=%d\n",
	    me->core.name, size->width, size->height);

  if (me->core.width != size->width
      || me->core.height != size->height)
    {
      XClearArea(me->core.display, me->core.window,
		 me->core.x, me->core.y,
		 me->core.width, me->core.height,
		 None);

      me->core.width = size->width;
      me->core.height = size->height;

      computePoints(me);
    }
}


static void expose(me, event)
     Jet me;
     XEvent *event;		/* ARGSUSED */
{
  ArrowJet aj = (ArrowJet) me;

  XFillPolygon(me->core.display, me->core.window,
	       aj->arrow.gc,
	       aj->arrow.pt, aj->arrow.num_pts,
	       Nonconvex, CoordModeOrigin);
  XDrawLines(me->core.display, me->core.window,
	     aj->arrow.gc_reverse,
	     aj->arrow.pt, aj->arrow.num_pts, CoordModeOrigin);
}


int fillArrow(me, fill, data)
     Jet me;
     int fill;
     caddr_t data;		/* ARGSUSED */
{
  ArrowJet aj = (ArrowJet) me;

  if (fill)
    XFillPolygon(me->core.display, me->core.window,
		 aj->arrow.gc_fill,
		 aj->arrow.pt, aj->arrow.num_pts,
		 Nonconvex, CoordModeOrigin);
  else
    XFillPolygon(me->core.display, me->core.window,
		 aj->arrow.gc,
		 aj->arrow.pt, aj->arrow.num_pts,
		 Nonconvex, CoordModeOrigin);
  XDrawLines(me->core.display, me->core.window,
	     aj->arrow.gc_reverse,
	     aj->arrow.pt, aj->arrow.num_pts, CoordModeOrigin);
  return 0;
}


void setDirection(me, dir)
     ArrowJet me;
     int dir;
{
  XClearArea(me->core.display, me->core.window,
	     me->core.x, me->core.y,
	     me->core.width, me->core.height,
	     None);
  me->arrow.direction = dir;

  computePoints((Jet) me);
  expose((Jet) me, NULL);
}



static void computePoints(me)
     Jet me;
{
  ArrowJet aj = (ArrowJet) me;
  int length = MIN(me->core.width, me->core.height);
  int length2 = length - (2 * aj->arrow.padding);

  aj->arrow.x = me->core.x + (me->core.width - length)/2;
  aj->arrow.y = me->core.y + (me->core.height - length)/2;

  if (aj->arrow.direction % 2)	/* the diagonals are all "odd" */
    {
/*
 *  For an arrow pointing northeast, the points go something like this:
 *
 *  07-----6	Point 0 and 7 are identical, to close the figure.
 *   \     |
 *    1    |	For the other diagonals, you get the idea...
 *   /     |
 *  2   4  |
 *   \ / \ |
 *    3   \5
 */
      int i;
      double len25 = length2 * .25;
      double len35 = length2 * .35;
      double len40 = length2 * .40;

#define PT aj->arrow.pt
#define PAD aj->arrow.padding
#define len15  (len40-len25)
#define len75  (len35+len40)
#define len100 (len25+len75)
#define LEN  (PAD + length2 - 1)

      aj->arrow.num_pts = 8;

      /*
       * Assign the "X" coordinates first...
       */
      switch (aj->arrow.direction)
	{
	case 1:		/* northeast */
	case 3:		/* southeast */
	  PT[0].x = PT[7].x = PAD + len15;
	  PT[1].x = PAD + len40;
	  PT[2].x = PAD;
	  PT[3].x = PAD + len35;
	  PT[4].x = PAD + len75;
	  PT[5].x = PT[6].x = PAD + length2;
	  break;

	case 5:		/* southwest */
	  break;

	case 7:		/* northwest */
	  break;
	}

      /*
       * Now assign the "Y" coordinates...
       */
      switch (aj->arrow.direction)
	{
	case 1:		/* northeast */
	  PT[0].y = PT[6].y = PT[7].y = LEN - len100;
	  PT[1].y = LEN - len75;
	  PT[2].y = LEN - len35;
	  PT[3].y = LEN;
	  PT[4].y = LEN - len40;
	  PT[5].y = LEN - len15 /*len40 + len25*/;
	  break;

	case 3:		/* southeast */
	  PT[0].y = PT[6].y = PT[7].y = PAD + len100;
	  PT[1].y = PAD + len75;
	  PT[2].y = PAD + len35;
	  PT[3].y = PAD;
	  PT[4].y = PAD + len40;
	  PT[5].y = PAD + len15;
	  break;

	case 2:		/* east */
	case 6:		/* west */
	  break;
	}

      for (i=0; i < aj->arrow.num_pts; i++)
	{
	  PT[i].x += aj->arrow.x;
	  PT[i].y += aj->arrow.y;
	}
    }
  else				/* N, E, S, W are all "even" */
    {
/*
 *  For an arrow pointing north, the points go something like this: 
 *      0--9       
 *     /    \	   Points 0 and 9 are distinct points, because of
 *    /      \	   the fact that we divide the width by 2 in order
 *   /        \	   to get the midpoint, and so for an even-width
 *  1          8   arrow, the "tip" of the arrow will be off-center
 *  2--3    6--7   by one pixel.
 *     |    |
 *     4----5	   Similar ordering for the other major directions.
 */
      int l[8], x[8], y[8], i;
      int length3 = (length + 1)/2 - 1;
      int length4 = length2 * 2/7;

      aj->arrow.num_pts = 10;

      l[0] = aj->arrow.padding;
      l[1] = length3;
      l[2] = length/2;
      l[3] = aj->arrow.padding + length2 - 1;
      l[4] = aj->arrow.padding + length4;
      l[5] = l[3] - length4;
      l[6] = length3 + 1;
      l[7] = l[2] - 1;

      for (i=0; i<8; i++)
	{
	  x[i] = l[i] + aj->arrow.x;
	  y[i] = l[i] + aj->arrow.y;
	}

      /*
       * Assign the "X" coordinates first...
       */
      switch (aj->arrow.direction)
	{
	case 0:		/* north */
	case 4:		/* south */
	  PT[0].x = x[1];
	  PT[1].x = PT[2].x = x[0];
	  PT[3].x = PT[4].x = x[4];
	  PT[5].x = PT[6].x = x[5];
	  PT[7].x = PT[8].x = x[3];
	  PT[9].x = x[2];
	  break;

	case 2:		/* east */
	  PT[0].x = PT[9].x = x[3];
	  PT[1].x = PT[8].x = x[2];
	  PT[2].x = PT[3].x = PT[6].x = PT[7].x = x[7];
	  PT[4].x = PT[5].x = x[0];
	  break;

	case 6:		/* west */
	  PT[0].x = PT[9].x = x[0];
	  PT[1].x = PT[8].x = x[1];
	  PT[2].x = PT[3].x = PT[6].x = PT[7].x = x[6];
	  PT[4].x = PT[5].x = x[3];
	  break;
	}

      /*
       * Now assign the "Y" coordinates...
       */
      switch (aj->arrow.direction)
	{
	case 0:		/* north */
	  PT[0].y = PT[9].y = y[0];
	  PT[1].y = PT[8].y = y[1];
	  PT[2].y = PT[3].y = PT[6].y = PT[7].y = y[6];
	  PT[4].y = PT[5].y = y[3];
	  break;

	case 4:		/* south */
	  PT[0].y = PT[9].y = y[3];
	  PT[1].y = PT[8].y = y[2];
	  PT[2].y = PT[3].y = PT[6].y = PT[7].y = y[7];
	  PT[4].y = PT[5].y = y[0];
	  break;

	case 2:		/* east */
	case 6:		/* west */
	  PT[0].y = y[1];
	  PT[1].y = PT[2].y = y[0];
	  PT[3].y = PT[4].y = y[4];
	  PT[5].y = PT[6].y = y[5];
	  PT[7].y = PT[8].y = y[3];
	  PT[9].y = y[2];
	  break;
#undef PT
	}
    }
}
