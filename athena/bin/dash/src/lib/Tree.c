/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/Tree.c,v $
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
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/Tree.c,v 1.1 1991-09-03 11:10:16 vanharen Exp $";
#endif	lint

#include "mit-copyright.h"
#include <stdio.h>
#include <ctype.h>
#include "Jets.h"
#include "Tree.h"

#define offset(field) XjOffset(TreeJet,field)

static XjResource resources[] = {
  { XjNx, XjCX, XjRInt, sizeof(int),
     offset(core.x), XjRString, XjInheritValue },
  { XjNy, XjCY, XjRInt, sizeof(int),
     offset(core.y), XjRString, XjInheritValue },
  { XjNwidth, XjCWidth, XjRInt, sizeof(int),
     offset(core.width), XjRString, XjInheritValue },
  { XjNheight, XjCHeight, XjRInt, sizeof(int),
     offset(core.height), XjRString, XjInheritValue },
  { XjNtree, XjCTree, XjRString, sizeof(char *),
     offset(tree.tree), XjRString, "" }
};

#undef offset

static void initialize();

TreeClassRec treeClassRec = {
  {
    /* class name */	"Tree",
    /* jet size */	sizeof(TreeRec),
    /* initialize */	initialize,
    /* prerealize */    NULL,
    /* realize */       NULL,
    /* event */		NULL,
    /* expose */	NULL,
    /* querySize */     NULL,
    /* move */		NULL,
    /* resize */        NULL,
    /* destroy */       NULL,
    /* resources */	resources,
    /* number of 'em */	XjNumber(resources)
  }
};

JetClass treeJetClass = (JetClass)&treeClassRec;

extern int numJetClasses;
extern JetClass *jetClasses[];

static void initialize(me)
     TreeJet me;
{
  int index, i;
  char *parse, class[50], name[50];
  Jet parent, last;

  if (strlen(me->tree.tree) == 0)
    {
      char errtext[100];

      sprintf(errtext, "empty tree: %s", me->core.name);
      XjWarning(errtext);
      return;
    }

  last = (Jet)me;
  parent = XjParent(me);
  parse = me->tree.tree;

  while (*parse != '\0')
    {
      while (isspace(*parse))
	parse++;

      switch(*parse)
	{
	case '{':
	  if (last == NULL)
	    XjWarning("no consecutive {'s allowed; ignored");
	  else
	    {
	      parent = last;
	      last = NULL;
	    }
	  parse++;
	  break;

	case '}':
	  last = parent;
	  parent = XjParent(parent);
	  if (last == (Jet)me)
	    return;
	  parse++;
	  break;

	default:
	  index = 0;
	  while (isalnum(*parse))
	    class[index++] = *parse++;
	  class[index] = '\0';

	  while (isspace(*parse))
	    parse++;

	  index = 0;
	  while (isalnum(*parse))
	     name[index++] = *parse++;
	  name[index] = '\0';

	  if (*parse == ';')
	    parse++;

#ifdef DEBUG
	  fprintf(stdout, "create a %s named %s, child of %s\n",
		  class, name, parent->core.name);
#endif
	  
	  if (strcasecmp("null", class) != 0)
	    {
	      for (i = 0; i < numJetClasses; i++)
		if (!strcmp(class, (*jetClasses[i])->core_class.className))
		  {
		    last = XjVaCreateJet(name,
					 (*jetClasses[i]),
					 parent,
					 NULL, NULL);
		    break;
		  }
	    }
	  else
	    i = 0;

	  if (i == numJetClasses)
	    {
	      char errtext[100];

	      sprintf(errtext, "unknown class: %s", class);
	      XjWarning(errtext);
	    }
	}
    }
}
