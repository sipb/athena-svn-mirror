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
 * R P C _ U T I L
 *
 * Utilities used by client and server.
 */

#include "rpc_p.h"

#ifdef MSDOS
#  include "rpc_seq.c"  /* only data in module causes probs for MSC */
#endif

#ifdef MSDOS
#define PATH_DELIM	'\\'
#else
#ifdef vms
#define PATH_DELIM	']'
#else
#define PATH_DELIM	'/'
#endif
#endif

/*
 * Byte swapping macros.
 */

#ifndef cray

#ifdef MSDOS
#  define swab_16(p) (*(p) = swab_$short(*(p)))
#  define swab_32(p) (*(p) = swab_$long(*(p)))
extern u_short swab_$short(u_short);
extern u_long swab_$long(u_long);
#else
#define swab_16(p) { \
    register char t; \
    t = ((char *) (p))[0]; \
    ((char *) (p))[0] = ((char *) (p))[1]; \
    ((char *) (p))[1] = t; \
}

#define swab_32(p) { \
    register char t; \
    t = ((char *) (p))[0]; \
    ((char *) (p))[0] = ((char *) (p))[3]; \
    ((char *) (p))[3] = t; \
    t = ((char *) (p))[1]; \
    ((char *) (p))[1] = ((char *) (p))[2]; \
    ((char *) (p))[2] = t; \
}
#endif

#else

#define swab_16(p) { \
    u_long l1 = *(u_long *)(p); \
    u_long l2; \
    char *q1 = (char *) &l1; \
    char *q2 = (char *) &l2; \
    q2[6] = q1[7]; \
    q2[7] = q1[6]; \
    *(u_long *)(p) = l2; \
}

#define swab_32(p) { \
    u_long l1 = *(u_long *)(p); \
    u_long l2; \
    char *q1 = (char *) &l1; \
    char *q2 = (char *) &l2; \
    q2[4] = q1[7]; \
    q2[5] = q1[6]; \
    q2[6] = q1[5]; \
    q2[7] = q1[4]; \
    *(u_long *)(p) = l2; \
}

#endif

#define swab_uuid(p) { \
    swab_32(&(p)->time_high); \
    swab_16(&(p)->time_low); \
}

#ifdef TASKING

#ifdef apollo

/*
 * Alias malloc/free to make rws_$ calls on Apollos when tasking is enabled
 * because the rws_$ calls have been made reentrant and malloc/free hasn't
 * (yet).
 */

#define malloc(n) \
    rws_$alloc_heap_pool(rws_$stream_tm_pool, (long) (n))
#define free(p) { \
    status_$t status; \
    rws_$release_heap_pool((p), rws_$stream_tm_pool, &status); \
}

#endif

#endif


/*
 * Private RPC utility data.
 *
 * On Apollos, this data with NOT be in the DATA$ section and hence will
 * be per-process when this module is used in a global library.
 */

    /*
     * Data used by rpc_$lock / unlock.
     */

#ifdef TASKING
#define MUTEX_TIMEOUT (4 * 120)         /* 1/4 sec ticks -- 2 minutes */
internal mutex_$lock_rec_t mutex[2];
internal boolean inited_mutex;
#endif

    /*
     * Stuff used by rpc_$periodically.
     */

#ifdef TASKING

#define TASK_STACK_SIZE    (64 * 1024)
#define TASK_PRIORITY      1

typedef struct {
    void (*proc)();
    u_short interval;
    char *name;
} periodic_info_t;

#else

#define MAX_TASKS 5

typedef struct {
    void (*proc)();                 /* Procedure to call */
    u_long interval;                /* Time between calls */
    u_long next_time;               /* Next time to call */
} NEAR task_db_elt_t;

internal task_db_elt_t task_db[MAX_TASKS];

internal boolean NEAR alarm_set = false; /* Has alarm(2) been called yet? */
internal u_short NEAR n_tasks;           /* # of periodic tasks set up */
internal int NEAR old_alarm_value;       /* Saved alarm(2) value */
internal (* NEAR old_alarm_handler)();    /* Saved SIGALRM handler */

#endif

#ifdef NON_REENTRANT_SOCKETS

    /*
     * On NON_REENTRANT_SOCKETS systems, a weak attempt will be made to
     * make sure the socket I/O routines are not call recursively (e.g.
     * from an interrupt).
     */

internal boolean NEAR in_sockets = false;
#define IN_SOCKETS(x) in_sockets = x

#else

#define IN_SOCKETS(x)

#endif

    /*
     * The following variables are the "defining instance" and static
     * initialization of variables mentioned (and documented) in "rpc_p.h".
     */

globaldef rpc_$encr_epv_t **rpc_$encr_rgy = NULL;

globaldef uuid_$t uuid_$nil = {0};

globaldef u_char rpc_$local_drep_packed[4] = {
    (INT_REP << 4) | CHAR_REP,
    FLOAT_REP,
    0,
    0,
};

globaldef rpc_$drep_t rpc_$local_drep = {
    INT_REP,
    CHAR_REP,
    FLOAT_REP,
    0
};

globaldef char rpc_$ascii_to_ebcdic[] = {
     0x00, 0x01, 0x02, 0x03, 0x37, 0x2D, 0x2E, 0x2F,
     0x16, 0x05, 0x25, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
     0x10, 0x11, 0x12, 0x13, 0x3C, 0x3D, 0x32, 0x26,
     0x18, 0x19, 0x3F, 0x27, 0x1C, 0x1D, 0x1E, 0x1F,
     0x40, 0x5A, 0x7F, 0x7B, 0x5B, 0x6C, 0x50, 0x7D,
     0x4D, 0x5D, 0x5C, 0x4E, 0x6B, 0x60, 0x4B, 0x61,
     0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
     0xF8, 0xF9, 0x7A, 0x5E, 0x4C, 0x7E, 0x6E, 0x6F,
     0x7C, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
     0xC8, 0xC9, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,
     0xD7, 0xD8, 0xD9, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6,
     0xE7, 0xE8, 0xE9, 0xAD, 0xE0, 0xBD, 0x5F, 0x6D,
     0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
     0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
     0x97, 0x98, 0x99, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6,
     0xA7, 0xA8, 0xA9, 0xC0, 0x4F, 0xD0, 0xA1, 0x07
};

globaldef char rpc_$ebcdic_to_ascii[] = {
     0x20, 0x01, 0x02, 0x03, 0x3F, 0x09, 0x3F, 0x10,
     0x3F, 0x3F, 0x3F, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
     0x10, 0x11, 0x12, 0x13, 0x3F, 0x3F, 0x08, 0x3F,
     0x18, 0x19, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
     0x3F, 0x3F, 0x1C, 0x3F, 0x3F, 0x3F, 0x17, 0x1B,
     0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x05, 0x06, 0x07,
     0x00, 0x00, 0x16, 0x00, 0x3F, 0x1E, 0x3F, 0x04,
     0x3F, 0x3F, 0x3F, 0x3F, 0x14, 0x15, 0x00, 0x1A,
     0x20, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
     0x3F, 0x3F, 0x3F, 0x2E, 0x3C, 0x28, 0x2B, 0x7C,
     0x26, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
     0x3F, 0x3F, 0x21, 0x24, 0x2A, 0x29, 0x3B, 0x5E,
     0x2D, 0x2F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
     0x3F, 0x3F, 0x3F, 0x2C, 0x25, 0x5F, 0x3E, 0x3F,
     0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
     0x3F, 0x60, 0x3A, 0x23, 0x40, 0x27, 0x3D, 0x22,
     0x3F, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
     0x68, 0x69, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
     0x3F, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70,
     0x71, 0x72, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
     0x3F, 0x7E, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
     0x79, 0x7A, 0x3F, 0x3F, 0x3F, 0x5B, 0x3F, 0x3F,
     0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
     0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x5D, 0x3F, 0x3F,
     0x7B, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
     0x48, 0x49, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
     0x7D, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50,
     0x51, 0x52, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
     0x5C, 0x3F, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
     0x59, 0x5A, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
     0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
     0x38, 0x39, 0x7C, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F
};

#ifndef NO_STATS
globaldef rpc_$stats_t rpc_$stats = {0};
#endif

globaldef boolean rpc_$debug = {false};

globaldef u_long rpc_$max_debug = {0};

globaldef boolean rpc_$lossy = {false};


#ifdef GLOBAL_LIBRARY
#include "set_sect.pvt.c"
#endif

#ifdef __STDC__
#ifdef MISPACKED_HDR
internal void pack_hdr_uuid(char *p, u_int offset, uuid_$t *val);
internal void pack_hdr(rpc_$pkt_t *pkt);
internal void unpack_hdr_uuid(char *p, u_int offset, uuid_$t *val, boolean little);
internal void unpack_hdr(rpc_$pkt_t *pkt);
#endif
internal void periodic_task(char *_info, long len);
internal void handle_alarm(void);
#endif

#ifdef MAX_DEBUG
extern void rpc_$dump_pkt_hdr();
extern void rpc_$dump_raw_pkt();
#endif

#ifndef NO_RPC_PRINTF

/*
 * R P C _ $ P R I N T F
 */

int rpc_$printf(format, args)
char *format;
int *args;
{
    struct timeval tv;
    
#ifdef TASKING

    char buff[200], buff2[200];

    vsprintf(buff, format, &args);
    sprintf(buff2, "[%08x] %s", task_$get_handle(), buff);
    write(2, buff2, strlen(buff2));

#else

    gettimeofday(&tv, NULL);
    fprintf(stderr, "%d.%06.6d: ", tv.tv_sec, tv.tv_usec);
#if defined(MSDOS) || defined(SYS5) || defined(vms)
    vfprintf(stderr, format, (char *) &args);
#else
    _doprnt(format, &args, stderr);
#endif
    fflush(stderr);     /* Just in case */

#endif
}

#endif


/*
 * R P C _ $ D I E
 *
 */

void rpc_$die(text, file, line)
char *text;
char *file;
int line;
{
    char *p = rindex(file, PATH_DELIM);

    eprintf("(rpc) *** FATAL ERROR \"%s\" at %s\\%d ***\n",
            text, p == NULL ? file : p + 1, line);
    abort();
}


#ifdef DEBUG

/*
 * R P C _ $ P K T _ N A M E S
 *
 * Return the string name for a type of packet.  This can't simply be a variable
 * because of the vagaries of global libraries.
 */

char *rpc_$pkt_name(ptype)
rpc_$ptype_t ptype;
{
    static char *names[MAX_PKT_TYPE + 1] = {
        "request",
        "ping",
        "response",
        "fault",
        "working",
        "nocall",
        "reject",
        "ack",
        "quit",
        "fack",
        "quack"
    };

    return((int) ptype > MAX_PKT_TYPE ? "BOGUS PACKET TYPE" : names[(int) ptype]);
}

#endif


/*
 * R P C _ $ L O C K / U N L O C K
 *
 * LOCK and UNLOCK are used to insure that only one thread of execution
 * is occuring during the execution of many of the RPC runtime routines.
 * Typically, LOCK is done early; UNLOCK/LOCK pairs surrounds
 * time-consuming but "safe" code; finally, UNLOCK is done at exit.
 *
 * On Apollos, we use the mutex facility.  On vanilla Unix, we simply mask
 * out SIGALRM since that's the only source of multiple threads of execution.
 */

void rpc_$lock(which, file, line)
int which;
char *file;
int line;
{
#ifdef TASKING
    time_$clock_t timeout;

    if (! inited_mutex) {
        inited_mutex = true;
        mutex_$init(&mutex[CLIENT_MUTEX]);
        mutex_$init(&mutex[SERVER_MUTEX]);
    }

    /*
     * If debugging, wait forever -- we might be under a debugger.  Otherwise
     * wait a prudent amount of time.
     */

    if (DEBUG_VAR) {
        time_$high32(timeout) = -1;
        time_$low16(timeout)  = -1;
    }
    else {
        time_$high32(timeout) = MUTEX_TIMEOUT;
        time_$low16(timeout)  = 0;
    }

    if (! mutex_$lock(&mutex[which], timeout)) {
        eprintf("(rpc_$lock) Can't get %d lock at %s\\%d\n", which, file, line);
        abort();
    }
#else
    pfm_$inhibit_faults();
#endif
}


void rpc_$unlock(which)
int which;
{
#ifdef TASKING
    mutex_$unlock(&mutex[which]);
#else
    pfm_$enable_faults();
#endif
}


/*
 * Byte offsets of the beginning of each logical field in the packet as
 * it appears on the wire.
 */

#define PHO_rpc_vers     0  /* u_char    */
#define PHO_ptype        1  /* u_char    */
#define PHO_flags        2  /* u_char    */
#define PHO_pad1         3  /* u_char    */
#define PHO_drep         4  /* u_char[4] */
#define PHO_object       8  /* uuid_$t   */
#define PHO_if_id       24  /* uuid_$t   */
#define PHO_actuid      40  /* uuid_$t   */
#define PHO_server_boot 56  /* u_long    */
#define PHO_if_vers     60  /* u_long    */
#define PHO_seq         64  /* u_long    */
#define PHO_opnum       68  /* u_short   */
#define PHO_ihint       70  /* short     */
#define PHO_ahint       72  /* short     */
#define PHO_len         74  /* u_short   */
#define PHO_fragnum     76  /* u_short   */
#define PHO_auth_type   78  /* u_char    */
#define PHO_pad3        79  /* u_char    */

#ifdef MISPACKED_HDR

/*
 * P A C K _ H D R
 *
 * This procedure does the header compacting described in the "Packet header
 * storage layout" comment in "rpc_p.h".  Note all packing is done from the
 * bottom (end of structure) because we're compressing the data "downward"
 * (toward higher addresses) and we don't want to overwrite data we haven't
 * packed yet.
 */

    /*
     * Macros for packing short, long, and UUID fields.  Note that the short and
     * long packers have to depend on the endianness of the machine because
     * we don't want depend on how the compiler deals with casting pointers
     * to shorts/longs into char *'s.
     */

#define pack_hdr_byte(p, offset, val) \
    p[offset] = (val)

#if INT_REP == rpc_$drep_int_little_endian

#define pack_hdr_short(p, offset, val) { \
    register u_short tmp = val; \
    p[(offset) + 1] = (u_char) ((tmp) >>  8); \
    p[(offset) + 0] = (u_char) ((tmp) >>  0); \
}

#define pack_hdr_long(p, offset, val) { \
    register u_long tmp = val; \
    p[(offset) + 3] = (u_char) ((tmp) >> 24); \
    p[(offset) + 2] = (u_char) ((tmp) >> 16); \
    p[(offset) + 1] = (u_char) ((tmp) >>  8); \
    p[(offset) + 0] = (u_char) ((tmp) >>  0); \
}

#else

#define pack_hdr_short(p, offset, val) { \
    register u_short tmp = val; \
    p[(offset) + 1] = (u_char) ((tmp) >>  0); \
    p[(offset) + 0] = (u_char) ((tmp) >>  8); \
}

#define pack_hdr_long(p, offset, val) { \
    register u_long tmp = val; \
    p[(offset) + 3] = (u_char) ((tmp) >>  0); \
    p[(offset) + 2] = (u_char) ((tmp) >>  8); \
    p[(offset) + 1] = (u_char) ((tmp) >> 16); \
    p[(offset) + 0] = (u_char) ((tmp) >> 24); \
}

#endif

internal void pack_hdr_uuid(p, offset, val)
char *p;
u_int offset;
uuid_$t *val;
{
    pack_hdr_byte (p, offset + 15, val->host[6]);
    pack_hdr_byte (p, offset + 14, val->host[5]);
    pack_hdr_byte (p, offset + 13, val->host[4]);
    pack_hdr_byte (p, offset + 12, val->host[3]);
    pack_hdr_byte (p, offset + 11, val->host[2]);
    pack_hdr_byte (p, offset + 10, val->host[1]);
    pack_hdr_byte (p, offset +  9, val->host[0]);
    pack_hdr_byte (p, offset +  8, val->family);
    pack_hdr_short(p, offset +  6, val->reserved);
    pack_hdr_short(p, offset +  4, val->time_low);
    pack_hdr_long (p, offset +  0, val->time_high);
}

internal void pack_hdr(pkt)
rpc_$pkt_t *pkt;
{
    char *p = ((char *) pkt) + PKT_IO_OFFSET;

    pack_hdr_byte (p, PHO_pad3,        0);
    pack_hdr_byte (p, PHO_auth_type,   pkt->hdr.auth_type);
    pack_hdr_short(p, PHO_fragnum,     pkt->hdr.fragnum);
    pack_hdr_short(p, PHO_len,         pkt->hdr.len);
    pack_hdr_short(p, PHO_ahint,       pkt->hdr.ahint);
    pack_hdr_short(p, PHO_ihint,       pkt->hdr.ihint);
    pack_hdr_short(p, PHO_opnum,       pkt->hdr.opnum);
    pack_hdr_long (p, PHO_seq,         pkt->hdr.seq);
    pack_hdr_long (p, PHO_if_vers,     pkt->hdr.if_vers);
    pack_hdr_long (p, PHO_server_boot, pkt->hdr.server_boot);
    pack_hdr_uuid (p, PHO_actuid,      &pkt->hdr.actuid);
    pack_hdr_uuid (p, PHO_if_id,       &pkt->hdr.if_id);
    pack_hdr_uuid (p, PHO_object,      &pkt->hdr.object);
    pack_hdr_byte (p, PHO_drep + 3,    pkt->hdr.drep[3]);
    pack_hdr_byte (p, PHO_drep + 2,    pkt->hdr.drep[2]);
    pack_hdr_byte (p, PHO_drep + 1,    pkt->hdr.drep[1]);
    pack_hdr_byte (p, PHO_drep + 0,    pkt->hdr.drep[0]);
    pack_hdr_byte (p, PHO_pad1,        0);
    pack_hdr_byte (p, PHO_flags,       pkt->hdr.flags);
    pack_hdr_byte (p, PHO_ptype,       pkt->hdr.PTYPE);
    pack_hdr_byte (p, PHO_rpc_vers,    pkt->hdr.rpc_vers);
}


/*
 * U N P A C K _ H D R
 *
 * This procedure does the header uncompacting described in the "Packet header
 * storage layout" comment in "rpc_p.h".  Note that integer data conversion is
 * done (if necessary) at the same time as unpacking.
 */

    /*
     * Macros for unpacking short, long, and UUID fields.
     */

#define UCHAR(c) ((c) & 0xff)

#define unpack_hdr_byte(p, offset, val) \
    val = UCHAR(p[offset]);

#define unpack_hdr_short(p, offset, val, little) { \
    val = little ? \
            (UCHAR(p[(offset) + 0]) <<  0) | \
            (UCHAR(p[(offset) + 1]) <<  8) \
          : \
            (UCHAR(p[(offset) + 1]) <<  0) | \
            (UCHAR(p[(offset) + 0]) <<  8); \
}

#define unpack_hdr_long(p, offset, val, little) { \
    val = little ? \
            (UCHAR(p[(offset) + 0]) <<  0) | \
            (UCHAR(p[(offset) + 1]) <<  8) | \
            (UCHAR(p[(offset) + 2]) << 16) | \
            (UCHAR(p[(offset) + 3]) << 24) \
          : \
            (UCHAR(p[(offset) + 3]) <<  0) | \
            (UCHAR(p[(offset) + 2]) <<  8) | \
            (UCHAR(p[(offset) + 1]) << 16) | \
            (UCHAR(p[(offset) + 0]) << 24); \
}

internal void unpack_hdr_uuid(p, offset, val, little)
char *p;
u_int offset;
uuid_$t *val;
boolean little;
{
    unpack_hdr_long (p, offset +  0, val->time_high, little);
    unpack_hdr_short(p, offset +  4, val->time_low,  little);
    unpack_hdr_short(p, offset +  6, val->reserved,  little);
    unpack_hdr_byte (p, offset +  8, val->family);
    unpack_hdr_byte (p, offset +  9, val->host[0]);
    unpack_hdr_byte (p, offset + 10, val->host[1]);
    unpack_hdr_byte (p, offset + 11, val->host[2]);
    unpack_hdr_byte (p, offset + 12, val->host[3]);
    unpack_hdr_byte (p, offset + 13, val->host[4]);
    unpack_hdr_byte (p, offset + 14, val->host[5]);
    unpack_hdr_byte (p, offset + 15, val->host[6]);
}

internal void unpack_hdr(pkt)
rpc_$pkt_t *pkt;
{
    char *p = ((char *) pkt) + PKT_IO_OFFSET;
    boolean little = drep_int_rep(&p[PHO_drep]) == rpc_$drep_int_little_endian;

    unpack_hdr_byte (p, PHO_rpc_vers,    pkt->hdr.rpc_vers);
    unpack_hdr_byte (p, PHO_ptype,       pkt->hdr.PTYPE);
    unpack_hdr_byte (p, PHO_flags,       pkt->hdr.flags);
    unpack_hdr_byte (p, PHO_pad1,        pkt->hdr.pad1);
    unpack_hdr_byte (p, PHO_drep + 0,    pkt->hdr.drep[0]);
    unpack_hdr_byte (p, PHO_drep + 1,    pkt->hdr.drep[1]);
    unpack_hdr_byte (p, PHO_drep + 2,    pkt->hdr.drep[2]);
    unpack_hdr_byte (p, PHO_drep + 3,    pkt->hdr.drep[3]);
    unpack_hdr_uuid (p, PHO_object,      &pkt->hdr.object, little);
    unpack_hdr_uuid (p, PHO_if_id,       &pkt->hdr.if_id, little);
    unpack_hdr_uuid (p, PHO_actuid,      &pkt->hdr.actuid, little);
    unpack_hdr_long (p, PHO_server_boot, pkt->hdr.server_boot, little);
    unpack_hdr_long (p, PHO_if_vers,     pkt->hdr.if_vers, little);
    unpack_hdr_long (p, PHO_seq,         pkt->hdr.seq, little);
    unpack_hdr_short(p, PHO_opnum,       pkt->hdr.opnum, little);
    unpack_hdr_short(p, PHO_ihint,       pkt->hdr.ihint, little);
    unpack_hdr_short(p, PHO_ahint,       pkt->hdr.ahint, little);
    unpack_hdr_short(p, PHO_len,         pkt->hdr.len, little);
    unpack_hdr_short(p, PHO_fragnum,     pkt->hdr.fragnum, little);
    unpack_hdr_byte (p, PHO_auth_type,   pkt->hdr.auth_type);
    unpack_hdr_byte (p, PHO_pad3,        pkt->hdr.pad3);
}

#endif

/*
 * Byte offsets of the beginning of each logical field in the packet body
 * as it appears on the wire. Necessary for runtime "internal" packets
 * that have bodies (e.g. reject and fault packets) since these packets
 * don't get their data marshalled/unmarshalled by NIDL stubs.
 */

#define PBO_st           0  /* status_$t */


/*
 * R P C _ $ S E T _ P K T _ B O D Y _ S T
 *
 * Marshall a status_$t into the first four bytes of a pkt's body.
 */

void rpc_$set_pkt_body_st(pkt, st_all)
rpc_$pkt_t *pkt;
u_long st_all;
{
    char *p = pkt->body.args;

#ifdef MISPACKED_HDR
    pack_hdr_long(p, PBO_st, st_all);
#else
    bcopy(&st_all, p+PBO_st, 4);
#endif
}


/*
 * R P C _ $ G E T _ P K T _ B O D Y _ S T
 *
 * Unmarshall the first four bytes of a pkt's body into a status_$t.
 */

void rpc_$get_pkt_body_st(pkt, st)
rpc_$pkt_t *pkt;
status_$t *st;
{
    u_long st_all;
    char *p = pkt->body.args;
#ifdef MISPACKED_HDR
    boolean little = drep_int_rep(pkt->hdr.drep) == rpc_$drep_int_little_endian;
#endif

#ifdef MISPACKED_HDR
    unpack_hdr_long(p, PBO_st, st_all, little);
#else
    bcopy(p + PBO_st, &st_all, 4);
    if (drep_int_rep(pkt->hdr.drep) != rpc_$local_drep.int_rep)
        swab_32(&st_all);
#endif
    st->all = st_all;
}


#if ! defined(MISPACKED_HDR) && ! defined(MSDOS)

void rpc_$swab_header(pkt)
rpc_$pkt_t *pkt;
{
    swab_16(&pkt->hdr.len);
    swab_32(&pkt->hdr.seq);
    swab_16(&pkt->hdr.fragnum);
    swab_uuid(&pkt->hdr.object);
    swab_uuid(&pkt->hdr.if_id);
    swab_32(&pkt->hdr.if_vers);
    swab_16(&pkt->hdr.opnum);
    swab_16(&pkt->hdr.ihint);
    swab_16(&pkt->hdr.ahint);
    swab_uuid(&pkt->hdr.actuid);
    swab_32(&pkt->hdr.server_boot);
}

#endif

/*
 * R P C _ $ R E C V F R O M
 *
 * Receive a packet from a socket.  Note this is the only place in the
 * runtime that recvfrom(2) is called.  Check the packet (doing checksumming,
 * uncompacting, and data conversion on the header, if appropriate) after
 * we get one.  Also note that "pmax" must include PKT_IO_OFFSET padding.
 */

int rpc_$recvfrom(s, pkt, pmax, from, cksum)
int s;
register rpc_$pkt_t *pkt;
u_long pmax;
register rpc_$sockaddr_t *from;
rpc_$cksum_t *cksum;
{
    int from_len = sizeof(from->sa);
    int recv_len;
    status_$t st;

    *cksum = NULL;

#ifdef NON_REENTRANT_SOCKETS
    if (in_sockets)
        return(-1);
#endif

    /*
     * Try to receive a packet.
     */

    IN_SOCKETS(true);

    recv_len = recvfrom(s, ((char *) pkt) + PKT_IO_OFFSET, (int) pmax - PKT_IO_OFFSET, 0,
                        &from->sa, &from_len);

    IN_SOCKETS(false);

#ifdef MAX_DEBUG
    if (rpc_$max_debug & 2 && recv_len > 0) {
        printf("\n(rpc_$recvfrom) - recv_len: %d\n", recv_len);
        rpc_$dump_raw_pkt(((char *) pkt) + PKT_IO_OFFSET, recv_len);
    }
#endif

    if (recv_len < 0) {
        if (rpc_$debug && errno != EWOULDBLOCK)
            perror("(rpc_$recvfrom) recvfrom failed");              /* Use perror to get VMS errors too */
        return (recv_len);
    }

    from->len = from_len;

    if (recv_len == 0)
        return (recv_len);

    /*
     * Do checksum now (before munging header!), if appropriate.
     */

    if (rpc_$encr_rgy != NULL && cksum != NULL) {
        register int auth_type = ((char *) pkt)[PHO_auth_type + PKT_IO_OFFSET];

        if (auth_type != NULL && rpc_$encr_rgy[auth_type] != NULL) {
            *cksum = rpc_$prexform(auth_type, (char *) pkt + PKT_IO_OFFSET,
                        recv_len - rpc_$encr_type_overhead(auth_type), &st);
            if (st.all != status_$ok) {
                errno = EINVAL;
                return (-1);
            }
        }
    }

#ifdef MISPACKED_HDR
    /*
     * Unpack and convert data representation on header for "mispacked header"
     * systems.
     */

    unpack_hdr(pkt);
#endif

    /*
     * Do some basic sanity checks on the header.
     */

    if (pkt->hdr.rpc_vers != RPC_VERS) {
        dprintf("(rpc_$unpack_hdr) Bad RPC version (%u)\n", pkt->hdr.rpc_vers);
        errno = EINVAL;
        return (-1);
    }

    if ((u_short) pkt_type(pkt) > MAX_PKT_TYPE) {
        dprintf("(rpc_$unpack_hdr) Bad pkt type (%d)\n", (int) pkt_type(pkt));
        errno = EINVAL;
        return (-1);
    }

#ifndef MISPACKED_HDR
    /*
     * Do data representation conversion on header if necessary.
     */

    if (drep_int_rep(pkt->hdr.drep) != rpc_$local_drep.int_rep)
        rpc_$swab_header(pkt);
#endif

#ifdef MAX_DEBUG
    if (rpc_$max_debug & 1) {
        printf("\n(rpc_$unpack_hdr) - recv_len: %d\n", recv_len);
        rpc_$dump_pkt_hdr(&pkt->hdr);
        printf("\n");
    }
#endif

    if (recv_len < pkt->hdr.len + PKT_HDR_SIZE) {
        dprintf("(rpc_$unpack_hdr) Packet too short; is %u, data len is %u\n",
                recv_len, pkt->hdr.len);
        errno = EINVAL;
        return (-1);
    }

#ifndef NO_STATS
    rpc_$stats.rcvd++;
    rpc_$stats.pstats[(u_short) pkt_type(pkt)].rcvd++;
#endif

    return (recv_len);
}


/*
 * R P C _ $ S E N D T O
 *
 * Send a packet out a socket.  Note this is the only place in the runtime that
 * sendto(2) is called.  Note also that for mispacked header machines, this is
 * the place the header gets compacted.  (We must save and restore the unpacked
 * header though since our caller wants the packet intact.)
 */

boolean rpc_$sendto(s, pkt, to, encr)
int s;
register rpc_$pkt_t *pkt;
rpc_$sockaddr_t *to;
rpc_$encr_t *encr;
{
    register int len;
    int r;
    status_$t st;
    u_long seq;

#ifdef MISPACKED_HDR
    rpc_$pkt_hdr_t saved_hdr;
#endif

#ifdef DEBUG
    if (rpc_$lossy && rand() % 5 == 0)
        return (true);
#endif

#ifdef NON_REENTRANT_SOCKETS
    if (in_sockets)
        return(false);
#endif

    len = PKT_HDR_SIZE +
          ((pkt->hdr.flags & PF_FORWARDED) ?
            4 /* sizeof(drep) */ + sizeof(rpc_$sockaddr_t) :
            0) +
          pkt->hdr.len;

    seq = pkt->hdr.seq;

    if (encr != NULL)
        pkt->hdr.auth_type = rpc_$auth_type(encr);

    copy_drep(pkt->hdr.drep, rpc_$local_drep_packed);

#ifdef MAX_DEBUG
    if (rpc_$max_debug & 1) {
        printf("\n(rpc_$sendto) - len: %d\n", len);
        rpc_$dump_pkt_hdr(&pkt->hdr);
        printf("\n");
    }
#endif

#ifdef MISPACKED_HDR
    bcopy(pkt, &saved_hdr, sizeof pkt->hdr);
    pack_hdr(pkt);
#endif

    if (encr) {
        rpc_$encr_send_xform(encr, ((char *) pkt) + PKT_IO_OFFSET, (u_long) len,
                             seq, ((char *) pkt) + PKT_IO_OFFSET + len,
                             &st);
        if (st.all != status_$ok)
            pfm_$signal(st);
        len += rpc_$encr_overhead(encr);
    }

#ifdef MAX_DEBUG
    if (rpc_$max_debug & 2) {
        printf("\n(rpc_$sendto) - len: %d\n", len);
        rpc_$dump_raw_pkt(((char*) pkt) + PKT_IO_OFFSET, len);
    }
#endif

    IN_SOCKETS(true);

    r = sendto(s, ((char *) pkt) + PKT_IO_OFFSET, len, 0, &to->sa, sizeof(struct sockaddr));

    IN_SOCKETS(false);

    if (r <= 0)
        if (rpc_$debug)
            perror("(rpc_$sendto) sendto failed");              /* Use perror to get VMS errors too */

#ifndef NO_STATS
    if (r >= 0) {
        rpc_$stats.sent++;
        rpc_$stats.pstats[(u_short) pkt_type(pkt)].sent++;
    }
#endif

#ifdef MISPACKED_HDR
    bcopy(&saved_hdr, pkt, sizeof pkt->hdr);
#endif

    return (r >= 0);
}


/*
 * R P C _ $ M A L L O C / R P C _ $ F R E E
 *
 * Shells over "malloc" and "free".  "rpc_$malloc" raises an exception if "malloc"
 * can't allocate the necessary space (i.e. returns NULL).
 */

void *rpc_$malloc(n)
int n;
{
    char *p;

#ifdef MSDOS
    pfm_$inhibit_faults();
#endif

    p = (char *) malloc(n);

#ifdef MSDOS
    pfm_$enable_faults();
#endif

    if (p == NULL)
        raisec(rpc_$cant_malloc);

    return((void *) p);
}

void rpc_$free(p)
#ifdef __STDC__
void *p;
#else
char *p;                /* Hack: pcc's sometimes get unhappy with "void *"s */
#endif
{
#ifdef MSDOS
    pfm_$inhibit_faults();
#endif

    free(p);

#ifdef MSDOS
    pfm_$enable_faults();
#endif
}


#if defined(MSDOS) && !defined(NO_NEAR)

/*
 * R P C _ $ N M A L L O C / R P C _ $ N F R E E
 *
 * Shells over "_nmalloc" and "_nfree", MS/DOS C RTL functions that allocate
 * storage out of the "near heap" (which can be accessed more efficiently).
 * "rpc_$nmalloc" raises an exception if "_nmalloc" can't allocate the
 * necessary space (i.e. returns NULL).
 */

#undef rpc_$nmalloc
#undef rpc_$nfree

void near *rpc_$nmalloc(n)
int n;
{
    void near *p;

    pfm_$inhibit_faults();
    p = (char near *) _nmalloc(n);
    pfm_$enable_faults();

    if (p == NULL)
        raisec(rpc_$cant_nmalloc);

    return(p);
}

void rpc_$nfree(p)
void near *p;
{
    pfm_$inhibit_faults();
    _nfree(p);
    pfm_$enable_faults();
}

#endif

/*
 * R P C _ $ A L L O C _ P K T
 *
 * Allocate storage for a packet.
 */

rpc_$ppkt_p_t rpc_$alloc_pkt(len)
u_long len;
{
    return ((rpc_$ppkt_p_t) rpc_$malloc((u_int) (sizeof(rpc_$pkt_hdr_t) + len)));
}


/*
 * R P C _ $ F R E E _ P K T
 *
 * Free storage allocated by "rpc_$alloc_pkt".
 */

void rpc_$free_pkt(p)
rpc_$ppkt_p_t p;
{
    rpc_$free(p);
}


/*
 * R P C _ $ A L L O C _ L I N K E D _ P K T
 *
 * Allocate storage for a "linked packet" whose length is specified by "len".
 * If "cpkt" is non-NULL, copy it into the newly allocated packet.
 */

rpc_$linked_pkt_t *rpc_$alloc_linked_pkt(cpkt, len)
rpc_$pkt_t *cpkt;
u_long len;
{
    register rpc_$linked_pkt_t *pkt;

    pkt = (rpc_$linked_pkt_t *) rpc_$malloc(sizeof(rpc_$linked_pkt_t)
                                       - sizeof(rpc_$pkt_t)
                                       + sizeof(rpc_$pkt_hdr_t)
                                       + ((u_int) len));


    if (cpkt != NULL)
        bcopy(cpkt, &pkt->pkt, (int) (sizeof(rpc_$pkt_hdr_t) + len));

    pkt->refcnt = 1;
    pkt->link = NULL;

    return (pkt);
}


/*
 * R P C _ $ F R E E _ L I N K E D _ P K T
 *
 * Free a packet allocated by "rpc_$alloc_linked_pkt".  Maintain reference
 * count and free only when count goes to zero.
 */

void rpc_$free_linked_pkt(pkt)
register rpc_$linked_pkt_t *pkt;
{
    ASSERT(pkt->refcnt > 0);

    pkt->refcnt--;

    if (pkt->refcnt == 0)
        rpc_$free(pkt);
}


/*
 * R P C _ $ F R E E _ F R A G _ L I S T
 *
 * Free a list of frags.
 */

void rpc_$free_frag_list(fl)
register rpc_$frag_list_t *fl;
{
    register rpc_$linked_pkt_t *nf;

    while (fl->head != NULL) {
        nf = fl->head->link;
        rpc_$free_linked_pkt(fl->head);
        fl->head = nf;
    }
}


/*
 * R P C _ $ I N S E R T _ I N _ F R A G _ L I S T
 *
 * Insert a fragment in sorted position (highest first) into a frag list.
 * Ignore duplicates.  Return "true" if the frag list is complete, false
 * otherwise.
 */

void rpc_$insert_in_frag_list(fl, lpkt, complete)
register rpc_$frag_list_t *fl;
rpc_$linked_pkt_t *lpkt;
boolean *complete;
{
    rpc_$pkt_t *pkt = &lpkt->pkt;
    boolean last_frag = pkt->hdr.flags & PF_LAST_FRAG;
    register rpc_$linked_pkt_t *l;  /* Used in scanning of frag list */
    rpc_$linked_pkt_t *pl;          /* Travels one frag behind in scan */

    *complete = false;

    if (fl->head == NULL) {
        fl->len     = pkt->hdr.len;
        fl->head    = fl->tail = lpkt;
        fl->highest = pkt->hdr.fragnum == 0 ? 0 : -1;
        fl->last    = last_frag ? pkt->hdr.fragnum : -1;
        lpkt->link = NULL;
        lpkt->refcnt++;
        return;                 /* I suppose we should worry about 1-frag calls */
    }

    if (last_frag)
        fl->last = pkt->hdr.fragnum;

    l = fl->head;
    pl = NULL;

    while (l != NULL) {

        /*
         * Is the the next on the list lower-numbered than the one we just got?
         * If so, slip the new one in just before it.
         */

        if (l->pkt.hdr.fragnum < pkt->hdr.fragnum) {
            if (pl == NULL) {
                fl->head = lpkt;
                lpkt->link = l;
                lpkt->refcnt++;
            }
            else {
                pl->link = lpkt;
                lpkt->link = l;
                lpkt->refcnt++;
            }
            break;
        }

        /*
         * Is the next on the list the same as the one we just got?  If so,
         * ignore the new one.
         */

        if (l->pkt.hdr.fragnum == pkt->hdr.fragnum) {
#ifndef NO_STATS
            rpc_$stats.dup_frags_rcvd++;
#endif
            return;
        }

        pl = l;
        l = l->link;
    }

    /*
     * If we exhausted the list, we must thread the new one onto the end.
     */

    if (l == NULL) {
        pl->link = lpkt;
        lpkt->link = NULL;
        fl->tail = lpkt;
        lpkt->refcnt++;
    }

    fl->len += pkt->hdr.len;

    /*
     * Recompute the highest consec frag.
     */

    if (fl->tail->pkt.hdr.fragnum != 0)
        ;
    else {
        short new_high, i;

        l = fl->head;
        new_high = i = -1;

        while (l != NULL) {
            if ((short) l->pkt.hdr.fragnum != i)
                new_high = i = l->pkt.hdr.fragnum;
            if ((short) l->pkt.hdr.fragnum < fl->highest)
                break;
            i--;
            l = l->link;
        }
        if (new_high > fl->highest)
            fl->highest = new_high;
    }

    /*
     * If the highest is known (i.e. not -1) and the highest and the last
     * frag #'s match, we must have all the frags.  Indicate that in the
     * "complete" output flag.
     */

    *complete = (fl->highest == fl->last && fl->highest != -1);
}


/*
 * R P C _ $ R E A S S E M B L E _ F R A G _ L I S T
 *
 * Turn a complete list of incoming frags into one large packet with all
 * the frag bodies appended together.  Free the fragments along the way.
 */

rpc_$pkt_t *rpc_$reassemble_frag_list(fl)
register rpc_$frag_list_t *fl;
{
    register rpc_$pkt_t *bpkt;
    rpc_$linked_pkt_t *nf;
    char *body;

    bpkt = (rpc_$pkt_t *) rpc_$alloc_pkt(fl->len);

    bpkt->hdr = fl->head->pkt.hdr;
    bpkt->hdr.len = 0;                  /* Show up bugs right away! */

    body = ((char *) &bpkt->body) + fl->len;

    while (fl->head != NULL) {
        body -= fl->head->pkt.hdr.len;
        bcopy(&fl->head->pkt.body, body, (int) fl->head->pkt.hdr.len);
        nf = fl->head->link;
        rpc_$free_linked_pkt(fl->head);
        fl->head = nf;
    }

    return (bpkt);
}


#ifdef TASKING

/*
 * P E R I O D I C _ T A S K
 *
 * Base routine of task that's created by "rpc_$periodically".
 */

internal void periodic_task(_info, len)
char *_info;
long len;
{
    periodic_info_t *info = (periodic_info_t *) _info;
    status_$t fst;
    pfm_$cleanup_rec crec;

    fst = pfm_$p_cleanup(&crec);
    if (fst.all != pfm_$cleanup_set) {
        if (fst.all != fault_$quit && fst.all != fault_$stop)
            eprintf("(periodic_task) \"%s\" task exited, status=%08lx\n", info->name, fst);
        pfm_$signal(fst);
    }

    while (true) {
        sleep(info->interval);
        (*info->proc)();
    }
}


/*
 * R P C _ $ P E R I O D I C A L L Y
 *
 * Schedule a procedure to be called periodically.
 */

void rpc_$periodically(proc, name, interval)
void (*proc)();
char *name;
u_long interval;
{
    periodic_info_t info;
    status_$t st;
    task_$handle_t task;

    info.proc     = proc;
    info.interval = interval;
    info.name     = name;

    task = task_$create(periodic_task, (char *) &info, sizeof info, TASK_STACK_SIZE, TASK_PRIORITY,
                        task_$until_level_exit, 0, &st);

    if (st.all != status_$ok) {
        DIE("Can't create periodic task");
        return;
    }

    task_$set_name(task, name, strlen(name), &st);
}


/*
 * R P C _ $ S T O P _ P E R I O D I C
 *
 * This procedure is a no-op in a tasking environment.  We don't bother
 * to eliminate tasks that aren't currently needed.
 */

void rpc_$stop_periodic(proc)
void (*proc)();
{
}


/*
 * R P C _ $ C R E A T E _ T A S K
 *
 * Useful shell over "task_$create".
 */

void rpc_$create_task(proc, name)
void (*proc)();
char *name;
{
    status_$t st;
    task_$handle_t task;

    task = task_$create(proc, NULL, 0, TASK_STACK_SIZE, TASK_PRIORITY,
                        task_$until_level_exit, 0, &st);

    if (st.all != status_$ok) {
        DIE("Can't create task");
        return;
    }

    task_$set_name(task, name, strlen(name), &st);
}

#else

/*
 * H A N D L E _ A L A R M
 *
 * Handle SIGALRM.  This is a very crude scheme for doing tasking when you
 * don't actually have tasking.  We simply have a table of routines that
 * are periodically called from this routine.
 */

#define ALARM_INTERVAL 1

internal void handle_alarm()
{
    register u_short i;
    u_long now;
    extern int pfm_$fault_inh_count;

    /*
     * Checked to see if we're "inhibited".  You might think that we
     * shouldn't be here if we're inhibited, but here's why you might be
     * wrong:  On pre-R3 System V systems, you can't "hold" signals.  If
     * we set things up to ignore alarms, the alarm would go off and we
     * wouldn't get an opportunity to reset it for later.  Thus, we must
     * keep it going, but ignore it here.
     *
     * The other reason we make this check is because in
     * "rpc_$stop_periodic", when we try to turn off the alarm (restore
     * it and the handler to its previous state), we must temporarily unhold
     * (on BSD and S5R3) the alarm signal so that in case one is pending,
     * we get a chance to handle it here before we remove our handler.
     */

    if (pfm_$fault_inh_count > 0)
        goto RE_ARM;

    now = time(NULL);

    for (i = 0; i < n_tasks; i++) {
        register task_db_elt_t *ate = &task_db[i];

        if (ate->proc != NULL && now >= ate->next_time) {
            (*ate->proc)();
            ate->next_time = now + ate->interval;
        }
    }

RE_ARM: ;

#ifndef MSDOS

#if defined(SYS5) || defined(VMS)
    signal(SIGALRM, handle_alarm);
#endif

    /*
     * Reset the alarm one more time iff the alarm is still to be set.
     */

    if (alarm_set)
        alarm(ALARM_INTERVAL);

#endif
}


/*
 * R P C _ $ P E R I O D I C A L L Y
 *
 * Schedule a procedure to be called periodically.
 */

void rpc_$periodically(proc, name, interval)
void (*proc)();
char *name;
u_long interval;
{
    register task_db_elt_t *ate;
    register u_short i;
    short null_i = -1;

    /*
     * Search for an empty slot in the task table.
     */

    for (i = 0; i < n_tasks; i++)
        if (task_db[i].proc == NULL) {
            null_i = i;
            break;
        }

    if (null_i != -1)
        ate = &task_db[null_i];
    else {
        ASSERT(n_tasks < MAX_TASKS);
        ate = &task_db[n_tasks];
        n_tasks++;
    }

    /*
     * Set up an alarm interrupt if one is not already set up.
     */

    if (! alarm_set) {
        alarm_set = true;
#ifdef MSDOS
        alarm_$start(handle_alarm, ALARM_INTERVAL);
#else
        old_alarm_handler = signal(SIGALRM, handle_alarm);
        old_alarm_value = alarm(ALARM_INTERVAL);
#endif
    }

    pfm_$inhibit();
    ate->proc      = proc;
    ate->interval  = interval;
    ate->next_time = time(NULL) + interval;
    pfm_$enable();
}


/*
 * R P C _ $ S T O P _ P E R I O D I C
 *
 * Remove a periodic task.  If no tasks are left, turn off the alarm.
 */

void rpc_$stop_periodic(proc)
void (*proc)();
{
    register u_short i;
    register boolean tasks_left = false;

    /*
     * Find and remove the specified task procedure from our database.
     */

    for (i = 0; i < n_tasks; i++) {
        register task_db_elt_t *ate = &task_db[i];
        if (ate->proc == proc)
            ate->proc = NULL;
        else if (ate->proc != NULL)
            tasks_left = true;
    }

    /*
     * If the above scan showed there were no tasks left, turn off our alarm.
     * Re-establish any previous alarm and handler.
     */

    if (! tasks_left) {
#ifndef MSDOS
        alarm_set = false;
        alarm(0);
#ifdef BSD
        /*
         * Quick like a bunny, make sure alarms are enabled so we take any
         * pending alarms before removing our handler.
         */
        sigsetmask(sigsetmask(~ (1 << (SIGALRM - 1))));
#endif
        signal(SIGALRM, old_alarm_handler);
        if (old_alarm_value != 0)
            alarm(old_alarm_value);
#endif
    }
}

#endif


/*
 * R P C _ $ F I L L _ F R A G
 *
 * Construct a fragment from a large packet.  Returns "true" if the fragment
 * is the last one.
 */

boolean rpc_$fill_frag(lpkt, llen, fragnum, mtu, pkt)
register rpc_$pkt_t *lpkt;      /* The "large" packet */
u_long llen;                    /* Number of data bytes in large packet */
u_short fragnum;                /* The number (zero-based) of the frag we want */
u_long mtu;                     /* The max body size */
register rpc_$pkt_t *pkt;       /* Place to stick the frag */
{
    u_long n_bytes_left = llen - (fragnum * mtu);
    boolean last_frag = n_bytes_left <= mtu;

    pkt->hdr = lpkt->hdr;
    pkt->hdr.len = last_frag ? n_bytes_left : mtu;
    pkt->hdr.flags |= PF_FRAG;
    pkt->hdr.fragnum = fragnum;

    if (last_frag)
        pkt->hdr.flags |= PF_LAST_FRAG;

    bcopy(&lpkt->body.args[llen - n_bytes_left], &pkt->body, (int) pkt->hdr.len);
    return (last_frag);
}




/*
 * R P C _ $ C V T _ S T R I N G
 *
 * Called by stubs to copy and convert a string in one character
 * representation to another.
 */

void rpc_$cvt_string(src_drep, dst_drep, src, dst)
rpc_$drep_t src_drep;
rpc_$drep_t dst_drep;
rpc_$char_p_t src;
rpc_$char_p_t dst;
{
    if (src_drep.char_rep == dst_drep.char_rep)
        strcpy((char *) dst, (char *) src);
    else if (dst_drep.char_rep == rpc_$drep_char_ascii)
        while (*dst++=rpc_$ebcdic_to_ascii[*src++]);
    else
        while (*dst++=rpc_$ascii_to_ebcdic[*src++]);
}


/*
 * R P C _ $ B L O C K _ C O P Y
 *
 * Standard memory copy routine.  Here so stubs don't have to worry about
 * BSD "bcopy" vs. SYS5 "memcpy".
 */

void rpc_$block_copy(src, dst, count)
rpc_$byte_p_t src;
rpc_$byte_p_t dst;
u_long count;
{
    bcopy((char *) src, (char *) dst, (int) count);
}


/*
 * R P C _ $ S T A T U S _ P R I N T
 *
 * Print out a message with a status code.
 */

void rpc_$status_print(text, st)
char *text;
status_$t st;
{
#if defined(MSDOS) && defined(NOT_YET) /*???*/
    printf("%s status %08lx\n", text, st.all);
#else
    char buff[100];
    extern char *error_$c_text();

    printf("%s%s\n", text, error_$c_text(st, buff, sizeof buff));
#endif
    fflush(stdout);
}


/*
 * R P C _ $ T E R M I N A T I O N _ F A U L T
 *
 * Return "true" iff status is one that's likely to be one that's the
 * result of one of the normal "kill yourself" faults.
 */

boolean rpc_$termination_fault(st)
status_$t st;
{
    return(
#if defined(apollo) && defined(pre_sr10)
        st.all == 0x09010002 ||     /* unix_sig_$int_f */
        st.all == 0x09010003 ||     /* unix_sig_$quit_f */
        st.all == 0x0901000f ||     /* unix_sig_$term_f */
        st.all == 0x09010009 ||     /* unix_sig_$kill_f */
#endif
        st.all == fault_$interrupt ||
        st.all == fault_$quit ||
        st.all == fault_$stop
        );
}


/*
 * R P C _ $ R E G I S T E R _ E N C R T Y P E
 *
 */

void rpc_$register_encrtype(auth_type, epv, st)
u_long auth_type;
rpc_$encr_epv_t *epv;
status_$t *st;
{
    register int i;

    if (auth_type >= rpc_$max_auth_type) {
        st->all = rpc_$invalid_auth_type;
        return;
    }

    if (rpc_$encr_rgy == NULL) {
        rpc_$encr_rgy = (rpc_$encr_epv_t **)
                            rpc_$malloc(rpc_$max_auth_type * sizeof (rpc_$encr_epv_t *));
        for (i = 0; i < rpc_$max_auth_type; i++)
            rpc_$encr_rgy[i] = NULL;
    }

    rpc_$encr_rgy[auth_type] = epv;
    st->all = status_$ok;
}

#ifdef GLOBAL_LIBRARY

/*
 * R P C _ $ G L O B A L _ L I B _ I N I T
 *
 */

void rpc_$global_lib_init()
{
    extern char *unix_$static;

    if (unix_$static == 0)
        unix_$init();

    ddslib_$init();
}

#endif


/*
 * R P C _ $ D U M P _ S T A T S
 *
 */

void rpc_$dump_stats(f)
FILE *f;
{
#ifndef NO_STATS
    register u_short i;

#define prstat(text, stat) fprintf(f, text, rpc_$stats.stat)
    prstat("\t# of remote calls out:\t%lu\n",  calls_out);
    prstat("\t# of remote calls in:\t%lu\n",   calls_in);
    prstat("\t# of pkts sent:\t%lu\n",         sent);

    for (i = 0; i <= MAX_PKT_TYPE; i++)
        fprintf(f, "\t\t%s\t%lu\n", rpc_$pkt_name((rpc_$ptype_t) i), rpc_$stats.pstats[i].sent);

    prstat("\t# of pkts rcvd:\t%lu\n",         rcvd);

    for (i = 0; i <= MAX_PKT_TYPE; i++)
        fprintf(f, "\t\t%s\t%lu\n", rpc_$pkt_name((rpc_$ptype_t) i), rpc_$stats.pstats[i].rcvd);

    prstat("\t# of frag resends:\t%lu\n",      frag_resends);
    prstat("\t# of dup frags rcvd:\t%lu\n",    dup_frags_rcvd);
#undef prstat
#endif
}


#ifdef MAX_DEBUG

/*
 * R P C _ $ D U M P _ P K T _ H D R
 */

void rpc_$dump_pkt_hdr(p)
rpc_$pkt_hdr_t *p;
{
    uuid_$string_t ubuff;

    eprintf("rpc_vers: %d\n", p->HDR.rpc_vers);
    eprintf("xptype: %d (%s)\n", p->HDR.PTYPE, rpc_$pkt_name(p->HDR.PTYPE));
    eprintf("flags: 0x%x\n", p->HDR.flags);
    eprintf("int_rep: %d\n", drep_int_rep(p->HDR.drep));
    eprintf("char_rep: %d\n", drep_char_rep(p->HDR.drep));
    eprintf("float_rep: %d\n", drep_float_rep(p->HDR.drep));
    uuid_$encode(&(p->HDR.object), ubuff);
    eprintf("object uuid: %s\n", ubuff);
    uuid_$encode(&(p->HDR.if_id), ubuff);
    eprintf("interface uuid: %s\n", ubuff);
    uuid_$encode(&(p->HDR.actuid), ubuff);
    eprintf("activity uuid: %s\n", ubuff);
    eprintf("server boot time: %ld\n", p->HDR.server_boot);
    eprintf("interface version: %ld\n", p->HDR.if_vers);
    eprintf("sequence number: %ld\n", p->HDR.seq);
    eprintf("operation number: %d\n", p->HDR.opnum);
    eprintf("interface hint: %d\n", p->HDR.ihint);
    eprintf("activity hint: %d\n", p->HDR.ahint);
    eprintf("fragment #: %d\n", p->HDR.fragnum);
    eprintf("length of body: %d\n", p->HDR.len);
}

void rpc_$dump_raw_pkt(pkt, len)
unsigned char *pkt;
int len;
{
    register int i;

    for (i = 0; i < len; i++){
        if (i%16 == 0) {
            if (i > 0)
                printf("\n");
            printf("%04x: ", i);
        }
        if ((i+8)%16 == 0)
            printf(" ");
        printf("%02x ", pkt[i]);
    }
    printf("\n");
}

#endif

#ifdef FTN_INTERLUDES

void rpc_$status_print_(text, st)
char *text;
status_$t st;
{
    rpc_$status_print(text, st);
}

#endif

#if defined(apollo) && defined(pre_sr10)

/*
 * Starting at sr10.2, /lib/error contains the following routine.  Pre-sr10
 * systems get it from /lib/ddslib.  sr10 and sr10.1 system can get it simply
 * by updating to the sr10.2 /lib/error.
 */

/*
 * E R R O R _ $ C _ T E X T
 *
 * Decode a status code into a string.
 */

char *
error_$c_text(st, buff, maxlen)
    status_$t st;
    char *buff;
    int maxlen;
{
    char *sub_np, *mod_np, *err_p;
    register short sub_nl, mod_nl, err_l;
    std_$call void error_$find_text();

    error_$find_text(st, sub_np, sub_nl, mod_np, mod_nl, err_p, err_l);

    if (sub_nl == 0)
        sprintf(buff, "status %08lx", st.all);
    else if (mod_nl == 0)
        sprintf(buff, "status %08lx (%.*s)", st.all, sub_nl, sub_np);
    else if (err_l == 0)
        sprintf(buff, "status %08lx (%.*s/%.*s)", st.all, sub_nl, sub_np, mod_nl, mod_np);
    else
        sprintf(buff, "%.*s (%.*s/%.*s)", err_l, err_p, sub_nl, sub_np, mod_nl, mod_np);

    return (buff);
}

#endif
