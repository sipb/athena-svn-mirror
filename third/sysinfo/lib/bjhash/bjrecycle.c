/*
 * $Revision: 1.1.1.1 $
--------------------------------------------------------------------
By Bob Jenkins, September 1996.  recycle.c
You may use this code in any way you wish, and it is free.  No warranty.

This manages memory for commonly-allocated structures.
It allocates BJRESTART to BJREMAX items at a time.
Timings have shown that, if malloc is used for every new structure,
  malloc will consume about 90% of the time in a program.  This
  module cuts down the number of mallocs by an order of magnitude.
This also decreases memory fragmentation, and freeing structures
  only requires freeing the root.
--------------------------------------------------------------------
*/

#include "defs.h"	/* Need this for -xarch=v9 */
#ifndef BJSTANDARD
#include "bjstandard.h"
#endif
#ifndef BJRECYCLE
#include "bjrecycle.h"
#endif

bjreroot_t *bjremkroot(size)
size_t  size;
{
   bjreroot_t *r = (bjreroot_t *)xcalloc(1, sizeof(bjreroot_t));
   r->list = (bjrecycle_t *)0;
   r->trash = (bjrecycle_t *)0;
   r->size = bjalign(size);
   r->logsize = BJRESTART;
   r->numleft = 0;
   return r;
}

void  bjrefree(r)
bjreroot_t *r;
{
   bjrecycle_t *temp;
   if (temp = r->list) while (r->list)
   {
      temp = r->list->next;
      free((char *)r->list);
      r->list = temp;
   }
   free((char *)r);
   return;
}

/* to be called from the macro renew only */
char  *bjrenewx(r)
bjreroot_t *r;
{
   bjrecycle_t *temp;
   if (r->trash)
   {  /* pull a node off the trash heap */
      temp = r->trash;
      r->trash = temp->next;
      (void)memset((void *)temp, 0, r->size);
   }
   else
   {  /* allocate a new block of nodes */
      r->numleft = r->size*((ub4)1<<r->logsize);
      if (r->numleft < BJREMAX) ++r->logsize;
      temp = (bjrecycle_t *)xcalloc(1, sizeof(bjrecycle_t) + r->numleft);
      temp->next = r->list;
      r->list = temp;
      r->numleft-=r->size;
      temp = (bjrecycle_t *)((char *)(r->list+1)+r->numleft);
   }
   return (char *)temp;
}

