#ifndef x_driver_MODULE
#define x_driver_MODULE

/* This file is part of the Project Athena Zephyr Notification System.
 * It is one of the source files comprising zwgc, the Zephyr WindowGram
 * client.
 *
 *      Created by:     Marc Horowitz <marc@athena.mit.edu>
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/zwgc/X_driver.h,v $
 *      $Author: ghudson $
 *	$Id: X_driver.h,v 1.5 1997-09-14 22:13:39 ghudson Exp $
 *
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 */


#include <zephyr/mit-copyright.h>

#include <X11/Xlib.h>

extern Display *dpy;

extern char *get_string_resource();
extern int get_bool_resource();

#endif
