/* This file is part of the Project Athena Zephyr Notification System.
 * It is one of the source files comprising zwgc, the Zephyr WindowGram
 * client.
 *
 *      Created by:     Marc Horowitz <marc@athena.mit.edu>
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/zwgc/pointer.h,v $
 *      $Author: ghudson $
 *	$Id: pointer.h,v 1.5 1997-09-14 22:14:33 ghudson Exp $
 *
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 */


#include <zephyr/mit-copyright.h>

#ifndef pointer_MODULE
#define pointer_MODULE

#ifdef __STDC__
typedef void *pointer;
#else
typedef char *pointer;
#endif

#endif
