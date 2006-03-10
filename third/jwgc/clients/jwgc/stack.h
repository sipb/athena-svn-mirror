/****************************************************************************/
/*                                                                          */
/*               A generic stack type based on linked lists:                */
/*                                                                          */
/****************************************************************************/

#ifndef TYPE_T_stack_TYPE
#define TYPE_T_stack_TYPE

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

typedef struct _TYPE_T_stack {
    struct _TYPE_T_stack *next;
    TYPE_T data;
} *TYPE_T_stack;

#define  TYPE_T_stack_create()           ((struct _TYPE_T_stack *) NULL)

#define  TYPE_T_stack_empty(stack)       (!(stack))

#define  TYPE_T_stack_top(stack)         ((stack)->data)

#define  TYPE_T_stack_pop(stack)  { TYPE_T_stack old = (stack);\
				    (stack) = old->next;\
				    free(old); }

#define  TYPE_T_stack_push(stack,object) \
           { TYPE_T_stack new = (struct _TYPE_T_stack *)\
	       malloc(sizeof (struct _TYPE_T_stack));\
	     new->next = (stack);\
	     new->data = object;\
	     (stack) = new; }

#endif
