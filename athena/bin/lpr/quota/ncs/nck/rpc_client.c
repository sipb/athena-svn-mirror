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
 * R P C _ C L I E N T
 *
 * This module implements the client side of the NCA/RPC protocol.
 */

#include "rpc_p.h"

    /*
     * RPC client handle type.  The "hhdr" field must remain first.
     * See comments on "handle_hdr_t" definition in "rpc_p.h".
     */

typedef struct {
    handle_hdr_t hhdr;          /* common server/client handle header */
    rpc_$pkt_hdr_t HDR;         /* bound header */
    rpc_$sockaddr_t addr;       /* address of server */
    u_short flags;              /* flags; see below */
    u_short refcnt;             /* reference count */
    uuid_$t actuid;             /* last activity to use this handle */
    u_short ahint;              /* ahint returned by server for that activity */
    rpc_$client_auth_t *auth;   /* authentication info */
    rpc_$encr_t *encr;          /* encryption state */
} NEAR client_handle_t;

#define HF_HOST_BOUND       1   /* handle's sockaddr has a host */
#define HF_SERVER_BOUND     2   /* handle's sockaddr has a real port */
#define HF_HAVE_FAMILY      4   /* handle's sockaddr has a valid proto family */
#define HF_SHORT_TIMEOUT    8   /* calls made with this handle should timeout more
                                   quickly if we never get any response from server */

#ifdef MSDOS
#  define HANDLE_CAST(h) ((client_handle_t *) ((long) (h)))
#else
#  define HANDLE_CAST(h) ((client_handle_t *) (h))
#endif

/*
 * Private RPC client data.
 *
 * On Apollos, this data with NOT be in the DATA$ section and hence will
 * be per-process when this module is used in a global library.
 */

internal boolean NEAR initialized_level;     /* level init done for current program level? */
internal boolean NEAR initialized_acker;     /* acker (task) initialized? */
#ifndef TASKING
internal boolean NEAR synchronous_acks = true;
                                        /* T => send acks before exiting "rpc_$sar" */
#endif

    /*
     * Pool of available sockets
     */

typedef struct {
    boolean inuse;                  /* this slot in pool inuse? */
    int fd;                         /* file descriptor of socket */
    u_long family;                  /* address family this socket is in */
#ifdef UNIX
    int pid;                        /* PID of process that opened */
#endif
} NEAR sock_t;

#define SOCKET_POOL_SIZE 64

internal sock_t sock_pool[SOCKET_POOL_SIZE];

    /*
     * Acknowledgement database.  Keeps track of what we must ack.
     */

typedef struct {
    uuid_$t             actuid;
    rpc_$sockaddr_t     addr;
    u_long              seq;
    u_short             ahint;
    u_long              server_boot;
} NEAR ack_db_elt_t;

#define ACK_DB_SIZE     32

internal ack_db_elt_t ack_db[ACK_DB_SIZE];

    /*
     * Call database.  Keeps track of what activities are making what calls.
     */

typedef struct {
    boolean             inuse;
    uuid_$t             actuid;
    u_long              seq;
    client_handle_t     *handle;
} NEAR call_info_t;

#define MAX_ACTIVITIES  32

internal call_info_t call_info[MAX_ACTIVITIES];

extern u_long rpc_$seq;         /* next sequence number to use */
internal u_short NEAR nsockets; /* number of sockets in socket pool */

#ifdef GLOBAL_LIBRARY

typedef struct level_info_t {
    u_short nsockets;
    boolean initialized_level;
    boolean initialized_acker;
} level_info_t;

#define MAX_LEVELS 20

internal level_info_t level_info[MAX_LEVELS];

#endif

#ifdef MAX_DEBUG
internal char NEAR sastring_buff[100];
#endif


    /* Maximum number of broadcast addresses we can handle  */

#ifdef MSDOS
#  define MAX_BRD_ADDRS   4   /* Keep interrupt stack size down */
#else
#  define MAX_BRD_ADDRS   32
#endif

#ifdef GLOBAL_LIBRARY
#include "set_sect.pvt.c"
#endif


typedef enum {
    await_data_available,
    await_timeout,
    await_select_failed,
    await_unknown_error
} await_value_t;

internal sock_t *alloc_socket
    PROTOTYPE((u_long family, status_$t *st));
internal void free_socket
    PROTOTYPE((sock_t *sock));
internal void update_ack_db
    PROTOTYPE((rpc_$pkt_t *pkt, rpc_$sockaddr_t *from));
internal boolean send_pkt
    PROTOTYPE((sock_t *sock, rpc_$pkt_t *pkt, rpc_$sockaddr_t *addr, boolean broadcast, rpc_$encr_t *encr));
internal boolean c_recv_pkt
    PROTOTYPE((sock_t *sock, client_handle_t *p, rpc_$pkt_t *pkt, u_long pmax, u_long seq, uuid_$t *actuid, rpc_$sockaddr_t *from, rpc_$cksum_t *cksum));
internal call_info_t *register_call
    PROTOTYPE((uuid_$t *actuid, client_handle_t *p, u_long seq));
internal call_info_t *find_call
    PROTOTYPE((uuid_$t *actuid));
internal void init_hdr
    PROTOTYPE((rpc_$pkt_hdr_t *pkt_hdr, uuid_$t *obj));
internal void ack_replies
    PROTOTYPE((void));
internal void init
    PROTOTYPE((boolean start_acker));
internal await_value_t await_reply
    PROTOTYPE((sock_t *sock, u_long timeout, boolean *locked));
internal void quit_server
    PROTOTYPE((sock_t *sock, client_handle_t *p, rpc_$pkt_t *pkt, rpc_$sockaddr_t *addr));


/*
 * S A S T R I N G
 *
 * Return a pointer to a printed sockaddr.
 */

#ifdef MAX_DEBUG

internal char *sastring(saddr, slen)
socket_$addr_t *saddr;
u_long slen;
{
    if (! rpc_$debug)
        return ("NA");
    else {
        status_$t st;
        socket_$string_t n;
        u_long port;
        u_long nl = sizeof n;

        socket_$to_numeric_name(saddr, slen, n, &nl, &port, &st);
        if (st.all != status_$ok)
            return ("ERROR");
        else {
            sprintf(sastring_buff, "%*.s[%lu]", (int) nl, n, port);
            return (sastring_buff);
        }
    }
}

#else

#define sastring(junk1, junk2) ""

#endif


/*
 * A L L O C _ S O C K E T
 *
 * Allocate a socket of the specified address family.  If there's an already
 * open socket of the right kind in our pool of open socket, return it.
 * Otherwise, open a new socket and return it.
 */

internal sock_t *alloc_socket(family, st)
u_long family;
status_$t *st;
{
    register u_short i;
    int sock;
    socket_$addr_t addr;
    register sock_t *sp;
#ifdef UNIX
    int mypid = getpid();
    int ppid = getppid();
#endif

    st->all = status_$ok;

    /*
     * Look for a socket that: (a) is not in use, (b) is from the right family,
     * and [for Unix only] (c) was opened by this process.
     */

    for (i = 0; i < nsockets; i++) {
        sp = &sock_pool[i];
        if ((! sp->inuse)
            && sp->family == family
#ifdef UNIX
            && sp->pid == mypid
#endif
            )
        {
            sp->inuse = true;
            return (sp);
        }
    }

    /*
     * We get here if there wasn't a socket in the pool.
     */

    if (nsockets >= SOCKET_POOL_SIZE)
        DIE("Ran out of sockets");

    /*
     * Make a socket of the right kind and enter it in the pool.
     */

    sock = socket((int) family, SOCK_DGRAM, 0);
    if (sock < 0) {
        dprintf("(alloc_socket) Can't create socket, errno=%d\n", errno);
        st->all = rpc_$cant_create_sock;
        return (NULL);
    }

    bzero((char *) &addr, sizeof addr);
    addr.family = (socket_$addr_family_t) family;
    if (bind(sock, &addr, sizeof(struct sockaddr)) < 0) {
        dprintf("(alloc_socket) Can't bind socket, errno=%d\n", errno);
        close_socket(sock);
        st->all = rpc_$cant_bind_sock;
        return (NULL);
    }

#if defined(apollo) && defined(GLOBAL_LIBRARY)
    /*
     * Switch up and "protect" the file descriptor we just got to
     * lessen the change of it getting clobbered by some slobby programs
     * that like to "for i := 0 to NFILE-1 do close(i)".
     */

    sock = ios_$switch((ios_$id_t) sock, ios_$max, st);
    ios_$set_switch_flag((ios_$id_t) sock, ios_$protected, true, st);
#endif

    set_socket_non_blocking(sock);

#ifdef UNIX
    fcntl(sock, F_SETFD, 1);            /* Set "close on exec" */
#endif

    /*
     * Find a slot in the socket pool to use.
     */

#ifndef BSD
    sp = &sock_pool[nsockets++];
#else
    /*
     * Cope with the case in which a vfork'd child has allocated a socket.
     * The problem is that between vfork and exec, "sock_pool" is shared
     * between parent and child.  However, a socket opened by the child
     * is not open in the parent.  Hence "sock_pool" and the set of open
     * sockets for the parent process could be inconsistent.  Now that
     * we're allocating a slot in the pool, instead of simply extending
     * the pool (i.e. incrementing "nsockets"), we'll look for any "dead"
     * entries -- i.e. ones not owned by the current process or its
     * parent.  (The parent check is in case the current process is a
     * vfork child.)  If we find a dead entry, we'll allocate it to
     * the new socket.
     */

    for (i = 0; i < nsockets; i++)
        if (sock_pool[i].pid != mypid && sock_pool[i].pid != ppid)
            break;

    if (i < nsockets)
        sp = &sock_pool[i];
    else
        sp = &sock_pool[nsockets++];
#endif

    sp->inuse  = true;
    sp->fd     = sock;
    sp->family = (socket_$addr_family_t) family;

#ifdef UNIX
    sp->pid    = mypid;
#endif

    return (sp);
}

/*
 * F R E E _ S O C K E T
 *
 * Return a socket to the pool of open sockets.
 *
 * Note that we explicitly DON'T do certain checking -- like to make sure
 * we find the socket we're looking for or that the socket wasn't already
 * free -- because of bad interaction at program level exit:  The RPC
 * mark/release handler may be called before some tasks get a chance to
 * clean up (i.e. call this procedure).  Not checking is a sleazy way out,
 * but I think it'll work.  No one should be calling us with a bogus socket
 * anyway, right?
 */

internal void free_socket(sock)
sock_t *sock;
{
    sock->inuse = false;
}


/*
 * U P D A T E _ A C K _ D B
 *
 * Update the ack database based on a new sequence number.
 */

internal void update_ack_db(pkt, from)
rpc_$pkt_t *pkt;
rpc_$sockaddr_t *from;
{
    register u_short i;
    register short nil_i = -1;

    /*
     * Scan through the ack database to see if the specified activity already
     * has an ack entry.  If one is found, update it with the new seq #.
     */

    for (i = 0; i < ACK_DB_SIZE; i++)
        if (nil_i != -1 && uidequal(ack_db[i].actuid, uuid_$nil))
            nil_i = i;
        else if (uidequal(ack_db[i].actuid, pkt->hdr.actuid) && pkt->hdr.seq > ack_db[i].seq) {
            ack_db[i].seq = pkt->hdr.seq;
            return;
        }

    /*
     * We get here if no entry was found.  If we discovered an empty entry
     * fill it in with the specified activity.  Note that not being able
     * to insert an entry is not fatal since all that will happen is that
     * the server will retransmit the response.
     */

    if (nil_i != -1) {
        ack_db[nil_i].actuid      = pkt->hdr.actuid;
        ack_db[nil_i].seq         = pkt->hdr.seq;
        ack_db[nil_i].ahint       = pkt->hdr.ahint;
        ack_db[nil_i].server_boot = pkt->hdr.server_boot;
        copy_sockaddr_r(from, &ack_db[nil_i].addr);
    }
}


/*
 * S E N D _ P K T
 *
 * Fill in the packet header from the specified handle, and send the packet
 * over the specified socket to the specified place.
 */

internal boolean send_pkt(sock, pkt, addr, broadcast, encr)
register sock_t *sock;
rpc_$pkt_t *pkt;
register rpc_$sockaddr_t *addr;
boolean broadcast;
rpc_$encr_t *encr;
{
    boolean b;
    socket_$addr_t brd_addrs[MAX_BRD_ADDRS];
    u_long brd_addrs_len[MAX_BRD_ADDRS];
    u_long num_brd_addrs = MAX_BRD_ADDRS;
    u_short i;
    status_$t st;
#ifdef SO_BROADCAST
    int setsock_val = 1;
    int r;
#endif

    if (! broadcast)
        b = rpc_$sendto(sock->fd, pkt, addr, encr);
    else {
        socket_$inq_broad_addrs((u_long) addr->sa.family,
                                socket_$inq_port(&addr->sa, addr->len, &st),
                                brd_addrs, brd_addrs_len, &num_brd_addrs, &st);
        if (st.all != status_$ok) {
            dprintf("(send_pkt) cannot enable broadcast, status=%08lx\n", st.all);
            return (false);
        }
#ifdef SO_BROADCAST
        errno = 0;
        r = setsockopt(sock->fd, SOL_SOCKET, SO_BROADCAST, &setsock_val, sizeof(setsock_val));
        if (r < 0) {
            dprintf("(send_pkt) cannot enable broadcast, errno=%d\n", errno);
            return (false);
        }
#endif
        b = false;
        for (i = 0; i < num_brd_addrs; i++) {
            rpc_$sockaddr_t baddr;
            copy_sockaddr(&brd_addrs[i], (int) brd_addrs_len[i], &baddr.sa, &baddr.len);
            b |= rpc_$sendto(sock->fd, pkt, &baddr, encr);
        }
    }

    return (b);
}


/*
 * C _ R E C V _ P K T
 *
 * Read a reply message from a socket.  Verify that it looks basically
 * kosher; i.e. that it's to the right place, of the right length and
 * to the right call.  Note that if we get any kind of request or ping packet,
 * we call the server-listener dispatch routine on the assumption that
 * the packet is a callback to us.
 */

internal boolean c_recv_pkt(sock, p, pkt, pmax, seq, actuid, from, cksum)
sock_t *sock;
client_handle_t *p;
register rpc_$pkt_t *pkt;
u_long pmax;
u_long seq;
uuid_$t *actuid;
rpc_$sockaddr_t *from;
rpc_$cksum_t *cksum;
{
    status_$t st;
    int recv_len;
    boolean for_this_activity;
    rpc_$sockaddr_t tfrom;
#if defined(apollo) && defined(GLOBAL_LIBRARY) && ! defined(pre_sr10)
#   define rs_auth_$if_spec rs_auth_$if_spec_pure_data$
    extern rpc_$if_spec_t rs_auth_$if_spec;
#endif

    recv_len = rpc_$recvfrom(sock->fd, pkt, pmax, &tfrom, cksum);

    if (recv_len <= 0) {
        dprintf("(c_recv_pkt) len <= 0\n");
        return (false);
    }

    for_this_activity = uidequal(pkt->hdr.actuid, *actuid);

    /*
     * If we get a request or ping packet, it might be a callback -- call
     * the server dispatch routine with the packet.  For the packet to
     * be considered a callback the activity UID in the packet must be
     * our activity UID.
     *
     * For backward compatibility to a time when callbacks didn't have
     * the original caller's activity UID, if the call is on the "conv_"
     * or "rs_auth_" interface, we accept it too.
     *
     * When handling callbacks we take note of the sequence number in the
     * callback.  The sequence number in the callback is derived from the
     * sequence number in the original call.  In subsequent calls by the
     * client, we need to use yet larger sequence numbers to maintain
     * the invariant that no two distinct calls ever have the same
     * [activity UID, sequence #] pair.
     */

    if ((pkt_type(pkt) == rpc_$request || pkt_type(pkt) == rpc_$ping)
        && (for_this_activity
            || uidequal(pkt->hdr.if_id, conv_$if_spec.id)
#if defined(apollo) && defined(GLOBAL_LIBRARY) && ! defined(pre_sr10)
            || uidequal(pkt->hdr.if_id, rs_auth_$if_spec.id)
#endif
        )
        )
    {
        UNLOCK(CLIENT_MUTEX);
        if (pkt->hdr.seq > rpc_$seq)
            rpc_$seq = pkt->hdr.seq + 1;
        dprintf("(c_recv_pkt) Rcvd callback (ptype=%s)\n", rpc_$pkt_name(pkt_type(pkt)));
        rpc_$listen_dispatch((u_long) sock->fd, (rpc_$ppkt_p_t) pkt, *cksum, &tfrom.sa, tfrom.len, &st);
        LOCK(CLIENT_MUTEX);
        return (false);
    }

    /*
     * Is this packet for my activity?
     */

    if (! for_this_activity) {
        dprintf("(c_recv_pkt) Rcvd packet not for my activity\n");
        return (false);
    }

    /*
     * Does this packet have the right sequence number?
     */

    if (pkt->hdr.seq != seq) {
        if (pkt->hdr.seq < seq)
            /* Should we do an ack here or simply ignore the old pkt??? */
            dprintf("(c_recv_pkt) old ack\n");
        else
            dprintf("(c_recv_pkt) Rcvd packet with bad seq (should be %lu, is %lu; ptype=%s)\n",
                    seq, pkt->hdr.seq, rpc_$pkt_name(pkt_type(pkt)));
        return (false);
    }

    /*
     * If we have the server boot time, then insure that the incoming
     * packet's server boot time field matches what we have.
     */

    if (p->hdr.server_boot != 0 && p->hdr.server_boot != pkt->hdr.server_boot)
        raisec(nca_status_$wrong_boot_time);

    copy_sockaddr_r(&tfrom, from);

    return (true);
}


/*
 * R E G I S T E R _ C A L L
 *
 * Register a call for an activity.  We save away the (1) sequence number and
 * (2) binding handle the activity is using.  This way, if we get called back,
 * (see "conv_$who_are_you") we can (1) pass back the activity's sequence number
 * and (2) stick the server's boot time ("in" argument to call back) in the
 * binding handle for later use.
 */

#define unregister_call(p) \
    (p)->inuse = false;

internal call_info_t *register_call(actuid, p, seq)
uuid_$t *actuid;
register client_handle_t *p;
u_long seq;
{
    register u_short i;

    for (i = 0; i < MAX_ACTIVITIES; i++)
        if (! call_info[i].inuse) {
            call_info[i].inuse  = true;
            call_info[i].actuid = *actuid;
            call_info[i].seq    = seq;
            call_info[i].handle = p;

            return(&call_info[i]);
        }

    DIE("Can't register call");
}


/*
 * F I N D _ C A L L
 *
 * Find the call being made by the specified activity.  If no call in progress,
 * return nil; otherwise, return the pointer to the call-info record for the
 * call.
 */

internal call_info_t *find_call(actuid)
uuid_$t *actuid;
{
    register u_short i;

    for (i = 0; i < MAX_ACTIVITIES; i++)
        if (call_info[i].inuse && uidequal(call_info[i].actuid, *actuid))
            return (&call_info[i]);

    return (NULL);
}


/*
 * C O N V _ $ W H O _ A R E _ Y O U
 *
 * This procedure is called remotely by a server that has no "connection"
 * information about a client from which it has just received a call.  This
 * call executes in the context of that client (i.e. it is a "call back").
 * We return the current sequence number of the client and his "identity".
 * We accept the boot time from the server for later use in calls to the
 * same server.
 */

void conv_$who_are_you(h, actuid, boot_time, seq, st)
handle_t h;       /* Not really */
uuid_$t *actuid;
u_long boot_time;
u_long *seq;
status_$t *st;
{
    register call_info_t *ci;

    st->all = status_$ok;

    ci = find_call(actuid);

    if (ci == NULL) {
        st->all = rpc_$not_in_call;
        return;
    }

    /*
     * Either our binding handle's server boot time must be null, or what it
     * contains better be what the server is passing to us now.
     */

    if (ci->handle->hdr.server_boot == 0)
        ci->handle->hdr.server_boot = boot_time;
    else if (ci->handle->hdr.server_boot != boot_time) {
        st->all = nca_status_$you_crashed;
        return;
    }

    *seq = ci->seq;
}


/*
 * R P C _ $ G E T _ H A N D L E
 *
 * Get handle based on activity uuid.  This is provided to allow implementers
 * of authentication packages to replace the "conv_$" callback with their
 * own callback.
 */

handle_t rpc_$get_handle(actuid, st)
uuid_$t *actuid;
status_$t *st;
{
    register call_info_t *ci;
    handle_t h;

    ci = find_call(actuid);
    if (ci == NULL) {
        st->all = rpc_$not_in_call;
        h = NULL;
    }
    else {
        st->all = status_$ok;
        h = (handle_t) ci->handle;
    }

#ifdef apollo
#if ! _ISP__A88K
    set_a0_hack(h);
#endif
#endif

    return h;
}

/*
 * I N I T _ H D R
 *
 * Initialize the packet header in an RPC handle.
 */

internal void init_hdr(pkt_hdr, obj)
register rpc_$pkt_hdr_t *pkt_hdr;
uuid_$t *obj;
{
    bzero(pkt_hdr, sizeof(*pkt_hdr));

    copy_drep(pkt_hdr->HDR.drep, rpc_$local_drep_packed);

    pkt_hdr->HDR.rpc_vers = RPC_VERS;
    pkt_hdr->HDR.ihint    = NO_HINT;
    pkt_hdr->HDR.ahint    = NO_HINT;

    if (obj != NULL)
        pkt_hdr->HDR.object = *obj;
}


/*
 * A C K _ R E P L I E S
 *
 * Ack all unacked replies.
 */

#define ACK_REPLIES_INTERVAL      ((u_long) 3)

internal void ack_replies()
{
    register u_short i;
    rpc_$spkt_t pkt;
    sock_t *sock;
    status_$t st;
    uuid_$string_t buff;

    LOCK(CLIENT_MUTEX);

    for (i = 0; i < ACK_DB_SIZE; i++) {
        register ack_db_elt_t *ae = &ack_db[i];

        if (uidequal(ae->actuid, uuid_$nil))
            continue;

        uuid_$encode(&ae->actuid, buff);
        dprintf("(ack_replies) Acking [%s, %lu]\n", buff, ae->seq);

        sock = alloc_socket((u_long) ae->addr.sa.family, &st);
        if (st.all != status_$ok)
            break;

        init_hdr(&pkt.HDR, NULL);

        set_pkt_type(&pkt, rpc_$ack);
        pkt.hdr.server_boot = ae->server_boot;
        pkt.hdr.actuid      = ae->actuid;
        pkt.hdr.ahint       = ae->ahint;
        pkt.hdr.seq         = ae->seq;

        if (! send_pkt(sock, (rpc_$pkt_t *) &pkt, &ae->addr, false, (rpc_$encr_t *) 0))
            dprintf("(ack_replies) Can't send ACK, errno=%d\n", errno);

        free_socket(sock);

        ae->actuid = uuid_$nil;
    }

    UNLOCK(CLIENT_MUTEX);
}


/*
 * I N I T
 *
 * Initialize stuff for this module.  The "start_acker" parameter says whether
 * to consider starting an acker task.  We try to avoid doing this out of
 * paranoia:  we'd really rather not start up the tasking machinery if we
 * can avoid it.
 */

internal void init(start_acker)
boolean start_acker;
{
    status_$t st;

#ifdef GLOBAL_LIBRARY
    rpc_$global_lib_init();
#endif

    if (! initialized_level) {
        initialized_level = true;
        rpc_$register(&conv_$if_spec, conv_$server_epv, &st);
    }

    if (start_acker && ! initialized_acker) {
        initialized_acker = true;
        rpc_$periodically(ack_replies, "RPC reply acknowledger", ACK_REPLIES_INTERVAL);
    }
}


/*
 * R P C _ $ A L L O C _ H A N D L E
 *
 * Allocate and initialize a handle for an object.  The first call made
 * using this handle will be broadcast unless "rpc_$set_binding" is called
 * first.   The family argument specifies an initial address family to
 * use.  It can be overwritten by "rpc_$set_binding".  You can specify
 * "socket_$unspec" (AF_UNSPEC or "0") for the family (although the resulting
 * handle will not be usable until "rpc_$set_binding" is called).
 */

handle_t rpc_$alloc_handle(obj, family, st)
uuid_$t *obj;
u_long family;
status_$t *st;
{
    register client_handle_t *p;

    st->all = status_$ok;
#ifdef GLOBAL_LIBRARY
    rpc_$global_lib_init();
#endif

    p = (client_handle_t *) rpc_$nmalloc(sizeof(client_handle_t));

    p->hhdr.data_offset = sizeof(rpc_$pkt_hdr_t);
    p->hhdr.cookie      = CLIENT_COOKIE;

    p->refcnt         = 1;
    p->addr.sa.family = (socket_$addr_family_t) family;
    p->addr.len       = sizeof p->addr.sa;
    p->flags          = 0;
    p->auth           = NULL;
    p->encr           = NULL;

    if (family != socket_$unspec)
        p->flags |= HF_HAVE_FAMILY;

    init_hdr(&p->HDR, obj);

    rpc_$clear_binding((handle_t) p, st);

#ifdef apollo
#if ! _ISP__A88K
    set_a0_hack(p);
#endif
#endif

    return ((handle_t) p);
}


/*
 * R P C _ $ S E T _ B I N D I N G
 *
 * Set (or reset) the sockaddr contained in a handle.
 *
 * What port is used:  the port in the supplied sockaddr or the well-known
 * port (if there is one)?  If the sockaddr's port is defined (i.e. it
 * is not "socket_$unspec_port") that port is used.  If the sockaddr's
 * port is NOT defined, then if the interface has a well-known port, that
 * port is used.  If neither the sockaddr nor the interface specify a port,
 * then forwarding agent's port is used.
 */

void rpc_$set_binding(h, saddr, slen, st)
handle_t h;
socket_$addr_t *saddr;
u_long slen;
status_$t *st;
{
    register client_handle_t *p = HANDLE_CAST(h);

    rpc_$clear_binding(h, st);

    copy_sockaddr(saddr, (int) slen, &p->addr.sa, &p->addr.len);

    p->flags |= HF_HOST_BOUND | HF_HAVE_FAMILY;
    if (socket_$inq_port(&p->addr.sa, p->addr.len, st) != socket_$unspec_port)
        p->flags |= HF_SERVER_BOUND;

    st->all = status_$ok;
}


/*
 * R P C _ $ I N Q _ B I N D I N G
 *
 * For client handles:  return the sockaddr contained in the handle.  For server
 * handles:  call the server-side runtime to get the sockaddr that identifies the
 * caller.
 */

void rpc_$inq_binding(h, saddr, slen, st)
handle_t h;
socket_$addr_t *saddr;
u_long *slen;
status_$t *st;
{
    register client_handle_t *p = HANDLE_CAST(h);

    if (p->hhdr.cookie == CLIENT_COOKIE) {
        if ((p->flags & HF_HOST_BOUND) == 0) {
            st->all = rpc_$unbound_handle;
            return;
        }

        copy_sockaddr(&p->addr.sa, p->addr.len, saddr, slen);
    }
    else
        rpc_$get_callers_addr(h, saddr, slen);

    st->all = status_$ok;
}


/*
 * R P C _ $ I N Q _ O B J E C T _ C L I E N T
 *
 * Return the object ID contained in a handle.
 */

void rpc_$inq_object_client(h, obj, st)
handle_t h;
uuid_$t *obj;
status_$t *st;
{
    client_handle_t *p = HANDLE_CAST(h);

    *obj = p->hdr.object;
    st->all = status_$ok;
}


/*
 * R P C _ $ C L E A R _ S E R V E R _ B I N D I N G
 *
 * Remove any association the handle's sockaddr has to a specific server
 * process.  The next call made on this handle will go to the same host
 * as before, but may end up being forwarded to another server.  Useful
 * in error recovery.
 */

void rpc_$clear_server_binding(h, st)
handle_t h;
status_$t *st;
{
    register client_handle_t *p = HANDLE_CAST(h);

    socket_$set_port(&p->addr.sa, &p->addr.len, (long) socket_$unspec_port, st);

    p->flags &= ~ HF_SERVER_BOUND;

    p->hdr.server_boot = 0;
    p->hdr.ihint       = NO_HINT;
    p->hdr.ahint       = NO_HINT;

    st->all = status_$ok;
}


/*
 * R P C _ $ C L E A R _ B I N D I N G
 *
 * Remove any sockaddr association in a handle.  The next call made on
 * this handle will be broadcast unless "rpc_$set_binding" is called first.
 * Useful in error recovery.
 */

void rpc_$clear_binding(h, st)
handle_t h;
status_$t *st;
{
    register client_handle_t *p = HANDLE_CAST(h);

    rpc_$clear_server_binding(h, st);
    p->flags &= ~ HF_HOST_BOUND;

    st->all = status_$ok;
}


/*
 * R P C _ $ B I N D
 *
 * Allocate and initialize a handle for an object and then set the handle's
 * binding to the specified sockaddr.  Note that this call is simply a
 * shorthand for calls to "rpc_$alloc_handle" and "rpc_$set_binding".
 */

handle_t rpc_$bind(obj, saddr, slen, st)
uuid_$t *obj;
socket_$addr_t *saddr;
u_long slen;
status_$t *st;
{
    register client_handle_t *p;
    status_$t xst;

    p = HANDLE_CAST(rpc_$alloc_handle(obj, (u_long) saddr->family, st));

    if (st->all == status_$ok)
        rpc_$set_binding((handle_t) p, saddr, slen, st);

    if (st->all != status_$ok) {
        rpc_$free_handle((handle_t) p, &xst);
        p = NULL;
    }

#ifdef apollo
#if ! _ISP__A88K
    set_a0_hack(p);
#endif
#endif

    return ((handle_t) p);

}


/*
 * R P C _ $ S E T _ S H O R T _ T I M E O U T
 *
 * Set or clear short-timeout mode on a handle.  When a call is made with
 * a handle that's in short-timeout mode, the call fails quickly if no
 * sign of alivedness can be gotten from the server.  Note that as soon
 * as the server shows signs of being alive, standard timeouts apply for
 * the remainder of the call.  Returns the previous setting of the mode.
 */

ndr_$ulong_int rpc_$set_short_timeout(h, on, st)
handle_t h;
u_long on;
status_$t *st;
{
    register client_handle_t *p = HANDLE_CAST(h);
    boolean oldval = (p->flags & HF_SHORT_TIMEOUT) != 0;

    if (on)
        p->flags |= HF_SHORT_TIMEOUT;
    else
        p->flags &= ~HF_SHORT_TIMEOUT;

    st->all = status_$ok;
    return ((u_long) oldval);
}


/*
 * R P C _ $ S E T _ A S Y N C _ A C K
 *
 * This is a no-op in a tasking environment.
 *
 * Otherwise, sets whether reply acking is allowed to happen asynchronously
 * (i.e. *after* "rpc_$sar" return).  In general it is good to run in
 * async-ack mode because in many cases, an explicit ack is unnecessary
 * because another call may shortly follow and *it* can act as an (implicit)
 * ack.  Unfortunately, async-ack mode requires that the alarm be left
 * on after "rpc_$sar" returns.  If the program is unlucky enough to be
 * blocked in a system call that is (say) reading from a "slow device",
 * the system call will return EINTR after the alarm handler returns.  Many
 * programs are not willing to deal with this situation.  Thus, the default
 * behavior of the runtime is *not* run in async-ack mode.
 */

void rpc_$set_async_ack(b)
u_long b;
{
#ifndef TASKING
    synchronous_acks = (b == false);
#endif
}


/*
 * R P C _ $ F R E E _ H A N D L E
 *
 * Free an RPC binding made by "rpc_$bind" or "rpc_$alloc_handle".  Note that
 * the binding won't actually be freed until this routine is called the
 * number of times "rpc_$dup_handle" has been called for the same handle.
 */

void rpc_$free_handle(h, st)
handle_t h;
status_$t *st;
{
    register client_handle_t *p = HANDLE_CAST(h);

    st->all = status_$ok;

    if (-- p->refcnt == 0) {
        p->hhdr.cookie = -1;    /* Improve debuggability */
        if (p->encr)
            rpc_$encr_destroy(p->encr);
        p->encr = NULL;         /* avoid confusing next call */
        if (p->auth)
            rpc_$auth_destroy(p->auth);
        p->auth = NULL;
        rpc_$nfree(p);
    }
}


/*
 * R P C _ $ D U P _ H A N D L E
 *
 * Make a duplicate of a bound RPC handle.
 */

handle_t rpc_$dup_handle(h, st)
handle_t h;
status_$t *st;
{
    register client_handle_t *p = HANDLE_CAST(h);

    st->all = status_$ok;

    /*
     * Just bump the reference count since the the handle is read-only.
     */

    p->refcnt++;

#ifdef apollo
#if ! _ISP__A88K
    set_a0_hack(p);
#endif
#endif

    return (h);
}


/*
 * A W A I T _ R E P L Y
 *
 * Wait for either a message to arrive or a timeout period to elapse.
 */

internal await_value_t await_reply(sock, timeout, locked)
sock_t *sock;
u_long timeout;
boolean *locked;
{
    register int n_found;
    fd_set readfds;
    struct timeval t;
    u_long end_time;
    int err;
    u_long now;

    end_time = time(NULL) + timeout;

retry:

    *locked = false;
    UNLOCK(CLIENT_MUTEX);

    t.tv_sec = timeout;
    t.tv_usec = 0;

    FD_ZERO(&readfds);
    FD_SET(sock->fd, &readfds);

    n_found = select(sock->fd + 1, &readfds, NULL, NULL, &t);

    /*
     * Try to copy out "errno" before (in a tasking environment) some other
     * task's action clobbers it.  Not terribly safe even so, so we really
     * don't count on it's value to decide (for example) that the "select"
     * called failed miserably.  Let's hear it for global state :-)
     */

    err = errno;

    LOCK(CLIENT_MUTEX);
    *locked = true;

    switch (n_found) {
        case 1:
            return (await_data_available);

        case 0:
            dprintf("(await_reply) timeout\n");
            return (await_timeout);

        case -1:
            if (err != EINTR)  
                dprintf("(await_reply) select failed, errno=%d\n", err);

            now = time(NULL);

            if (now >= end_time) {
                dprintf("(await_reply) timeout (EINTR)\n");
                return (await_timeout);
            }
            else {
                timeout = end_time - now;
                goto retry;
            }
            

        default:
            eprintf("(await_reply) select returned junk, errno=%d\n", err);
            return (await_unknown_error);
    }
}


/*
 * Q U I T _ S E R V E R
 *
 * Send a "quit" to the server and wait a little while for a "quack" back.  We
 * assume that we're inhibited when entered.
 */

#define MAX_QUITS 3

internal void quit_server(sock, p, pkt, addr)
register sock_t *sock;
client_handle_t *p;
rpc_$pkt_t *pkt;
rpc_$sockaddr_t *addr;
{
    pfm_$cleanup_rec crec;
    register u_short i;
    status_$t fst, st;
    rpc_$spkt_t tpkt, rpkt;
    fd_set readfds;
    struct timeval t;
    rpc_$cksum_t cksum = NULL;

    fst = pfm_$p_cleanup(&crec);
    if (fst.all == pfm_$cleanup_set) {
        pfm_$enable();
        tpkt.hdr = pkt->hdr;
        set_pkt_type(&tpkt, rpc_$quit);
        tpkt.hdr.len = 0;

        t.tv_sec = 1;
        t.tv_usec = 0;

        FD_ZERO(&readfds);
        FD_SET(sock->fd, &readfds);

        for (i = 0; i <= MAX_QUITS - 1; i++) {
            send_pkt(sock, (rpc_$pkt_t *) &tpkt, addr, false, p->encr);
            if (select(sock->fd + 1, &readfds, NULL, NULL, &t) == 1 &&
                c_recv_pkt(sock, p, (rpc_$pkt_t *) &rpkt, (long) sizeof rpkt, tpkt.hdr.seq,
                          &tpkt.hdr.actuid, addr, &cksum) &&
                (pkt_type(&rpkt) == rpc_$quack ||
                 pkt_type(&rpkt) == rpc_$response ||
                 pkt_type(&rpkt) == rpc_$nocall))
            {
                break;
            }
        }

        pfm_$p_rls_cleanup(&crec, &st);
        pfm_$inhibit();
    }
    else
        pfm_$inhibit();
}


/*
 * R P C _ $ S A R
 *
 * Send and await reply.
 *
 * The caller supplies a pointer to the marshalled in arguments.  (Note
 * that the INs must actually start "data_offset" bytes from the beginning
 * of the buffer.)  The caller also supplies a buffer into which it wants
 * the output arguments placed.
 *
 * If the OUTs can fit into that buffer, the RPC runtime will place them
 * there, "data_offset" bytes from the start.  If they can NOT fit, the
 * output arguments will be placed into a buffer allocated by the runtime,
 * and the "must_free" out parameter will be set to "true".  (The caller
 * of "rpc_$sar" will have to free the OUTs buffer.)  In either case, "routs"
 * will point to where the OUTs really are.
 */

#define MAX_REQUESTS            30
#define MAX_PINGS               30

#define PING_WAIT_TIMEOUT       1
#define SHORT_BROADCAST_TIMEOUT 1
#define BROADCAST_TIMEOUT       5
#define FRAG_TIMEOUT            1

void rpc_$sar(h, opts, ifspec, opn, _ins, ilen, _outs, omax, r_outs, olen, drep, must_free, st)
handle_t h;             /* RPC handle */
u_long opts;            /* SAR options */
rpc_$if_spec_t *ifspec; /* Interface to call */
u_long opn;             /* Operation within interface */
rpc_$ppkt_p_t _ins;     /* -> input pkt */
u_long ilen;            /* Length of input data (not incl. header) */
rpc_$ppkt_p_t _outs;    /* -> output pkt */
u_long omax;            /* Length of output pkt (incl. header) */
rpc_$ppkt_p_t *r_outs;  /* [out] -> to place to put ptr to place where output pkt really ended up */
u_long *olen;           /* [out] Length of output data (not incl. header) */
rpc_$drep_t *drep;      /* [out] -> to place to put drep of output data */
boolean *must_free;     /* [out] -> to place to indicate whether output data must be freed */
status_$t *st;          /* [out] Return status */
{
    register client_handle_t *p;
    client_handle_t *p_;
    register rpc_$pkt_t *outs;
    rpc_$pkt_t *ins;
    sock_t *sock;           /* desc. for socket we're sending & recving on */
    u_short request_count;  /* # of requests sent */
    long wait_time;         /* Time to wait for a pkt to arrive */
    u_short wait_count;     /* # of times we've waited since sending a request pkt */
    u_short ping_count;     /* # of unack'd pings we've sent */
    uuid_$t actuid;         /* Our activity UID */
    boolean idem;           /* T => call is to idempotent procedure */
    boolean maybe;          /* T => call is to maybe procedure */
    boolean broadcast;      /* T => call is to be broadcast */
    rpc_$spkt_t tpkt;      /* Temporary pkt used for sending to server */
    rpc_$sockaddr_t from, to_addr;
    rpc_$sockaddr_t *to;
    boolean request_sent;   /* At least 1 request was sent */
    boolean large_in;       /* T => input is large */
    long mtu;               /* Max transmissible unit (i.e. pkt size) */
    call_info_t *ci;        /* -> registered info for this call */
    u_long wait_start;      /* Time we started waiting */
    boolean short_timeout;  /* T => just ping once.  See comments below */
    pfm_$cleanup_rec crec;
    status_$t fst, rst;
    boolean free_handle;    /* Free handle upon completing call */
    boolean locked;         /* T => Client mutex obtained */
    u_long seq;             /* Sequence # used in this call */
    u_long cookie;          /* Cookie from handle */
    u_short n_pkts_per_blast;
    u_short in_fragnum;     /* # of next frag we hope to send */
    boolean sending_frags;  /* T => In the process of sending a large request */
    boolean rcving_frags;   /* T => In the process of rcving a large response */
    rpc_$frag_list_t rfl;   /* Response frag list */
    rpc_$linked_pkt_t *lpkt; /* Temporary linked pkt */
    boolean have_all_frags; /* T => last frag inserted completed list */
    rpc_$cksum_t cksum = NULL;  /* checksum value for last recv'ed pkt  */

    CHECK_MISPACKED_HDR(true);

    ins = (rpc_$pkt_t *) _ins;
    outs = (rpc_$pkt_t *) _outs;

    free_handle = false;
    locked = false;
    request_sent = false;
    ci = NULL;
    broadcast = false;
    sock = NULL;
    in_fragnum = 0;

    rfl.head = NULL;

    /*
     * Setup a cleanup handler that sends a "quit" packet, unregisters the
     * call, frees the socket, and unlock the mutex.  Note that we use "p_"
     * and not "p" since "p" is declared to be "register" and who knows if
     * it's gonna have the right stuff here.
     */

    fst = pfm_$p_cleanup(&crec);
    if (fst.all != pfm_$cleanup_set) {
        if (request_sent && ! broadcast)
            quit_server(sock, p_, ins, to);
        if (cksum != NULL && rpc_$encr_rgy[p_->hdr.auth_type] != NULL) {
            rpc_$destroy_prexform(p_->hdr.auth_type, cksum);
            cksum = NULL;
        }
        if (ci != NULL)
            unregister_call(ci);
        if (sock != NULL)
            free_socket(sock);
        if (free_handle)
            rpc_$free_handle((handle_t) p_, st);
        if (locked)
            UNLOCK(CLIENT_MUTEX);
        pfm_$signal(fst);
    }

    /*
     * See whether the handle is a client or server handle.  If it's a
     * client handle, just use the current task's activity UID.  If it's
     * a server handle, we're doing a callback.  Ask the server-side runtime
     * for the correct binding, activity UID (i.e. the one for the original
     * caller) and sequence number.
     */

    cookie = (HANDLE_CAST(h))->hhdr.cookie;

    if (cookie == CLIENT_COOKIE) {
        p = HANDLE_CAST(h);
        rpc_$get_my_activity(&actuid);
        seq = rpc_$seq++;
    }
    else if (cookie == SERVER_COOKIE) {
        p = HANDLE_CAST(rpc_$server_to_client_handle(h, &actuid, &seq, st));
        if (st->all != status_$ok)
            pfm_$signal(*st);
        free_handle = true;
    }
    else
        raisec(1l);

    p_ = p;     /* Non-"register" version that's safe for use in cleanup handler above */

    /*
     * To SAR, one must have a family to SAR on!
     */

    if ((p->flags & HF_HAVE_FAMILY) == 0) {
        dprintf("(rpc_$sar) comm_failure: don't HF_HAVE_FAMILY\n");
        raisec(nca_status_$comm_failure);
    }   

    if (ilen > rpc_$max_body_size)
        raisec(rpc_$in_args_too_big);

    /*
     * Initialize various local variables and output parameters.
     */

    *r_outs = (rpc_$ppkt_p_t) outs;     /* Assume real outs will be client's buffer */
    *must_free = false;
    *olen = 0;
    st->all = status_$ok;

    rcving_frags = false;
    mtu = socket_$max_pkt_size((u_long) p->addr.sa.family, st) - PKT_HDR_SIZE;
    idem = false;
    maybe = false;
    request_count = 0;

    /*
     * Short timeout flag.  Initialize from handle flag.  If this flag
     * is true, we'll only ping for a little while.  This flag is cleared
     * as soon as we hear from the server.  I.e. the intent is to not wait
     * a long time for dead servers (or broken networks), but once we've
     * heard from a server, we always ping for a long time if we (hopefully
     * only temporarily) lose contact with the server.
     */

    short_timeout = (p->flags & HF_SHORT_TIMEOUT) != 0;

    if (opts & rpc_$brdcst)
        rpc_$clear_binding(h, st);

    /*
     * If the handle's sockaddr has a specified port, use it.  Otherwise,
     * if there is no port specified by the interface, use the forwarder's
     * port.  Otherwise, use the port specified by the interface.
     */

    if (socket_$inq_port(&p->addr.sa, p->addr.len, st) != socket_$unspec_port)
        to = &p->addr;
    else {
        copy_sockaddr_r(&p->addr, &to_addr);

        if (ifspec->port[to_addr.sa.family] == socket_$unspec_port)
            socket_$set_wk_port(&to_addr.sa, &to_addr.len, (u_long) socket_$wk_fwd, st);
        else
            socket_$set_port(&to_addr.sa, &to_addr.len, (u_long) ifspec->port[to_addr.sa.family], st);

        to = &to_addr;
    }

    LOCK(CLIENT_MUTEX);
    locked = true;

    /*
     * Check to see if header in handle has the same activity and interface
     * as the one we're going to use now.  If it doesn't, update it and
     * set the hints to be "unspecified".  Note that if it is the same,
     * we'll use the hints in the handle's header.  (They don't have to
     * be right -- the server will recover if they're wrong.)  Note that
     * handles can be used by multiple activities at the same time (which
     * is how the activities can be different) and for multiple interfaces
     * (which is how the interface can be different).
     */

    if (! uidequal(p->hdr.actuid, actuid)) {
        p->hdr.actuid = actuid;
        p->hdr.ahint  = NO_HINT;
    }

    if (! uidequal(p->hdr.if_id, ifspec->id) || p->hdr.if_vers != ifspec->vers) {
        p->hdr.if_id   = ifspec->id;
        p->hdr.if_vers = ifspec->vers;
        p->hdr.ihint   = NO_HINT;
    }

    /*
     * Fill in the header in the user-supplied packet.
     */

    ins->hdr = p->hdr;

    set_pkt_type(ins, rpc_$request);

    ins->hdr.opnum   = opn;
    ins->hdr.seq     = seq;
    ins->hdr.len     = ilen;
    ins->hdr.flags   = 0;

    ins->hdr.flags |= PF_BLAST_OUTS;    /* See note on PF_BLAST_OUTS in "rpc_p.h" */

    large_in = (ilen > mtu);

    if ((p->flags & HF_HOST_BOUND) == 0) {
        ins->hdr.flags |= PF_BROADCAST | PF_IDEMPOTENT;
        broadcast = idem = true;
    }
    if (opts & rpc_$maybe) {
        ins->hdr.flags |= PF_MAYBE | PF_IDEMPOTENT;
        maybe = idem = true;
    }
    if (opts & rpc_$idempotent) {
        ins->hdr.flags |= PF_IDEMPOTENT;
        idem = true;
    }

    if (p->auth && ! idem && ! broadcast) {
        if (!p->encr)
            p->encr = rpc_$auth_get_encr(p->auth, h, st);
        if (st->all != status_$ok) {
            dprintf("(rpc_$sar) can't get encr, status=%08lx\n", st->all);
            pfm_$signal(*st);
        }
    }

    if (p->encr)
        mtu -= rpc_$encr_overhead(p->encr);

    if (broadcast && large_in)
        raisec(rpc_$in_args_too_big);

    init(! idem);       /* Initialize; start acker only if this is a non-idem call */

    sock = alloc_socket((u_long) to->sa.family, st);
    if (st->all != status_$ok)
        raise(*st);

    ci = register_call(&actuid, p, ins->hdr.seq);

    if (large_in)
        n_pkts_per_blast = MAX_PKTS_PER_BLAST;     /* 1 for non-local net target? */

#ifndef NO_STATS
    rpc_$stats.calls_out++;
#endif

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

    /*
     * Send a "request" packet.  If we fail to send, sleep a bit and try again.
     * (Don't do this forever.)  Otherwise enter the WAIT state.
     */

SEND_REQUEST:

    wait_count = 0;

    if (request_count++ >= MAX_REQUESTS) {
        dprintf("(rpc_$sar) comm_failure: exceeded MAX_REQUESTS\n");
        raisec(nca_status_$comm_failure);
    }
    request_sent = true;

    if (! large_in) {
        send_pkt(sock, ins, to, broadcast, p->encr);
        if (maybe)
            goto DONE;
        sending_frags = false;
    }
    else {
        u_short i = 0;
        boolean last_frag_in_blast;
        boolean last_frag_in_rqst;

#ifndef NO_STATS
        rpc_$stats.frag_resends += in_fragnum - ins->hdr.fragnum;
#endif
        in_fragnum = ins->hdr.fragnum;

        do {
            rpc_$pkt_t fpkt;

            last_frag_in_rqst  = rpc_$fill_frag(ins, ilen, in_fragnum, mtu, &fpkt);
            last_frag_in_blast = (i == n_pkts_per_blast - 1) || last_frag_in_rqst;

            if (! last_frag_in_blast)
                fpkt.hdr.flags |= PF_NO_FACK;

            send_pkt(sock, &fpkt, to, broadcast, p->encr);

            i++, in_fragnum++;

        } while (! last_frag_in_blast);

        sending_frags = ! last_frag_in_rqst;
    }

    goto WAIT;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

    /*
     * WAIT state
     *
     * Wait for a reply or timeout.  The length of time we will wait is fixed
     * for broadcast requests.  For non-broadcasts requests, we will wait
     * for exponentially increasing amounts of time (up to some limit)
     * the more times we wait.  If we're in the middle of either sending
     * or receiving fragments, be less generous about how long we'll wait.
     */

WAIT:

    wait_start = time(NULL);

    if (sending_frags || rcving_frags)
        wait_time = FRAG_TIMEOUT;
    else if (broadcast)
        wait_time = (short_timeout ?
                        SHORT_BROADCAST_TIMEOUT : BROADCAST_TIMEOUT);
    else {
        wait_time = 1 << ((wait_count < 10) ? wait_count : 10);
        wait_count++;
    }

REWAIT:

    switch (wait_time <= 0 ? await_timeout : await_reply(sock, wait_time, &locked)) {
        case await_data_available:
            if (cksum != NULL && rpc_$encr_rgy[p->hdr.auth_type] != NULL) {
                rpc_$destroy_prexform(p->hdr.auth_type, cksum);
                cksum = NULL;
            }
            if (! c_recv_pkt(sock, p, outs, omax, ins->hdr.seq, &actuid, &from, &cksum)) {
                wait_time -= time(NULL) - wait_start;
                goto REWAIT;
            }

            short_timeout = false;

            switch (pkt_type(outs)) {
                case rpc_$response:
                    goto GOT_RESPONSE;
                case rpc_$fack:
                    goto GOT_FACK;
                case rpc_$working:
                    goto GOT_WORKING;
                case rpc_$fault:
                case rpc_$reject:
                    rpc_$get_pkt_body_st(outs, &rst);
                    raise(rst);
                default:
                    dprintf("(rpc_$sar) Anomolous response to request (ptype=%s)\n",
                            rpc_$pkt_name(pkt_type(outs)));
                    goto SEND_REQUEST;
            }

        case await_timeout:
            if (broadcast) {
                dprintf("(rpc_$sar) comm_failure: broadcast timeout\n");
                raisec(nca_status_$comm_failure);
            }
            if (sending_frags)
                goto SEND_REQUEST;
            else {
                dprintf("(rpc_$sar) Starting to ping\n");
                ping_count = 0;
                goto SEND_PING;
            }

        case await_select_failed:
        case await_unknown_error:
            DIE("select failed");
    }

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

    /*
     * Send a "ping" packet.  If we fail to send, sleep a bit and try again.
     * (Don't do this forever.)  Otherwise enter the PING_WAIT state.
     */
    
SEND_PING:

    ping_count++;

    if (ping_count >= MAX_PINGS || (short_timeout && ping_count > 1)) {
        dprintf("(rpc_$sar) Too many pings...signaling\n");
        raisec(nca_status_$comm_failure);
    }

    dprintf("(rpc_$sar) ping %d\n", ping_count);
    tpkt.hdr = ins->hdr;

    set_pkt_type(&tpkt, rpc_$ping);
    tpkt.hdr.len = 0;

    if (! send_pkt(sock, (rpc_$pkt_t *) &tpkt, to, false, p->encr)) {
        sleep(1);
        goto SEND_PING;
    }
    else
        goto PING_WAIT;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

    /*
     * PING_WAIT state
     *
     * Wait for a packet to arrive.  We're hoping it's going to be a response
     * to a "ping" packet we sent.  We'll cope with other responses.
     */

PING_WAIT:

    switch (await_reply(sock, (long) PING_WAIT_TIMEOUT, &locked)) {

        case await_data_available:
            if (cksum != NULL && rpc_$encr_rgy[p->hdr.auth_type] != NULL) {
                rpc_$destroy_prexform(p->hdr.auth_type, cksum);
                cksum = NULL;
            }

            if (! c_recv_pkt(sock, p, outs, omax, ins->hdr.seq, &actuid, &from, &cksum))
                goto PING_WAIT;

            short_timeout = false;

            switch (pkt_type(outs)) {
                case rpc_$response:
                    goto GOT_RESPONSE;
                case rpc_$working:
                    goto GOT_WORKING;
                case rpc_$nocall:
                    goto SEND_REQUEST;
                case rpc_$fault:
                case rpc_$reject:
                    rpc_$get_pkt_body_st(outs, &rst);
                    raise(rst);
                default:
                    dprintf("(rpc_$sar) Anomolous response to ping (ptype=%s)\n",
                            rpc_$pkt_name(pkt_type(outs)));
                    goto SEND_PING;
            }

        case await_timeout:
            goto SEND_PING;

        case await_select_failed:
        case await_unknown_error:
            DIE("select failed");
    }

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

    /*
     * Received a "working" packet.  Make sure we're not sending fragments.
     * (Someone screwed up if we are.)  Enter the WAIT state.
     */

GOT_WORKING:

    if (sending_frags) {
        dprintf("(rpc_$sar) Rcvd \"working\" during frag send!\n");
        raisec(nca_status_$proto_error);
    }

    dprintf("(rpc_$sar) Rcvd \"working\" pkt; seq=%lu\n", outs->hdr.seq);

    ins->hdr.ahint = outs->hdr.ahint;
    ins->hdr.ihint = outs->hdr.ihint;

    goto WAIT;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

    /*
     * Received a "frag ack" packet.  Make sure it's frag # matches the frag
     * we thing we just sent.  Make sure we're really in the middle of doing
     * frag sends.  Set things up for the next frag and then send it.
     */

GOT_FACK:

    if (! large_in) {
        dprintf("(rpc_$sar) Got \"fack\" for non-large request!\n");
        raisec(nca_status_$proto_error);
    }

    /*
     * Ignore very old facks.
     */

    if (outs->hdr.fragnum + 1 < ins->hdr.fragnum)
        goto WAIT;

    /*
     * Arrange to start next blast at the frag one higher than the last the
     * one just fack'd.
     */

    ins->hdr.fragnum = outs->hdr.fragnum + 1;

    request_count = 0;
    goto SEND_REQUEST;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

    /*
     * Received a "response" packet.  Make sure we're not sending fragments.
     * (Someone screwed up if we are.)  See if this is a fragmentary response.
     * Handle if it is, otherwise, we're done.
     */

GOT_RESPONSE:

    /*
     * If we're playing the authentication game, check that the reply
     * is authentic.
     */

    if (p->encr && ! idem && ! broadcast) {
        if (outs->hdr.auth_type != rpc_$auth_type(p->encr))
            raisec(rpc_$invalid_auth_type);
        unpack_drep(drep, outs->hdr.drep);
        rpc_$encr_recv_xform(p->encr, *drep,
                             &outs->body.args[outs->hdr.len],
                             cksum, &outs->hdr.seq, st);
        if (st->all != status_$ok)
            raisec(st->all);
        if (cksum != NULL && rpc_$encr_rgy[p->hdr.auth_type] != NULL) {
            rpc_$destroy_prexform(p->hdr.auth_type, cksum);
            cksum = NULL;
        }
    }

    if (outs->hdr.flags & PF_FRAG)
        goto GOT_FRAG_RESPONSE;

    if (rcving_frags) {
        dprintf("(rpc_$sar) Got non-frag \"response\" during frag recv!\n");
        raisec(nca_status_$proto_error);
    }

    *olen = outs->hdr.len;
    goto GOT_RESPONSE_COMMON;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

    /*
     * Handle a fragmentary response (either the first, or subsequent
     * response frag).   Glom this frag onto the aggregate result.  If
     * it's not the last frag, send a "frag ack" packet to prod the server
     * into continuing and continuing waiting.  Otherwise, we're done.
     */

GOT_FRAG_RESPONSE:

    rcving_frags = true;

    lpkt = rpc_$alloc_linked_pkt(outs, (u_long) outs->hdr.len);
    rpc_$insert_in_frag_list(&rfl, lpkt, &have_all_frags);
    rpc_$free_linked_pkt(lpkt);

    if (! have_all_frags) {
        if ((outs->hdr.flags & PF_NO_FACK) == 0) {
            tpkt.hdr = ins->hdr;
            set_pkt_type(&tpkt, rpc_$fack);
            tpkt.hdr.len = 0;
            tpkt.hdr.ahint = outs->hdr.ahint;
            tpkt.hdr.fragnum = rfl.highest;
            send_pkt(sock, (rpc_$pkt_t *) &tpkt, to, false, p->encr);
        }
        goto WAIT;
    }

    *olen = rfl.len;
    *r_outs = (rpc_$ppkt_p_t) rpc_$reassemble_frag_list(&rfl);
    *must_free = true;

    goto GOT_RESPONSE_COMMON;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

    /*
     * Got a complete response.  If the request was to a non-idempotent
     * procedure, update our acknowledgement database.  Update handle,
     * cleanup, and return to caller.
     */

GOT_RESPONSE_COMMON:

    if (! idem) {
        update_ack_db(outs, &from);
#ifndef TASKING
        if (synchronous_acks) {
            ack_replies();
            rpc_$stop_periodic(ack_replies);
            initialized_acker = false;
        }
#endif
    }

    /*
     * Update the hints in the handle's version of the header (for use in
     * later RPC calls).  Note we must make sure that the handle hasn't been
     * snarfed by a different activity/interface since we started this call
     * before updating the hints.
     */

    if (uidequal(p->hdr.if_id, ifspec->id) && p->hdr.if_vers == ifspec->vers)
        p->hdr.ihint = outs->hdr.ihint;

    if (uidequal(p->hdr.actuid, actuid))
        p->hdr.ahint = outs->hdr.ahint;

    p->hdr.server_boot = outs->hdr.server_boot;
    p->flags |= HF_HOST_BOUND | HF_SERVER_BOUND;

    copy_sockaddr_r(&from, &p->addr);

    unpack_drep(drep, outs->hdr.drep);

    goto DONE;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

    /*
     * Cleanup after a successful call.
     */

DONE:
    if (cksum != NULL && rpc_$encr_rgy[p->hdr.auth_type] != NULL) {
        rpc_$destroy_prexform(p->hdr.auth_type, cksum);
        cksum = NULL;
    }

    if (free_handle) {
        free_handle = false;
        rpc_$free_handle((handle_t) p, st);
    }

    free_socket(sock);
    unregister_call(ci);

    pfm_$p_rls_cleanup(&crec, st);

    UNLOCK(CLIENT_MUTEX);
}


/*
 * R P C _ $ N A M E _ T O _ S O C K A D D R
 *
 * Given a string "name" of a machine and a port number, return a
 * socket_$addr_t that describes that port on that machine.  Note that
 * the "family" field of "saddr" must be initialized prior to calling
 * this procedure.  The family argument determines how the string
 * name is converted to internal form.
 */

void rpc_$name_to_sockaddr(name, name_len, port, family, saddr, slen, st)
rpc_$string_t name;
u_long name_len;
u_long port;
u_long family;
socket_$addr_t *saddr;
u_long *slen;
status_$t *st;
{
#ifdef GLOBAL_LIBRARY
    rpc_$global_lib_init();
#endif
    socket_$from_name(family, (ndr_$char *) name, name_len, port, saddr, slen, st);
}


/*
 * R P C _ $ S O C K A D D R _ T O _ N A M E
 *
 * Given a sockaddr, return a string "name" of a machine and a port
 * number.
 */

void rpc_$sockaddr_to_name(saddr, slen, name, name_len, port, st)
socket_$addr_t *saddr;
u_long slen;
rpc_$string_t name;
u_long *name_len;
u_long *port;
status_$t *st;
{
#ifdef GLOBAL_LIBRARY
    rpc_$global_lib_init();
#endif
    socket_$to_name(saddr, slen, (ndr_$char *) name, name_len, port, st);
}


/*
 * R P C _ $ S E T _ A U T H
 *
 * Associate an "auth" structure with a given handle.
 */

void rpc_$set_auth(h, auth, st)
handle_t h;
rpc_$client_auth_t *auth;
status_$t *st;
{
    register client_handle_t *clh;
    u_long cookie = (HANDLE_CAST(h))->hhdr.cookie;

    if (cookie == CLIENT_COOKIE)
        clh = HANDLE_CAST(h);
    else if (cookie == SERVER_COOKIE)
        raisec(1L);             /* Should be a real exception here. */

    st->all = status_$ok;               /* optimist */

    if (clh->auth)
        rpc_$auth_destroy(clh->auth);

    clh->auth = auth;
}


/*
 * R P C _ $ I N Q _ A U T H
 *
 * Get auth pointer associated with a handle.
 */

rpc_$client_auth_t *rpc_$inq_auth(h, st)
handle_t h;
status_$t *st;
{
    register client_handle_t *clh;
    u_long cookie = (HANDLE_CAST(h))->hhdr.cookie;

    if (cookie == CLIENT_COOKIE)
        clh = HANDLE_CAST(h);
    else if (cookie == SERVER_COOKIE)
        raisec(1L);             /* Should be a real exception here. */

    st->all = status_$ok;
    return (clh->auth);
}


/*
 * R P C _ $ S E T _ E N C R
 *
 * Associate an encryption structure with a handle.
 */

void rpc_$set_encr(h, encr, st)
handle_t h;
status_$t *st;
rpc_$encr_t *encr;
{
    register client_handle_t *clh;
    u_long cookie = (HANDLE_CAST(h))->hhdr.cookie;

    if (cookie == CLIENT_COOKIE) {
        st->all = status_$ok;           /* optimist */
        clh = HANDLE_CAST(h);
        if (clh->encr)
            rpc_$encr_destroy(clh->encr);
        clh->encr = encr;
    }
    else if (cookie == SERVER_COOKIE)
        rpc_$set_server_encr(h, encr, st);
    else
        raisec(rpc_$invalid_handle);            /* "Can't happen" */
}


/*
 * R P C _ $ I N Q _ E N C R
 *
 * Get the encryption structure associated with a handle.
 */

rpc_$encr_t *rpc_$inq_encr (h, st)
handle_t h;
status_$t *st;
{
    register client_handle_t *clh;
    u_long cookie = (HANDLE_CAST(h))->hhdr.cookie;

    if (cookie == CLIENT_COOKIE) {
        st->all = status_$ok;
        clh = HANDLE_CAST(h);
        return (clh->encr);
    }
    else if (cookie == SERVER_COOKIE)
        return (rpc_$inq_server_encr (h, st));
    else
        raisec(rpc_$invalid_handle);            /* can't happen */
}

#ifdef GLOBAL_LIBRARY

/*
 * R P C _ $ C L I E N T _ M A R K _ R E L E A S E
 *
 * Support for Apollo multiple programs per process ("program levels").
 */

void rpc_$client_mark_release(is_mark, level, st, is_exec)
boolean *is_mark;
u_short *level;
status_$t *st;
boolean *is_exec;
{
    st->all = status_$ok;

    if (*is_mark) {
        level_info_t *li = &level_info[*level];

        li->nsockets          = nsockets;
        li->initialized_level = initialized_level;
        li->initialized_acker = initialized_acker;

        initialized_level = false;
    }
    else {
        u_short i;
        level_info_t *li = &level_info[*level + 1];

        for (i = li->nsockets; i < nsockets; i++)
            ios_$set_switch_flag((ios_$id_t) sock_pool[i].fd, ios_$protected, false, st);

        nsockets              = li->nsockets;
        initialized_level     = li->initialized_level;
        initialized_acker     = li->initialized_acker;
    }
}

#endif

#ifdef FTN_INTERLUDES

u_long rpc_$alloc_handle_(obj, family, st)
uuid_$t *obj;
u_long *family;
status_$t *st;
{
    return ((u_long) rpc_$alloc_handle(obj, *family, st));
}

void rpc_$set_binding_(h, saddr, slen, st)
handle_t *h;
socket_$addr_t *saddr;
u_long *slen;
status_$t *st;
{
    rpc_$set_binding(*h, saddr, *slen, st);
}

void rpc_$inq_binding_(h, saddr, slen, st)
handle_t *h;
socket_$addr_t *saddr;
u_long *slen;
status_$t *st;
{
    rpc_$inq_binding(*h, saddr, slen, st);
}

void rpc_$clear_binding_(h, st)
handle_t *h;
status_$t *st;
{
    rpc_$clear_binding(*h, st);
}

u_long rpc_$bind_(obj, saddr, slen, st)
uuid_$t *obj;
socket_$addr_t *saddr;
u_long *slen;
status_$t *st;
{
    return ((u_long) rpc_$bind(obj, saddr, *slen, st));
}

u_long rpc_$set_short_timeout_(h, on, st)
handle_t *h;
u_long *on;
status_$t *st;
{
    return (rpc_$set_short_timeout(*h, *on, st));
}

void rpc_$set_async_ack_(b)
u_long *b;
{
    rpc_$set_async_ack(*b);
}

void rpc_$free_handle_(h, st)
handle_t *h;
status_$t *st;
{
    rpc_$free_handle(*h, st);
}

u_long rpc_$dup_handle_(h, st)
handle_t *h;
status_$t *st;
{
    return ((u_long) rpc_$dup_handle(*h, st));
}

void rpc_$name_to_sockaddr_(name, name_len, port, family, saddr, slen, st)
rpc_$string_t name;
u_long *name_len;
u_long *port;
u_long *family;
socket_$addr_t *saddr;
u_long *slen;
status_$t *st;
{
    rpc_$name_to_sockaddr(name, *name_len, *port, *family, saddr, slen, st);
}

void rpc_$sockaddr_to_name_(saddr, slen, name, name_len, port, st)
socket_$addr_t *saddr;
u_long *slen;
rpc_$string_t name;
u_long *name_len;
u_long *port;
status_$t *st;
{
    rpc_$sockaddr_to_name(saddr, *slen, name, name_len, port, st);
}

#endif
