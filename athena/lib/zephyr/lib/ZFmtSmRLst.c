/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for the ZFormatSmallRawNoticeList function.
 *
 *	Created by:	John Kohl
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZFmtSmRLst.c,v $
 *	$Author: jtkohl $
 *
 *	Copyright (c) 1988 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
/* $Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZFmtSmRLst.c,v 1.1 1988-06-15 17:22:06 jtkohl Exp $ */

#ifndef lint
static char rcsid_ZFormatRawNoticeList_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZFmtSmRLst.c,v 1.1 1988-06-15 17:22:06 jtkohl Exp $";
#endif lint

#include <zephyr/mit-copyright.h>

#include <zephyr/zephyr_internal.h>

Code_t ZFormatSmallRawNoticeList(notice, list, nitems, buffer, ret_len)
    ZNotice_t *notice;
    char *list[];
    int nitems;
    ZPacket_t buffer;
    int *ret_len;
{
    Code_t retval;
    int hdrlen, i, size;
    
    if ((retval = Z_FormatRawHeader(notice, buffer, Z_MAXHEADERLEN, &hdrlen))
	!= ZERR_NONE)
	return (retval);

    size = 0;
    for (i=0;i<nitems;i++)
	size += strlen(list[i])+1;

    *ret_len = hdrlen+size;

    if (*ret_len > Z_MAXPKTLEN)
	return (ZERR_PKTLEN);

    ptr = buffer+hdrlen;

    for (;nitems;nitems--, list++) {
	i = strlen(*list)+1;
	bcopy(*list, ptr, i);
	ptr += i;
    }

    return (ZERR_NONE);
}
