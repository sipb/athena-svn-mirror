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
static char rcsid[] =
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/Form.c,v 1.2 1991-12-17 10:28:32 vanharen Exp $";
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

static void realize(), resize(), expose();
/*static Boolean event_handler(); */

FormClassRec formClassRec = {
  {
    /* class name */	"Form",
    /* jet size */	sizeof(FormRec),
    /* initialize */	NULL,
    /* prerealize */    NULL,
    /* realize */       realize,
    /* event */		NULL /*event_handler*/,
    /* expose */	expose,
    /* querySize */     NULL,
    /* move */		NULL,
    /* resize */        resize,
    /* destroy */       NULL,
    /* resources */	resources,
    /* number of 'em */	XjNumber(resources)
  }
};

JetClass formJetClass = (JetClass)&formClassRec;

#define UNDEF -1
#define ATTACH -2

static void parse(ptr, name, top, left, bottom, right,
		  topstr, leftstr, bottomstr, rightstr)
     char **ptr, *name, *topstr, *leftstr, *bottomstr, *rightstr;
     int *top, *left, *bottom, *right;
{
  char *p;
  int i = 0;

  p = *ptr;

  while (*p != ':' && *p != '\0')
    name[i++] = *p++;

  name[i] = '\0';

  *top = *left = *bottom = *right = UNDEF;

  if (*p == '\0')
    {
      *ptr = p;
      return;
    }
  else
    p++; /* skip : */

  while (isspace(*p)) p++;

  if (*p != '\0' && *p != '-')
    {
      if (!isdigit(*p))
	{
	  while (isalnum(*p))
	    *leftstr++ = *p++;
	  *leftstr = '\0';
	  *left = ATTACH;
	}
      else
	{
	  *left = atoi(p);
	  while (isdigit(*p)) p++;
	}
    }
  
  if (*p == '-') p++;
  while (isspace(*p)) p++;

  if (*p != '\0' && *p != '-')
    {
      if (!isdigit(*p))
	{
	  while (isalnum(*p))
	    *topstr++ = *p++;
	  *topstr = '\0';
	  *top = ATTACH;
	}
      else
	{
	  *top = atoi(p);
	  while (isdigit(*p)) p++;
	}
    }
  
  if (*p == '-') p++;
  while (isspace(*p)) p++;

  if (*p != '\0' && *p != '-')
    {
      if (!isdigit(*p))
	{
	  while (isalnum(*p))
	    *rightstr++ = *p++;
	  *rightstr = '\0';
	  *right = ATTACH;
	}
      else
	{
	  *right = atoi(p);
	  while (isdigit(*p)) p++;
	}
    }
  
  if (*p == '-') p++;
  while (isspace(*p)) p++;

  if (*p != '\0' && *p != '-')
    {
      if (!isdigit(*p))
	{
	  while (isalnum(*p))
	    *bottomstr++ = *p++;
	  *bottomstr = '\0';
	  *bottom = ATTACH;
	}
      else
	{
	  *bottom = atoi(p);
	  while (isdigit(*p)) p++;
	}
    }

  if (*p == '-') p++;
  while (isspace(*p)) p++;
  if (*p == ';')
    p++;

  *ptr = p;
}

#define spec(x) (x != UNDEF)
#define attach(x) (x == ATTACH)

/*
 * This code *really* needs to be done in a better way.
 */
static void resize(me, mysize)
     FormJet me;
     XjSize *mysize;
{
  Jet child, tmp;
  char *ptr;
  char name[50], topstr[50], leftstr[50], bottomstr[50], rightstr[50];
  int x, y, width, height;
  int top, left, bottom, right;
  XjSize size;
  char errtext[100];

  me->core.width = mysize->width;
  me->core.height = mysize->height;

  ptr = me->form.form;

  while (*ptr != '\0')
    {
      parse(&ptr, name, &top, &left, &bottom, &right,
	    topstr, leftstr, bottomstr, rightstr);

      for (child = me->core.child; child != NULL; child = child->core.sibling)
	if (strcmp(name, child->core.name) == 0)
	  {
	    XjQuerySize(child, &size);
	    size.width += 2 * (child->core.borderWidth + me->form.padding);
	    size.height += 2 * (child->core.borderWidth + me->form.padding);

	    x = child->core.x;
	    y = child->core.y;
	    width = height = 10; /* just in case */
	      
	    if (spec(left) && !spec(right))
	      {
		if (attach(left))
		  {
		    tmp = XjFindJet(leftstr, (Jet) me);
		    if (tmp == NULL)
		      {			
		       sprintf(errtext, "form: couldn't find jet %s", leftstr);
			XjWarning(errtext);
		     }
		    else
		      x = tmp->core.x + tmp->core.width +
			2 * (tmp->core.borderWidth + me->form.padding);
		  }
		else
		  x = (left * me->core.width) / 100;
		width = size.width;
	      }

	    if (!spec(left) && spec(right))
	      {
		width = size.width;
		if (attach(right))
		  {
		    tmp = XjFindJet(rightstr, (Jet) me);
		    if (tmp == NULL)
		      {
		      sprintf(errtext, "form: couldn't find jet %s", rightstr);
			XjWarning(errtext);
		      }
		    else
		      x = tmp->core.x - width - 1;
		  }
		else
		  x = ((right * me->core.width) / 100) - width;
	      }

	    if (spec(left) && spec(right))
	      {
		if (attach(left))
		  {
		    tmp = XjFindJet(leftstr, (Jet) me);
		    if (tmp == NULL)
		      {
		      sprintf(errtext, "form: couldn't find jet %s", leftstr);
			XjWarning(errtext);
		      }
		    else
		      x = tmp->core.x + tmp->core.width +
			2 * (tmp->core.borderWidth + me->form.padding);
		  }
		else
		  x = (left * me->core.width) / 100;

		if (attach(right))
		  {
		    tmp = XjFindJet(rightstr, (Jet) me);
		    if (tmp == NULL)
		      {
		      sprintf(errtext, "form: couldn't find jet %s", rightstr);
			XjWarning(errtext);
		      }
		    else
		      width = tmp->core.x - x;
		  }
		else
		  width = ((right * me->core.width) / 100) - x;
	      }

	    if (spec(top) && !spec(bottom))
	      {
		if (attach(top))
		  {
		    tmp = XjFindJet(topstr, (Jet) me);
		    if (tmp == NULL)
		      {
			sprintf(errtext, "form: couldn't find jet %s", topstr);
			XjWarning(errtext);
		      }
		    else
		      y = tmp->core.y + tmp->core.height +
			2 * (tmp->core.borderWidth + me->form.padding);
		  }
		else
		  y = (top * me->core.height) / 100;
		height = size.height;
	      }

	    if (!spec(top) && spec(bottom))
	      {
		height = size.height;
		if (attach(bottom))
		  {
		    tmp = XjFindJet(bottomstr, (Jet) me);
		    if (tmp == NULL)
		      {
		     sprintf(errtext, "form: couldn't find jet %s", bottomstr);
			XjWarning(errtext);
		      }
		    else
		      y = tmp->core.y - height - 1;
		  }
		else
		  y = ((bottom * me->core.height) / 100) - height;
	      }

	    if (spec(top) && spec(bottom))
	      {
		if (attach(top))
		  {
		    tmp = XjFindJet(topstr, (Jet) me);
		    if (tmp == NULL)
		      {
			sprintf(errtext, "form: couldn't find jet %s", topstr);
			XjWarning(errtext);
		      }
		    else
		      y = tmp->core.y + tmp->core.height +
			2 * (tmp->core.borderWidth + me->form.padding);
		  }
		else
		  y = (top * me->core.height) / 100;

		if (attach(bottom))
		  {
		    tmp = XjFindJet(bottomstr, (Jet) me);
		    if (tmp == NULL)
		      {
		     sprintf(errtext, "form: couldn't find jet %s", bottomstr);
			XjWarning(errtext);
		      }
		    else
		      height = tmp->core.y - y;
		  }
		else
		  height = ((bottom * me->core.height) / 100) - y;
	      }

	    XjMove(child, x + me->form.padding, y + me->form.padding);
	    size.width = width -
	      2 * (child->core.borderWidth + me->form.padding);
	    size.height = height -
	      2 * (child->core.borderWidth + me->form.padding);
	    XjResize(child, &size);
/*
	    fprintf(stdout, "%d %d %d %d\n%d %d %d %d\n",
		    left, top, right, bottom,
		    x, y, width, height);
*/
	    break;
	  }
    }
}

static void realize(me)
     FormJet me;		/* ARGSUSED */
{
/*
  XjSize size;

  size.width = me->core.width;
  size.height = me->core.height;
  XjResize(me, &size);
  XjRegisterWindow(me->core.window, (Jet) me);
  XjSelectInput(me->core.display, me->core.window,
		ExposureMask | StructureNotifyMask);
*/
}

static void expose(me, event)
     Jet me;
     XEvent *event;
{
  Jet child;

  if (event->xexpose.count == 0)
    {
      for (child = me->core.child; child != NULL; child = child->core.sibling)
	if (child->core.classRec->core_class.expose != NULL)
	  child->core.classRec->core_class.expose(child, event);
    }
}

/*
static Boolean event_handler(me, event)
     FormJet me;
     XEvent *event;
{
  Jet child;

  switch(event->type)
    {
    case ConfigureNotify:
      if (event->xconfigure.width != me->core.width ||
	  event->xconfigure.height != me->core.height)
	{
	  XjSize size;

	  size.width = event->xconfigure.width;
	  size.height = event->xconfigure.height;
	  XjResize((Jet) me, &size);
	}
      break;
    default:
      return False;
    }
  return True;
}
*/


void setForm(me, form)
     FormJet me;
     char *form;
{
  me->form.form = XjNewString(form);
}
