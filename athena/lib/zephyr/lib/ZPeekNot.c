/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for ZPeekNotice function.
 *
 *	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZPeekNot.c,v $
 *	$Author: lwvanels $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
/* $Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZPeekNot.c,v 1.7 1991-12-04 13:48:10 lwvanels Exp $ */

#ifndef lint
static char rcsid_ZPeekNotice_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZPeekNot.c,v 1.7 1991-12-04 13:48:10 lwvanels Exp $";
#endif

#include <zephyr/mit-copyright.h>

#include <zephyr/zephyr_internal.h>

Code_t ZPeekNotice(notice, from)
    ZNotice_t *notice;
    struct sockaddr_in *from;
{
    char *buffer;
    int len;
    Code_t retval;
	
    if ((retval = ZPeekPacket(&buffer, &len, from)) != ZERR_NONE)
	return (retval);

    return (ZParseNotice(buffer, len, notice));
}
