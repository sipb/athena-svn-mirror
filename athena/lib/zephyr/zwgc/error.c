/* This file is part of the Project Athena Zephyr Notification System.
 * It is one of the source files comprising zwgc, the Zephyr WindowGram
 * client.
 *
 *      Created by:     Marc Horowitz <marc@athena.mit.edu>
 *
 *      $Id: error.c,v 1.5 1999-01-22 23:20:15 ghudson Exp $
 *
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 */

#include <sysdep.h>

#if (!defined(lint) && !defined(SABER))
static const char rcsid_error_c[] = "$Id: error.c,v 1.5 1999-01-22 23:20:15 ghudson Exp $";
#endif

#include <zephyr/mit-copyright.h>

int error_code;
