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
 * M S _ F T P
 *
 * MS/DOS / INET-family / FTP Software -specific utilities.  (MS/DOS) 
 *
 */

#include <4bsddefs.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>

extern char ms_ftpa_dummy;      /* See ms_ftpa.asm for details */
static dummy_proc() { ms_ftpa_dummy = 0; }


int socket_nck(af, type, protocol)
int af;
int type;
int protocol;
{
    return socket(af, type, protocol);
}


int set_socket_non_blocking(fd)
int fd;
{
    fcntl(fd, F_SETFL, FNDELAY);
}


int close_socket(s)
int s;
{
    return bsd_close(s); 
}


int select_nck(nfds, readfds, writefds, exceptfds, timeout)
int nfds;
void *readfds, *writefds, *exceptfds;
struct timeval *timeout;
{
    return select(nfds, readfds, writefds, exceptfds, timeout);
}


int recvfrom_nck(s, buf, len, flags, from, fromlen)
int s;
char *buf;
int len, flags;
void *from;
int *fromlen;
{
    return recvfrom(s, buf, len, flags, from, fromlen);
}

int sendto_nck(s, msg, len, flags, to, tolen)
int s;
char *msg;
int len;
int flags;
void *to;
int tolen;
{
    return sendto(s, msg, len, flags, to, tolen);
}

int getsockname_nck(s, saddr, slen)
int s;
void *saddr;
int *slen;
{
    getsockname(s, saddr, slen);
}

int bind_nck(s, saddr, saddrlen)
int s;
void *saddr;
int saddrlen;
{
    bind(s, saddr, saddrlen);
}
