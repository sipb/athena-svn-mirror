/* This file is taken from the Project Athena Zephyr Notification System.
 * It is one of the source files comprising zwgc, the Zephyr WindowGram
 * client.
 *
 *      Created by:     Marc Horowitz <marc@athena.mit.edu>
 *
 *      $Id: xselect.h,v 1.2 1999-01-22 23:17:11 ghudson Exp $
 *
 *      Copyright (c) 1991 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 */


#include "mit-copyright.h"

#ifndef _XSELECT_H_
#define _XSELECT_H_

extern void xselInitAtoms();
extern int xselGetOwnership();
extern int xselProcessSelection();
extern void xselOwnershipLost();
extern void xselGiveUpOwnership();

#endif /* _XSELECT_H_ */
