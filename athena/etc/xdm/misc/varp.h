#ifndef _VARP_H_
#define _VARP_H_

typedef struct _var {
  char *name;
  int length;
  void *value;
} var;

typedef struct _varlist {
  int size;
  int used;
  var **vars;
} varlist;

#include "var.h"

#endif /* _VARP_H_ */
