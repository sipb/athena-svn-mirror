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
 * R P C _ L S N
 *
 * The modules "rpc_server.c" and "rpc_lsn.c" together implement the server
 * side NCA/RPC protocol.
 *
 * "rpc_server.c" has all the low-level packet shuffling stuff.  "rpc_lsn.c"
 * has most of the upper level routines that might be called by an actual server
 * application.
 */

#include "rpc_p.h"

#ifdef EMBEDDED_LLBD
#  include "llb.h"
#endif

#ifdef TASKING
#  define MAX_LISTENERS       10
#else
#  define MAX_LISTENERS       1
#endif

/*
 * Private RPC listen data.
 *
 * On Apollos, this data with NOT be in the DATA$ section and hence will
 * be per-process when this module is used in a global library.
 */

    /*
     * Server socket information.  List of sockets server must listen on.
     */

#define MAX_SOCKETS 5

typedef struct {
    int sock;                   /* File descriptor (stream ID) of socket */
} NEAR sinfo_t;

internal sinfo_t sinfo[MAX_SOCKETS];

    /*
     * Info about forwarded packets.  Hold info about the last partial forwared
     * packet we received.
     */

typedef struct {
    u_long seq;
    u_short fragnum;
    uuid_$t actuid;
    rpc_$fpkt_hdr_t fhdr;
    boolean valid;
} NEAR fwd_state_t;

    /*
     * Miscellaneous state variables
     */

internal boolean NEAR initialized;           /* T => rpc_$listen has been called at least once */
internal short NEAR n_sockets;               /* # of sockets we listen on */
internal u_short NEAR n_waiters;             /* # of server tasks waiting */
internal int NEAR max_listeners;             /* max # of listener tasks allowed */
internal boolean NEAR shut_down;             /* T => server is to shut down */
internal boolean NEAR allow_remote_shutdown; /* T => "rrpc_$shutdown" allowed */
internal rpc_$shut_check_fn_t NEAR shut_check_fn;  /* non-nil => pointer to shutdown check function */
internal fwd_state_t fwd_state;              /* Info about forwarded pkts */

#ifdef TASKING
internal short n_listeners;             /* current # of listener tasks */
internal short n_listen_failures;       /* # of listener tasks that died */
internal boolean listen_task_starting;  /* T => a listerner task is starting up */
internal ec2_$eventcount_t check_for_pkts_ec;   /* advance this EC to prod the pkt checker task */
internal task_$handle_t listen_tasks[MAX_LISTENERS]; /* task handles for all the listen tasks */
internal task_$handle_t task_that_shut_down;    /* handle of task that executed "rpc_$shutdown" */
#endif

#ifdef GLOBAL_LIBRARY
#include "set_sect.pvt.c"
#endif

internal void use_family
    PROTOTYPE((u_long family, u_long port, socket_$addr_t *saddr, u_long *slen, status_$t *st));
internal boolean s_recv_pkt
    PROTOTYPE((int sock, rpc_$pkt_t *pkt, rpc_$sockaddr_t *from, rpc_$cksum_t *cksum));
internal void check_for_pkt_common
    PROTOTYPE((struct fd_set *fds));
#ifndef TASKING
internal void check_for_pkt
    PROTOTYPE((void));
#else
internal void check_for_pkts
    PROTOTYPE((void));
#endif
internal void rlisten
    PROTOTYPE((void));

/*
 * U S E _ F A M I L Y
 *
 * Internal routine (used by "rpc_$use_family" and "rpc_$use_family_wk")
 * to create and register a socket to be used to listen on.
 */

internal void use_family(family, port, saddr, slen, st)
u_long family;
u_long port;
socket_$addr_t *saddr;
u_long *slen;
status_$t *st;
{
    register int sock;
    socket_$net_addr_t naddr;
    u_long nlen = sizeof naddr;
    int t_slen = sizeof *saddr;
    register sinfo_t *si;

    st->all = status_$ok;

#ifdef GLOBAL_LIBRARY
    rpc_$global_lib_init();
#endif

    LOCK(SERVER_MUTEX);

    if (n_sockets >= MAX_SOCKETS) {
        UNLOCK(SERVER_MUTEX);
        st->all = rpc_$too_many_sockets;
        return;
    }

    sock = socket((int) family, SOCK_DGRAM, 0);
    if (sock < 0) {
        UNLOCK(SERVER_MUTEX);
        dprintf("(use_family) Can't create socket\n");
        st->all = rpc_$cant_create_sock;
        return;
    }

    set_socket_non_blocking(sock);

    bzero((char *) saddr, sizeof *saddr);

    saddr->family = family;

    if (port != socket_$unspec_port)
        socket_$set_port(saddr, slen, port, st);

    if (bind(sock, saddr, sizeof(struct sockaddr)) < 0) {
        dprintf("(use_family) Can't bind socket, errno=%d\n", errno);
        if (errno == EADDRINUSE)
            st->all = rpc_$addr_in_use;
        else
            st->all = rpc_$cant_bind_sock;
        goto ERROR_EXIT;
    }

    /*
     * Get the real "name" of the local socket.  Note that since "getsockname"
     * doesn't always fill in the local network address, we do that manually.
     */

    if (getsockname(sock, saddr, &t_slen) < 0) {
        dprintf("(rpc_$use_family) Can't getsockname, errno=%d\n", errno);
        st->all = rpc_$cant_create_sock;
        goto ERROR_EXIT;
    }

    *slen = t_slen;

    socket_$inq_my_netaddr(family, &naddr, &nlen, st);
    if (st->all != status_$ok) {
        dprintf("(rpc_$use_family) Can't get my netaddr\n");
        goto ERROR_EXIT;
    }

    socket_$set_netaddr(saddr, slen, &naddr, nlen, st);

    si = &sinfo[n_sockets];

    si->sock = sock;

#ifdef TASKING

    /*
     * When tasking, we have to worry about the case of a task calling
     * "use_family" when there's already another task in "listen".  We
     * want the latter task to add this newly created socket to the set
     * of sockets it's listening on.  So, if there are already some listener
     * tasks and all of them are currently waiting, send a signal to the
     * first listen task so we're sure to have at least one listener
     * listening to the new socket.
     */

    if (n_listeners > 0 && n_waiters == n_listeners) {
        status_$t fst;

        fst.all = CHECK_SOCKETS_FAULT;
        task_$signal(listen_tasks[0], fst, st);
    }

#endif

    n_sockets++;

    UNLOCK(SERVER_MUTEX);

    st->all = status_$ok;
    return;

ERROR_EXIT:

    UNLOCK(SERVER_MUTEX);
    close_socket(sock);
}


/*
 * R P C _ $ U S E _ F A M I L Y
 *
 * Declare that you want to listen for calls coming in over the specified
 * protocol family.  Returns the sockaddr that will be used.
 */

void rpc_$use_family(family, saddr, slen, st)
u_long family;
socket_$addr_t *saddr;
u_long *slen;
status_$t *st;
{
    use_family(family, (u_long) socket_$unspec_port, saddr, slen, st);
}


/*
 * R P C _ $ U S E _ F A M I L Y _ W K
 *
 * Like "rpc_$use_family" except says that you want socket address to have
 * its port be the port in the specified interface descriptor.
 */

void rpc_$use_family_wk(family, ifspec, saddr, slen, st)
u_long family;
rpc_$if_spec_t *ifspec;
socket_$addr_t *saddr;
u_long *slen;
status_$t *st;
{
    use_family(family, (u_long) ifspec->port[family], saddr, slen, st);
}


/*
 * S _ R E C V _ P K T
 *
 * Receive a packet.  This is the only routine in the server side that calls
 * "recvfrom".  It copes with forwarded packets (generated by someone's
 * having called "rpc_$forward") by extracting the original sender's
 * address from the packet and returning it as the "from" address, rather
 * than the "from" address returned by "recvfrom".
 */

internal boolean s_recv_pkt(sock, pkt, from, cksum)
int sock;
register rpc_$pkt_t *pkt;
rpc_$sockaddr_t *from;
rpc_$cksum_t *cksum;
{
    int olen;

    olen = rpc_$recvfrom(sock, pkt, (u_long) sizeof *pkt, from, cksum);

    if (olen < 0)
        return (false);

    /*
     * Maybe handle the second part of a forwarded packet.  If we've
     * previously received a full-sized packet that needed forwarding,
     * and the current packet is from the same activity as that full-sized
     * packet and has the same sequence number, then the current packet
     * must be the real packet that was forwarded.  Return the original
     * saved forwarding info and the current packet.
     * 
     * Note that this code is moderately "weak".  If a bunch of full-size
     * packets that need forwarding arrive at the machine in a bunch, we
     * may lose some of them.  That should be OK since the sender will
     * just retransmit them.  A better (but harder) approach would be to
     * handle this muck in "rpc_server.c" where we could hang the partial
     * forwarded packets off the activity blocks.
     */

    if (fwd_state.valid && 
        (pkt->hdr.flags & PF_FORWARDED) == 0 && 
        uidequal(fwd_state.actuid, pkt->hdr.actuid) &&
        fwd_state.seq == pkt->hdr.seq &&
        fwd_state.fragnum == pkt->hdr.fragnum)
    {
        copy_drep(pkt->hdr.drep, fwd_state.fhdr.drep);
        *from = fwd_state.fhdr.addr;
        fwd_state.valid = false;
    }

    /*
     * If this is a forwarded packet, extract the real sender of the
     * message and convert the packet so it looks like an unforwarded
     * packet.  Cope with multi-packet forwarding.  (See comments in
     * "rpc_$forward".)
     */

    if (pkt->hdr.flags & PF_FORWARDED) {
        rpc_$fpkt_t *fpkt = (rpc_$fpkt_t *) pkt;
        register u_short i, j;

        /*
         * If the "forwarded in 2 pieces" flag is set, remember the info
         * so we can use it when the rest of the forwarded pkt arrives.
         * Return "false" to indicate that we received no packet.
         */

        if (pkt->hdr.flags2 & PF2_FORWARDED_2) {
            fwd_state.valid   = true;
            fwd_state.fhdr    = fpkt->fhdr;
            fwd_state.actuid  = fpkt->hdr.actuid;
            fwd_state.seq     = fpkt->hdr.seq;
            fwd_state.fragnum = fpkt->hdr.fragnum;
            return (false);             /* No packet this time */
        }

        /*
         * We must have the whole forwarded pkt right now.  Copy out the
         * forwarded info and then slide the body up in place.
         */

        copy_drep(pkt->hdr.drep, fpkt->fhdr.drep);
        *from = fpkt->fhdr.addr;
        for (i = 0, j = pkt->hdr.len; j > 0; i++, j--)
            pkt->body.args[i] = fpkt->body.args[i];
    }

    return (true);
}


/*
 * R P C _ $ L I S T E N _ R E C V
 *
 * Receive one RPC packet.
 *
 * This routine is (mainly) intended to be used by programs that want to read
 * from socket directly and then sometimes do RPC-like stuff with what they've
 * read.
 */

void rpc_$listen_recv(sock, _pkt, cksum, from, from_len, ptype, obj, if_id, st)
u_long sock;
rpc_$ppkt_p_t _pkt;
rpc_$cksum_t *cksum;
socket_$addr_t *from;
u_long *from_len;
u_long *ptype;
uuid_$t *obj;
uuid_$t *if_id;
status_$t *st;
{
    register rpc_$pkt_t *pkt = (rpc_$pkt_t *) _pkt;
    rpc_$sockaddr_t addr;

    st->all = status_$ok;

    if (! s_recv_pkt((int) sock, pkt, &addr, cksum)) {
        st->all = rpc_$cant_recv;
        return;
    }

    copy_sockaddr(&addr.sa, addr.len, from, from_len);

    *ptype    = (u_long) pkt_type(pkt);
    *obj      = pkt->hdr.object;
    *if_id    = pkt->hdr.if_id;
}


/*
 * R P C _ $ F O R W A R D
 *
 * Forward one packet.  This routine is intended mainly to be used in
 * conjunction with "rpc_$listen_recv".  I.e. the packet returned by
 * "rpc_$listen_recv" is the input to this routine.  Further, the whole
 * forwarding business is strictly the domain of the local location broker
 * (llbd).  See "llbd.c" for more on forwarding.
 * 
 * Note that if the sum of the RPC pkt header, the "forward" header, and
 * the data length is larger than will fit in a single packet, we forward
 * the packet as two packets (otherwise we cram it all into one packet).
 * The first one contains the RPC pkt header and the "forward" header and
 * the second one contains the identical RPC pkt header and the original
 * data.  N.B. don't confuse all this with packet fragmentation and
 * reassembly, which is network-visible, not a intra-machine hack like
 * this is.
 */

void rpc_$forward(sock, from, from_len, to, to_len, _pkt, st)
u_long sock;
socket_$addr_t *from;
u_long from_len;
socket_$addr_t *to;
u_long to_len;
rpc_$ppkt_p_t _pkt;
status_$t *st;
{
    register rpc_$pkt_t *pkt = (rpc_$pkt_t *) _pkt;
    rpc_$fpkt_t fpkt;
    rpc_$sockaddr_t addr;
    long mtu;

    mtu = socket_$max_pkt_size((u_long) to->family, st) - PKT_HDR_SIZE;

    fpkt.hdr = pkt->hdr;
    fpkt.hdr.flags |= PF_FORWARDED;

    copy_drep(fpkt.fhdr.drep, pkt->hdr.drep);

    copy_sockaddr(from, from_len, &fpkt.fhdr.addr.sa, &fpkt.fhdr.addr.len);
    copy_sockaddr(to, to_len, &addr.sa, &addr.len);

    if (pkt->hdr.len + sizeof(rpc_$fpkt_hdr_t) <= mtu) {
        bcopy((char *) &pkt->body, (char *) &fpkt.body, (int) pkt->hdr.len);
        rpc_$sendto((int) sock, (rpc_$pkt_t *) &fpkt, &addr, (rpc_$encr_t *) NULL);
    }
    else {
        fpkt.hdr.flags2 |= PF2_FORWARDED_2;
        fpkt.hdr.len = 0;
        rpc_$sendto((int) sock, (rpc_$pkt_t *) &fpkt, &addr, (rpc_$encr_t *) NULL);
        rpc_$sendto((int) sock, pkt, &addr, (rpc_$encr_t *) NULL);
    }
}


/*
 * C H E C K _ F O R _ P K T _ C O M M O N
 *
 * Underlying routine for "check_for_pkt" and "check_for_pkts".  Takes the
 * select fd set, finds the all the sockets that have data ready, reads
 * the socket and processes the packets.  We don't use "select" itself since
 * in some implementations we're using our bogus select that can't handle more
 * than one socket at a time.
 */

internal void check_for_pkt_common(fds)
struct fd_set *fds;
{
    status_$t st;
    rpc_$pkt_t pkt;
    rpc_$sockaddr_t from;
    register u_short i;
    rpc_$cksum_t cksum;

    for (i = 0; i < n_sockets; i++)
        if (FD_ISSET(sinfo[i].sock, fds)) {
            if (s_recv_pkt(sinfo[i].sock, &pkt, &from, &cksum)) {
                if (pkt_type(&pkt) != rpc_$request) {
                    dprintf("(check_for_pkt_common) Rcvd (ptype=%s)\n", rpc_$pkt_name(pkt_type(&pkt)));
                    rpc_$listen_dispatch((u_long) sinfo[i].sock, (rpc_$ppkt_p_t) &pkt, cksum, &from.sa, (u_long) from.len, &st);
                } 
                else
                    dprintf("(check_for_pkt_common) dropping request packet in interrupt\n");
            }
        }
}


#ifndef TASKING

#define CHECK_FOR_PKT_INTERVAL ((u_long) 2)

/*
 * C H E C K _ F O R _ P K T
 *
 * Check all our sockets for any data.  If a non-request packet is found,
 * process it.  This routine is used only in non-tasking environments.
 *
 */

internal void check_for_pkt()
{
    struct fd_set readfds;
    int n_found;
    register u_short i;
    struct timeval tv;

    if (n_waiters > 0)
        return;

    FD_ZERO(&readfds);

    for (i = 0; i < n_sockets; i++)
        FD_SET(sinfo[i].sock, &readfds);

    timerclear(&tv);
    n_found = select(FD_SETSIZE, &readfds, NULL, NULL, &tv);

    if (n_found <= 0 || n_waiters > 0)
        return;

    check_for_pkt_common(&readfds);
}

#else

/*
 * C H E C K _ F O R _ P K T S
 *
 * Base of task created by "rpc_$listen".  This task is responsible for
 * handling non-request packets that arrive while all the listen tasks
 * are busy.  It waits on an EC that's advanced from "listen" when it
 * "listen" notices that there are no listeners waiting.
 */

internal void check_for_pkts()
{
    ec2_$ptr_t ecps[1];
    long ecvs[1];
    status_$t st;
    struct fd_set readfds;
    register u_short i;
    int n_found;

    ecps[0] = &check_for_pkts_ec;
    ecvs[0] = 1;

    while (true) {
        dprintf("(check_for_pkts) Waiting...\n");
        ec2_$wait(ecps, ecvs, 1, &st);
        dprintf("(check_for_pkts) Checking for pkts...\n");
        ecvs[0] = check_for_pkts_ec.value + 1;

        while (true) {
            FD_ZERO(&readfds);

            for (i = 0; i < n_sockets; i++)
                FD_SET(sinfo[i].sock, &readfds);

            n_found = select(FD_SETSIZE, &readfds, NULL, NULL, NULL);

            if (n_waiters > 0)
                break;

            if (n_found > 0)
                check_for_pkt_common(&readfds);
        }
    }
}

#endif


/*
 * L I S T E N  (internal)
 *
 * Server listen loop.  Internal version.
 */

internal void rlisten()
{
    register rpc_$linked_pkt_t *pkt;
    rpc_$sockaddr_t from;
    status_$t st, fst;
    register sinfo_t *si;
#ifdef TASKING
    u_short this_listener;
#endif
    pfm_$cleanup_rec crec;
    u_int which;
    int n_found;
    u_short i;
    struct fd_set readfds;
    int nfds;
    rpc_$cksum_t cksum;

#ifdef TASKING
    this_listener = n_listeners++;
    listen_tasks[this_listener] = task_$get_handle();
    dprintf("(rlisten) task %08lx\n", listen_tasks[this_listener]);
    listen_task_starting = false;
#endif

    /*
     * Set up a backstop cleanup handler.  If we get a cleanup status that
     * results from a "quit_activity" signal, just ignore it.  Treat
     * the "check sockets" status similarly.  (See comment by "task_$signal" in
     * "use_family".) Otherwise if we get a cleanup status of non-zero
     * (non-OK), resignal; on zero cleanup statuses, just return -- someone
     * must have done an "rpc_$shutdown".
     */

    fst = pfm_$p_cleanup(&crec);
    if (fst.all == QUIT_ACTIVITY_FAULT) {
        dprintf("(rlisten) Spurious quit_activity fault ignored\n");
        pfm_$p_reset_cleanup(&crec, &st);
    }
#ifdef TASKING
    else if (fst.all == CHECK_SOCKETS_FAULT) {
        dprintf("(rlisten) Received check-sockets fault\n");
        pfm_$p_reset_cleanup(&crec, &st);
    }
#endif
    else if (fst.all != pfm_$cleanup_set) {
        if (fst.all != status_$ok && ! rpc_$termination_fault(fst)) {
#ifdef TASKING
            n_listen_failures++;
            eprintf("(rlisten) task %08lx exited, st=%08lx\n", listen_tasks[this_listener], fst);
#else
            eprintf("(rlisten) Exiting, st=%08lx\n", fst);
#endif
        }
#ifdef TASKING
        listen_tasks[this_listener] = -1;
#endif
        if (fst.all != status_$ok)
            pfm_$signal(fst);
        else {
            pfm_$enable();
            return;
        }
    }

    pkt = NULL;

    /*
     * Loop forever (or until shutdown), waiting for incoming packets.
     */

    do {

        /*
         * Wait for a packet.
         */

        nfds = 0;

        FD_ZERO(&readfds);

        for (i = 0; i < n_sockets; i++) {
            int s = sinfo[i].sock;

            if (s + 1 > nfds)
                nfds = s + 1;

            FD_SET(s, &readfds);
        }

        n_waiters++;

        n_found = select(nfds, &readfds, NULL, NULL, NULL);

        n_waiters--;

        if (n_found <= 0) {
            if (errno != EINTR)
                dprintf("(rlisten) select failed: %d, errno=%d\n", n_found, errno);
            continue;
        }

        for (which = 0; which < n_sockets; which++)
            if (FD_ISSET(sinfo[which].sock, &readfds))
                break;

        ASSERT(which < n_sockets);

        si = &sinfo[which];

        /*
         * Loop until there's nothing there.
         */

        do {
            if (pkt == NULL)
                pkt = rpc_$alloc_linked_pkt(NULL, (u_long) MAX_PKT_SIZE);

            cksum = NULL;

            if (! s_recv_pkt(si->sock, &pkt->pkt, &from, &cksum))
                break;

#ifdef TASKING

            /*
             * If there are no waiters, and we have a request packet, then
             * we see if we can start another listener.  If we've maxed
             * out, we prod the packet checker task (it'll eye out for
             * non-request packets).
             */

            if (n_waiters == 0 && pkt_type(&pkt->pkt) == rpc_$request)
                if (n_listeners >= max_listeners)
                    ec2_$advance(&check_for_pkts_ec, &st);
                else if (! listen_task_starting) {
                    char name[32];
                    extern void rlisten();

                    listen_task_starting = true;
                    sprintf(name, "RPC server listener #%d", n_listeners);
                    rpc_$create_task(rlisten, name);
                }

#endif

            /*
             * Process the packet.  If it got threaded onto a list, drop
             * our reference to it.
             */

            rpc_$int_listen_dispatch((u_long) si->sock, pkt, cksum, &from.sa, from.len, &st);

            if (pkt->refcnt > 1) {
                LOCK(SERVER_MUTEX);
                if (pkt->refcnt > 1) {
                    rpc_$free_linked_pkt(pkt);
                    pkt = NULL;
                }
                UNLOCK(SERVER_MUTEX);
            }

        } while (! shut_down);

    } while (! shut_down);

    pfm_$p_rls_cleanup(&crec, &st);

#ifdef TASKING
    listen_tasks[this_listener] = -1;
#endif
}


/*
 * R P C _ $ L I S T E N
 *
 * Server listen loop.  This call does not return.
 *
 * Get RPC call packets, dispatch them to server side stub.
 *
 * "max_calls" is the maximum number of calls the server should be allowed
 * to process concurrently.
 */

void rpc_$listen(max_calls, st)
u_long max_calls;
status_$t *st;
{
    register u_short i;

    CHECK_MISPACKED_HDR(true);

    /*
     * Initialize some static variables.
     */

    max_listeners = (max_calls > MAX_LISTENERS) ? MAX_LISTENERS : max_calls;
    shut_down = false;

#ifdef TASKING
    n_listeners = 0;
    n_listen_failures = 0;
    listen_task_starting = true;        /* In a way */
#endif

    n_waiters = 0;

#ifdef TASKING
    ec2_$init(&check_for_pkts_ec);
#endif

    /*
     * Initialize some stuff if this is the first time we've been called.
     * Schedule some asynchronous activity.  Register our willingness to
     * handle the "rrpc_" interface.
     */

    if (! initialized) {
#ifdef EMBEDDED_LLBD
        socket_$addr_t saddr;
        u_long slen;
        u_long n_families = socket_$num_families;
        socket_$addr_family_t families[socket_$num_families];
#endif

        rpc_$register(&rrpc_$if_spec, rrpc_$server_epv, st);

#ifdef EMBEDDED_LLBD
        socket_$valid_families(&n_families, families, st);

        /*
         * Currently under MSDOS, only have single process so llb interface
         * must be registered in this process.  Forwarding is moot.
         */

        rpc_$register(&llb_$if_spec, llb_$server_epv, st);
        slen = sizeof(saddr);
        rpc_$use_family_wk((u_long) families[0], &llb_$if_spec, &saddr, &slen, st);
#endif

        rpc_$start_activity_scanner();
#ifdef TASKING
        rpc_$create_task(check_for_pkts, "RPC ping checker");
#else
        rpc_$periodically(check_for_pkt, "RPC packet checker", CHECK_FOR_PKT_INTERVAL);
#endif
    }

    initialized = true;

    /*
     * Enter the listen loop.
     */

    rlisten();

#ifdef TASKING
    /*
     * Kill off all the listen tasks.
     */

    for (i = 1; i < n_listeners; i++)
        if (listen_tasks[i] != -1 && listen_tasks[i] != task_that_shut_down) {
            status_$t xst;
            xst.all = status_$ok;
            task_$signal(listen_tasks[i], xst, st);
        }
#endif

    st->all = status_$ok;
}


/*
 * R P C _ $ S H U T D O W N
 *
 * Stop processing incoming calls.  "rpc_$listen" returns.  In a tasking
 * environment, all "listen tasks" are killed.  This procedure can be called
 * from within a remoted procedure; the call completes and the server shuts
 * down after replying to the caller.
 */

void rpc_$shutdown(st)
status_$t *st;
{
    shut_down = true;

#ifdef TASKING

    task_that_shut_down = task_$get_handle();
    if (task_that_shut_down != listen_tasks[0]) {
        status_$t xst;
        xst.all = status_$ok;
        task_$signal(listen_tasks[0], xst, st);
    }
#endif

    st->all = status_$ok;
}


/*
 * R P C _ $ A L L O W _ R E M O T E _ S H U T D O W N
 *
 * Allow/disallow remote callers to shutdown server via "rrpc_$shutdown".
 * If "allow" is false, remote shutdowns are disallowed.  If "allow" is
 * "true" and "cproc" is nil, then remote shutdowns are allowed.  If "allow"
 * is "true" and "cproc" is non-nil, then when a remote shutdown request
 * arrives, the function denoted by "cproc" is called and the shutdown
 * is allowed iff the function returns "true" at that time.
 */

void rpc_$allow_remote_shutdown(allow, cproc, st)
u_long allow;
rpc_$shut_check_fn_t cproc;
status_$t *st;
{
    st->all = status_$ok;
    allow_remote_shutdown = allow;
    shut_check_fn = cproc;
}

/*
 * R R P C _ $ S H U T D O W N
 *
 * Shutdown server.  This is a remoted procedure.
 */

void rrpc_$shutdown(h, st)
handle_t h;
status_$t *st;
{
    if (! allow_remote_shutdown || (shut_check_fn != NULL && ! (*shut_check_fn)(h, st)))
        st->all = rrpc_$shutdown_not_allowed;
    else
        rpc_$shutdown(st);
}


/*
 * R R P C _ $ A R E _ Y O U _ T H E R E
 *
 * A trivial procedure to check to see if a server is answering requests.
 */

void rrpc_$are_you_there(h, st)
handle_t h;
status_$t *st;
{
    st->all = status_$ok;
}


/*
 * R R P C _ $ I N Q _ S T A T S
 *
 * Return some interesting statistics.
 */

void rrpc_$inq_stats(h, max_stats, stats, l_stat, st)
handle_t h;
u_long max_stats;
rrpc_$stat_vec_t stats;
long *l_stat;
status_$t *st;
{
#ifndef NO_STATS
#  define set_stat(i, v) if (max_stats >= i + 1) { stats[i] = v; *l_stat = i; }

    set_stat(rrpc_$sv_calls_in,         rpc_$stats.calls_in);
    set_stat(rrpc_$sv_rcvd,             rpc_$stats.rcvd);
    set_stat(rrpc_$sv_sent,             rpc_$stats.sent);
    set_stat(rrpc_$sv_calls_out,        rpc_$stats.calls_out);
    set_stat(rrpc_$sv_frag_resends,     rpc_$stats.frag_resends);
    set_stat(rrpc_$sv_dup_frags_rcvd,   rpc_$stats.dup_frags_rcvd);

#  undef set_stat
#else
    *l_stat = -1;
#endif

    st->all = status_$ok;
}

#ifdef GLOBAL_LIBRARY

/*
 * R P C _ $ L S N _ M A R K _ R E L E A S E
 *
 * Support for Apollo multiple programs per process ("program levels").
 *
 * This code does what's barely necessary so that you can invoke a server
 * sequentially in the same process.  It doesn't handle the case of a server
 * invoking another server in-process.  This seems pretty unlikely, no?
 *
 */

void rpc_$lsn_mark_release(is_mark, level, st, is_exec)
boolean *is_mark;
u_short *level;
status_$t *st;
boolean *is_exec;
{
    u_short i;

    st->all = status_$ok;

    if (! *is_mark) {
        n_sockets = 0;
        initialized = false;
        allow_remote_shutdown = false;
        shut_check_fn = NULL;
    }
}
#endif

#ifdef FTN_INTERLUDES

void rpc_$listen_recv_(sock, _pkt, cksum, from, from_len, ptype, obj, if_id, st)
u_long *sock;
rpc_$ppkt_p_t _pkt;
rpc_$cksum_t *cksum;
socket_$addr_t *from;
u_long *from_len;
u_long *ptype;
uuid_$t *obj;
uuid_$t *if_id;
status_$t *st;
{
    rpc_$listen_recv(*sock, _pkt, cksum, from, from_len, ptype, obj, if_id, st);
}

void rpc_$forward_(sock, from, from_len, to, to_len, _pkt, st)
u_long *sock;
socket_$addr_t *from;
u_long *from_len;
socket_$addr_t *to;
u_long *to_len;
rpc_$ppkt_p_t _pkt;
status_$t *st;
{
    rpc_$forward(*sock, from, *from_len, to, *to_len, _pkt, st);
}

void rpc_$shutdown_(st)
status_$t *st;
{
    rpc_$shutdown(st);
}

void rpc_$allow_remote_shutdown_(allow, cproc, st)
u_long *allow;
rpc_$shut_check_fn_t cproc;
status_$t *st;
{
    rpc_$allow_remote_shutdown(*allow, cproc, st);
}

void rpc_$use_family_(family, saddr, slen, st)
u_long *family;
socket_$addr_t *saddr;
u_long *slen;
status_$t *st;
{
    rpc_$use_family(*family, saddr, slen, st);
}

void rpc_$use_family_wk_(family, ifspec, saddr, slen, st)
u_long *family;
rpc_$if_spec_t *ifspec;
socket_$addr_t *saddr;
u_long *slen;
status_$t *st;
{
    rpc_$use_family_wk(*family, ifspec, saddr, slen, st);
}

void rpc_$listen_(max_calls, st)
u_long *max_calls;
status_$t *st;
{
    rpc_$listen(*max_calls, st);
}

#endif

