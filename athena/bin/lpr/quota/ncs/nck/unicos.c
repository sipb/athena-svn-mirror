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
 * U N I C O S
 *
 * Cray UNICOS System V Unix compatibility routines.
 */

#include "std.h"
#include "base.h"
#include "socket.h"

/*
 * the following interludes are necessary due to cray's non-standard
 * struct sockaddr and struct sockaddr_in layouts.
 */

#undef bind
#undef sendto
#undef recvfrom
#undef getsockname

int bind_nck(s, name, namelen)
int s;
socket_$addr_t *name;
int namelen;
{
    status_$t st;
    struct sockaddr saddr;
    int cc;

    socket_$to_local_rep(name, &saddr, &st);
    cc = bind(s, &saddr, sizeof saddr);
    return(cc);
}

int sendto_nck(s, msg, len, flags, to, tolen)
int s;
char *msg;
int len;
int flags;
socket_$addr_t *to;
int tolen;
{
    status_$t st;
    struct sockaddr saddr;
    int cc;

    socket_$to_local_rep(to, &saddr, &st);
    cc = sendto(s, msg, len, flags, &saddr, sizeof saddr);
    return(cc);
}

int recvfrom_nck(s, buf, len, flags, from, fromlen)
int s;
char *buf;
int len;
int flags;
socket_$addr_t *from;
int *fromlen;
{
    status_$t st;
    struct sockaddr saddr;
    int flen = sizeof saddr;
    int cc;

    cc = recvfrom(s, buf, len, flags, &saddr, &flen);
    if (cc != -1) {
        socket_$from_local_rep(from, &saddr, &st);
        *fromlen = sizeof *from;
    }
    return(cc);
}

int getsockname_nck(s, name, namelen)
int s;
socket_$addr_t *name;
int *namelen;
{
    status_$t st;
    struct sockaddr saddr;
    int nlen = sizeof saddr;
    int cc;

    cc = getsockname(s, &saddr, &nlen);
    if (cc != -1) {
        socket_$from_local_rep(name, &saddr, &st);
        *namelen = sizeof *name;
    }
    return(cc);
}
