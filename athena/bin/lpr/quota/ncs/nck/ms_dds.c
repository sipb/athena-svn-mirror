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
 * M S _ D D S
 *
 * MS/DOS / DDS-family -specific utilities.  (MS/DOS) 
 *
 * Written by Kevin Ackley of Systems Guild Inc.
 */

#include "std.h"
#include "msdos.h"
#include "socket.h"
#include "xport.h"

extern void *rpc_$malloc(int);
extern void rpc_$free(void *);

/*===========================================================================
Berkley socket functions:

- sockets are buffer one packet deep receiving, not buffered transmitting.
- cid identifies which rbuf got data, the rbuf contains the socket number
  where it is queued.
- some of the data structs here probably should be marked as volatile
- keep trying to redo crucial commands if the xport is temp unable to 
  process them?
===========================================================================*/

#define AF_DDS    13
#define dds_$broadcast_node 0xefffffff

struct dds_addr {
    long        network;
    long        node;
};

struct sockaddr_dds {
    short           sdds_family;
    short           sdds_port;
    struct dds_addr sdds_addr;
};

struct dds_net_addr {
    short family;
    struct dds_addr na;
};

struct dds_host_id {
    short family;
    long node;
};

/*** Get network,node,port from dds sockaddr */
#define GET_ADDR(saddr, netno, addr) { \
    *((u_long*) (netno)) = ntohl((saddr)->sdds_addr.network);   \
    *((u_long*) &(addr)[0]) = ntohl((saddr)->sdds_addr.node);   \
    *((u_short*) &(addr)[6]) = ntohs((saddr)->sdds_port);       \
}

/*** Set family,network,node,port in dds sockaddr */
#define SET_ADDR(saddr, netno, addr) { \
    (saddr)->sdds_family = socket_$dds;                 \
    (saddr)->sdds_addr.network = htonl(*((u_long*) (netno)));   \
    (saddr)->sdds_addr.node = htonl(*((u_long*) &(addr)[0]));   \
    (saddr)->sdds_port = htons(*((u_short*) &(addr)[6]));       \
}

/*===========================================================================
The socket implementation that NCK expects is constructed out of the Microsoft
XPORT network layer primitives.  Three primitives do most of the work: send a
datagram, broadcast a datagram, and receive a datagram.  The socket library is
notified asynchronously of command completion through a completion routine
specified in each XPORT command.  Only the completion routine for receive
commands is assumed to be called asynchronously (thus transmit data buffers are
immediately reusable after the command is issued).

Several quirks in the function of the XPORT receive command make life somewhat
more difficult:
    1) The address of the buffer to hold the incoming data must be specified
when the receive command is issued (not, for example, allocated in the 
completion routine).
    2) Each receive command is bound to a specific port, and will not receive
packets addressed to other ports.
    3) The XPORT commands may not be issued in a completion routine.
    4) The maximum number of simultaneous outstanding receive commands is not a
large number (possibly as small as 16 to 32).
    5) XPORT commands can fail in an interrupt because the driver is busy.
Restarting a recv command must be retried later.

Journey of a received packet:  Each socket (port) that NCK could receive
packets on has several outstanding receive commands issued for that port. Each
command has its own separate data buffer (called an rbuf, which must be the
maximum NCK packet size of 1024 bytes).  When a packet arrives on a port, the
asynchronous notification routine (ANR) puts the rbuf on a queue for that
particular port (called an rbq).  When NCK polls to see whether a packet has
arrived on a port or set of ports, the rbq is consulted.  If a packet is in
the queue, the data is copied out of the rbuf and a new receive command for the
same port is issued.

Since is it is anticipated that the depth of received packet queueing might,
in the future, vary from port to port, the depth of the receive queue is
specified separately from the default number of rbufs allocated for a socket.
===========================================================================*/

#define MAX_SOCKETS             32      /* max # of sockets that can be simultaneously open */
#define MAX_PACKET_SIZE         1024

#define DFLT_RBUFS_PER_SOCKET   8
#define MAX_RBUFS_PER_SOCKET    16      /* must be power of 2 */
#define MAX_TOTAL_RBUFS         128

#define FIRST_USER_PORT         32      /* first avail port for RPC */
#define MAX_PORTS               (MAX_SOCKETS+FIRST_USER_PORT)

typedef struct {
    u_short         sock_no;        /* sockno that owns this rbuf       */
    u_short         cmd_completed;  /* set when anr is called           */
    xport_$acb_rep   acb;            /* completion acb from xport driver */
    char            data[MAX_PACKET_SIZE]; /* buffer to receive data in */
    u_short         couldnt_restart;/* error in restarting recv cmd     */
#ifdef SOCKET_DEBUG
    long    recv_cnt;           /* packets received (incr in ANR)       */
    long    recv_started_cnt;   /* started recv cmd                     */
    long    recv_failure_cnt;   /* failed to start a recv cmd           */
#endif
} rep_recv_buf;
typedef rep_recv_buf* recv_buf;

/*** Pool of receive buffers waiting for packet (rbufs are malloc'd) */
internal recv_buf rbuf_pool[MAX_TOTAL_RBUFS] = {0};

/*** Recovery mechanism when restarting recv cmd fails */
internal u_char try_restart = 0;  /* count of outstanding failed recvs */
#define CHECK_RECV() \
    {if (try_restart) restart_recv();}

/***
 *** Queue for each socket of received dgrams.  Set from interrupt.  rbq[s] is
 *** a list of rbuf_no's (i.e. indices into rbufs) for socket s.  rbq_head[s]
 *** is the index into rbq[s] of the head; rbq_tail[s] is the index of the tail.
 ***/
internal u_char rbq_head[MAX_SOCKETS] = {0};  /* index of head of q (last used)  */
internal u_char rbq_tail[MAX_SOCKETS] = {0};  /* index of end of q (last filled) */
internal u_char rbq_q[MAX_SOCKETS][MAX_RBUFS_PER_SOCKET] = {0};

#define FIRST_CID               16                          /* 16 */
#define SEND_CID                (FIRST_CID + 1)             /* 17 */
#define CANCEL_CID              (SEND_CID + 1)              /* 18 */
#define FIRST_RBUF_CID          (CANCEL_CID + 1)            /* 19 */
#define RBUF_NO_TO_CID(rbuf_no) ((rbuf_no) + FIRST_RBUF_CID)
#define CID_TO_RBUF_NO(cid)     ((cid) - FIRST_RBUF_CID)

#define QUEUE_EMPTY(sockno) (rbq_head[sockno] == rbq_tail[sockno])

/*** Pool of sockets, socket descriptor is index */
internal struct {
    u_char  in_use;             /* is socket allocated?             */
    u_char  bound;              /* has port been bound?             */
    struct  sockaddr_dds saddr; /* the local sockaddr (port mostly) */
    short   slen;               /* length of local saddr            */
    u_char  n_rbufs;            /* depth of recv buffering          */
#ifdef SOCKET_DEBUG
    long    xmit_cnt;           /* packets sent successfully        */
    long    xmit_failure_cnt;   /* failed to xmit                   */
#endif
} sock_pool[MAX_SOCKETS] = {0};

/*** Bit vector of ports that have been allocated */
internal u_short ports_in_use[(MAX_PORTS-1)/sizeof(u_short) + 1] = {0};

internal u_char socket_cleanup_set = 0;

#ifdef SOCKET_DEBUG
/*** Count XPORT error return codes */
#  define MAX_XPORT_$ERROR  0x50
long    xport_$error_cnt[MAX_XPORT_$ERROR+1];
#endif

/*** Local functions */
internal restart_recv(void);
internal unalloc_rbufs(int s);
internal void socket_cleanup(void);
internal socket_$dump_rbufs(int s, int level, FILE* fp);
internal socket_$dump_all_rbufs(int level, FILE* fp);

int socket_nck(af, type, protocol)
int af;         /* should be AF_DDS */
int type;       /* must be SOCK_DGRAM */
int protocol;   /* ignored */
{
    register short i;

    CHECK_RECV();
    if (af != AF_DDS) {
        errno = EAFNOSUPPORT;
        return(-1);
    }
    ASSERT(type == SOCK_DGRAM);

    alarm_$disable();
    if (!socket_cleanup_set) {
        socket_cleanup_set = 1;
        atexit(socket_cleanup);
    }

    /*** Find an unused socket */
    for (i = 0; i < MAX_SOCKETS; i++) {
        if (!sock_pool[i].in_use)
            break;
    }
    if (i == MAX_SOCKETS) {
        errno = EMFILE;  /* out of descriptors */
        alarm_$enable();
        return(-1);
    }
    sock_pool[i].in_use = 1;

    sock_pool[i].bound = 0;
    sock_pool[i].n_rbufs = DFLT_RBUFS_PER_SOCKET;
#ifdef SOCKET_DEBUG
    sock_pool[i].xmit_cnt = 0L;
    sock_pool[i].xmit_failure_cnt = 0L;
#endif
    alarm_$enable();
    return(i);
}

int bind_nck(s, saddr, saddrlen)
int s;
socket_$addr_t* saddr;
int saddrlen;
{
    struct sockaddr_dds* dds_saddr = (struct sockaddr_dds*) saddr;
    register u_short port;
    register int i;

    CHECK_RECV();
    if (s < 0 || MAX_SOCKETS <= s || !sock_pool[s].in_use) {
        errno = EBADF;  /* bad descriptor, not gotten from socket() */
        return(-1);
    }
    if (sock_pool[s].bound) {
        errno = EINVAL; /* already bound */
        return(-1);
    }
    if (saddr->family != socket_$dds) {
        errno = EADDRNOTAVAIL; /* only know dds family, for now */
        return(-1);
    }

    /*** Allocate rbufs for this socket, number to use should be set by now */
    if (!alloc_rbufs(s)) {
        errno = ENOBUFS; /* out of buffer space */
        return(-1); /*??? is it reasonable to report this in this function? */
    }
    
    saddrlen = min(sizeof(struct sockaddr_dds), saddrlen);

    port = ntohs(dds_saddr->sdds_port);

    /*** If given port is 0, then allocate an unused port */
    if (port == 0) {
        alarm_$disable();
        for (i = FIRST_USER_PORT; i < MAX_PORTS; i++) {
            if (!(ports_in_use[i>>4] & (1<<(i&15))))
                break;
        }
        if (i == MAX_PORTS) {
            errno = EADDRNOTAVAIL;  /* all ports alloc'd */
            alarm_$enable();
            return(-1);
        }
    
        port = i;

        /*** Mark port as used */
        ports_in_use[port>>4] |= (1<<(port&15));
        alarm_$enable();

        /*** Save the port alloc'd in sockaddr */
        dds_saddr->sdds_port = htons(port);

    } else {  /* binding to a specific port */
#ifdef NOT_YET  /*!!! Some of the well known ports are < FIRST_USER_PORT */
        if (port < FIRST_USER_PORT || MAX_PORTS <= port) {
            errno = EADDRNOTAVAIL; /* port out of bounds */
            return(-1);
        }
#endif
        if (ports_in_use[port>>4] & (1<<(port&15))) {
            errno = EADDRNOTAVAIL;  /* port bound twice */
            return(-1);
        }

        /*** Mark port as used */
        alarm_$disable();
        ports_in_use[port>>4] |= (1<<(port&15));
        alarm_$enable();
    }

    /*** Keep a copy of the sockaddr */
    ASSERT(saddrlen <= sizeof(sock_pool[s].saddr));
    memcpy((char*) &sock_pool[s].saddr, (char*) dds_saddr, saddrlen);

    /*** Start out with an empty receive queue */
    rbq_head[s] = rbq_tail[s] = 0;

    /*** Start dgram recv cmd for each rbuf */
    for (i = 0; i < MAX_TOTAL_RBUFS; i++) { 
        if (rbuf_pool[i] != NULL && rbuf_pool[i]->sock_no == s) {
            if (!recv_start_rbuf(i)) {
                errno = EADDRNOTAVAIL;  /* local addr probably bad */
                return(-1);
            }
        }
    }

    sock_pool[s].bound = 1;
    sock_pool[s].slen = saddrlen;
    return(0);
}

int select_nck(nfds, readfds, writefds, exceptfds, timeout)
int nfds;
fd_set* readfds;
fd_set* writefds;   /* ignored */
fd_set* exceptfds;  /* ignored */
struct timeval* timeout; /* only seconds field used */
{
    short do_timeout;
    short wait;
    long end_time;
    register short i;
    fd_set got_mask;
    short got_count;

    /***  Calc if/when should timeout */
    if (timeout == NULL)
        do_timeout = 0;
    else {
        do_timeout = 1;
        if (timeout->tv_sec == 0)
            wait = 0;
        else {
            wait = 1;
#ifdef USE_INTERRUPT
            alarm_$on();
            end_time = alarm_$time() + SEC_TO_TICK(timeout->tv_sec);
#else
            end_time = time(NULL) + timeout->tv_sec;
#endif
        }
    }

    FD_ZERO(&got_mask);

    for (got_count = 0; got_count == 0; ) {
        CHECK_RECV();

        /*** Check each socket in mask for packets */
        for (i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET(i, readfds)) {
                if (!sock_pool[i].in_use || !sock_pool[i].bound) {
                    errno = EBADF;
                    return(-1);
                }
                if (!QUEUE_EMPTY(i)) {
                    /*??? check status? what if cancelled previously? */
                    got_count++;
                    FD_SET(i, &got_mask);
                }
            }
        }
#ifdef USE_INTERRUPT
        if (do_timeout && (!wait || end_time <= alarm_$time()))
            break;  /* time is up */
#else
        if (do_timeout && (!wait || end_time <= time(NULL)))
            break;  /* time is up */
#endif

        /*** If not in interrupt, allow MSDOS to poll ^C,^break flag */
        if (!alarm_$in_alarm())
            kbhit();   /*??? ungetch, oops */
    }

    FD_COPY(&got_mask, readfds);
    
    /*** Note that writefds and exceptfds are not zeroed */
    return(got_count);
}

int recvfrom_nck(s, buf, len, flags, from, fromlen)
register int s;         /* receiving socket fd */
char* buf;              /* data buffer */
int len;                /* length of receiving data buffer */
int flags;              /* ignored */
socket_$addr_t* from;   /* addr of sender */
int* fromlen;           /* length of from sockaddr */
{
    u_char rbuf_no; /* index of rbuf that received data */
    xport_$acb acb;  /* copy of completion acb saved by anr */

    CHECK_RECV();
    if (s < 0 || MAX_SOCKETS <= s || !sock_pool[s].in_use) {
        errno = EBADF;
        return(-1);
    }
    if (!sock_pool[s].in_use || !sock_pool[s].bound) {
        errno = EBADF;
        return(-1);
    }

    /*** Don't block if no packets ready */
    alarm_$disable();
    if (QUEUE_EMPTY(s)) {
        errno = EWOULDBLOCK; /* no packets to read */
        alarm_$enable();
        return(-1);
    }

#ifdef SOCKET_MAX_DEBUG
    if (socket_$max_debug) {
        printf("recvfrom: packet ready\n");
        socket_$dump(s, 1, stdout);
    }
#endif

    /*** Remove rbuf number from head of queue */
    rbq_head[s] = (rbq_head[s]+1) & (MAX_RBUFS_PER_SOCKET-1);
    rbuf_no = rbq_q[s][rbq_head[s]];
    alarm_$enable();

    acb = &rbuf_pool[rbuf_no]->acb; /* a copy of acb from xport driver */

    /*** Check for receive errors */
    if (acb->err != 0) {
        recv_start_rbuf(rbuf_no);
        errno = EADDRNOTAVAIL;      /*??? better error? */
        return(-1);
    }

    /*** If not enough room, say would block */
    if (len < acb->len) {
        errno = EWOULDBLOCK;
        return(-1);
    }

    /*** Get the real amount of data ready */
    len = acb->len;

    /*** Copy the data to the callers buffer */
    memcpy(buf, rbuf_pool[rbuf_no]->data, len);

    /*** Return the addr of the sender */
    ASSERT(*fromlen >= sizeof(struct sockaddr_dds));
    SET_ADDR(((struct sockaddr_dds*) from), acb->rnet, acb->radr);
    *fromlen = sizeof(struct sockaddr_dds);

    /*** Start recv up again */
    (void) recv_start_rbuf(rbuf_no);

    return(len);
}

int sendto_nck(s, msg, len, flags, to, tolen)
int s;              /* sending socket fd */
char* msg;          /* data buffer to send */
int len;            /* length of data buffer */
int flags;          /* ignored */
socket_$addr_t* to; /* address of destination */
int tolen;          /* length of to sockaddr */
{
    xport_$tcb_rep rtcb;
    xport_$tcb tcb = &rtcb;
    int retval;
    extern void far dds_$null_handler();

    CHECK_RECV();
    if (s < 0 || MAX_SOCKETS <= s || !sock_pool[s].in_use) {
        errno = EBADF;
        return(-1);
    }
    if (!sock_pool[s].bound) {
        errno = EBADF;
        return(-1);
    }
    if (to->family != socket_$dds) {
        errno = EADDRNOTAVAIL;
        return(-1);
    }

    xport_$zero_tcb(tcb);

    if (((struct sockaddr_dds*) to)->sdds_addr.node == htonl(dds_$broadcast_node))
        tcb->command = XPORT_$SEND_BROADCAST;
    else
        tcb->command = XPORT_$SEND_DATAGRAM;
    tcb->cid = SEND_CID;
    tcb->length = len;
    tcb->baddr = msg;
    tcb->async = dds_$null_handler;

    GET_ADDR(&sock_pool[s].saddr, tcb->lnet, tcb->laddr);
    GET_ADDR(((struct sockaddr_dds*) to), tcb->rnet, tcb->raddr);

#ifdef SOCKET_MAX_DEBUG
    if (socket_$max_debug) {
        printf("sending datagram on socket[%d]:\n", s);
        xport_$print_tcb(tcb, 1, stdout);
    }
#endif

    retval = xport_$command(tcb);

#ifdef SOCKET_DEBUG
    if (0 <= retval && retval <= MAX_XPORT_$ERROR)
        xport_$error_cnt[retval]++;
#endif

    if (retval != 0) {
#ifdef SOCKET_DEBUG
        if (socket_$debug)
            fprintf(stderr, "Error (sendto_nck): cmd failed, retval=0x%x (%s)\n",
                    retval, xport_$error_name(retval));
#endif
        switch (retval) {
            case XPORT_$INTERFACE_BUSY:
            case XPORT_$TOO_MANY_COMMANDS:
                errno = EWOULDBLOCK;
                break;
            case XPORT_$ILLEGAL_BUFFER_LENGTH:
                errno = EMSGSIZE;
                break;
            /*??? xport not installed (256)? */
            default:
                errno = EADDRNOTAVAIL; 
                break;
        }
#ifdef SOCKET_DEBUG
        sock_pool[s].xmit_failure_cnt++;
#endif
        return(-1);
    }

#ifdef SOCKET_DEBUG
    sock_pool[s].xmit_cnt++;
#endif

    /*** Don't wait for completion, command is synchronous */
    return(len);
}

int close_socket(s)
int s;
{
    int port;
    int i;

    CHECK_RECV();
    alarm_$disable();
    if (s < 0 || MAX_SOCKETS <= s || !sock_pool[s].in_use) {
        errno = EBADF;
        alarm_$enable();
        return(-1);
    }
    if (!sock_pool[s].in_use) {
        errno = EBADF;
        alarm_$enable();
        return(-1);
    }
    if (sock_pool[s].bound) {
        /*** Deallocate port */
        port = ntohs(sock_pool[s].saddr.sdds_port);
        ports_in_use[port>>4] &= ~(1 << (port&15));

        /*** Stop all pending recv commands */
        for (i = 0; i < MAX_TOTAL_RBUFS; i++) { 
            if (rbuf_pool[i] != NULL && rbuf_pool[i]->sock_no == s) {
                if (!rbuf_pool[i]->couldnt_restart)
                    (void) recv_cancel_rbuf(i); /* succ value ignored here */
            }
        }

        /*** Free alloc'd rbufs for this socket */
        unalloc_rbufs(s);
    }

    sock_pool[s].in_use = 0;
    alarm_$enable();
    return(0);
}

internal void socket_cleanup()
{
    int i;

    socket_cleanup_set = 0;

    for (i = 0; i < MAX_SOCKETS; i++) {
        if (sock_pool[i].in_use)
            close_socket(i);
    }
}

int set_socket_non_blocking(fd)
int fd;
{
    /* do nothing, sockets here do the right thing */
}

int getsockname_nck(s, saddr, slen)
int s;
socket_$addr_t* saddr;
int* slen;
{
    CHECK_RECV();
    alarm_$disable();
    if (s < 0 || MAX_SOCKETS <= s || !sock_pool[s].in_use) {
        errno = EBADF;
        alarm_$enable();
        return(-1);
    }
    if (!sock_pool[s].in_use || !sock_pool[s].bound) {
        errno = EBADF;
        alarm_$enable();
        return(-1);
    }
    if (*slen < sock_pool[s].slen) {
        errno = ENOBUFS; /*??? not in man page for this function */
        alarm_$enable();
        return(-1);
    }

    ASSERT(sock_pool[s].slen <= sizeof(*saddr));
    memcpy((char*) saddr, (char*) &sock_pool[s].saddr, sock_pool[s].slen);
    *slen = sock_pool[s].slen;
    alarm_$enable();
    return(0);
}

internal int recv_start_rbuf(rbuf_no)
int rbuf_no;
{
    int s;
    recv_buf rbuf;
    xport_$tcb_rep rtcb;
    xport_$tcb tcb = &rtcb;
    int retval;
    extern void far dds_$recv_handler();

    rbuf = rbuf_pool[rbuf_no];

    s = rbuf->sock_no;

    xport_$zero_tcb(tcb); /*??? necc?, time */

    tcb->command = XPORT_$RECEIVE_DATAGRAM; /* either addressed or broadcast */
    tcb->cid = RBUF_NO_TO_CID(rbuf_no);
    tcb->length = MAX_PACKET_SIZE;
    tcb->baddr = rbuf->data;
    tcb->async = dds_$recv_handler;

    GET_ADDR(&sock_pool[s].saddr, tcb->lnet, tcb->laddr);
    /*** Driver ignores remote addr for this command */

    /*** This flag will be set from interrupt */
    rbuf->cmd_completed = 0;
    xport_$zero_acb(&rbuf->acb); /*??? necc?, time */

#ifdef SOCKET_MAX_DEBUG
    if (socket_$max_debug) {
        printf("getting ready to receive on socket[%d]:\n", s);
        xport_$print_tcb(tcb, 1, stdout);
    }
#endif

    retval = xport_$command(tcb);

#ifdef SOCKET_DEBUG
    if (0 <= retval && retval <= MAX_XPORT_$ERROR)
        xport_$error_cnt[retval]++;
#endif

    if (retval != 0) {
        if (!rbuf->couldnt_restart) {    /* if first failure */
            try_restart++;
            rbuf->couldnt_restart = 1;
        }
#ifdef SOCKET_DEBUG
        if (socket_$debug)
            fprintf(stderr, "Error (recv_start_rbuf): failed, retval=0x%x (%s)\n",
                    retval, xport_$error_name(retval));
        rbuf->recv_failure_cnt++;
#endif
        return(0);
    } else {
        if (rbuf->couldnt_restart) {    /* if was trying a restart */
            try_restart--;
            rbuf->couldnt_restart = 0;
        }
#ifdef SOCKET_DEBUG
        rbuf->recv_started_cnt++;
#endif
    }
    return(1);      
}

internal int recv_cancel_rbuf(rbuf_no)
int rbuf_no;
{
    recv_buf rbuf = rbuf_pool[rbuf_no];
    xport_$tcb_rep rtcb;
    xport_$tcb tcb = &rtcb;
    int retval;
    extern void far dds_$null_handler();

    /*** No need to cancel if already completed */
    if (rbuf->cmd_completed)
        return(1);

    xport_$zero_tcb(tcb);  /*??? need this? */

    tcb->command = XPORT_$CANCEL;
    tcb->cid = CANCEL_CID;
    tcb->vcid = RBUF_NO_TO_CID(rbuf_no);/* which cid to cancel */
    tcb->async = dds_$null_handler;   /* the do nothing handler */

#ifdef SOCKET_MAX_DEBUG
    if (socket_$max_debug) {
        printf("cancelling receive on socket[%d] rbuf[%d]:\n", 
                rbuf->sock_no, rbuf_no);
        xport_$print_tcb(tcb, 1, stdout);
    }
#endif

    retval = xport_$command(tcb);

#ifdef SOCKET_DEBUG
    if (0 <= retval && retval <= MAX_XPORT_$ERROR)
        xport_$error_cnt[retval]++;
#endif

    if (retval != 0) {
        /*** Got BIG problems if not canceled and not completed */
        if (!rbuf->cmd_completed) {
#ifdef SOCKET_DEBUG
            fprintf(stderr, "Error (recv_cancel_rbuf): cmd failed, retval=0x%x (%s)\n",
                    retval, xport_$error_name(retval));
#else
            fprintf(stderr, "Error (recv_cancel_rbuf): cmd failed, retval=0x%x\n",
                    retval);
#endif
        }
    }

    /*** Do not wait for async notification (its synchronous anyways). */
    return(retval == 0);
}   

internal int alloc_rbufs(s)
int s;
{
    int i, r;
    recv_buf rbuf;

    for (i = 0; i < sock_pool[s].n_rbufs; i++) {
        alarm_$disable();
        for (r = 0; r < MAX_TOTAL_RBUFS; r++) {
            if (rbuf_pool[r] == NULL) {
                rbuf = (recv_buf) rpc_$malloc(sizeof(rep_recv_buf));
                if (rbuf == NULL) {
                    alarm_$enable();
                    goto failed;
                }
                rbuf->sock_no = s;
                rbuf->cmd_completed = 0;
                rbuf->couldnt_restart = 0;
                xport_$zero_acb(&rbuf->acb);
#ifdef SOCKET_DEBUG
                rbuf->recv_cnt = 0;
                rbuf->recv_started_cnt = 0;
                rbuf->recv_failure_cnt = 0;
#endif
                rbuf_pool[r] = rbuf;
                break;
            }
        }
        alarm_$enable();
        if (r == MAX_TOTAL_RBUFS)
            goto failed;
    }
    return(1);  /* return success */

failed:
    /*** Free up any allocated rbufs before ran out */
    unalloc_rbufs(s);
    return(0);  /* return failure */
}

internal unalloc_rbufs(s)
int s;
{
    int r;
    recv_buf rbuf;

    for (r = 0; r < MAX_TOTAL_RBUFS; r++) {
        rbuf = rbuf_pool[r];
        if (rbuf != NULL && rbuf->sock_no == s) {
            alarm_$disable();
            rbuf_pool[r] = NULL;
            alarm_$enable();
            rpc_$free((char*) rbuf);
        }
    }
}

/*
 * Try to restart a failed recv cmd.
 */
internal restart_recv()
{
    int r;

    for (r = 0; r < MAX_TOTAL_RBUFS; r++) {
        alarm_$disable();
        if (rbuf_pool[r] != NULL && rbuf_pool[r]->couldnt_restart) {
            if (!recv_start_rbuf(r)) {
#ifdef SOCKET_DEBUG
                if (socket_$debug)
                    fprintf(stderr, "(restart_recv): failed to restart rbuf%d\n", r);
#endif
            }
        }
        alarm_$enable();
    }
}

#ifdef SOCKET_DEBUG

socket_$dump(s, level, fp)
register int s;
int level;
FILE* fp;
{
    register int i;

    if (s < 0 || MAX_SOCKETS <= s) {
        fprintf(fp, "socket_$dump: socket %d out of bounds\n", s);
        return;
    }
    util_$tab(level, fp);
    fprintf(fp, "in_use: %d\n", sock_pool[s].in_use);
    util_$tab(level, fp);
    fprintf(fp, "bound: %d\n", sock_pool[s].bound);
    if (sock_pool[s].bound) {
        u_long nlen;
        char name_buf[30];
        u_long rport;
        status_$t st;

        nlen = sizeof(name_buf);
        socket_$to_numeric_name((socket_$addr_t*) &sock_pool[s].saddr, 
                (u_long) sizeof(socket_$addr_t),
                name_buf, &nlen, &rport, &st);
        util_$tab(level, fp);
        if (st.all != 0)
            fprintf(fp, "socket_$dump: can't convert remote address to string\n");
        else
            fprintf(fp, "saddr:%s[%lu]\n", name_buf, rport);
    }
    util_$tab(level, fp);
    fprintf(fp, "xmit_cnt: %ld\n", sock_pool[s].xmit_cnt);
    util_$tab(level, fp);
    fprintf(fp, "xmit_failure_cnt: %ld\n", sock_pool[s].xmit_failure_cnt);
    util_$tab(level, fp);
    fprintf(fp, "n_rbufs: %d\n", sock_pool[s].n_rbufs);
    if (sock_pool[s].in_use && sock_pool[s].bound) {
        util_$tab(level+1, fp);
        fprintf(fp, "rqb_head[%d]: %d\n", s, rbq_head[s]);
        util_$tab(level+1, fp);
        fprintf(fp, "rqb_tail[%d]: %d\n", s, rbq_tail[s]);
        util_$tab(level+1, fp);
        fprintf(fp, "rqb_q[%d]: ", s);
        for (i = 0; i < MAX_RBUFS_PER_SOCKET; i++)
            fprintf(fp, "%2d ", rbq_q[s][i]);
        fprintf(fp, "\n");
        util_$tab(level+1, fp);
        fprintf(fp, "rbufs:\n");
        socket_$dump_rbufs(s, level+2, fp);
    }
}

socket_$dump_all(level, fp)
int level;
FILE* fp;
{
    register int i;

    util_$tab(level, fp);
    fprintf(fp, "ports_in_use: ");
    for (i = 0; i < sizeof(ports_in_use)/sizeof(u_short)/8; i++)
        printf("0x%04x ", ports_in_use[i]);
    fprintf(fp, "(max=%d)\n", MAX_PORTS);
    for (i = 0; i < MAX_SOCKETS; i++) {
        if (sock_pool[i].in_use) {
            util_$tab(level, fp);
            fprintf(fp, "socket[%d]:\n", i);
            socket_$dump(i, level+1, fp);
        }
    }       

    util_$tab(level, fp);
    fprintf(fp, "XPORT error counts:\n");
    for (i = 0; i <= MAX_XPORT_$ERROR; i++) {
        if (strcmp(xport_$error_name(i), "??unknown??")==0 && xport_$error_cnt[i]==0)
            continue;
        util_$tab(level+1, fp);
        fprintf(fp, "%2d: %5ld %s\n", i, xport_$error_cnt[i], xport_$error_name(i));
    }
}

internal socket_$dump_rbufs(s, level, fp)
register int s;
int level;
FILE* fp;
{
    register int r;
    recv_buf rbuf;

    for (r = 0; r < MAX_TOTAL_RBUFS; r++) {
        rbuf = rbuf_pool[r];
        if (rbuf != NULL && rbuf->sock_no == s) {
            util_$tab(level, fp);
            fprintf(fp, "rbuf[%d]:\n", r);
            util_$tab(level+1, fp);            
            fprintf(fp, "recv_cnt: %ld\n", rbuf->recv_cnt);
            util_$tab(level+1, fp);            
            fprintf(fp, "recv_started_cnt: %ld\n", rbuf->recv_started_cnt);
            util_$tab(level+1, fp);            
            fprintf(fp, "recv_failure_cnt: %ld\n", rbuf->recv_failure_cnt);
            util_$tab(level+1, fp);            
            fprintf(fp, "cmd_completed: %d\n", rbuf->cmd_completed);
            if (rbuf->cmd_completed) {
                util_$tab(level+1, fp);
                fprintf(fp, "acb:\n");
                xport_$print_acb(&rbuf->acb, level+2, fp);
            }
        }
    }       
    
}

internal socket_$dump_all_rbufs(level, fp)
int level;
FILE* fp;
{
    int got_one;
    int r;
    recv_buf rbuf;

    got_one = 0;
    for (r = 0; r < MAX_TOTAL_RBUFS; r++) {
        
        if ((rbuf = rbuf_pool[r]) != NULL) {
            got_one = 1;
            util_$tab(level, fp);
            fprintf(fp, "rbuf[%d]:\n", r);
            util_$tab(level+1, fp);
            fprintf(fp, "sock_no: %d\n", rbuf->sock_no);
            util_$tab(level+1, fp);
            if (!rbuf->cmd_completed) {
                fprintf(fp, "cmd not completed\n");
            } else {
                fprintf(fp, "acb:\n");
                xport_$print_acb(&rbuf->acb, level+2, fp);
            }
        }
    }       
    if (!got_one) {
        util_$tab(level, fp);
        fprintf(fp, "rbuf_pool is empty\n");
    }   
}

#endif

char dds_$anr_stack[256];             /* stack grows down in memory */

/*** Disable stack probe, called from interrupt, STKHQQ not adjusted */
#pragma check_stack(off)

/*
 * Called from the anr interrupt.
 * Assume interrupts are disabled.
 */
dds_$anr_handler(acb)  
xport_$acb_rep far* acb;
{
    register int s;
    recv_buf rbuf;

    rbuf = rbuf_pool[CID_TO_RBUF_NO(acb->cid)];

    /*** Set the completed flag */
    rbuf->cmd_completed = 1;

    /*** Make a copy of the acb in the rbuf */
    xport_$anr_copy((xport_$acb_rep far*) &(rbuf->acb), acb, sizeof(xport_$acb_rep));

#ifdef SOCKET_DEBUG
    rbuf->recv_cnt++;
#endif

    /*** Put the rbuf_no in the queue */
    s = rbuf->sock_no;
    rbq_tail[s] = (rbq_tail[s] + 1) & (MAX_RBUFS_PER_SOCKET - 1);
    rbq_q[s][rbq_tail[s]] = CID_TO_RBUF_NO(acb->cid);
}

#pragma check_stack()  /* stack probe reverts to command line selection */

int gethostname(name, len)
char* name;
int len;
{
    socket_$net_addr_t my_naddr;
    u_long nlen;
    status_$t st;
    socket_$addr_t saddr;
    u_long slen;
    u_long namelen;
    u_long port;

    socket_$inq_my_netaddr((u_long) socket_$dds, &my_naddr, &nlen, &st);
    if (st.all)
        return(-1);  /*??? set errno */
    socket_$set_netaddr(&saddr, &slen, &my_naddr, nlen, &st);
    if (st.all)
        return(-1);  /*??? set errno */
    namelen = (u_long) len;
    socket_$to_name(&saddr, slen, name, &namelen, &port, &st);
    return(st.all ? -1 : 0);  /*??? set errno */
}
