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
 * V M S _ S E L E C T
 *
 * The module implements select(2) and recvfrom(2).  This module is included
 * in both the TWG and Excelan object libraries.  (Excelan doesn't implement
 * select(2) and we don't trust TWG's implementation.)  Our needs are small.  
 * Don't assume the versions defined in here are completely general.  See
 * also "vms_twg.c" and "vms_excelan.c".
 */

#if defined(TWG)

#include "std.h"
#include <netinet/in.h>
#include <sys/ioctl.h>

#elif defined(EXCELAN)

struct timeval {
    long tv_sec;
    long tv_usec;
};
#include string
#include errno
#include assert
#include "exos$etc:ttcp.h"

#else

*** ERROR: TWG OR EXCELAN MUST BE DEFINED ***

#endif

#include "nbase.h"
#include "pfm.h"

#include ssdef
#include iodef

#define TIMER_ID        0xdead
#define MAX_PKT_SIZE    1024
#define MIN(x, y)       ((x) < (y) ? (x) : (y))

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

/*
 * The "select data" is the per-socket structure describing this module's
 * knowledge about the state of a socket.  Select data structs are linked
 * together off of "sd_head".  The structs are keyed by the "chan" (channel
 * number) field.
 */

struct select_data_t {
    int chan;
    boolean nbio;       /* Non-blocking I/O? */
    enum { sds_free, sds_empty, sds_in_select, sds_full } state;
    int recv_len;
    char buf[MAX_PKT_SIZE];
    struct sockaddr_with_len from;
    struct select_data_t *link;
};

static struct select_data_t *sd_head = NULL;

extern boolean qio_failed(), socket_has_data();

/*------------------------------------------------------------*/

static struct select_data_t *get_select_data(chan)
int chan;
{
    struct select_data_t *sd;
    struct select_data_t *free_sd = NULL;

    pfm_$inhibit();

    for (sd = sd_head; sd != NULL; sd = sd->link)
        if (sd->state == sds_free)
            free_sd = sd;
        else 
            if (sd->chan == chan) {
                pfm_$enable();
                return(sd);
            }

    if (free_sd != NULL)
        sd = free_sd;
    else {
        sd = (struct sd *) malloc(sizeof(struct select_data_t));
        sd->link = sd_head;
        sd_head = sd;
    }

    sd->chan = chan;
    sd->nbio = false;
    sd->state = sds_empty;

    pfm_$enable();

    return(sd);
}
  
/*------------------------------------------------------------*/

int close_socket(chan)
int chan;
{
    struct select_data_t *sd = get_select_data(chan);

    if (sd != NULL)
        sd->state = sds_free;

    return(netclose(chan)); 
}

/*------------------------------------------------------------*/

int set_socket_non_blocking(chan)
int chan;
{
    struct select_data_t *sd = get_select_data(chan);

    sd->nbio = true;
}

/*------------------------------------------------------------*/

recvfrom_nck(chan, buf, len, flags, from, fromlen)
int chan;
char *buf;
int len, flags;
struct sockaddr *from;
int *fromlen;
{
    struct select_data_t *sd = get_select_data(chan);
    int recv_len, bytes_to_read;

    pfm_$inhibit();

    switch (sd->state) {
        case sds_full:  
            recv_len = sd->recv_len;
            memcpy(buf, sd->buf, MIN(recv_len, len));
            *fromlen = MIN(sd->from.len, *fromlen);
            memcpy(from, &sd->from.sa, *fromlen);
            sd->state = sds_empty;
            break;

        case sds_in_select:
        case sds_empty:
            if (sd->nbio && ! socket_has_data(chan)) {
                errno = EWOULDBLOCK;
                recv_len = -1;
                break;
            }
               
            recv_len = recvfrom_qiow(chan, buf, len, flags, from, fromlen);
            break;
        }

    pfm_$enable();

    return(recv_len);
}

/*------------------------------------------------------------*/

int select_nck(n, readfds, writefds, exceptfds, timeout)
long n;
void *readfds, *writefds, *exceptfds;
struct timeval *timeout;
{
    long evflags;
    int status;
    int io_flag;
    int chan;
    long timeout_val[2];
    status_$t fst, st;
    pfm_$cleanup_rec crec;
    struct select_data_t *sd;
    boolean got_data;
    long sock_efn = -1;
    long timer_efn = -1;
#if defined(TWG)
    short iosb[4];
#elif defined(EXCELAN)
    struct ex_iosb iosb;
    struct SOioctl scntl;
#endif

    /*
     * Find the channel that was stuffed into the read fd set. 
     */

    chan = _findchan(readfds);
    if (chan == 0)
        return(0);

    sd = get_select_data(chan);

    if (sd->state == sds_full)
        return (1);

    /*
     * Are we just polling?  If so, just check to see if anything's there.
     */

    if (timeout != NULL && timeout->tv_sec == 0 && timeout->tv_usec == 0) 
        return(socket_has_data(chan) ? 1 : 0);

    assert(sd->state != sds_in_select); 
    sd->state = sds_in_select;

    fst = pfm_$cleanup(crec);
    if (fst.all != pfm_$cleanup_set) {
        if (sock_efn != -1)
            lib$free_ef(&sock_efn);
        if (timer_efn != -1)
            lib$free_ef(&timer_efn);
        if (sd->state == sds_in_select) {
            sd->state = sds_empty;
            sys$cancel(chan);
            sys$cantim(TIMER_ID, NULL);
        }
        pfm_$signal(fst);
    }

    /*
     * No timeout value supplied.  Block until some data arrives on the channel.
     */
 
    if (timeout == NULL) {
        int recv_len, from_len;
 
        from_len = sizeof(sd->from.sa);
 
        recv_len = recvfrom_qiow(chan, sd->buf, sizeof(sd->buf), 0, &sd->from.sa, &from_len);
        if (recv_len <= 0) {
            pfm_$rls_cleanup(crec, st);
            return(-1);
        }
 
        sd->from.len = from_len;
        sd->recv_len = recv_len;
        sd->state = sds_full;

        pfm_$rls_cleanup(crec, st);
        return(1);
    }

    /*
     * Standard case of a timed wait for something to come in on the socket. 
     */

    /*
     * Start asynch QIO on socket.
     */

    lib$get_ef(&sock_efn);
    assert(sock_efn != -1);

#if defined(TWG)
    status = sys$qio(sock_efn, chan, IO$_READVBLK, &iosb, NULL, NULL,
                     sd->buf, sizeof(sd->buf),
                     0,
                     &sd->from, sizeof(sd->from.sa),
                     NULL);
#elif defined(EXCELAN)
    scntl.hassa = 1;
    status = sys$qio(sock_efn, chan, EX__RECEIVE, &iosb, NULL, NULL,
                     sd->buf, sizeof(sd->buf),
                     &scntl,
                     NULL, NULL, NULL);
#endif

    if (status != SS$_NORMAL) {
        lib$free_ef(&sock_efn);
        sd->state = sds_empty;
        pfm_$rls_cleanup(crec, st);
        return(-1);
    }

    /*
     * Setup timer event.
     */

    timeout_val[0] = -10 * 1000 * 1000 * timeout->tv_sec;
    timeout_val[1] = -1;
    lib$get_ef(&timer_efn);
    assert(timer_efn != -1);
    assert(sock_efn / 32 == timer_efn / 32);        /* Must be in same cluster */

    status = sys$setimr(timer_efn, timeout_val, NULL, TIMER_ID);
    assert(status == SS$_NORMAL); 

    /*
     * Wait for either the QIO or timer to complete.
     */

    evflags = (1 << (sock_efn % 32)) | (1 << (timer_efn % 32));
    sys$wflor(sock_efn, evflags);

    /*
     * See if the socket event flag was set.  Cancel the timer in any case.
     * If we didn't get anything on the socket, cancel the socket qio and
     * return.
     */

    status = sys$readef(sock_efn, &evflags);
    got_data = (status == SS$_WASSET);

    sys$cantim(TIMER_ID, NULL);
    lib$free_ef(&timer_efn);
    lib$free_ef(&sock_efn);
    timer_efn = sock_efn = -1;

    if (! got_data) {
        sd->state = sds_empty;
        sys$cancel(chan);
        pfm_$rls_cleanup(crec, st);
        return(0);
    }

    /*
     * Check the socket qio completion.
     */

    if (qio_failed(SS$_NORMAL, &iosb)) {
        sd->state = sds_empty;
        pfm_$rls_cleanup(crec, st);
        return(-1);
    }

    /*
     * Everything's OK.  We save away the data we read.
     */

#if defined(TWG)
    sd->recv_len = iosb[1];
#elif defined(EXCELAN)
    sd->recv_len = iosb.ex_count;
    sd->from.sa = scntl.sa.skt;
    sd->from.len = sizeof(scntl.sa.skt);
#endif
    sd->state = sds_full;

    pfm_$rls_cleanup(crec, st);
    return(1);

}
