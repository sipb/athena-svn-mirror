#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include "stack.h"
#include "delete_errs.h"
#include "errors.h"

extern char *malloc(), *realloc();
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
	  if (size - count < bytes) {
	       size += size - count + bytes;
	       if (size % STACK_INC)
		    size += STACK_INC - size % STACK_INC;
	       stack = (caddr_t) (stack ? realloc((char *) stack,
						  (unsigned) size) :
				  malloc((unsigned) size));
	       if (! stack) {
		    set_error(errno);
		    error("malloc");
		    return error_code;
	       }
	  }
	  bcopy(data, stack + count, bytes);
	  count += bytes;
	  return 0;
     case STACK_POP:
	  if (count == 0) {
	       set_status(STACK_EMPTY);
	       return error_code;
	  }
	  else {
	       count -= bytes;
	       bcopy(stack + count, data, bytes);
	       if (count % STACK_INC == 0) {
		    size -= STACK_INC;
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
