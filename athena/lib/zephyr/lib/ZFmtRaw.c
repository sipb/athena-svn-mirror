/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for the ZFormatRawNotice function.
 *
 *	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZFmtRaw.c,v $
 *	$Author: jfc $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
/* $Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZFmtRaw.c,v 1.7 1991-06-20 14:29:34 jfc Exp $ */

#ifndef lint
static char rcsid_ZFormatRawNotice_c[] = "$Id: ZFmtRaw.c,v 1.7 1991-06-20 14:29:34 jfc Exp $";
#endif

#include <zephyr/zephyr_internal.h>

Code_t ZFormatRawNotice(notice, buffer, ret_len)
    register ZNotice_t *notice;
    char **buffer;
    int *ret_len;
{
    char header[Z_MAXHEADERLEN];
    int hdrlen;
    Code_t retval;

    if ((retval = Z_FormatRawHeader(notice, header, sizeof(header),
				    &hdrlen, (char **) 0)) != ZERR_NONE)
	return (retval);

    *ret_len = hdrlen+notice->z_message_len;

    /* *ret_len is never 0, don't have to worry about malloc(0) */
    if (!(*buffer = malloc((unsigned) *ret_len)))
	return (ENOMEM);

    bcopy(header, *buffer, hdrlen);
    bcopy(notice->z_message, *buffer+hdrlen, notice->z_message_len);

    return (ZERR_NONE);
}
