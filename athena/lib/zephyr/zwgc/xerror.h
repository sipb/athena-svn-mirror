/* This file is part of the Project Athena Zephyr Notification System.
 * It is one of the source files comprising zwgc, the Zephyr WindowGram
 * client.
 *
 *      Created by:     Marc Horowitz <marc@athena.mit.edu>
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/zwgc/xerror.h,v $
 *      $Author: ghudson $
 *
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 */

#ifndef _XERROR_H_
#define _XERROR_H_

#if (!defined(lint) && !defined(SABER))
static const char rcsid_xerror_h[] = "$Id: xerror.h,v 1.2 1997-09-14 22:14:55 ghudson Exp $";
#endif

#include <zephyr/mit-copyright.h>

extern int xerror_happened;

void begin_xerror_trap();
void end_xerror_trap();

#endif
