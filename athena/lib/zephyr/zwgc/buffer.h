/* This file is part of the Project Athena Zephyr Notification System.
 * It is one of the source files comprising zwgc, the Zephyr WindowGram
 * client.
 *
 *      Created by:     Marc Horowitz <marc@athena.mit.edu>
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/zwgc/buffer.h,v $
 *      $Author: jtkohl $
 *	$Id: buffer.h,v 1.3 1989-11-08 14:35:32 jtkohl Exp $
 *
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 */


#include <zephyr/mit-copyright.h>

#ifndef buffer_MODULE
#define buffer_MODULE

#include "new_string.h"

extern string buffer_to_string();
extern void clear_buffer();
extern void append_buffer();

#endif
