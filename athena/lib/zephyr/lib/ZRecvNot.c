/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for ZReceiveNotice function.
 *
 *	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZRecvNot.c,v $
 *	$Author: ghudson $
 *
 *	Copyright (c) 1987,1988 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
/* $Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZRecvNot.c,v 1.13 1994-11-01 17:51:59 ghudson Exp $ */

#ifndef lint
static char rcsid_ZReceiveNotice_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZRecvNot.c,v 1.13 1994-11-01 17:51:59 ghudson Exp $";
#endif

#include <zephyr/mit-copyright.h>

#include <zephyr/zephyr_internal.h>

Code_t ZReceiveNotice(notice, from)
    ZNotice_t *notice;
    struct sockaddr_in *from;
{
    char *buffer;
    struct _Z_InputQ *nextq;
    int len, auth;
    Code_t retval;

    if ((retval = Z_WaitForComplete()) != ZERR_NONE)
	return (retval);

    nextq = Z_GetFirstComplete();

    len = nextq->packet_len;
    
    if (!(buffer = (char *) malloc((unsigned) len)))
	return (ENOMEM);

    if (from)
	*from = nextq->from;
    
    (void) memcpy(buffer, nextq->packet, len);

    auth = nextq->auth;
    Z_RemQueue(nextq);
    
    if ((retval = ZParseNotice(buffer, len, notice)) != ZERR_NONE)
	return (retval);
    notice->z_checked_auth = auth;
    return ZERR_NONE;
}
