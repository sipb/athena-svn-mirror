/*
 * ==========================================================================
 * Confidential and Proprietary.  Copyright 1987 by Apollo Computer Inc.,
 * Chelmsford, Massachusetts.  Unpublished -- All Rights Reserved Under
 * Copyright Laws Of The United States.
 * 
 * Apollo Computer Inc. reserves all rights, title and interest with respect
 * to copying, modification or the distribution of such software programs
 * and associated documentation, except those rights specifically granted
 * by Apollo in a Product Software Program License, Source Code License
 * or Commercial License Agreement (APOLLO NETWORK COMPUTING SYSTEM) between
 * Apollo and Licensee.  Without such license agreements, such software
 * programs may not be used, copied, modified or distributed in source
 * or object code form.  Further, the copyright notice must appear on the
 * media, the supporting documentation and packaging as set forth in such
 * agreements.  Such License Agreements do not grant any rights to use
 * Apollo Computer's name or trademarks in advertising or publicity, with
 * respect to the distribution of the software programs without the specific
 * prior written permission of Apollo.  Trademark agreements may be obtained
 * in a separate Trademark License Agreement.
 * ========================================================================== 
 *
 * V M S _ T W G 
 *
 * TWG-specific VMS support.  We reimplement sendto(2) also because of apparent
 * AST problems.  See also "vms_select.c".
 */

#include "nbase.h"
#include "std.h"

#include <sys/ioctl.h>
#include <netinet/in.h>

#include iodef
#include ssdef

#undef sendto

/*------------------------------------------------------------*/

/*
 * Note that the TWG recvfrom QIO (IO$_READVBLK) require that their "p4"
 * be *not* a sockaddr, but a structure that consists of the length followed
 * by a sockaddr.  That way, they can return the sockaddr length as part
 * of the sockaddr result parameter.  Thus, the need for the
 * "sockaddr_with_len" struct.
 */

struct sockaddr_with_len {  
    short len;
    struct sockaddr sa;
};

/*------------------------------------------------------------*/

boolean qio_failed(status, iosb)
int status;
short *iosb;
{
    if (status != SS$_NORMAL) {
        errno = EIO;
        return(true);
    }

    if ((iosb[0] & 1) == 0) {
        errno = (0x7fff & iosb[0]) >> 3;
        return(true);
    }

    return(false);
}

/*------------------------------------------------------------*/

sendto_nck(chan, buf, len, flags, to, tolen)
int chan;
char *buf;
int len, flags;
struct sockaddr *to;
int tolen;
{
    int status;
    short iosb[4];

    status = sys$qiow(0, chan, IO$_WRITEVBLK, &iosb, NULL, NULL,
                      buf, len,
                      flags,
                      to, tolen,
                      NULL);

    if (qio_failed(status, iosb))  
        return(-1);

    return(len);
}

/*------------------------------------------------------------*/

boolean socket_has_data(chan)
int chan;
{
    int nbytes;

    ioctl(chan, FIONREAD, &nbytes);
    return(nbytes > 0);
}

/*------------------------------------------------------------*/

recvfrom_qiow(chan, buf, len, flags, from, fromlen)
int chan;
char *buf;
int len;
int flags;
struct sockaddr *from;
int *fromlen;
{
    short iosb[4];
    int status;
    struct sockaddr_with_len lfrom;

    status = sys$qiow(0, chan, IO$_READVBLK, &iosb, NULL, NULL,
                      buf, len,
                      flags,
                      &lfrom, *fromlen,
                      NULL);

    if (qio_failed(status, iosb))
        return(-1);

    *from = lfrom.sa;
    *fromlen = lfrom.len;

    return(iosb[1]);
}

