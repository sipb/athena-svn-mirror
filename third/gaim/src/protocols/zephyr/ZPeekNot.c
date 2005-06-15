/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for ZPeekNotice function.
 *
 *	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/third/gaim/src/protocols/zephyr/ZPeekNot.c,v $
 *	$Author: ghudson $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
/* $Header: /afs/dev.mit.edu/source/repository/third/gaim/src/protocols/zephyr/ZPeekNot.c,v 1.1.1.1 2005-06-15 16:40:33 ghudson Exp $ */

#ifndef lint
static char rcsid_ZPeekNotice_c[] = "$Header: /afs/dev.mit.edu/source/repository/third/gaim/src/protocols/zephyr/ZPeekNot.c,v 1.1.1.1 2005-06-15 16:40:33 ghudson Exp $";
#endif

#include "internal.h"

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
