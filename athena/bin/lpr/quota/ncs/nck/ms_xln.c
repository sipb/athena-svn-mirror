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
 * M S _ X L N
 *
 * MS/DOS / INET-family / Excelan-specific utilities.  (MS/DOS) 
 *
 */

/*
 * MS/C includes
 */

#include <stdlib.h> 
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

/*
 * XLN includes 
 */

#pragma pack(2)
#include <sys/init.h>
#include <sys/extypes.h>
#include <sys/exerrno.h>
#include <sys/socket.h>
#include <sys/soioctl.h>
#include <sys/exosopt.h>
#include <sys/drerror.h>
#include <sys/reply.h>
#include <netinet/in.h>
#include <ex_ioctl.h>
#pragma pack()

extern void *rpc_$malloc(int);
extern void rpc_$free(void *);

#ifdef MAX_DEBUG
#  define ASSERT(cond)  assert(cond)
#  ifndef SOCKET_DEBUG
#    define SOCKET_DEBUG
#  endif
#else
#  define ASSERT(cond)
#endif

/*
 * Reference the Excelan socket library (llibsock.lib) in the .obj file
 * so that anyone who binds with nck_xln.lib will automatically pick up 
 * the XLN socket library as well (assuming they defined the LIB 
 * environment variable correctly).
 */
#pragma comment(lib, "llibsock")


struct timeval {
    long tv_sec;
    long tv_usec;
};

u_long ntohl(), htonl();

#define IN_CLASSA(i)        (((long) (i) & 0x80000000l) == 0)
#define IN_CLASSA_NET       0xff000000l
#define IN_CLASSA_NSHIFT    24
#define IN_CLASSA_HOST      0x00ffffffl
#define IN_CLASSA_MAX       0x80

#define IN_CLASSB(i)        (((long) (i) & 0xc0000000l) == 0x80000000l)
#define IN_CLASSB_NET       0xffff0000l
#define IN_CLASSB_NSHIFT    16
#define IN_CLASSB_HOST      0x0000ffffl
#define IN_CLASSB_MAX       0x10000l

#define IN_CLASSC(i)        (((long) (i) & 0xc0000000l) == 0xc0000000l)
#define IN_CLASSC_NET       0xffffff00l
#define IN_CLASSC_NSHIFT    8
#define IN_CLASSC_HOST      0x000000ffl

#define MAX_SOCKETS 32
#define BUFLEN 1024

static int initialized = 0;

struct aio {
    char *buf;
    struct reply reply;
};

struct sdb {
    u_char ndelay;
    u_char stype;
    u_short port;
    struct aio recv;  
    struct aio send;  
};

struct sdb *sdb[MAX_SOCKETS];

extern void xln_anr_a();  /* in ms_xlna.asm */

/*
 * S O C K E T _ C L E A N U P 
 *
 * Close all open sockets.  This is an "atexit" routine.
 */

static void socket_cleanup()
{
    int i;

    for (i = 0; i < MAX_SOCKETS; i++)
        close_socket(i);
}


#define CHECK_SOCKET(s) { if (s < 0 || s > MAX_SOCKETS || sdb[s] == NULL) return (-1); }

/*
 * S O C K E T _ N C K 
 */

int socket_nck(af, type, protocol)
int af;
int type;
int protocol;
{
    struct sockaddr_in addr;
    int s;
    struct sdb *sp;

    /*** Excelan understands other families (e.g. 13), best avoid confusion */
    if (af != AF_INET) {
        errno = EAFNOSUPPORT;
        return(-1);
    }

    if (! initialized) {
        initialized = 1;
        atexit(socket_cleanup);
    }    

    memset(&addr, 0, sizeof addr);
    addr.sin_family = af;

    s = socket(type, (struct sockproto *) NULL, &addr, 0);

    if (s < 0)
        return (s);

    if (s >= MAX_SOCKETS) {
        soclose(s);
        errno = ENOTSOCK;       /* Best we can do */
        return (-1);
    }

    sp = (struct sdb *) rpc_$malloc(sizeof(struct sdb));
    if (sp == NULL) {
        errno = ENOBUFS;
        return (-1);
    }

    sp->stype = type;
    sp->ndelay = 0;

    /*
     * Set up recv reply so it looks like a "soreceive_anr" has finished.  Note
     * that "select_poll" depends on this.
     */

    sp->recv.buf = NULL;
    sp->recv.reply.exos_reply = 0;

    sp->send.buf = NULL;
    sp->send.reply.exos_reply = 0;

    /*
     * We'll need our port in "close_socket" and we can't seem to get "socketaddr"
     * work from there so do it here.
     */

    socketaddr(s, &addr);
    sp->port = addr.sin_port;

    sdb[s] = sp;

    return (s);
}


/*
 * B I N D _ N C K 
 *
 * Excelan doesn't have bind -- you must specify the local address/port in
 * socket().  Here's how we hack around this:  If the specified port is zero,
 * then the bind'er doesn't care and we can just use the address/port that
 * was assigned when socket() was called.  Otherwise, close the socket that
 * was passed in and open a new one using the specified address.  We cross
 * our fingers, say our prayers, and hope that we get the same socket file
 * descriptor back on the new socket().
 */

int bind_nck(s, saddr, saddrlen)
int s;
struct sockaddr_in *saddr;
int saddrlen;
{
    int s2;

    CHECK_SOCKET(s);

    if (saddr->sin_port == 0)
        return (0);

    soclose(s);

    s2 = socket((int) sdb[s]->stype, (struct sockproto *) NULL, saddr, 0);
    if (s2 < 0)
        return (s2);

    ASSERT(s2 == s);                /* Must get back same socket */

    sdb[s]->port = saddr->sin_port;

    return (0);
}


/*
 * S E T _ S O C K E T _ N O N _ B L O C K I N G
 */

int set_socket_non_blocking(s)
int s;
{
#ifdef NOTDEF
    short arg = 1;
    soioctl(s, FIONBIO, &arg);
#endif
    sdb[s]->ndelay = 1;   
}


/*
 * C L O S E _ S O C K E T
 */

int close_socket(s)
int s;
{
    struct aio *sap = &sdb[s]->send;
    struct aio *rap = &sdb[s]->recv;

    CHECK_SOCKET(s);

    /*
     * If we have a send buffer (a) wait until any I/O is done (ANR must be called
     * before we close up shop), and (b) free the buffer.
     */    

    if (sap->buf != NULL) {
        while (sap->reply.exos_reply == ENOT_DONE)
            ;
        rpc_$free(sap->buf);
    }

    /*
     * If we have a receive buffer (a) if there's a pending "sorecvfrom_anr" we
     * have to send something to ourselves so we can get the stupid ANR to run
     * before we close up shop, and (b) free the buffer.
     */

    if (rap->buf != NULL) {
        if (rap->reply.exos_reply == ENOT_DONE) {
            struct sockaddr_in addr;

            addr.sin_family = AF_INET;
            addr.sin_port = sdb[s]->port;

            addr.sin_addr.S_baddr.s_b1 = 127;
            addr.sin_addr.S_baddr.s_b2 = 0;
            addr.sin_addr.S_baddr.s_b3 = 0;
            addr.sin_addr.S_baddr.s_b4 = 0;     /* 0 or 1 as a function of NETLOAD -A */

            do {
                int r = sosend(s, &addr, "JUNKSTUFF", 9);
            }
            while (rap->reply.exos_reply == ENOT_DONE);
        }
        rpc_$free(rap->buf);
    }

    rpc_$free(sdb[s]);
    sdb[s] = NULL;

    return (soclose(s)); 
}


static int select_poll(nfds, readfds)
int nfds;
long *readfds;
{
    int s, r;
    long rfds = *readfds; 

    *readfds = 0;
    r = 0;

    for (s = 0; s < nfds; s++)
        if ((rfds & (1 << s)) != 0) {
            CHECK_SOCKET(s);
            if (sdb[s]->recv.reply.exos_reply != ENOT_DONE) {
                r++;
                *readfds |= (1 << s);
            }
        }

    return (r);
}


/*
 * S E L E C T _ N C K 
 *
 * We don't use XLN soselect because it can't be called from interrupt level.
 */

int select_nck(nfds, readfds, writefds, exceptfds, timeout)
int nfds;
long *readfds, *writefds, *exceptfds;
struct timeval *timeout;
{
    int r;
    long stop_time;
    long save_readfds;
    extern long time();

    ASSERT(writefds == NULL && exceptfds == NULL);

    /*
     * Timeout value of 0 means just do a single scan of the sockets and return.
     * Note that this is the path that will be called from interrupt level (in
     * "check_for_pkt" in "rpc_lsn.c").
     */

    if (timeout != NULL && timeout->tv_sec == 0 && timeout->tv_usec == 0)
        return (select_poll(nfds, readfds));

    ASSERT(! alarm_$in_alarm()); 

    /*
     * Null timeout pointer means wait forever.  Otherwise, compute stop time.
     */

    if (timeout != NULL)
        stop_time = time(NULL) + timeout->tv_sec;

    save_readfds = *readfds;

    do {
        *readfds = save_readfds;
        r = select_poll(nfds, readfds);
        if (r != 0)
            return (r);
        kbhit();
    } while (timeout == NULL || time(NULL) < stop_time);

    return (0);
}


/*
 * X L N  _ A N R
 *
 * Async notification routine (called from "xln_anr_a" -- the assembly language
 * routine named in the call to Excelan ANR socket calls).  We get the reply and
 * copy it into the user control block (which is pointed to from the reply).
 */

#pragma check_stack(off)  /* disable stack probe, called from interrupt */

void xln_anr(retdcbp, retcode)
char *retdcbp;
int retcode;
{
    static struct reply *replyp;            /* Static to save stack space */
    static struct reply temp_reply;         /* ditto */

    if (getreply(&temp_reply, retdcbp) != 0)
        return;

    replyp = (struct reply *) temp_reply.usercb;
    if (replyp == NULL)
        return;

    memcpy(replyp, &temp_reply, sizeof(temp_reply));

    /* 
     * If a driver error occurred, record it over top of any board error.
     */

    if (retcode < 0)
        replyp->exos_reply = retcode;
}

#pragma check_stack()  /* stack probe reverts to command line selection */

/* 
 * R E C V F R O M _ N C K 
 * 
 * This routine can be called from interrupt level and the manual says you
 * can't call "soreceive" from interrupt level -- you must call "soreceive_anr".
 * So we do all the hair associated with that.
 */

int recvfrom_nck(s, buf, len, flags, from, fromlen)
int s;
char *buf;
int len, flags;
void *from;
int *fromlen;
{
    struct aio *ap = &sdb[s]->recv;
    static struct sockaddr junk;
    int r; 

    CHECK_SOCKET(s);

    ASSERT(sdb[s]->ndelay);                 /* Only non-blocking I/O allowed */

    /*
     * If I/O in progress (not done), return now.
     */

    if (ap->reply.exos_reply == ENOT_DONE) {
        errno = EWOULDBLOCK;
        return (-1);
    }

    /*
     * On the first call to this routine, there won't be a receive buffer
     * allocated and no "sorecvfrom_anr" operation will be pending.  In this case,
     * just alloc up the buffer.  Otherwise, process the results of the 
     * "sorecvfrom_anr" that must have finished.  
     */

    if (ap->buf == NULL) {
        ap->buf = rpc_$malloc(BUFLEN);
        if (ap->buf == NULL) {
            errno = ENOBUFS;    
            return (-1);
        }
        errno = EWOULDBLOCK;
        r = -1;
    }
    else {
        if (ap->reply.exos_reply != 0) {
            errno = ap->reply.exos_reply;
            r = -1;
        }
        else {
            *fromlen = sizeof(struct sockaddr);
            memcpy(from, &ap->reply.params.soaddr, *fromlen);

            r = min(len, ap->reply.count); 
            memcpy(buf, ap->buf, r);
        }
    }

    /*
     * Start up the next receive.
     */

    ap->reply.exos_reply = ENOT_DONE;
    if (soreceive_anr(s, &junk, ap->buf, BUFLEN, xln_anr_a, &ap->reply) < 0) {
        ap->reply.exos_reply = 0;        
        return (-1);
    }

    return (r);
}


/*
 * S E N D T O _ N C K 
 *
 * This routine can be called from interrupt level and the manual says you
 * can't call "sosend" from interrupt level -- you must call "sosend_anr".
 * So we copy the pkt (ugh) into our allocated buffer and start up the async
 * send.
 */

int sendto_nck(s, msg, len, flags, to, tolen)
int s;
char *msg;
int len;
int flags;
void *to;
int tolen;
{
    struct aio *ap = &sdb[s]->send;

    CHECK_SOCKET(s);

    /*
     * Send in progress?  If so and we're in the alarm handler, just drop this
     * one; if we're not, wait until the send is done.
     */

    if (ap->reply.exos_reply == ENOT_DONE) {
        if (alarm_$in_alarm()) {
            errno = EALREADY;   
            return (-1);
        }
        while (ap->reply.exos_reply == ENOT_DONE)
            ;
    }

    /*
     * Alloc up the send buffer now if we have to.
     */

    if (ap->buf == NULL) {
        ap->buf = rpc_$malloc(BUFLEN);
        if (ap->buf == NULL) {
            errno = ENOBUFS;
            return (-3);
        }
    }

    /*
     * Copy the pkt into our buffer and start up the send.
     */

    memcpy(ap->buf, msg, len);

    ap->reply.exos_reply = ENOT_DONE;
    sosend_anr(s, to, ap->buf, len, xln_anr_a, &ap->reply); 

    return (len);
}


/*
 * G E T S O C K N A M E _ N C K
 */

int getsockname_nck(s, saddr, slen)
int s;
struct sockaddr *saddr;
int *slen;
{
    *slen = sizeof(struct sockaddr);
    socketaddr(s, saddr);
}


/*
 * Miscellaneous BSD address manipulation procedures.
 */

long inet_addr(name)
char *name;
{
    unsigned int b[4];
    unsigned long a;

    if (sscanf(name, "%d.%d.%d.%d", &b[0], &b[1], &b[2], &b[3]) < 4)
        return (-1);

	/*** Note that bytes are in network order */
    ((char *) &a)[0] = b[0];
    ((char *) &a)[1] = b[1];
    ((char *) &a)[2] = b[2];
    ((char *) &a)[3] = b[3];
    return(a);
}


char *inet_ntoa(addr)
struct in_addr addr;
{
    static char buff[20];

    sprintf(buff, "%u.%u.%u.%u", 
            (unsigned char) addr.S_un.S_un_b.s_b1, (unsigned char) addr.S_un.S_un_b.s_b2,
            (unsigned char) addr.S_un.S_un_b.s_b3, (unsigned char) addr.S_un.S_un_b.s_b4);

    return (buff);
}


struct in_addr inet_makeaddr(net, lna)
unsigned long net;
unsigned long lna;
{
    struct in_addr ia;

    if (net < IN_CLASSA_MAX)
        ia.s_addr = (net << IN_CLASSA_NSHIFT) | (lna & IN_CLASSA_HOST);
    else if (net < IN_CLASSB_MAX)
        ia.s_addr = (net << IN_CLASSB_NSHIFT) | (lna & IN_CLASSB_HOST);
    else
        ia.s_addr = (net << IN_CLASSC_NSHIFT) | (lna & IN_CLASSC_HOST);

    ia.s_addr = htonl(ia.s_addr);

    return (ia);
}


long inet_netof(addr)
struct in_addr addr;
{
    unsigned long a = ntohl(addr.s_addr);

    if (IN_CLASSA(a))
        return (((a) & IN_CLASSA_NET) >> IN_CLASSA_NSHIFT);
    if (IN_CLASSB(a))
        return (((a) & IN_CLASSB_NET) >> IN_CLASSB_NSHIFT);

    return (((a) & IN_CLASSC_NET) >> IN_CLASSC_NSHIFT);

}


int gethostname(name, namelen)
char *name;
int namelen;
{
    long infoaddr = 0x00000000l;
    struct exosopt exosopts;
    char *p, *raddr();
    int bd;
    FILE* dummy;
    int retval;

    bd = brdopen(0, 1);
    brdioctl(bd, BRDADDR, &infoaddr);
    brdread(bd, &exosopts, sizeof exosopts);
    brdclose(bd);

    /*???
     * Temp hack to get around bug in Excelan's raddr() and rhost()
     * functions (don't close "hosts" file).
     */
    dummy = fopen("con", "r");
    ASSERT(dummy != NULL);
    fclose(dummy);

    p = raddr(* (long *) exosopts.xo_iaddr);

    /*??? close file again, since raddr left one open */
    retval = fclose(dummy);
/*???
    ASSERT(retval == 0);
*/

    if (p == NULL)
        return (-1);

    strncpy(name, p, namelen-1);
    name[namelen-1] = '\0';

    free(p);    /* free up heap buf allocated in raddr() */
    return (0);
}


#ifdef SOCKET_DEBUG

socket_$dump(s, level, fp)
int s;
int level;
FILE* fp;
{
    /*???*/
}

socket_$dump_all(level, fp)
int level;
FILE* fp;
{
    /*???*/
}

#endif
