/****************************************************************************/
/*                                                                          */
/*               A generic stack type based on linked lists:                */
/*                                                                          */
/****************************************************************************/

#ifndef string_stack_TYPE
#define string_stack_TYPE

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

typedef struct _string_stack {
    struct _string_stack *next;
    string data;
} *string_stack;

#define  string_stack_create()           ((struct _string_stack *) NULL)

#define  string_stack_empty(stack)       (!(stack))

#define  string_stack_top(stack)         ((stack)->data)

#define  string_stack_pop(stack)  { string_stack old = (stack);\
				    (stack) = old->next;\
				    free(old); }

#define  string_stack_push(stack,object) \
           { string_stack new = (struct _string_stack *)\
	       malloc(sizeof (struct _string_stack));\
	     new->next = (stack);\
	     new->data = object;\
	     (stack) = new; }

#endif
