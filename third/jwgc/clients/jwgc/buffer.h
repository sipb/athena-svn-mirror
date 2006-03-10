#ifndef buffer_MODULE
#define buffer_MODULE

/*
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 *
 *      Modified for jwgc by Daniel Henninger.
 */

#include "mit-copyright.h"

#include "new_string.h"

extern string buffer_to_string();
extern void clear_buffer();
extern void append_buffer();

#endif
