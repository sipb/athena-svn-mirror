/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/SmplLabel.c,v $
 * $Author: vanharen $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#if  (!defined(lint))  &&  (!defined(SABER))
static char *rcsid =
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/SmplLabel.c,v 1.1 1993-07-02 00:25:35 vanharen Exp $";
#endif

#include "mit-copyright.h"
#include <stdio.h>
#include "Jets.h"
#include "SmplLabel.h"

extern int DEBUG;

#define offset(field) XjOffset(SmplLabelJet,field)

static XjResource resources[] = {
  { XjNx, XjCX, XjRInt, sizeof(int),
      offset(core.x), XjRString, XjInheritValue },
  { XjNy, XjCY, XjRInt, sizeof(int),
      offset(core.y), XjRString, XjInheritValue },
  { XjNwidth, XjCWidth, XjRInt, sizeof(int),
      offset(core.width), XjRString, XjInheritValue },
  { XjNheight, XjCHeight, XjRInt, sizeof(int),
      offset(core.height), XjRString, XjInheritValue },
  { XjNjustifyX, XjCJustify, XjRJustify, sizeof(int),
      offset(s_label.justifyX), XjRString, XjCenterJustify },
  { XjNjustifyY, XjCJustify, XjRJustify, sizeof(int),
      offset(s_label.justifyY), XjRString, XjCenterJustify },
  { XjNlabel, XjCLabel, XjRString, sizeof(char *),
      offset(s_label.label), XjRString, "noname" },
  { XjNicon, XjCIcon, XjRPixmap, sizeof(XjPixmap *),
      offset(s_label.pixmap), XjRString, NULL },
  { XjNforeground, XjCForeground, XjRColor, sizeof(int),
      offset(s_label.foreground), XjRString, XjDefaultForeground },
  { XjNbackground, XjCBackground, XjRColor, sizeof(int),
      offset(s_label.background), XjRString, XjDefaultBackground },
  { XjNreverseVideo, XjCReverseVideo, XjRBoolean, sizeof(Boolean),
      offset(s_label.reverseVideo), XjRBoolean, (caddr_t)False },
  { XjNfont, XjCFont, XjRFontStruct, sizeof(XFontStruct *),
      offset(s_label.font), XjRString, XjDefaultFont },
  { XjNpadding, XjCPadding, XjRInt, sizeof(int),
      offset(s_label.padding), XjRString, "2" },
};

#undef offset

static void initialize(), expose(), realize(), querySize(), move(),
  destroy(), resize();

SmplLabelClassRec smplLabelClassRec = {
  {
    /* class name */		"SimpleLabel",
    /* jet size   */		sizeof(SmplLabelRec),
    /* classInitialize */	NULL,
    /* classInitialized? */	1,
    /* initialize */		initialize,
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

JetClass smplLabelJetClass = (JetClass)&smplLabelClassRec;

static void initialize(me)
     SmplLabelJet me;
{
  me->s_label.inverted = 0;
  me->s_label.x = me->core.x;
  me->s_label.y = me->core.y;
}

/*
 * Things are currently broken screenwise.
 * It will be fun to fix later. :)
 */
static void realize(me)
     SmplLabelJet me;
{
  unsigned long valuemask;
  XGCValues values;

  if (me->s_label.reverseVideo)
    {
      int temp;

      temp = me->s_label.foreground;
      me->s_label.foreground = me->s_label.background;
      me->s_label.background = temp;
    }

  values.foreground = me->s_label.foreground;
  values.background = me->s_label.background;
  values.font = me->s_label.font->fid;
  values.function = GXcopy;
  values.graphics_exposures = False;
  valuemask = ( GCForeground | GCBackground | GCFont |
	       GCFunction | GCGraphicsExposures );

  me->s_label.gc_reverse = XjCreateGC(me->core.display,
				    me->core.window,
				    valuemask,
				    &values);

  values.foreground = me->s_label.background;
  values.background = me->s_label.foreground;
  valuemask = ( GCForeground | GCBackground | GCFont |
	       GCFunction | GCGraphicsExposures );

  me->s_label.gc = XjCreateGC(me->core.display,
			    me->core.window,
			    valuemask,
			    &values);
}

static void destroy(me)
     SmplLabelJet me;
{
  XjFreeGC(me->core.display, me->s_label.gc);
  XjFreeGC(me->core.display, me->s_label.gc_reverse);
  if(me->s_label.pixmap)
    XjFreePixmap(me->core.display, me->s_label.pixmap->pixmap);
}

static void querySize(me, size)
     SmplLabelJet me;
     XjSize *size;
{
  if (DEBUG)
    printf ("QS(label) '%s'\n", me->core.name);

  if (me->s_label.pixmap)
    {
      size->width = me->s_label.pixmap->width;
      size->height = me->s_label.pixmap->height;
    }
  else
    {
      size->width = XTextWidth(me->s_label.font,
			       me->s_label.label,
			       strlen(me->s_label.label));
      size->height = me->s_label.font->ascent + me->s_label.font->descent;
    }
}

static void move(me, x, y)
     SmplLabelJet me;
     int x, y;
{
  if (DEBUG)
    printf ("MV(label) '%s' x=%d,y=%d\n", me->core.name, x, y);

  me->s_label.x += (x - me->core.x);
  me->s_label.y += (y - me->core.y);

  me->core.x = x;
  me->core.y = y;
}

static void resize(me, size)
     SmplLabelJet me;
     XjSize *size;
{
  if (DEBUG)
    printf ("RS(label) '%s' w=%d,h=%d\n", me->core.name,
	    size->width, size->height);

  if (me->core.width != size->width
      || me->core.height != size->height)

    {
      /*  This isn't right... */
      if (me->s_label.gc != NULL)
	XFillRectangle(me->core.display, me->core.window,
		       me->s_label.gc,
		       me->core.x, me->core.y,
		       me->core.width, me->core.height);

      me->core.width = size->width;
      me->core.height = size->height;

      me->s_label.x = me->core.x;
      me->s_label.y = me->core.y;

      /*
       * Handle X Justification
       */
      if (me->s_label.justifyX == Center)
	{
	  if (me->s_label.pixmap)
	    me->s_label.x += (me->core.width - me->s_label.pixmap->width) / 2;
	  else
	    me->s_label.x += (me->core.width
			      - XTextWidth(me->s_label.font,
					   me->s_label.label,
					   strlen(me->s_label.label))) / 2;
	}

      else if (me->s_label.justifyX == Right)
	{
	  me->s_label.x = me->core.x + me->core.width - me->s_label.padding;

	  if (me->s_label.pixmap)
	    me->s_label.x -= me->s_label.pixmap->width;
	  else
	    me->s_label.x -= XTextWidth(me->s_label.font,
					me->s_label.label,
					strlen(me->s_label.label));
	}

      else			/* LEFT justify */
	me->s_label.x += me->s_label.padding;



      /*
       * Handle Y Justification
       */
      if (me->s_label.justifyY == Center)
	{
	  if (me->s_label.pixmap)
	    me->s_label.y += (me->core.height - me->s_label.pixmap->height)/2;
	  else
	    me->s_label.y += ((me->core.height +
			       (me->s_label.font->ascent -
				me->s_label.font->descent))
			      / 2);
	}

      else if (me->s_label.justifyY == Bottom)
	{
	  if (me->s_label.pixmap)
	    me->s_label.y += (me->core.height - me->s_label.pixmap->height);
	  else
	    me->s_label.y += (me->core.height - me->s_label.font->descent);
	}

      else			/* TOP justify */
	{
	  if (! me->s_label.pixmap)
	    me->s_label.y += me->s_label.font->ascent;
	}
    }
}

static void expose(me, event)
     Jet me;
     XEvent *event;		/* ARGSUSED */
{
  SmplLabelJet slj = (SmplLabelJet) me;

  if (DEBUG)
    printf ("EX(label) '%s'\n", slj->core.name);

  if (slj->s_label.pixmap)
    {
      XCopyPlane(me->core.display,
		 slj->s_label.pixmap->pixmap,
		 me->core.window,
		 ((slj->s_label.inverted)
		  ? slj->s_label.gc_reverse : slj->s_label.gc),
		 0, 0,
		 slj->s_label.pixmap->width,
		 slj->s_label.pixmap->height,
		 slj->s_label.x,
		 slj->s_label.y,
		 1);
    }
  else
    XDrawString(me->core.display, me->core.window,
		((slj->s_label.inverted)
		 ? slj->s_label.gc : slj->s_label.gc_reverse),
		slj->s_label.x, slj->s_label.y,
		slj->s_label.label, strlen(slj->s_label.label));
}

void smplSetLabel(me, label)
     SmplLabelJet me;
     char *label;
{
  if (me->s_label.pixmap)
    return;

  if (me->s_label.gc != NULL)
    XFillRectangle(me->core.display, me->core.window,
		   me->s_label.gc,
		   me->core.x, me->core.y,
		   me->core.width, me->core.height);
  me->s_label.label = label;
  expose((Jet) me, NULL);
}

void smplSetPixmap(me, pixmap)
     SmplLabelJet me;
     XjPixmap *pixmap;
{
  if (me->s_label.gc != NULL)
    XFillRectangle(me->core.display, me->core.window,
		   me->s_label.gc,
		   me->core.x, me->core.y,
		   me->core.width, me->core.height);
  me->s_label.pixmap = pixmap;
  expose((Jet) me, NULL);
}

void smplSetState(me, which, state)
     SmplLabelJet me;
     int which;			/* ARGSUSED */
     Boolean state;
{
  me->s_label.inverted = state;

  expose((Jet) me, NULL);
}
