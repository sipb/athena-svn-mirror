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
 * Private declarations for the RPC runtime.
 */

#if defined(TASKING) && ! defined(apollo)
#  include <cps/cpsio.h>
#endif

#include "sysdep.h"
#include "std.h"

#include "pbase.h"

#ifdef apollo
#  include <apollo/proc2.h>
#  ifdef GLOBAL_LIBRARY
#    include <apollo/sys/ios.h>
#  else
#    include <apollo/ios.h>
#  endif
#  include <apollo/rws.h>
#endif

#include "pfm.h"

#ifdef TASKING
#  ifdef apollo
#    include <apollo/task.h>
#    include <apollo/mutex.h>
#    include <apollo/ec2.h>
#  else
#    include <cps/task.h>
#    include <cps/mutex.h>
#    include <cps/ec2.h>
#  endif
#endif

#ifdef DSEE
#  include "$(socket.idl).h"
#  include "$(rpc.idl).h"
#  include "$(conv.idl).h"
#  include "$(rrpc.idl).h"
#  include "$(uuid.idl).h"
#  include "$(fault.idl).h"
#else
#  include "socket.h"
#  include "rpc.h"
#  include "conv.h"
#  include "rrpc.h"
#  include "uuid.h"
#  include "fault.h"
#endif

#include "rpc_auth.h"

/* =============================================================================== */

    /*
     * Various macro hackery
     */

#ifdef DEBUG
#  define DEBUG_VAR         rpc_$debug
#else
#  define DEBUG_VAR         0
#endif

#ifdef NO_RPC_PRINTF
#  define rpc_$printf       printf
#endif

#define internal          static

#define DIE(text)           rpc_$die(text, __FILE__, __LINE__)

#define dprintf             (! DEBUG_VAR) ? 0 : rpc_$printf
#define eprintf             rpc_$printf

#define raise(st)           pfm_$signal(st)
#define raisec(l)           { status_$t _st; _st.all = (long) l; raise(_st); }

#define CLIENT_MUTEX        0
#define SERVER_MUTEX        1
#define LOCK(which)         rpc_$lock(which, __FILE__, __LINE__)
#define UNLOCK(which)       rpc_$unlock(which)

#ifdef apollo
#  define uidequal(u1, u2)  (u1 == u2)
#else
#  define uidequal(u1, u2)  (uuid_$equal(&(u1), &(u2)))
#endif

#ifndef __STDC__
#  define PROTOTYPE(x) ()
#else
#  define PROTOTYPE(x) x
#endif

/* =============================================================================== */

    /*
     * Common header for "struct server_handle_t" and "struct
     * client_handle_t".  The "data_offset" field must remain first since
     * this structure is overlayed with the "builtin" type "handle_t" (see
     * "idl_base.h") whose first field is "data_offset".  The "cookie"
     * field is used to validate the handles and distinguish the two types.
     */

typedef struct {
    u_short data_offset;    /* offset into packets that clients should use */
    u_long cookie;          /* magic cookie */
} handle_hdr_t;

#define CLIENT_COOKIE 0xeffaced
#define SERVER_COOKIE 0xc0c0a

/* =============================================================================== */

    /*
     * A sockaddr with a length
     */

typedef struct {
    u_long len;
    socket_$addr_t sa;
} rpc_$sockaddr_t;

#ifdef LESS_SPACE_MORE_TIME
#  define copy_sockaddr(sa1, len1, sa2, len2) \
    memcpy(sa2, sa1, (size_t) (*len2 = len1))
#else
#  define copy_sockaddr(sa1, len1, sa2, len2) { \
    u_short _i_; \
    char *s1 = (char *) sa1, *s2 = (char *) sa2; \
    *len2 = len1; \
    for (_i_ = len1; _i_ > 0; _i_--) \
        *s2++ = *s1++; \
}
#endif

#define copy_sockaddr_r(r1, r2) \
    copy_sockaddr(&((r1)->sa), (r1)->len, &((r2)->sa), &((r2)->len))

/* =============================================================================== */

    /*
     * Packet format definitions
     */

    /*
     * The current version of the NCA/RPC protocol
     */

#define RPC_VERS 4


    /*
     * The different kinds of RPC packets, annotated as to which direction they're
     * sent.
     */

typedef enum {
    rpc_$request,           /* 0  client -> server */
    rpc_$ping,              /* 1  client -> server */
    rpc_$response,          /* 2  server -> client */
    rpc_$fault,             /* 3  server -> client */
    rpc_$working,           /* 4  server -> client */
    rpc_$nocall,            /* 5  server -> client */
    rpc_$reject,            /* 6  server -> client */
    rpc_$ack,               /* 7  client -> server */
    rpc_$quit,              /* 8  client -> server */
    rpc_$fack,              /* 9  both directions  */
    rpc_$quack              /* 10 server -> client */
} rpc_$ptype_t;

#define MAX_PKT_TYPE ((int) rpc_$quack)


    /*
     * Packet header structure.
     *
     * Note that all the scalar fields are "naturally aligned" -- i.e.
     * aligned 0 MOD min(8, sizeof(field)).  This is done for maximum
     * portability and efficiency of field reference.  Note also that the
     * header is a integral multiple of 8 bytes in length.  This ensures
     * that (assuming the pkt is 8-byte aligned in memory), the start of
     * the data area is 8-byte aligned.  (8 bytes is the size of the largest
     * scalar we support.)  See important storage layout comments below.
     */

typedef union {
    double force_alignment;     /* Get highest alignment possible */
    struct {
        u_char  rpc_vers;       /* 00:01 RPC version */
        u_char  PTYPE;          /* 01:01 (rpc_$ptype_t) packet type */
        u_char  flags;          /* 02:01 flags (see PF_... below) */
        u_char  flags2;         /* 03:01 more flag (see PF2_... below) */
        u_char  drep[4];        /* 04:04 data type format of sender (see below) */
        uuid_$t object;         /* 08:16 object UID */
        uuid_$t if_id;          /* 24:16 interface UID */
        uuid_$t actuid;         /* 40:16 activity UID of caller */
        u_long  server_boot;    /* 56:04 time server booted */
        u_long  if_vers;        /* 60:04 version of interface */
        u_long  seq;            /* 64:04 sequence # -- monotonically increasing */
        u_short opnum;          /* 68:02 operation # within the trait */
        u_short ihint;          /* 70:02 interface hint (which interface w/in server) */
        u_short ahint;          /* 72:02 activity hint */
        u_short len;            /* 74:02 length of body */
        u_short fragnum;        /* 76:02 fragment # */
        u_char  auth_type;      /* 78:01 authentication type */
        u_char  pad3;           /* 79:01 pad to 8 byte boundary */
    } HDR;
} rpc_$pkt_hdr_t;

#define pkt_type(pkt)           ((rpc_$ptype_t) (pkt)->HDR.HDR.PTYPE)
#define set_pkt_type(pkt, pt)   (pkt)->HDR.HDR.PTYPE = (u_char) pt

#define NO_HINT ((u_short) 0xffff)

    /*
     * Packet flags (used in "flags" field in packet header), indicating
     * in packets of which direction(s) they're used.  (The non-bidirectional
     * flags are ignored if set in packets of the wrong direction.)
     *
     * Note on PF_BLAST_OUTS:  Early versions of NCK did not blast large
     * ins/outs but were designed to work with senders that did blast large
     * ins/outs.  (I.e. early versions supported the PF_NO_FACK bit which
     * is now used by new senders that want to blast multiple frags.)
     * Unfortunately, while the design was correct, the implementation
     * wasn't.  Old clients have a bug in handling blasted outs.  So by
     * default, new servers won't blast large outs.  The PF_BLAST_OUTS
     * flag is set by new clients to tell the server that the client can
     * handle blasts.
     *
     * Note on forwarding:  Forwarding is a purely intra-machine function;
     * i.e. The PF_FORWARDED and PF2_FORWARDED2 bits should never be set
     * on packets sent on the wire.  It's pretty sleazy to be stealing
     * these bits (i.e. make them unavailable for the on-the-wire protocol),
     * but that's life.  If in the future we're willing to go through
     * more work (i.e. do intra-machine forwarding through some other means),
     * we could free up those bits.  See comments by definition of
     * "rpc_$forward" to understand forwarding.
     */

#define PF_FORWARDED    0x01        /* (client -> server) Packet was forwarded */
#define PF_LAST_FRAG    0x02        /* (both directions)  Packet is the last fragment */
#define PF_FRAG         0x04        /* (both directions)  Packet is a fragment */
#define PF_NO_FACK      0x08        /* (both directions)  Don't send an FACK for this FRAG */
#define PF_MAYBE        0x10        /* (client -> server) "maybe" request */
#define PF_IDEMPOTENT   0x20        /* (client -> server) "idempotent" request */
#define PF_BROADCAST    0x40        /* (client -> server) "broadcast" request */
#define PF_BLAST_OUTS   0x80        /* (client -> server) out's can be blasted */

    /*
     * Packet flags (used in "flags2" field in packet header).
     */

#define PF2_FORWARDED_2 0x01        /* (client -> server) Packet is being forwarded in two pieces */
#define PF2_RESERVED02  0x02
#define PF2_RESERVED04  0x04
#define PF2_RESERVED08  0x08
#define PF2_RESERVED10  0x10
#define PF2_RESERVED20  0x20
#define PF2_RESERVED40  0x40
#define PF2_RESERVED80  0x80

    /*
     * Data representation descriptor (drep)
     *
     * Note that the form of a drep "on the wire" is not captured by the
     * the "rpc_$drep_t" data type.  The actual structure -- a "packed
     * drep" -- is a vector of four bytes:
     *
     *      | MSB           LSB |
     *      |<---- 8 bits ----->|
     *      |<-- 4 -->|<-- 4 -->|
     *
     *      +---------+---------+
     *      | int rep | chr rep |
     *      +---------+---------+
     *      |     float rep     |
     *      +-------------------+
     *      |     reserved      |
     *      +-------------------+
     *      |     reserved      |
     *      +-------------------+
     *
     * The following macros manipulate data representation descriptors.
     * "copy_drep" copies one packed drep into another.  "unpack_drep"
     * copies from a packed drep into a variable of the type
     * "rpc_$drep_t".
     */

#ifdef CONVENTIONAL_ALIGNMENT
#  define copy_drep(dst, src) \
    (*((long *) (dst)) = *((long *) (src)))
#else
#  define copy_drep(dst, src) { \
    (dst)[0] = (src)[0]; \
    (dst)[1] = (src)[1]; \
    (dst)[2] = (src)[2]; \
    (dst)[3] = (src)[3]; \
  }
#endif

#define drep_int_rep(drep)   ((drep)[0] >> 4)
#define drep_char_rep(drep)  ((drep)[0] & 0xf)
#define drep_float_rep(drep) ((drep)[1] & 0xf)

#define unpack_drep(dst, src) {             \
    (dst)->int_rep   = drep_int_rep(src);   \
    (dst)->char_rep  = drep_char_rep(src);  \
    (dst)->float_rep = drep_float_rep(src); \
    (dst)->reserved  = 0;                   \
}


    /*
     * Packet header storage layout
     *
     * Not all C compilers (esp. those for machines whose smallest
     * addressible unit is not 8 bits) pack the following structure
     * "correctly" (i.e. into a storage layout that can be overlayed on
     * a vector of bytes that make up a packet that's just come off the
     * wire).  Thus, on some machines "rpc_$pkt_hdr_t" can not simply be
     * used on incoming packets (or used to set up outgoing packets).  We
     * call machines that have this problem "mispacked header machines".
     *
     * To fix this problem, on mispacked header machine, when a packet
     * arrives we expand the header bits (in place in the packet buffer)
     * so they correspond to the actual storage layout of "rpc_$pkt_hdr_t".
     * (This work happens in "rpc_$recvfrom" in "rpc_util.c".)  Analogously,
     * before sending a packet, we contract the header bits.  To minimize
     * data copying, we ensure that the packet buffer data type is slightly
     * larger than the largest possible packet that can appear on the wire.
     * We initiate receive operations slightly offset into the buffer thus
     * allowing us to expand the header "upwards" without having to move
     * the data part of the packet.  Analogously, on sends we pack the
     * header "downwards" and start the send operation slightly offset.
     *
     * PKT_HDR_SIZE is the actual size of a packet header (in bytes) as
     * it appears on the wire.  The size of "rpc_$pkt_hdr_t" can only be
     * the same or larger.  PKT_IO_OFFSET is the offset into the packet
     * buffer that we start I/O's to/from.
     *
     * NOTE VERY WELL that on mispacked header machines, the definition
     * of "rpc_$mispacked_hdr" in "rpc.idl" MUST be set to the difference
     * between the network and host header structure sizes.
     *
     * The CHECK_MISPACKED_HDR macro is used in a clever place in "rpc_server.c"
     * and "rpc_client.c".  It makes sure you haven't messed all this stuff
     * up and if you have, tells you what you should have done.
     */

#ifndef rpc_$mispacked_hdr
#  define rpc_$mispacked_hdr 0
#endif

#if rpc_$mispacked_hdr != 0
#  define MISPACKED_HDR
#endif

#define PKT_HDR_SIZE 80
#define PKT_IO_OFFSET rpc_$mispacked_hdr

#ifdef MAX_DEBUG
#  define CHECK_MISPACKED_HDR(junk) \
    if (PKT_HDR_SIZE + PKT_IO_OFFSET != sizeof(rpc_$pkt_hdr_t)) { \
        eprintf("\"rpc_$mispacked_hdr\" in \"rpc.idl\" should be %d (is currently %d)\n", \
                sizeof(rpc_$pkt_hdr_t) - PKT_HDR_SIZE, rpc_$mispacked_hdr); \
        DIE("mispacked header stuff messed up"); \
    }
#else
#  define CHECK_MISPACKED_HDR(junk)
#endif


    /*
     * Maximum packet size
     *
     * Due to a bug in old NIDL-generated stubs, we must pretend that packets
     * can be up to 8K even though the current "rpc.idl"
     * ("rpc_$max_pkt_size") says that datagrams are at most 1K long.  The
     * bug is that server stubs do "sizeof" to determine the output packet's
     * size rather than using the "omax" input parameter.  Formerly
     * "rpc_$max_pkt_size" was 8K; now it is 1K.  However, the result of
     * the "sizeof" expression was determined at compile time, not runtime,
     * so stubs that have not been recompiled with the "rpc.h" derived
     * from the new (1K) "rpc.idl" assume they have an 8K output buffer.
     * For compatibility, we must continue to provide them with an 8K buffer.
     * (The alternative would be to require that all stubs be recompiled.)
     */

#ifdef OLD_NIDL_STUBS
#  define MAX_PKT_SIZE (8 * 1024)
#else
#  define MAX_PKT_SIZE rpc_$max_pkt_size
#endif


    /*
     * Packet body structure
     *
     * The "body" part of a pkt is used only for certain pkt types (e.g.
     * request, response).
     *
     * Note that the actual maximum body size is (2**32)-1 (determined
     * by the fact that we store the length in a long).  However, in this
     * declaration the size of the body is declared to be smaller so that
     * local, non-pointer declarations of "rpc_$pkt_t" can be made.  (Stack
     * variable of size (2**32)-1 bytes don't fly.)  The upshot is that
     * the length used here yields an "rpc_$pkt_t" whose length is the
     * size of the largest datagram we expect to send or receive + the
     * PKT_IO_OFFSET (see above coments under "Packet header storage
     * layout").
     */

#define rpc_$max_body_size ((u_long) 0xffffffff)

typedef struct {
    char args[MAX_PKT_SIZE - sizeof(rpc_$pkt_hdr_t)];
} rpc_$pkt_body_t;


    /* "hdr"
     *
     * Note that in all the packet structs, the packet header field is
     * named "HDR".  We define a macro "hdr" which is "HDR.HDR" so that
     * all references to header fields can be of the form "hdr.fieldname"
     * rather than "HDR.HDR.fieldname" (i.e. so they can be oblivious
     * to the gross union hack in "rpc_$pkt_hdr_t").  "hdr" is an unfortunate
     * choice for a macro name.  This should be changed sometime.
     */

#define hdr HDR.HDR

    /*
     * Packet structure
     */

typedef struct {
    rpc_$pkt_hdr_t HDR;
    rpc_$pkt_body_t body;
} rpc_$pkt_t;

    /*
     * Small packet structure
     *
     * Used for packets with little (e.g. reject, fault) or no (e.g. ping, fack)
     * body.  Used in local, non-pointer declarations; doesn't eat up stack space
     * like "rpc_$pkt_t".
     */

typedef struct {
    rpc_$pkt_hdr_t HDR;
    struct {
        u_char args[4];     /* sizeof status_$t on the wire */
    } body;
} rpc_$spkt_t;

    /*
     * Forwarded packet structure
     *
     * This format for local use between a forwarding agent (LLBD) and a real
     * server.  This format does not appear on the wire.
     */

typedef struct {
    rpc_$sockaddr_t addr;       /* Original sender of packet */
    u_char drep[4];             /* Original data rep (describes body) */
} rpc_$fpkt_hdr_t;

typedef struct {
    rpc_$pkt_hdr_t HDR;
    rpc_$fpkt_hdr_t fhdr;
    rpc_$pkt_body_t body;
} rpc_$fpkt_t;

    /*
     * Packet structure for local use on a linked list of packets
     */

typedef struct linked_pkt_t {
    struct linked_pkt_t *link;
    u_short refcnt;
    u_short pad;
    rpc_$pkt_t pkt;
} rpc_$linked_pkt_t;

/* =============================================================================== */

    /*
     * Maximum number of frag packets that will be sent in a single "blast".
     */

#define MAX_PKTS_PER_BLAST 8

    /*
     * Linked list of request or response fragments.
     */

typedef struct {
    rpc_$linked_pkt_t *head;    /* head of frag list */
    rpc_$linked_pkt_t *tail;    /* tail of frag list */
    u_long len;                 /* # of frag body bytes so far */
    short highest;              /* # of the highest consec frag seen so far */
    short last;                 /* # of the last frag (-1 if unknown) */
} rpc_$frag_list_t;

/* =============================================================================== */

#define QUIT_ACTIVITY_FAULT (rpc_$mod + 0xffff)
#define CHECK_SOCKETS_FAULT (rpc_$mod + 0xfffe)

/* =============================================================================== */

globalref rpc_$encr_epv_t **rpc_$encr_rgy;

/* =============================================================================== */

    /*
     * ANSI interface declarations for rpclib modules not defined in "rpc.idl"
     * ("rpc.h") -- i.e. not exported to users.
     */

        /*
         * Interface for rpc_client.c
         */

void rpc_$inq_object_client
    PROTOTYPE((handle_t h, uuid_$t *obj, status_$t *st));

        /*
         * Interface for rpc_lsn.c
         */


        /*
         * Interface for rpc_server.c
         */

void rpc_$int_listen_dispatch
    PROTOTYPE((u_long sock, rpc_$linked_pkt_t *pkt, rpc_$cksum_t cksum, socket_$addr_t *from, u_long from_len, status_$t *st));
void rpc_$start_activity_scanner
    PROTOTYPE((void));
void rpc_$get_my_activity
    PROTOTYPE((uuid_$t *actuid));
handle_t rpc_$server_to_client_handle
    PROTOTYPE((handle_t p, uuid_$t *actuid, u_long *seq, status_$t *st));
void rpc_$get_callers_addr
    PROTOTYPE((handle_t p, socket_$addr_t *addr, u_long *len));

#ifdef GLOBAL_LIBRARY
void rpc_$server_mark_release
    PROTOTYPE((boolean *is_mark, u_short *level, status_$t *st, boolean *is_exec));
#endif

        /*
         * Interface for rpc_util.c
         */

#ifndef NO_RPC_PRINTF
/*lint +fvr  Lint will ignore variable return value use */
int rpc_$printf
    PROTOTYPE((char *format, ...));
/*lint -fvr */
#endif
void rpc_$die
    PROTOTYPE((char *text, char *file, int line));
#ifdef DEBUG
char *rpc_$pkt_name
    PROTOTYPE((rpc_$ptype_t ptype));
#else
#define rpc_$pkt_name(junk) ""
#endif
void rpc_$lock
    PROTOTYPE((int which, char *file, int line));
void rpc_$unlock
    PROTOTYPE((int which));
void rpc_$set_pkt_body_st
    PROTOTYPE((rpc_$pkt_t *pkt, u_long st_all));
void rpc_$get_pkt_body_st
    PROTOTYPE((rpc_$pkt_t *pkt, status_$t *st));
int rpc_$recvfrom
    PROTOTYPE((int s, rpc_$pkt_t *pkt, u_long pmax, rpc_$sockaddr_t *from, rpc_$cksum_t *cksum));
boolean rpc_$sendto
    PROTOTYPE((int s, rpc_$pkt_t *pkt, rpc_$sockaddr_t *to, rpc_$encr_t *encr));
void rpc_$free_frag_list
    PROTOTYPE((rpc_$frag_list_t *fl));
void rpc_$insert_in_frag_list
    PROTOTYPE((rpc_$frag_list_t *fk, rpc_$linked_pkt_t *pkt, boolean *complete));
rpc_$pkt_t *rpc_$reassemble_frag_list
    PROTOTYPE((rpc_$frag_list_t *fl));
void rpc_$periodically
    PROTOTYPE((void (*proc)(), char *name, u_long interval));
void rpc_$stop_periodic
    PROTOTYPE((void (*proc)()));
void rpc_$create_task
    PROTOTYPE((void (*proc)(), char *name));
boolean rpc_$fill_frag
    PROTOTYPE((rpc_$pkt_t *lpkt, u_long llen, u_short fragnum, u_long mtu, rpc_$pkt_t *pkt));
void rpc_$status_print
    PROTOTYPE((char *text, status_$t st));
rpc_$linked_pkt_t *rpc_$alloc_linked_pkt
    PROTOTYPE((rpc_$pkt_t *cpkt, u_long len));
void rpc_$free_linked_pkt
    PROTOTYPE((rpc_$linked_pkt_t *pkt));
boolean rpc_$termination_fault
    PROTOTYPE((status_$t st));
void rpc_$swab_header
    PROTOTYPE((rpc_$pkt_t *pkt));
#ifdef GLOBAL_LIBRARY
void rpc_$global_lib_init
    PROTOTYPE((void));
#endif

void *rpc_$malloc
    PROTOTYPE((int));
void rpc_$free
    PROTOTYPE((void *));

#if defined(MSDOS) && !defined(NO_NEAR)
void NEAR *rpc_$nmalloc
    PROTOTYPE((int));
void rpc_$nfree
    PROTOTYPE((void NEAR *));
#else
#  define rpc_$nmalloc rpc_$malloc
#  define rpc_$nfree rpc_$free
#endif

/* =============================================================================== */

    /*
     * Global variable definitions
     */


    /*
     * The nil UUID (global variable)
     */

globalref uuid_$t uuid_$nil;

    /*
     * Local data representation
     *
     * Macro values are defined in "sysdep.h".  "rpc_$local_drep_packed"
     * is the form that appears in packet headers; "rpc_$local_drep" is
     * the form used by stubs (and others).
     */

globalref u_char rpc_$local_drep_packed[4];

globalref rpc_$drep_t rpc_$local_drep;

    /*
     * ASCII<->EBCDIC character translation tables.
     */

globalref char rpc_$ascii_to_ebcdic[];

globalref char rpc_$ebcdic_to_ascii[];

#ifndef NO_STATS

    /*
     * Statistics of all forms
     */

typedef struct {
    u_long calls_out;           /* # of remote calls made */
    u_long calls_in;            /* # of remote calls processed */
    u_long sent;                /* Total # of pkts sent */
    u_long rcvd;                /* Total # of pkts rcvd */
    struct pkt_stats_t {        /* Breakdown of pkts sent/rcvd by pkt type */
        u_long sent;
        u_long rcvd;
    } pstats[MAX_PKT_TYPE + 1];
    u_long frag_resends;        /* # of frag sends that were dups of previous send */
    u_long dup_frags_rcvd;      /* # of duplicate frags rcvd */
} NEAR rpc_$stats_t;

globalref rpc_$stats_t rpc_$stats;

#endif

    /*
     * Debug flag
     *
     * If true, and library was compiled with -DDEBUG, various useful
     * information will be printed at various times.  Note that this variable
     * is always defined (i.e. not just under DEBUG) because it is marked
     * by the global library building procedure.
     */

globalref boolean rpc_$debug;      /* Always defined because of global library marking */

    /*
     * Lossy flag
     *
     * If true, and library was compiled with -DDEBUG, artificial send failures
     * will be generated for the purposes of debugging.
     */

globalref boolean rpc_$lossy;
