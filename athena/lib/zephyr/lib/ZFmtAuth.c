/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for the ZFormatAuthenticNotice function.
 *
 *	Created by:	Robert French
 *
 *	$Id: ZFmtAuth.c,v 1.20 1999-01-22 23:19:06 ghudson Exp $
 *
 *	Copyright (c) 1987,1988 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */

#ifndef lint
static char rcsid_ZFormatAuthenticNotice_c[] = "$Id: ZFmtAuth.c,v 1.20 1999-01-22 23:19:06 ghudson Exp $";
#endif

#include <internal.h>

#ifdef HAVE_KRB4
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
				    &hdrlen, &ptr, NULL)) != ZERR_NONE)
	return (retval);

#ifdef NOENCRYPTION
    newnotice.z_checksum = 0;
#else
    newnotice.z_checksum =
	(ZChecksum_t)des_quad_cksum(buffer, NULL, ptr - buffer, 0, session);
#endif
    if ((retval = Z_FormatRawHeader(&newnotice, buffer, buffer_len,
				    &hdrlen, NULL, NULL)) != ZERR_NONE)
	return (retval);

    ptr = buffer+hdrlen;

    if (newnotice.z_message_len+hdrlen > buffer_len)
	return (ZERR_PKTLEN);

    (void) memcpy(ptr, newnotice.z_message, newnotice.z_message_len);

    *len = hdrlen+newnotice.z_message_len;

    if (*len > Z_MAXPKTLEN)
	return (ZERR_PKTLEN);

    return (ZERR_NONE);
}
#endif
