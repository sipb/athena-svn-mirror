/****************************************************************************/
/*                                                                          */
/*               A generic stack type based on linked lists:                */
/*                                                                          */
/****************************************************************************/

#ifndef char_stack_TYPE
#define char_stack_TYPE

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

typedef struct _char_stack {
    struct _char_stack *next;
    char data;
} *char_stack;

#define  char_stack_create()           ((struct _char_stack *) NULL)

#define  char_stack_empty(stack)       (!(stack))

#define  char_stack_top(stack)         ((stack)->data)

#define  char_stack_pop(stack)  { char_stack old = (stack);\
				    (stack) = old->next;\
				    free(old); }

#define  char_stack_push(stack,object) \
           { char_stack new = (struct _char_stack *)\
	       malloc(sizeof (struct _char_stack));\
	     new->next = (stack);\
	     new->data = object;\
	     (stack) = new; }

#endif
