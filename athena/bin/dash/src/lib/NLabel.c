/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/NLabel.c,v $
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
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/NLabel.c,v 1.1 1993-07-02 00:26:12 vanharen Exp $";
#endif

#include "mit-copyright.h"
#include <stdio.h>
#include "Jets.h"
#include "NLabel.h"

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
  char *jetname;
  int len;
  XjSize child_size;
  int x, y;

  me->label.inverted = 0;
  x = me->label.x = me->core.x;
  y = me->label.y = me->label.leftY = me->label.middleY =
    me->label.rightY = me->core.y;
  me->label.leftJet = me->label.middleJet = me->label.rightJet = NULL;

  if (me->label.reverseVideo)
    {
      int temp;

      temp = me->label.foreground;
      me->label.foreground = me->label.background;
      me->label.background = temp;
    }

  len = strlen(me->core.name);
  jetname = XjMalloc(len + 3);
  strcpy(jetname, me->core.name);

  /*
   * Left Jet
   */
  strcat(jetname, "lj");
  if (me->label.leftPixmap)
    {
      me->label.leftX = x;
      me->label.leftJet = (SmplLabelJet)
	XjVaCreateJet(jetname, smplLabelJetClass, me,
		      XjNicon, me->label.leftPixmap,
		      XjNforeground, me->label.foreground,
		      XjNbackground, me->label.background,
		      XjNreverseVideo,
		      me->label.reverseVideo,
		      XjNfont, me->label.font,
		      XjNjustifyX, me->label.justifyX,
		      XjNjustifyY, me->label.justifyY,
		      XjNx, x,
		      XjNy, y,
		      XjNpadding, me->label.padding,
		      NULL, NULL);
      XjQuerySize(me->label.leftJet, &child_size);
      x += child_size.width;
    }

  /*
   * Middle Jet
   */
  jetname[len] = 'm';
  me->label.middleX = x;
  me->label.middleJet = (SmplLabelJet)
    XjVaCreateJet(jetname, smplLabelJetClass, me,
		  XjNlabel, me->label.label,
		  XjNicon, me->label.pixmap,
		  XjNforeground, me->label.foreground,
		  XjNbackground, me->label.background,
		  XjNreverseVideo, me->label.reverseVideo,
		  XjNfont, me->label.font,
		  XjNjustifyX, me->label.justifyX,
		  XjNjustifyY, me->label.justifyY,
		  XjNx, x,
		  XjNy, y,
		  XjNpadding, me->label.padding,
		  NULL, NULL);

  /*
   * Right Jet
   */
  jetname[len] = 'r';
  if (me->label.rightPixmap)
    {
      XjQuerySize(me->label.middleJet, &child_size);
      x += child_size.width;

      me->label.rightX = x;
      me->label.rightJet = (SmplLabelJet)
	XjVaCreateJet(jetname, smplLabelJetClass, me,
		      XjNicon, me->label.rightPixmap,
		      XjNforeground, me->label.foreground,
		      XjNbackground, me->label.background,
		      XjNreverseVideo, me->label.reverseVideo,
		      XjNfont, me->label.font,
		      XjNjustifyX, me->label.justifyX,
		      XjNjustifyY, me->label.justifyY,
		      XjNx, x,
		      XjNy, y,
		      XjNpadding, me->label.padding,
		      NULL, NULL);
    }
}

/*
 * Things are currently broken screenwise.
 * It will be fun to fix later. :)
 */
static void realize(me)
     LabelJet me;
{
}

static void destroy(me)
     LabelJet me;
{
  if (me->label.leftJet)
    XjDestroyJet(me->label.leftJet);
  if (me->label.middleJet)
    XjDestroyJet(me->label.middleJet);
  if (me->label.rightJet)
    XjDestroyJet(me->label.rightJet);
}

static void querySize(me, size)
     LabelJet me;
     XjSize *size;
{
  XjSize child_size;

  if (DEBUG)
    printf ("QS(label) '%s'\n", me->core.name);

  size->width = size->height = 0;

  if (me->label.leftJet)
    {
      XjQuerySize(me->label.leftJet, &child_size);
      size->width = child_size.width;
      size->height = child_size.height;
    }

  if (me->label.middleJet)
    {
      XjQuerySize(me->label.middleJet, &child_size);
      size->width += child_size.width;
      size->height = MAX(size->height, child_size.height);
    }

  if (me->label.rightJet)
    {
      XjQuerySize(me->label.rightJet, &child_size);
      size->width += child_size.width;
      size->height = MAX(size->height, child_size.height);
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

  me->label.middleX += (x - me->core.x);
  me->label.middleY += (y - me->core.y);
  if (me->label.middleJet)
    XjMove(me->label.middleJet, me->label.middleX, me->label.middleY);

  me->label.leftX += (x - me->core.x);
  me->label.leftY += (y - me->core.y);
  if (me->label.leftJet)
    XjMove(me->label.leftJet, me->label.leftX, me->label.leftY);

  me->label.rightX += (x - me->core.x);
  me->label.rightY += (y - me->core.y);
  if (me->label.rightJet)
    XjMove(me->label.rightJet, me->label.rightX, me->label.rightY);

  me->core.x = x;
  me->core.y = y;
}

static void resize(me, size)
     LabelJet me;
     XjSize *size;
{
  XjSize get_size, set_size;
  int width, height;

  if (DEBUG)
    printf ("RS(label) '%s' w=%d,h=%d\n", me->core.name,
	    size->width, size->height);

  if (me->core.width != size->width
      || me->core.height != size->height)
    {
      width = size->width;
      set_size.height = height = size->height;

      if (me->label.leftJet)
	{
	  XjQuerySize(me->label.leftJet, &get_size);
	  width -= get_size.width;

	  set_size.width = get_size.width;
	  XjResize(me->label.leftJet, &set_size);
	}
     
      if (me->label.rightJet)
	{
	  XjQuerySize(me->label.rightJet, &get_size);
	  width -= get_size.width;

	  set_size.width = get_size.width;
	  XjResize(me->label.rightJet, &set_size);
	}

      if (me->label.middleJet)
	{
	  set_size.width = width;
	  XjResize(me->label.middleJet, &set_size);
	}

      if (me->label.rightJet)
	{
	  me->label.rightX = me->label.middleX + width;
	  XjMove(me->label.rightJet, me->label.rightX, me->label.rightY);
	}
    }
}

static void expose(me, event)
     Jet me;
     XEvent *event;		/* ARGSUSED */
{
  LabelJet lj = (LabelJet) me;

  if (DEBUG)
    printf ("EX(label) '%s'\n", lj->core.name);

  if (lj->label.leftJet)
    XjExpose(lj->label.leftJet, event);

  if (lj->label.rightJet)
    XjExpose(lj->label.rightJet, event);

  if (lj->label.middleJet)
    XjExpose(lj->label.middleJet, event);
}

void setLabel(me, label)
     LabelJet me;
     char *label;
{
  if (me->label.middleJet)
    smplSetLabel(me->label.middleJet, label);
}

void setPixmap(me, pixmap)
     LabelJet me;
     XjPixmap *pixmap;
{
  if (me->label.middleJet)
    smplSetPixmap(me->label.middleJet, pixmap);
}

void setState(me, which, state)
     LabelJet me;
     int which;			/* ARGSUSED */
     Boolean state;
{
  me->label.inverted = state;
  if (me->label.middleJet)
    smplSetState(me->label.middleJet, which, state);
  if (me->label.rightJet)
    smplSetState(me->label.rightJet, which, state);
  if (me->label.leftJet)
    smplSetState(me->label.leftJet, which, state);
}
