/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/Form.c,v $
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
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/Form.c,v 1.3 1993-07-02 02:04:33 vanharen Exp $";
#endif

#include "mit-copyright.h"
#include <stdio.h>
#include <ctype.h>
#include "Jets.h"
#include "Form.h"

#define offset(field) XjOffset(FormJet,field)

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
     offset(form.padding), XjRString, "0" },
  { XjNform, XjCForm, XjRString, sizeof(char *),
     offset(form.form), XjRString, "" }
};

#undef offset

static void resize(), expose();

FormClassRec formClassRec = {
  {
    /* class name */		"Form",
    /* jet size */		sizeof(FormRec),
    /* classInitialize */	NULL,
    /* classInitialized? */	1,
    /* initialize */		NULL,
    /* prerealize */    	NULL,
    /* realize */       	NULL,
    /* event */			NULL,
    /* expose */		expose,
    /* querySize */     	NULL,
    /* move */			NULL,
    /* resize */        	resize,
    /* destroy */       	NULL,
    /* resources */		resources,
    /* number of 'em */		XjNumber(resources)
  }
};

JetClass formJetClass = (JetClass)&formClassRec;


#define UNDEF -1
#define ATTACH -2

#define LEFT 0
#define TOP 1
#define RIGHT 2
#define BOTTOM 3
#define SIDES 4


static void parse(ptr, name, sides, sidestr)
     char **ptr, *name, sidestr[SIDES][50];
     int sides[SIDES];
{
  char *p, *q;
  int i = 0;
  int j;

  p = *ptr;

  while (*p != ':' && *p != '\0')
    name[i++] = *p++;

  name[i] = '\0';

  if (*p == '\0')
    {
      *ptr = p;
      return;
    }
  else
    p++; /* skip the colon ":" */


  for (j=0; j < SIDES; j++)
    {
      sides[j] = UNDEF;

      while (isspace(*p)) p++;

      if (*p != '\0' && *p != '-')
	{
	  if (isdigit(*p))
	    {
	      sides[j] = atoi(p);
	      while (isdigit(*p)) p++;
	    }
	  else
	    {
	      q = sidestr[j];
	      while (isalnum(*p))
		*q++ = *p++;
	      *q = '\0';
	      sides[j] = ATTACH;
	    }
	}
  
      if (*p == '-') p++;
    }


  while (isspace(*p)) p++;
  if (*p == ';')
    p++;

  *ptr = p;
}

#define spec(x) (x != UNDEF)
#define attachment(x) (x == ATTACH)

/*
 * This code *really* needs to be done in a better way.
 */
static void resize(me, mysize)
     FormJet me;
     XjSize *mysize;
{
  Jet child, tmp[SIDES];
  char *ptr;
  char name[50], sidestr[SIDES][50];
  int x, y, width, height;
  int sides[SIDES];
  XjSize size;
  char errtext[100];
  int j;

  me->core.width = mysize->width;
  me->core.height = mysize->height;

  ptr = me->form.form;

  while (*ptr != '\0')
    {
      parse(&ptr, name, sides, sidestr);

      for (child = me->core.child; child != NULL; child = child->core.sibling)
	if (strcmp(name, child->core.name) == 0)
	  {
	    XjQuerySize(child, &size);
	    size.width += 2 * (child->core.borderWidth + me->form.padding);
	    size.height += 2 * (child->core.borderWidth + me->form.padding);

	    x = child->core.x;
	    y = child->core.y;
	    width = size.width;
	    height = size.height; /* just in case */
	      
	    /* 
	     *  Find all the attachment jets, if there are any.
	     */
	    for (j=0; j < SIDES; j++)
	      {
		tmp[j] = NULL;
		if (attachment(sides[j]))
		  {
		    if ((tmp[j] = XjFindJet(sidestr[j], (Jet) me)) == NULL)
		      {
			sprintf(errtext, "form: couldn't find jet `%s'",
				sidestr[j]);
			XjWarning(errtext);
		      }
		  }
	      }
		  

	    /*
	     *  Figure out the x coord and width.
	     */
	    if (spec(sides[LEFT]))
	      {
		if (tmp[LEFT])
		  x = tmp[LEFT]->core.x + tmp[LEFT]->core.width +
		    2 * (tmp[LEFT]->core.borderWidth + me->form.padding);
		else
		  x = (sides[LEFT] * me->core.width) / 100;

		if (spec(sides[RIGHT]))
		  {
		    if (tmp[RIGHT])
		      width = tmp[RIGHT]->core.x - x - 2*me->form.padding;
		    else
		      width = ((sides[RIGHT] * me->core.width) / 100) - x;
		  }
	      }

	    else		/* sides[LEFT] was not specified */
	      if (spec(sides[RIGHT]))
		{
		  if (tmp[RIGHT])
		    x = tmp[RIGHT]->core.x - width - 1;
		  else
		    x = ((sides[RIGHT] * me->core.width) / 100) - width;
		}


	    /*
	     *  Figure out the y coord and height.
	     */
	    if (spec(sides[TOP]))
	      {
		if (tmp[TOP])
		  y = tmp[TOP]->core.y + tmp[TOP]->core.height +
		    2 * (tmp[TOP]->core.borderWidth + me->form.padding);
		else
		  y = (sides[TOP] * me->core.height) / 100;

		if (spec(sides[BOTTOM]))
		  {
		    if (tmp[BOTTOM])
		      height = tmp[BOTTOM]->core.y - y - 2*me->form.padding;
		    else
		      height = ((sides[BOTTOM] * me->core.height) / 100) - y;
		  }
	      }

	    else		/* sides[TOP] was not specified */
	      if (spec(sides[BOTTOM]))
		{
		  if (tmp[BOTTOM])
		    y = tmp[BOTTOM]->core.y - height - 1;
		  else
		    y = ((sides[BOTTOM] * me->core.height) / 100) - height;
		}



	    XjMove(child, x + me->form.padding, y + me->form.padding);
	    size.width = width -
	      2 * (child->core.borderWidth + me->form.padding);
	    size.height = height -
	      2 * (child->core.borderWidth + me->form.padding);
	    XjResize(child, &size);

	    break;
	  }
    }
}


static void expose(me, event)
     Jet me;
     XEvent *event;
{
  Jet child;

  if (event->xexpose.count == 0)
    {
      for (child = me->core.child; child != NULL; child = child->core.sibling)
	XjExpose(child, event);
#ifdef notdefined
	if (child->core.classRec->core_class.expose != NULL)
	  child->core.classRec->core_class.expose(child, event);
#endif
    }
}


void setForm(me, form)
     FormJet me;
     char *form;
{
  me->form.form = XjNewString(form);
}
