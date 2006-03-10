#ifndef x_driver_MODULE
#define x_driver_MODULE

/*
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 *
 *      Modified for jwgc by Daniel Henninger.
 */

#include "mit-copyright.h"

#include <X11/Xlib.h>

extern Display *dpy;

extern char *get_string_resource();
extern int get_bool_resource();

#endif
