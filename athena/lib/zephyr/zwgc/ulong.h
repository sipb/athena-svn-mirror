/* This file is part of the Project Athena Zephyr Notification System.
 * It is one of the source files comprising zwgc, the Zephyr WindowGram
 * client.
 *
 *      Created by:     Marc Horowitz <marc@athena.mit.edu>
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/zwgc/ulong.h,v $
 *      $Author: jfc $
 *	$Id: ulong.h,v 1.2 1991-03-08 13:49:51 jfc Exp $
 *
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 */
#ifndef ulong_MODULE
#define ulong_MODULE

#if defined(_AIX) && defined(i386)
/* AIX 1.2 defines ulong in <sys/types.h>.  */
#include <sys/types.h>
#else
typedef unsigned long ulong;
#endif

#endif
