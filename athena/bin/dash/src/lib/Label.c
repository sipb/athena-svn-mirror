/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/Label.c,v $
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
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/Label.c,v 1.2 1993-07-02 00:46:57 vanharen Exp $";
#endif

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
  { XjNjustifyX, XjCJustify, XjRJustify, sizeof(int),
      offset(label.justifyX), XjRString, XjCenterJustify },
  { XjNjustifyY, XjCJustify, XjRJustify, sizeof(int),
      offset(label.justifyY), XjRString, XjCenterJustify },
  { XjNlabel, XjCLabel, XjRString, sizeof(char *),
      offset(label.label), XjRString, "noname" },
  { XjNicon, XjCIcon, XjRPixmap, sizeof(XjPixmap *),
      offset(label.pixmap), XjRString, NULL },
  { XjNleftIcon, XjCIcon, XjRPixmap, sizeof(XjPixmap *),
      offset(label.leftPixmap), XjRString, NULL },
  { XjNrightIcon, XjCIcon, XjRPixmap, sizeof(XjPixmap *),
      offset(label.rightPixmap), XjRString, NULL },
  { XjNforeground, XjCForeground, XjRColor, sizeof(int),
      offset(label.foreground), XjRString, XjDefaultForeground },
  { XjNbackground, XjCBackground, XjRColor, sizeof(int),
      offset(label.background), XjRString, XjDefaultBackground },
  { XjNreverseVideo, XjCReverseVideo, XjRBoolean, sizeof(Boolean),
      offset(label.reverseVideo), XjRBoolean, (caddr_t)False },
  { XjNfont, XjCFont, XjRFontStruct, sizeof(XFontStruct *),
      offset(label.font), XjRString, XjDefaultFont },
  { XjNpadding, XjCPadding, XjRInt, sizeof(int),
      offset(label.padding), XjRString, "2" },
};

#undef offset

static void initialize(), expose(), realize(), querySize(), move(),
  destroy(), resize();

LabelClassRec labelClassRec = {
  {
    /* class name */		"Label",
    /* jet size   */		sizeof(LabelRec),
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

JetClass labelJetClass = (JetClass)&labelClassRec;

static void initialize(me)
     LabelJet me;
{
  me->label.inverted = 0;
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

  if (me->label.reverseVideo)
    {
      int temp;

      temp = me->label.foreground;
      me->label.foreground = me->label.background;
      me->label.background = temp;
    }

  values.foreground = me->label.foreground;
  values.background = me->label.background;
  values.font = me->label.font->fid;
  values.function = GXcopy;
  values.graphics_exposures = False;
  valuemask = ( GCForeground | GCBackground | GCFont |
	       GCFunction | GCGraphicsExposures );

  me->label.gc_reverse = XjCreateGC(me->core.display,
				    me->core.window,
				    valuemask,
				    &values);

  values.foreground = me->label.background;
  values.background = me->label.foreground;
  valuemask = ( GCForeground | GCBackground | GCFont |
	       GCFunction | GCGraphicsExposures );

  me->label.gc = XjCreateGC(me->core.display,
			    me->core.window,
			    valuemask,
			    &values);
}

static void destroy(me)
     LabelJet me;
{
  XjFreeGC(me->core.display, me->label.gc);
  XjFreeGC(me->core.display, me->label.gc_reverse);
  if(me->label.pixmap)
    XjFreePixmap(me->core.display, me->label.pixmap->pixmap);
  if(me->label.leftPixmap)
    XjFreePixmap(me->core.display, me->label.leftPixmap->pixmap);
  if(me->label.rightPixmap)
    XjFreePixmap(me->core.display, me->label.rightPixmap->pixmap);
}

static void drawLabel(me, dpy, win, gc, x, y, font, label, len, size)
     LabelJet me;
     Display *dpy;
     Window win;
     GC gc;
     int x, y;
     XFontStruct *font;
     char *label;
     int len;
     XjSize *size;
{
  char *ptr1, *ptr2;
  int width = 0;

  size->width = 0;

/*
  unsigned long valuemask;
  XGCValues values;

  valuemask = ( GCForeground | GCBackground | GCFont );
  XGetGCValues(dpy, gc, valuemask, &values);
  if (values.font != font->fid)
    {
      values.font = font->fid;
      XChangeGC(dpy, gc, valuemask, values);
    }
*/

  ptr1 = ptr2 = label;
  while(*ptr2 != '\0' && ((int) (ptr2-label)) < len)
    {
      switch(*ptr2)
	{
	case '\n':
	  if (me)
	    {
	      XDrawString(dpy, win, gc, x, y, ptr1, (int) (ptr2-ptr1));
	      x = me->label.x;
	    }
	  width = XTextWidth(font, ptr1, (int) (ptr2-ptr1));
	  if (width > size->width)
	    size->width = width;
	  y += font->ascent + font->descent;
	  ptr1 = ptr2 + 1;
	  break;

	case '\\':
	  switch(*(ptr2 + 1))
	    {
	    case '\\':
	      break;

	    default:
	      break;
	    }
	  break;

	default:
	  break;
	}
      ptr2++;
    }

  if (ptr2-ptr1)
    {
      if (me)
	XDrawString(dpy, win, gc, x, y, ptr1, (int) (ptr2-ptr1));
      width = XTextWidth(font, ptr1, (int) (ptr2-ptr1));
      if (width > size->width)
	size->width = width;
      y += font->ascent + font->descent;
    }

  size->height = y;
}

static void querySize(me, size)
     LabelJet me;
     XjSize *size;
{
  if (DEBUG)
    printf ("QS(label) '%s'\n", me->core.name);

  if (me->label.pixmap)
    {
      size->width = me->label.pixmap->width;
      size->height = me->label.pixmap->height;
    }
  else
    {
#ifndef SINGLELINE
      drawLabel(NULL, NULL, NULL, NULL, 0, 0,
		me->label.font,
		me->label.label, strlen(me->label.label),
		size);
#else
      size->width = XTextWidth(me->label.font,
			       me->label.label,
			       strlen(me->label.label));
      size->height = me->label.font->ascent + me->label.font->descent;
#endif
    }

  if (me->label.leftPixmap)
    {
      size->width += me->label.leftPixmap->width;
      size->height = MAX(size->height, me->label.leftPixmap->height);
    }

  if (me->label.rightPixmap)
    {
      size->width += me->label.rightPixmap->width;
      size->height = MAX(size->height, me->label.rightPixmap->height);
    }
}

static void move(me, x, y)
     LabelJet me;
     int x, y;
{
  if (DEBUG)
    printf ("MV(label) '%s' x=%d,y=%d\n", me->core.name, x, y);

  me->label.x += (x - me->core.x);
  me->label.y += (y - me->core.y);

  me->label.leftX += (x - me->core.x);
  me->label.leftY += (y - me->core.y);

  me->label.rightX += (x - me->core.x);
  me->label.rightY += (y - me->core.y);

  me->core.x = x;
  me->core.y = y;
}

static void resize(me, size)
     LabelJet me;
     XjSize *size;
{
  if (DEBUG)
    printf ("RS(label) '%s' w=%d,h=%d\n", me->core.name,
	    size->width, size->height);

  if (me->core.width != size->width
      || me->core.height != size->height)

    {
#ifndef SINGLELINE
      XjSize l_size;

      drawLabel(NULL, NULL, NULL, NULL, 0, 0,
		me->label.font,
		me->label.label, strlen(me->label.label),
		&l_size);
#endif

      /*  This isn't right... */
      if (me->label.gc != NULL)
	XFillRectangle(me->core.display, me->core.window,
		       me->label.gc,
		       me->core.x, me->core.y,
		       me->core.width, me->core.height);

      me->core.width = size->width;
      me->core.height = size->height;

      me->label.x = me->label.leftX = me->core.x;
      me->label.y = me->label.leftY = me->label.rightY = me->core.y;
      if (me->label.rightPixmap)
	me->label.rightX = (me->core.x + me->core.width
			    - me->label.rightPixmap->width);

      /*
       * Handle X Justification
       */
      if (me->label.justifyX == Center)
	{
	  if (me->label.pixmap)
	    me->label.x += (me->core.width - me->label.pixmap->width) / 2;
	  else
	    {
#ifndef SINGLELINE
	      me->label.x += (me->core.width - l_size.width) / 2;
#else
	      me->label.x += (me->core.width
			      - XTextWidth(me->label.font,
					   me->label.label,
					   strlen(me->label.label))) / 2;
#endif
	    }
	}

      else if (me->label.justifyX == Right)
	{
	  if (me->label.rightPixmap)
	    me->label.x = me->label.rightX - me->label.padding;
	  else
	    me->label.x = me->core.x + me->core.width - me->label.padding;

	  if (me->label.pixmap)
	    me->label.x -= me->label.pixmap->width;
	  else
	    {
#ifndef SINGLELINE
	      me->label.x -= l_size.width;
#else
	      me->label.x -= XTextWidth(me->label.font,
					me->label.label,
					strlen(me->label.label));
#endif
	    }
	}

      else			/* LEFT justify */
	{
	  me->label.x += me->label.padding;
	  if (me->label.leftPixmap)
	    me->label.x += me->label.leftPixmap->width;
	}



      /*
       * Handle Y Justification
       */
      if (me->label.justifyY == Center)
	{
	  if (me->label.pixmap)
	    me->label.y += (me->core.height - me->label.pixmap->height) / 2;
	  else
	    {
#ifndef SINGLELINE
	      me->label.y += (me->core.height - l_size.height)/2
		+ me->label.font->ascent;
#else
	      me->label.y += ((me->core.height +
			       (me->label.font->ascent /* -
				me->label.font->descent*/))
			      / 2);
#endif
	    }

	  if (me->label.leftPixmap)
	    me->label.leftY += (me->core.height -
				me->label.leftPixmap->height) / 2;
	  if (me->label.rightPixmap)
	    me->label.rightY += (me->core.height -
				 me->label.rightPixmap->height) / 2;
	}

      else if (me->label.justifyY == Bottom)
	{
	  if (me->label.pixmap)
	    me->label.y += (me->core.height - me->label.pixmap->height);
	  else
	    me->label.y += (me->core.height - me->label.font->descent);

	  if (me->label.leftPixmap)
	    me->label.leftY += (me->core.height -
				me->label.leftPixmap->height);
	  if (me->label.rightPixmap)
	    me->label.rightY += (me->core.height -
				 me->label.rightPixmap->height);
	}

      else			/* TOP justify */
	{
	  if (! me->label.pixmap)
	    me->label.y += me->label.font->ascent;
	}
    }
}


static void expose(me, event)
     Jet me;
     XEvent *event;		/* ARGSUSED */
{
  LabelJet lj = (LabelJet) me;
  XjSize size;

  if (DEBUG)
    printf ("EX(label) '%s'\n", lj->core.name);

  if (lj->label.leftPixmap)
    {
      XCopyPlane(me->core.display,
		 lj->label.leftPixmap->pixmap,
		 me->core.window,
		 (lj->label.inverted) ? lj->label.gc_reverse : lj->label.gc,
		 0, 0,
		 lj->label.leftPixmap->width,
		 lj->label.leftPixmap->height,
		 lj->label.leftX,
		 lj->label.leftY,
		 1);
    }

  if (lj->label.rightPixmap)
    {
      XCopyPlane(me->core.display,
		 lj->label.rightPixmap->pixmap,
		 me->core.window,
		 (lj->label.inverted) ? lj->label.gc_reverse : lj->label.gc,
		 0, 0,
		 lj->label.rightPixmap->width,
		 lj->label.rightPixmap->height,
		 lj->label.rightX,
		 lj->label.rightY,
		 1);
    }

  if (lj->label.pixmap)
    {
      XCopyPlane(me->core.display,
		 lj->label.pixmap->pixmap,
		 me->core.window,
		 (lj->label.inverted) ? lj->label.gc_reverse : lj->label.gc,
		 0, 0,
		 lj->label.pixmap->width,
		 lj->label.pixmap->height,
		 lj->label.x,
		 lj->label.y,
		 1);
    }
  else
    {
#ifndef SINGLELINE
      drawLabel(lj, me->core.display, me->core.window,
		(lj->label.inverted) ? lj->label.gc : lj->label.gc_reverse,
		lj->label.x, lj->label.y,
		lj->label.font,
		lj->label.label, strlen(lj->label.label),
		&size);
#else
      XDrawString(me->core.display, me->core.window,
		  (lj->label.inverted) ? lj->label.gc : lj->label.gc_reverse,
		  lj->label.x, lj->label.y,
		  lj->label.label, strlen(lj->label.label));
#endif
    }
}

void setLabel(me, label)
     LabelJet me;
     char *label;
{
  if (me->label.pixmap)
    return;

  if (me->label.gc != NULL)
    XFillRectangle(me->core.display, me->core.window,
		   me->label.gc,
		   me->core.x, me->core.y,
		   me->core.width, me->core.height);
  me->label.label = label;
  expose((Jet) me, NULL);
}

void setPixmap(me, pixmap)
     LabelJet me;
     XjPixmap *pixmap;
{
  if (me->label.gc != NULL)
    XFillRectangle(me->core.display, me->core.window,
		   me->label.gc,
		   me->core.x, me->core.y,
		   me->core.width, me->core.height);
  me->label.pixmap = pixmap;
  expose((Jet) me, NULL);
}

void setState(me, which, state)
     LabelJet me;
     int which;			/* ARGSUSED */
     Boolean state;
{
  me->label.inverted = state;

  expose((Jet) me, NULL);
}
