/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/VaCreateJet.c,v $
 * $Author: ghudson $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#if  (!defined(lint))  &&  (!defined(SABER))
static char *rcsid =
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/VaCreateJet.c,v 1.2 1996-09-19 22:23:33 ghudson Exp $";
#endif

#include "mit-copyright.h"
#include "Jets.h"
#include <varargs.h>
#include <stdio.h>

#define MAXNAMELEN 500
static char className[MAXNAMELEN]; /* deserve to lose long before this */
static char instanceName[MAXNAMELEN];

/*
 * This function is broken...
 * In fact, lots of things are broken.
 * C is broken... Unix is broken... Why break such a fine tradition?
 * (Watch out for paradoxes!)
 */
#ifdef notdef 
void XjCopyValue(where, resource, value)
     Jet where;
     XjResource *resource;
     XjArgVal value;
{
  memcpy(where + resource->resource_offset,
	 &value,
	(resource->resource_size > 4) ? 4 : resource->resource_size);
}
#endif

#define XjCopyValue(where, resource, value) \
  memcpy((where) + (resource)->resource_offset, \
	&(value), \
	((resource)->resource_size > 4) ? 4 : (resource)->resource_size)

Jet XjVaCreateJet(name, class, parent, va_alist)
char *name;
JetClass class;
Jet parent;
va_dcl
{
  va_list args;
  char *classPtr, *instPtr;
  int resCount;
  char *valName;
  XjArgVal val;

  Jet jet, thisJet;
  int len;
  Boolean validName;

  jet = (Jet)XjMalloc((unsigned) class->core_class.jetSize);

  memset(jet, 0, class->core_class.jetSize);

  jet->core.classRec = class;
  jet->core.name = XjNewString(name);
  jet->core.display = parent->core.display;
  jet->core.window = parent->core.window; /* may get its own window later */
  jet->core.borderWidth = 0;	/* Jet must change this if desired; supplied
				   for reasonable geometry management */
  jet->core.need_expose = False; /* it'll get an expose soon enough... */

  jet->core.parent = parent;
  jet->core.sibling = parent->core.child;
  jet->core.child = NULL;

  parent->core.child = jet;

  /* now call the ClassInitialize procedure */
  if (! jet->core.classRec->core_class.classInitialized
      &&  jet->core.classRec->core_class.classInitialize != NULL)
    {
      jet->core.classRec->core_class.classInitialize(jet);
      jet->core.classRec->core_class.classInitialized = 1;
    }

  /* Generate the class and instance names */

  /*
   * Initialize the pointers to the place after the null, since
   * inside the loop we backup the length + 1 of the string to
   * add in order to skip over the '.' that we might want...
   * This hack makes it unnecessary to special case inside the
   * loop.
   */
  classPtr = className + MAXNAMELEN;
  instPtr = instanceName + MAXNAMELEN;
  classPtr[-1] = '\0'; instPtr[-1] = '\0';
  thisJet = jet;
  validName = True;

  while (thisJet != NULL)
    {
      if (thisJet->core.name)
	len = strlen(thisJet->core.name);
      else
	{
	  validName = False;
	  break;
	}

      if (len < (instPtr - instanceName)) /* includes '.' */
	{
	  instPtr -= len + 1;
	  memcpy(instPtr, thisJet->core.name, len);
	  instPtr[-1] = '.';
	}
      else
	{
	  XjWarning("Full jet instance name too long.");
	  validName = False;
	  break;
	}

      /* now do it all again for the class... */
      if (thisJet->core.classRec->core_class.className)
	len = strlen(thisJet->core.classRec->core_class.className);
      else
	{
	  validName = False;
	  break;
	}

      if (len < (classPtr - className)) /* includes . */
	{
	  classPtr -= len + 1;
	  memcpy(classPtr, thisJet->core.classRec->core_class.className, len);
	  classPtr[-1] = '.';
	}
      else
	{
	  XjWarning("Full jet class name too long.");
	  validName = False;
	  break;
	}

      thisJet = thisJet->core.parent;
    }

/*
  fprintf(stdout, "%s\n%s\n",
	  classPtr, instPtr);
*/

  if (validName)
    XjLoadFromResources(jet->core.display,
			jet->core.window,
			classPtr,
			instPtr,
			jet->core.classRec->core_class.resources,
			jet->core.classRec->core_class.num_resources,
			(caddr_t) jet);

  va_start(args);

  while (NULL != (valName = va_arg(args, char *)))
    {
      val = va_arg(args, XjArgVal);
/*      fprintf(stdout, "%d\n", val); */

      for (resCount = 0;
	   resCount < jet->core.classRec->core_class.num_resources;
	   resCount++)
	if (!strcmp(valName, jet->core.classRec->
		    core_class.resources[resCount].resource_name))
	  {
	    XjCopyValue(jet,
			&jet->core.classRec->core_class.resources[resCount],
			val);
	    break;
	  }

      if (resCount == jet->core.classRec->core_class.num_resources)
	fprintf(stdout, "no such resource name: %s\n", valName);
    }

  val = va_arg(args, XjArgVal); /* pop the last one */
  va_end(args);

  /* now call the initialize procedure */
  if (jet->core.classRec->core_class.initialize != NULL)
    jet->core.classRec->core_class.initialize(jet);

  return jet;
}
