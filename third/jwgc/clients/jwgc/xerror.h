#ifndef _XERROR_H_
#define _XERROR_H_

/*
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 *
 *      Modified for jwgc by Daniel Henninger.
 */

#include "mit-copyright.h"

extern int xerror_happened;

void begin_xerror_trap();
void end_xerror_trap();

#endif
