/* This file is part of the Project Athena Zephyr Notification System.
 * It is one of the source files comprising zwgc, the Zephyr WindowGram
 * client.
 *
 *      Created by:     Marc Horowitz <marc@athena.mit.edu>
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/zwgc/xselect.h,v $
 *      $Author: marc $
 *
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 */

#if (!defined(lint) && !defined(SABER))
static char rcsid_xselect_h[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/zwgc/xselect.h,v 1.2 1989-11-02 01:58:17 marc Exp $";
#endif

#include <zephyr/mit-copyright.h>

#ifndef _XSELECT_H_
#define _XSELECT_H_

extern void xselInitAtoms();
extern int xselGetOwnership();
extern int xselProcessSelection();
extern void xselOwnershipLost();
extern void xselGiveUpOwnership();

#endif
