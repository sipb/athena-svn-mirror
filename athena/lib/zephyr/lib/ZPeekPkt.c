/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for ZPeekPacket function.
 *
 *	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZPeekPkt.c,v $
 *	$Author: rfrench $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
/* $Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZPeekPkt.c,v 1.6 1988-05-17 21:23:09 rfrench Exp $ */

#ifndef lint
static char rcsid_ZPeekPacket_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZPeekPkt.c,v 1.6 1988-05-17 21:23:09 rfrench Exp $";
#endif lint

#include <zephyr/mit-copyright.h>

#include <zephyr/zephyr_internal.h>

Code_t ZPeekPacket(buffer, ret_len, from)
    char **buffer;
    int *ret_len;
    struct sockaddr_in *from;
{
    int retval;
    struct _Z_InputQ *nextq;
    
    if (ZGetFD() < 0)
	return (ZERR_NOPORT);

    if (ZQLength()) {
	if ((retval = Z_ReadEnqueue()) != ZERR_NONE)
	    return (retval);
    }
    else {
	if ((retval = Z_ReadWait()) != ZERR_NONE)
	    return (retval);
    }

    nextq = Z_GetFirstComplete();

    *ret_len = nextq->packet_len;
    
    if (!(*buffer = malloc(*ret_len)))
	return (ENOMEM);

    bcopy(nextq->packet, *buffer, *ret_len);

    if (from)
	*from = nextq->from;
	
    return (ZERR_NONE);
}
