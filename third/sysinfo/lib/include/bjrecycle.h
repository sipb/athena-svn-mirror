/*
 * $Revision: 1.1.1.1 $
--------------------------------------------------------------------
By Bob Jenkins, September 1996.  bjrecycle.h
You may use this code in any way you wish, and it is free.  No warranty.

This manages memory for commonly-allocated structures.
It allocates RESTART to REMAX items at a time.
Timings have shown that, if malloc is used for every new structure,
  malloc will consume about 90% of the time in a program.  This
  module cuts down the number of mallocs by an order of magnitude.
This also decreases memory fragmentation, and freeing all structures
  only requires freeing the root.
--------------------------------------------------------------------
*/

#ifndef BJSTANDARD
#include "bjstandard.h"
#endif

#ifndef BJRECYCLE
#define BJRECYCLE

#define BJRESTART    0
#define BJREMAX      65500

struct bjrecycle
{
   struct bjrecycle *next;
};
typedef  struct bjrecycle  bjrecycle_t;

struct bjreroot
{
   struct bjrecycle *list;     /* list of malloced blocks */
   struct bjrecycle *trash;    /* list of deleted items */
   size_t          size;     /* size of an item */
   size_t          logsize;  /* log_2 of number of items in a block */
   Word_t          numleft;  /* number of items left in this block */
};
typedef  struct bjreroot  bjreroot_t;

/* make a new recycling root */
bjreroot_t  *bjremkroot(/*_ size_t mysize _*/);

/* free a recycling root and all the items it has made */
void     bjrefree(/*_ struct bjreroot *r _*/);

/* get a new (cleared) item from the root */
#define bjrenew(r) ((r)->numleft ? \
   (((char *)((r)->list+1))+((r)->numleft-=(r)->size)) : bjrenewx(r))

char    *bjrenewx(/*_ struct bjreroot *r _*/);

/* delete an item; let the root recycle it */
/* void     bjredel(/o_ struct bjreroot *r, struct bjrecycle *item _o/); */
#define bjredel(root,item) { \
   ((bjrecycle_t *)item)->next=(root)->trash; \
   (root)->trash=(bjrecycle_t *)(item); \
}

#endif  /* BJRECYCLE */
