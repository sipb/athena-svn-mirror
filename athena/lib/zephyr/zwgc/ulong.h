/* This file is part of the Project Athena Zephyr Notification System.
 * It is one of the source files comprising zwgc, the Zephyr WindowGram
 * client.
 *
 *      Created by:     Marc Horowitz <marc@athena.mit.edu>
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/zwgc/ulong.h,v $
 *      $Author: probe $
 *	$Id: ulong.h,v 1.4 1993-11-21 06:23:50 probe Exp $
 *
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 */
#ifndef ulong_MODULE
#define ulong_MODULE

#include <sys/types.h>

#if defined(ultrix) || defined(vax)
typedef unsigned long ulong;
#endif

#endif
