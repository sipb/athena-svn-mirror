#ifndef _XSELECT_H_
#define _XSELECT_H_

/*
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 *
 *      Modified for jwgc by Daniel Henninger.
 */

#include "mit-copyright.h"

extern void xicccmInitAtoms();
extern int xselGetOwnership();
extern int xselProcessSelection();
extern void xselOwnershipLost();
extern void xselGiveUpOwnership();

extern int xwmprotoDelete();

extern Atom XA_WM_PROTOCOLS,XA_WM_DELETE_WINDOW;

#endif
