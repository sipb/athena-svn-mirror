/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/xcluster/Label.c,v $
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
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/xcluster/Label.c,v 1.1 1992-04-29 13:55:45 cfields Exp $";
#endif	lint

#include "mit-copyright.h"
#include <stdio.h>
#include "Jets.h"
#include "Label.h"

extern int DEBUG;

#define offset(field) XjOffset(LabelJet,field)

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
      offset(label.centerX), XjRBoolean, (caddr_t)False },
  { XjNcenterY, XjCCenter, XjRBoolean, sizeof(Boolean),
      offset(label.centerY), XjRBoolean, (caddr_t)False },
  { XjNlabel, XjCLabel, XjRString, sizeof(char *),
      offset(label.label), XjRString, "noname" },
  { XjNforeground, XjCForeground, XjRColor, sizeof(int),
      offset(label.foreground), XjRString, XjDefaultForeground },
  { XjNbackground, XjCBackground, XjRColor, sizeof(int),
      offset(label.background), XjRString, XjDefaultBackground },
  { XjNreverseVideo, XjCReverseVideo, XjRBoolean, sizeof(Boolean),
      offset(label.reverseVideo), XjRBoolean, (caddr_t)False },
  { XjNfont, XjCFont, XjRFontStruct, sizeof(XFontStruct *),
      offset(label.font), XjRString, XjDefaultFont }
};

#undef offset

static void initialize(), expose(), realize(), querySize(), move(),
  destroy(), resize();

LabelClassRec labelClassRec = {
  {
    /* class name */	"Label",
    /* jet size   */	sizeof(LabelRec),
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

JetClass labelJetClass = (JetClass)&labelClassRec;

static void initialize(me)
     LabelJet me;
{
  me->label.x = me->core.x;
  me->label.y = me->core.y;
}

/*
 * Things are currently broken screenwise.
 * It will be fun to fix later. :)
 */
static void realize(me)
     LabelJet me;
{
  unsigned long valuemask;
  XGCValues values;

  values.function = GXcopy;
  values.foreground = me->label.reverseVideo ?
    me->label.foreground : me->label.background;
  valuemask = GCForeground | GCFunction;

  me->label.gc_clear = XCreateGC(me->core.display,
				 me->core.window,
				 valuemask,
				 &values);

  values.function = GXcopy;
  values.foreground = me->label.reverseVideo ?
    me->label.background : me->label.foreground;
  values.font = me->label.font->fid;
  valuemask = GCForeground | GCFont | GCFunction;

  me->label.gc = XCreateGC(me->core.display,
			   me->core.window,
			   valuemask,
			   &values);
}

static void destroy(me)
     LabelJet me;
{
  XFreeGC(me->core.display, me->label.gc);
  XFreeGC(me->core.display, me->label.gc_clear);
}

static void querySize(me, size)
     LabelJet me;
     XjSize *size;
{
  if (DEBUG)
    printf ("QS(label) '%s'\n", me->label.label);

  size->width = XTextWidth(me->label.font,
			   me->label.label,
			   strlen(me->label.label));

  size->height = me->label.font->ascent + me->label.font->descent;
}

static void move(me, x, y)
     LabelJet me;
     int x, y;
{
  if (DEBUG)
    printf ("MV(label) '%s'\n", me->label.label);

  me->label.x += (x - me->core.x);
  me->label.y += (y - me->core.y);

  me->core.x = x;
  me->core.y = y;
}

static void resize(me, size)
     LabelJet me;
     XjSize *size;
{
  if (DEBUG)
    printf ("RSZ(label) '%s'\n", me->label.label);

  if (me->core.width != size->width
      || me->core.height != size->height)

    {
      /*  This isn't right... */
      if (me->label.gc_clear != NULL)
	XFillRectangle(me->core.display, me->core.window,
		       me->label.gc_clear,
		       me->core.x, me->core.y,
		       me->core.width, me->core.height);

      me->core.width = size->width;
      me->core.height = size->height;

      me->label.x = me->core.x;

      if (me->label.centerX)
	me->label.x += (me->core.width
			- XTextWidth(me->label.font,
				     me->label.label,
				     strlen(me->label.label))) / 2;

      me->label.y = me->core.y + me->label.font->ascent;
      if (me->label.centerY)
	me->label.y += (me->core.height -
			(me->label.font->ascent + me->label.font->descent)) / 2;
    }
}

static void expose(me, event)
     LabelJet me;
     XEvent *event;
{
  if (DEBUG)
    printf ("EXP(label) '%s'\n", me->label.label);

  XDrawString(me->core.display, me->core.window,
	      me->label.gc,
	      me->label.x, me->label.y,
	      me->label.label, strlen(me->label.label));
}

void setLabel(me, label)
     LabelJet me;
     char *label;
{
  me->label.label = label;
}
