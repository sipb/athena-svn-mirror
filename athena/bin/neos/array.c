/**********************************************************************
 * File Exchange client module to get sorted array from Paperlist
 *
 * $Id: array.c,v 1.1 1999-09-28 22:10:55 danw Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_array_c[] = "$Id: array.c,v 1.1 1999-09-28 22:10:55 danw Exp $";
#endif /* lint */

#include <fx/fxcl.h>
#include <fx/memory.h>

/* Another client module must define comparator for sorting */
int compar();

/*
 * get_array -- fill in the array of papers to collect
 */

get_array(list, ppp)
     Paperlist list;
     Paper ***ppp;
{
  Paperlist node;
  int i = 0, count = 0;

  /* Count papers in list */
  for(node = list; node; node = node->next) i++;

  if (i == 0) return(0);

  *ppp = NewArray(Paper *, i);
  if (!*ppp) return(0);

  /* Put papers into sorted array */
  for(node = list; node; node = node->next)
    (*ppp)[count++] = &node->p;
  qsort((char *) *ppp, count, sizeof(Paper *), compar);

  return(count);
}
