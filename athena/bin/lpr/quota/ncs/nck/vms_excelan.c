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
 * V M S _ E X C E L A N 
 *
 * Excelan-specific VMS support. 
 */

#include "nbase.h"
#include "pfm.h"

#include assert
#include descrip
#include ssdef
#include errno
#include perror
#include string

#include "exos$etc:ttcp.h"

#include <netdb.h>      /* From TWG */

struct timeval {
    long tv_sec;
    long tv_usec;
};

/*------------------------------------------------------------*/

boolean qio_failed(status, iosb)
int status;
struct ex_iosb *iosb;
{
    if (status != SS$_NORMAL) {
        errno = EIO;
        return(true);
    }

    if (iosb->ex_stat != SS$_NORMAL) {
        errno = iosb->ex_reply;
        return(true);
    }

    return(false);
}

/*------------------------------------------------------------*/

boolean socket_has_data(chan)
int chan;
{
    struct ex_iosb iosb;
    int nbytes;

    sys$qiow(0, chan, EX__IOCTL, &iosb, 0, 0, 0, 0, &nbytes, FIONREAD, 0, 0);
    return(nbytes > 0);
}

/*------------------------------------------------------------*/

int 
setsockopt(fd, opt, val, len)
    int fd;
    int opt;
    void *val;
    int len;
{
    /* assert(opt == SO_BROADCAST); */
}

/*------------------------------------------------------------*/

int 
socket(af, type, protocol)
    int af, type, protocol;
{

    $DESCRIPTOR(exos, "EX");    /* string descriptor for EX device */
    short qstatus;              /* return status */
    short channel;              /* channel number for EXOS device */
    struct SOioctl scntl;       /* socket control structure     */
    struct ex_iosb iosb;        /* I/O status block */

    assert(af == AF_INET);

    scntl.hassa = 0;
    scntl.hassp = 0;
    scntl.type = type;          /* set socket type */
    scntl.options = 0;          /* set socket options, assume no options */

    /* 
     * Assign the channel to EX device 
     */

    qstatus = sys$assign(&exos, &channel, 0, 0);
    if (qstatus != SS$_NORMAL)
        return (-1);

    /* 
     * Create a socket on the assigned channel
     */

    qstatus = sys$qiow(0, channel, EX__SOCKET, &iosb, 0, 0, 0, 0,
                      &scntl, 0, 0, 0);
    if (qio_failed(qstatus, &iosb))
        return (-1);

    return (channel);
}

/*------------------------------------------------------------*/

int netclose(chan)
int chan;   
{
    short qstatus;              /* return status */
    struct ex_iosb iosb;        /* I/O status block */

    qstatus = sys$qiow(0, chan, EX__CLOSE, &iosb, 0, 0, 0, 0, 0, 0, 0, 0);
    if (qio_failed(qstatus, &iosb))
        return (-1);

    qstatus = sys$dassgn(chan);

    return (0);
}

/*------------------------------------------------------------*/

recvfrom_qiow(
    int chan,
    char *buf,
    int len, 
    int flags,
    struct sockaddr *from,
    int *fromlen
    )
{
    struct ex_iosb iosb;        /* I/O status block */
    int status;
    struct SOioctl scntl;       /* socket control structure     */

    scntl.hassa = 1;

    status = sys$qiow(0, chan, EX__RECEIVE, &iosb, NULL, NULL,
                      buf, len,
                      &scntl,
                      NULL, NULL, NULL);
    if (qio_failed(status, &iosb))
        return(-1);

    *from = scntl.sa.skt;
    *fromlen = sizeof(scntl.sa.skt);

    return(iosb.ex_count);
}

/*------------------------------------------------------------*/

int 
bind(s, name, namelen)
    int s;
    struct sockaddr *name;
    int namelen;
{
    short qstatus;              /* return status */
    struct SOioctl scntl;       /* socket control structure */
    struct ex_iosb iosb;        /* I/O status block */
    static struct sockaddr_in maddr;

    /*
     * First close the socket that's open on the channel now.
     */

    qstatus = sys$qiow(0, s, EX__CLOSE, &iosb, 0, 0, 0, 0,
                      &scntl, 0, 0, 0);

    /*
     * Now recreate a socket with the specified sockaddr.
     */

    scntl.hassa = 1;
    scntl.hassp = 0; 
    scntl.type = SOCK_DGRAM;
    scntl.options = 0;
    scntl.sa.skt = *name;

    qstatus = sys$qiow(0, s, EX__SOCKET, &iosb, 0, 0, 0, 0,
                      &scntl, 0, 0, 0);
    if (qio_failed(qstatus, &iosb))
        return (-1);

    return (0);
}

/*------------------------------------------------------------*/

int 
getsockname(s, name, namelen)
    int s;
    struct sockaddr *name;
    int *namelen;
{
    short qstatus = 0;          /* return status */
    struct SOioctl scntl;       /* socket control structure */
    struct ex_iosb iosb;        /* I/O status block */

    /*
     * Get the socket name for the socket specified
     */

    qstatus = sys$qiow(0, s, EX__SOCKADDR, &iosb, 0, 0, 0, 0,
                      &scntl, 0, 0, 0);
    if (qio_failed(qstatus, &iosb))
        return (-1);

    *namelen = sizeof(scntl.sa.skt);
    *name = scntl.sa.skt;

    return (0);                 /* reply code if all goes well */
}

/*------------------------------------------------------------*/

int 
sendto_nck(s, msg, len, flags, to, tolen)
    int s;
    char *msg;
    int len, flags;
    struct sockaddr *to;
    int tolen;
{
    short qstatus;              /* return status */
    struct SOioctl scntl;       /* socket control structure */
    struct ex_iosb iosb;        /* I/O status block */

    /*
     * Send a datagram contained in 'msg' of length 'len' from the socket
     * specified 
     */

    scntl.hassa = 1;
    scntl.sa.skt = *to;

    qstatus = sys$qiow(0, s, EX__SEND, &iosb, 0, 0, msg, len, &scntl, 0, 0, 0);
    if (qio_failed(qstatus, &iosb))
        return (-1);

    return (iosb.ex_count);
}

/*------------------------------------------------------------*/

void 
gethostname(name, namelen)
    char *name;
    int namelen;
{
    int qstatus;
    short channel;
    struct ex_iosb iosb;        /* I/O status block */
    int zero = 0;
    int addr;
    char *namep;
    $DESCRIPTOR(exos, "EX");    /* string descriptor for EX device */

    name[0] = 0;

    qstatus = sys$assign(&exos, &channel, 0, 0);
    if (qstatus != SS$_NORMAL)
        return;

    qstatus = sys$qiow(0, channel, EX__ULOAD, &iosb, 0, 0, &addr, sizeof addr, &zero, 1, 0, 0);
    if (qstatus != SS$_NORMAL)
        goto DONE;

    namep = raddr(addr);
    if (namep == NULL)
        goto DONE;

    strncpy(name, namep, namelen);

DONE:

    sys$dassgn(channel);
}

/*------------------------------------------------------------*/

long 
ntohl(netlong)
    long netlong;
{
    unsigned char *p = &netlong;

    return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | (p[3] << 0);
}

/*------------------------------------------------------------*/

long 
inet_addr(cp)
    char *cp;
{
    unsigned long parts[4];
    unsigned long val;
    int npart;
    char c;

    short done = false;

    npart = 0;
    while (done == false) {

        val = 0;
        c = *cp;
        while (isdigit(c)) {
            val = (val * 10) + (c - '0');
            cp++;
            c = *cp;
        }
        if (*cp == '.') {
            if (npart >= 4)
                return (-1);
            parts[npart] = val;
            npart++;
            cp++;
        }
        else {
            if (*cp && !isspace(*cp))
                return (-1);
            if (npart >= 4)
                return (-1);
            parts[npart] = val;
            npart++;
            done = true;
        }
    }

    switch (npart) {
    case 1:
        val = parts[0];
        break;

    case 2:
        val = (parts[0] << 24) | (parts[1] & 0xffffff);
        break;

    case 3:
        val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
            (parts[2] & 0xffff);
        break;

    case 4:
        val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
            ((parts[2] & 0xff) << 8) | (parts[3] & 0xff);
        break;

    default:
        return (-1);
    }

    return (htonl(val));
}

/*------------------------------------------------------------*/

struct in_addr 
inet_makeaddr(net, lna)
    int net, lna;
{
    u_long in;

    if (net < 128)
        in = (net << 24) | lna;
    else if (net < 65536)
        in = (net << 16) | lna;
    else
        in = (net << 8) | lna;

    in = htonl(in);

    return (*(struct in_addr *) & in);
}

/*------------------------------------------------------------*/

char *
inet_ntoa(in)
    struct in_addr in;
{
    static char ascstr[80];

    sprintf(ascstr, "%d.%d.%d.%d\0", in.S_un.S_un_b.s_b1, in.S_un.S_un_b.s_b2,
            in.S_un.S_un_b.s_b3, in.S_un.S_un_b.s_b4);
    return (ascstr);
}

/*------------------------------------------------------------*/

int 
inet_netof(in)
    struct in_addr in;
{
    register u_long class_in_addr;

    class_in_addr = ntohl(in.s_addr);

    if (((((long) (class_in_addr)) & 0x80000000) == 0))
        return (((class_in_addr) & 0xff000000) >> 24);
    else if (((((long) (class_in_addr)) & 0xc0000000) == 0x80000000))
        return (((class_in_addr) & 0xffff0000) >> 16);
    else
        return (((class_in_addr) & 0xffffff00) >> 8);
}

/*------------------------------------------------------------*/

int 
inet_lnaof(in)
    struct in_addr in;
{
    register u_long class_in_addr;

    class_in_addr = ntohl(in.s_addr);

    if (((((long) (class_in_addr)) & 0x80000000) == 0))
        return ((class_in_addr) & 0x00ffffff);
    else if (((((long) (class_in_addr)) & 0xc0000000) == 0x80000000))
        return ((class_in_addr) & 0x0000ffff);
    else
        return ((class_in_addr) & 0x000000ff);
}
