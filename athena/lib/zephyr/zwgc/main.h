/* This file is part of the Project Athena Zephyr Notification System.
 * It is one of the source files comprising zwgc, the Zephyr WindowGram
 * client.
 *
 *      Created by:     Marc Horowitz <marc@athena.mit.edu>
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/zwgc/main.h,v $
 *      $Author: marc $
 *
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 */

#if (!defined(lint) && !defined(SABER))
static char rcsid_main_h[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/zwgc/main.h,v 1.2 1989-11-02 01:56:26 marc Exp $";
#endif

#include <zephyr/mit-copyright.h>

#ifndef main_MODULE
#define main_MODULE

extern char *subscriptions_filename_override;

/*
 *    void usage()
 *        Effects: Prints out a usage message on stderr then exits the
 *                 program with error code 1.
 */

extern void usage();

#endif
