/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/stack.c,v $
 * $Author: jik $
 *
 * This program is part of a package including delete, undelete,
 * lsdel, expunge and purge.  The software suite is meant as a
 * replacement for rm which allows for file recovery.
 * 
 * Copyright (c) 1989 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#if (!defined(lint) && !defined(SABER))
     static char rcsid_stack_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/stack.c,v 1.6 1990-06-07 13:26:52 jik Exp $";
#endif

#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include "stack.h"
#include "delete_errs.h"
#include "errors.h"
#include "mit-copyright.h"
#include "util.h"

extern char *realloc();
extern int errno;

#define STACK_INC 	25



int dostack(data, op, bytes)
caddr_t data;
int op, bytes;
{
     static caddr_t stack = (caddr_t) NULL;
     static int size = 0, count = 0;

     switch (op) {
     case EMPTY_STACK:
	  if (size) {
	       free(stack);
	       stack = (caddr_t) NULL;
	       size = count = 0;
	  }
	  return 0;
     case STACK_PUSH:
	  if (bytes == 0)
	       return 0;
	  if (size - count < bytes) {
	       do
		    size += STACK_INC;
	       while (size - count < bytes);
	       stack = (caddr_t) (stack ? realloc((char *) stack,
						  (unsigned) size) :
				  Malloc((unsigned) size));
	       if (! stack) {
		    size = count = 0;
		    set_error(errno);
		    error("Malloc");
		    return error_code;
	       }
	  }
#if defined(SYSV) || (defined(__STDC__) && !defined(__HIGHC__))
	  memcpy(stack + count, data, bytes);
#else
	  bcopy(data, stack + count, bytes);
#endif
	  count += bytes;
	  return 0;
     case STACK_POP:
	  if (bytes == 0)
	       return 0;
	  if (count == 0) {
	       set_status(STACK_EMPTY);
	       return error_code;
	  }
	  else {
	       int newblocks, newsize;

	       count -= bytes;
#if defined(SYSV) || (defined(__STDC__) && !defined(__HIGHC__))
	       memcpy(stack + count, data, bytes);
#else
	       bcopy(stack + count, data, bytes);
#endif
	       newblocks = count / STACK_INC + ((count % STACK_INC) ? 1 : 0);
	       newsize = newblocks * STACK_INC;
	       if (newsize < size) {
		    size = newsize;
		    stack = (caddr_t) realloc((char *) stack, (unsigned) size);
		    if (! stack) {
			 set_error(errno);
			 error("realloc");
			 return error_code;
		    }
	       }
	       return 0;
	  }
     default:
	  set_error(STACK_BAD_OP);
	  return error_code;
     }
}
