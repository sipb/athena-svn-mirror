#include "copyright.h"

/* $Header: /afs/dev.mit.edu/source/repository/third/emacs/oldXMenu/EvHand.c,v 1.1.1.3 2004-02-11 20:01:34 zacheiss Exp $ */
/* Copyright    Massachusetts Institute of Technology    1985	*/

/*
 * XMenu:	MIT Project Athena, X Window system menu package
 *
 * 	XMenuEventHandler - Set the XMenu asynchronous event handler.
 *
 *	Author:		Tony Della Fera, DEC
 *			December 19, 1985
 *
 */

#include "XMenuInt.h"

XMenuEventHandler(handler)
    int (*handler)();
{
    /*
     * Set the global event handler variable.
     */
    _XMEventHandler = handler;
}

