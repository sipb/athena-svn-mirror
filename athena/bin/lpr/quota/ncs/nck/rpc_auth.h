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
 * Interface to NCK authentication hooks.
 *
 * This whole file should probably be merged into rpc.idl
 */

#ifndef PROTOTYPE
#  ifndef __STDC__
#    define PROTOTYPE(x) ()
#  else
#    define PROTOTYPE(x) x
#  endif
#endif

    /*
     * Authentication is handled on a per-activity basis; while this
     * is arguably not necessarily the right thing (for example, it
     * means that multiple tasks in a client each have to
     * authenticate separately), it required the minimal change to
     * the existing architecture, as the per-connection
     * state on the server is associated with activities.
     *
     * These hooks are designed to allow support for different
     * authentication protocols.
     * 
     * It is not intended that end-users call these hooks directly;
     * instead, writers of authentication protocols should write
     * wrappers around these hooks.
     *
     * The kinds of wrappers needed should include, at a minimum:
     *
     * - a routine called during server initialization to allow the
     * use of this authentication protocol in calls to the server.
     *
     * - a routine to call inside the body of a remoted procedure (on
     * the server) to get information about the client (presumably
     * passed securely over the network).
     * 
     * - a routine to call to set up a particular client handle as
     * using this authentication protocol.
     *
     * An authentication protocol is identified by a one-byte integer
     * (0..255); legal values for this are assigned by Apollo.
     * A value of 0 indicates "no authentication"; these hooks are not used.
     *
     * The per-connection state is kept in data structures which
     * also contain pointers to the functions which implement the
     * operations for each type.  This is intended to form a "closure"
     * or an "object".
     *
     * There are three distinct types involved:
     *
     * rpc_$encr_t:  common to both client and server; handles any 
     *          per-packet encrypted verifiers which may be added.
     *
     * rpc_$client_auth_t: contains authentication state for the
     *          client side of a "connection".
     *
     * rpc_$server_auth_t: contains authentication state for the
     *          server side of a "connection".
     *
     * For the most part, the client and server types are used at
     * connection setup time only.
     *
     */

/*
 * Maximum allowable authentication type
 */
#define rpc_$max_auth_type 256

/*
 * Encryption support: rpc_$encr_t.
 *
 * One of these structures may be attached to the client side handle
 * or server activity info.
 *
 * "auth_type" is the code for the authentication type in question.
 *
 * "encr_overhead" is the size, in bytes, of the extra space needed
 * to add a verifier in each packet.
 *
 * "send_xform" is called immediately prior to a packet being sent.
 * It is passed a pointer to and the count of the exact bytes being
 * sent (exclusive of verifier), and a pointer to where it should
 * place the verifier; it should compute the verifier and place it
 * where told.
 *
 * "prexform" and "recv_xform" work together to invert what "send_xform"
 * does.
 *
 * Because the current NCK implementation unmarshalls
 * the RPC packet header in-place (and also modifies it during processing!),
 * it is necessary to do some of the work on the original bytes before
 * this unmarshalling takes place.  Because it is necessary to
 * unmarshall the header before the activity UUID can be read, and the
 * per-connection state discovered, this means that the operation has
 * to be split into two parts.
 *
 * prexform creates a "checksum object" (represented as a generic
 * pointer, rpc_$cksum_t) containing a "checksum" of the packet.
 * 
 * recv_xform is passed the connection state, the client's data
 * representation, a pointer to the verifier, and the checksum
 * returned by "prexform.
 *
 * "destroy_xform" is called to free up the resources allocated in prexform.
 *
 * "destroy" destroys the entire object.  If you feel paranoid, you
 * should overwrite any "secrets" with zeros before freeing the
 * storage allocated.
 */

typedef struct rpc_$encr_epv_t {
    u_long auth_type;
    u_long encr_overhead;
    void (*send_xform) PROTOTYPE((struct rpc_$encr_t *encr, char *pkt,
                                  u_long len, u_long seq, char *pkt_cksum, status_$t *st));
    void (*recv_xform) PROTOTYPE((struct rpc_$encr_t *encr, 
                                  rpc_$drep_t drep,
                                  char *pkt_cksum, rpc_$cksum_t real_cksum, u_long *seq, status_$t *st));
    void (*destroy) PROTOTYPE((struct rpc_$encr_t *encr));
    rpc_$cksum_t (*prexform) PROTOTYPE((char *pkt, u_long len, status_$t *st));
    void (*destroy_prexform) PROTOTYPE((rpc_$cksum_t));
    /*
     * XXX should perhaps also include routines for stubs to call to
     * encrypt/decrypt stuff.
     */
} rpc_$encr_epv_t;

typedef struct rpc_$encr_t {
    rpc_$encr_epv_t *epv;
} rpc_$encr_t;

/*
 * Syntactic sugar for invoking the above operations.
 */

#define rpc_$encr_send_xform(encr, pkt, len, seq, pkt_cksum, st) \
            (*(encr)->epv->send_xform)((encr), pkt, len, seq, pkt_cksum, st)
#define rpc_$encr_recv_xform(encr, drep, pkt_cksum, real_cksum, seq, st) \
            (*(encr)->epv->recv_xform)((encr), drep, pkt_cksum, real_cksum, seq, st)
#define rpc_$encr_destroy(encr) \
            (*(encr)->epv->destroy)(encr)
#define rpc_$encr_overhead(encr) \
            ((encr)->epv->encr_overhead)
#define rpc_$auth_type(encr) \
            ((encr)->epv->auth_type)

#define rpc_$encr_type_overhead(type) \
            (rpc_$encr_rgy[type]->encr_overhead)
#define rpc_$prexform(type, pkt, len, st) \
            (*rpc_$encr_rgy[type]->prexform)(pkt, len, st)
#define rpc_$destroy_prexform(type, ptr) \
            (*rpc_$encr_rgy[type]->destroy_prexform)(ptr)

/*
 * This routine is called to let the runtime know about a new
 * encryption type.
 */

void rpc_$register_encrtype
    PROTOTYPE ((u_long auth_type, rpc_$encr_epv_t *epv, status_$t *st));

/*
 * set_encr, inq_encr will become exported i/f's;
 * {set,inq}_server_encr will remain private.
 */

void rpc_$set_encr 
    PROTOTYPE((handle_t h, rpc_$encr_t *encr, status_$t *st));
void rpc_$set_server_encr 
    PROTOTYPE((handle_t h, rpc_$encr_t *encr, status_$t *st));
    
rpc_$encr_t *rpc_$inq_encr 
    PROTOTYPE((handle_t h, status_$t *st));
rpc_$encr_t *rpc_$inq_server_encr 
    PROTOTYPE((handle_t h, status_$t *st));

/*
 * Client side state:
 *
 * "get_encr" builds the encryption structure and returns it.
 * "destroy" destroys the client-side state; it is invoked
 * automatically when a handle is destroyed.
 */
typedef struct rpc_$client_auth_epv_t {
    struct rpc_$encr_t *(*get_encr) PROTOTYPE((struct rpc_$client_auth_t *a, 
                                               handle_t h, status_$t *st));
    void (*destroy) PROTOTYPE((struct rpc_$client_auth_t *a));
} rpc_$client_auth_epv_t;

typedef struct rpc_$client_auth_t {
    rpc_$client_auth_epv_t *epv;
} rpc_$client_auth_t;

/*
 * Syntactic sugar for invoking the above operations.
 */

#define rpc_$auth_get_encr(auth, h, st) \
            (*(auth)->epv->get_encr)((auth),h,st)
#define rpc_$auth_destroy(auth) \
            (*(auth)->epv->destroy)(auth)

void rpc_$set_auth 
    PROTOTYPE((handle_t h, rpc_$client_auth_t *auth, status_$t *st));
rpc_$client_auth_t *rpc_$inq_auth 
    PROTOTYPE((handle_t h, status_$t *st));

/*
 * Server side state:
 *
 * get_encr is called to create and initialize the encryption structure.
 *
 * destroy is called to free up anything allocated; it is called
 * automatically when the server runtime throws away the per-client
 * connection state.
 */

typedef struct rpc_$server_auth_epv_t {
    rpc_$encr_t *(*get_encr)
        PROTOTYPE((struct rpc_$server_auth_t *a, status_$t *st));
    void (*destroy) PROTOTYPE((struct rpc_$server_auth_t *a));
} rpc_$server_auth_epv_t;

typedef struct rpc_$server_auth_t {
    rpc_$server_auth_epv_t *epv;
} rpc_$server_auth_t;

/*
 * Syntactic sugar for invoking the above oeprations.
 */

#define rpc_$sauth_get_encr(auth, st) \
            (*(auth)->epv->get_encr)((auth),st)
#define rpc_$sauth_destroy(auth) \
            (*(auth)->epv->destroy)(auth)

/*
 * Server side miscellany:
 *
 * Authentication state is set up by a callback from the server to the
 * client.  This is handled by replacing the existing conv_$who_are_you()
 * routine (a remoted routine) with an interlude to another remoted
 * call which does that conv_$who_are_you does, plus whatever the
 * authentication protocol needs.
 */

typedef rpc_$server_auth_t *(*rpc_$way_fn_t)
    PROTOTYPE((handle_t h,
               uuid_$t *actuid,
               u_long boot_time, u_long *seqno, 
               status_$t *st));

void rpc_$register_authtype
    PROTOTYPE ((u_long auth_type, rpc_$way_fn_t way, status_$t *st));
void rpc_$set_sauth
    PROTOTYPE((handle_t h, rpc_$server_auth_t *auth, status_$t *st));
rpc_$server_auth_t *rpc_$inq_sauth
    PROTOTYPE((handle_t h, status_$t *st));
