/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/Icon.c,v $
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
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/Icon.c,v 1.3 1991-12-17 10:29:55 vanharen Exp $";
#endif

#include "mit-copyright.h"
#include <stdio.h>
#include "Jets.h"
#include "Icon.h"

extern int DEBUG;

#define offset(field) XjOffset(IconJet,field)

static XjResource resources[] = {
  { XjNx, XjCX, XjRInt, sizeof(int),
     offset(core.x), XjRString, XjInheritValue },
  { XjNy, XjCY, XjRInt, sizeof(int),
     offset(core.y), XjRString, XjInheritValue },
  { XjNwidth, XjCWidth, XjRInt, sizeof(int),
     offset(core.width), XjRString, XjInheritValue },
  { XjNheight, XjCHeight, XjRInt, sizeof(int),
     offset(core.height), XjRString, XjInheritValue },
  { XjNcenterX, XjCCenter, XjRBoolean, sizeof(Boolean),
     offset(icon.centerX), XjRBoolean, (caddr_t)False },
  { XjNcenterY, XjCCenter, XjRBoolean, sizeof(Boolean),
     offset(icon.centerY), XjRBoolean, (caddr_t)False },
  { XjNforeground, XjCForeground, XjRColor, sizeof(int),
      offset(icon.foreground), XjRString, XjDefaultForeground },
  { XjNbackground, XjCBackground, XjRColor, sizeof(int),
      offset(icon.background), XjRString, XjDefaultBackground },
  { XjNreverseVideo, XjCReverseVideo, XjRBoolean, sizeof(Boolean),
      offset(icon.reverseVideo), XjRBoolean, (caddr_t)False },
  { XjNicon, XjCIcon, XjRPixmap, sizeof(XjPixmap *),
      offset(icon.pixmap), XjRString, NULL },
};

#undef offset

static void initialize(), expose(), realize(), querySize(), move(),
  destroy(), resize();

IconClassRec iconClassRec = {
  {
    /* class name */	"Icon",
    /* jet size   */	sizeof(IconRec),
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

JetClass iconJetClass = (JetClass)&iconClassRec;

static void initialize(me)
     IconJet me;
{
  me->icon.inverted = 0;
  me->icon.x = me->core.x;
  me->icon.y = me->core.y;
}

/*
 * Things are currently broken screenwise.
 * It will be fun to fix later. :)
 */
static void realize(me)
     IconJet me;
{
  unsigned long valuemask;
  XGCValues values;

  if (me->icon.reverseVideo)
    {
      int temp;

      temp = me->icon.foreground;
      me->icon.foreground = me->icon.background;
      me->icon.background = temp;
    }

  values.function = GXcopy;
  values.foreground = me->icon.foreground;
  values.background = me->icon.background;
  values.graphics_exposures = False;
  valuemask = ( GCForeground | GCBackground | GCFunction
	       | GCGraphicsExposures );

  me->icon.gc = XjCreateGC(me->core.display,
			   me->core.window,
			   valuemask,
			   &values);

  values.foreground = me->icon.background;
  values.background = me->icon.foreground;

  me->icon.reversegc = XjCreateGC(me->core.display,
				  me->core.window,
				  valuemask,
				  &values);

}

static void destroy(me)
     IconJet me;
{
  XjFreeGC(me->core.display, me->icon.gc);
  XjFreeGC(me->core.display, me->icon.reversegc);
  if(me->icon.pixmap)
    XjFreePixmap(me->core.display, me->icon.pixmap->pixmap);
}

static void querySize(me, size)
     IconJet me;
     XjSize *size;
{
  if(!me->icon.pixmap)
    {
      size->width = 0;
      size->height = 0;
      return;
    }
  size->width = me->icon.pixmap->width;
  size->height = me->icon.pixmap->height;
}

static void move(me, x, y)
     IconJet me;
     int x, y;
{
  me->icon.x += (x - me->core.x);
  me->icon.y += (y - me->core.y);

  me->core.x = x;
  me->core.y = y;
}

static void resize(me, size)
     IconJet me;
     XjSize *size;
{
  me->core.width = size->width;
  me->core.height = size->height;

  me->icon.x = me->core.x;

  if (me->icon.centerX && me->icon.pixmap)
    me->icon.x += (me->core.width - me->icon.pixmap->width) / 2;

  me->icon.y = me->core.y;
  if (me->icon.centerY && me->icon.pixmap)
    me->icon.y += (me->core.height - me->icon.pixmap->height) / 2;

}

static void expose(me, event)
     IconJet me;
     XEvent *event;
{
  if (DEBUG)
    printf("Icon expose on: %s\n", me->core.name);

  if (me->icon.pixmap)
    {
/*
      int x = MAX(0, event->xexpose.x - me->icon.x);
      int y = MAX(0, event->xexpose.y - me->icon.y);

      XCopyPlane(me->core.display,
		 me->icon.pixmap->pixmap,
		 me->core.window,
		 (me->icon.inverted) ? me->icon.gc : me->icon.reversegc,
		 x, y,
		 MIN(event->xexpose.width, me->icon.pixmap->width - x),
		 MIN(event->xexpose.height, me->icon.pixmap->height - y),
		 MAX(me->icon.x, event->xexpose.x),
		 MAX(me->icon.y, event->xexpose.y),
		 1);
*/
      XCopyPlane(me->core.display,
		 me->icon.pixmap->pixmap,
		 me->core.window,
		 (me->icon.inverted) ? me->icon.gc : me->icon.reversegc,
		 0, 0,
		 me->icon.pixmap->width,
		 me->icon.pixmap->height,
		 me->icon.x,
		 me->icon.y,
		 1);
    }
}

void SetIcon(me, state)
     IconJet me;
     Boolean state;
{
  me->icon.inverted = state;

  if (me->icon.pixmap)
    {
      XCopyPlane(me->core.display,
		 me->icon.pixmap->pixmap,
		 me->core.window,
		 state ? me->icon.gc : me->icon.reversegc,
		 0, 0,
		 me->icon.pixmap->width,
		 me->icon.pixmap->height,
		 me->icon.x,
		 me->icon.y,
		 1);
    }
}
