/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for the ZFormatAuthenticNotice function.
 *
 *	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZFmtAuth.c,v $
 *	$Author: jfc $
 *
 *	Copyright (c) 1987,1988 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
/* $Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZFmtAuth.c,v 1.12 1991-06-18 13:44:32 jfc Exp $ */

#ifndef lint
static char rcsid_ZFormatAuthenticNotice_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZFmtAuth.c,v 1.12 1991-06-18 13:44:32 jfc Exp $";
#endif

#include <zephyr/mit-copyright.h>

#include <zephyr/zephyr_internal.h>

#ifdef KERBEROS
Code_t ZFormatAuthenticNotice(notice, buffer, buffer_len, len, session)
    ZNotice_t *notice;
    register char *buffer;
    register int buffer_len;
    int *len;
    C_Block session;
{
    ZNotice_t newnotice;
    char *ptr;
    int retval, hdrlen;

    newnotice = *notice;
    newnotice.z_auth = 1;
    newnotice.z_authent_len = 0;
    newnotice.z_ascii_authent = "";

    if ((retval = Z_FormatRawHeader(&newnotice, buffer, buffer_len,
				    &hdrlen, &ptr)) != ZERR_NONE)
	return (retval);

#ifdef NOENCRYPTION
    newnotice.z_checksum = 0;
#else
    newnotice.z_checksum = (ZChecksum_t)des_quad_cksum(buffer, NULL,
						       ptr - buffer, 0,
						       session);
#endif
    if ((retval = Z_FormatRawHeader(&newnotice, buffer, buffer_len,
				    &hdrlen, (char **) 0)) != ZERR_NONE)
	return (retval);

    ptr = buffer+hdrlen;

    if (newnotice.z_message_len+hdrlen > buffer_len)
	return (ZERR_PKTLEN);

    bcopy(newnotice.z_message, ptr, newnotice.z_message_len);

    *len = hdrlen+newnotice.z_message_len;

    if (*len > Z_MAXPKTLEN)
	return (ZERR_PKTLEN);

    return (ZERR_NONE);
}
#endif
