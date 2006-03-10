/****************************************************************************/
/*                                                                          */
/*               A generic stack type based on linked lists:                */
/*                                                                          */
/****************************************************************************/

#ifndef xmode_stack_TYPE
#define xmode_stack_TYPE

/*
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 *
 *      Modified for jwgc by Daniel Henninger.
 */

#include "mit-copyright.h"

#ifndef  NULL
#define  NULL 0
#endif

typedef struct _xmode_stack {
    struct _xmode_stack *next;
    xmode data;
} *xmode_stack;

#define  xmode_stack_create()           ((struct _xmode_stack *) NULL)

#define  xmode_stack_empty(stack)       (!(stack))

#define  xmode_stack_top(stack)         ((stack)->data)

#define  xmode_stack_pop(stack)  { xmode_stack old = (stack);\
				    (stack) = old->next;\
				    free(old); }

#define  xmode_stack_push(stack,object) \
           { xmode_stack new = (struct _xmode_stack *)\
	       malloc(sizeof (struct _xmode_stack));\
	     new->next = (stack);\
	     new->data = object;\
	     (stack) = new; }

#endif
