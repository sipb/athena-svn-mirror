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
 * R P C _ S E R V E R
 *
 * The modules "rpc_server.c" and "rpc_lsn.c" together implement the server
 * side of the NCA/RPC protocol.
 *
 * "rpc_server.c" has all the low-level packet shuffling stuff.  "rpc_lsn.c"
 * has most of the upper level routines that might be called by an actual server
 * application.
 */

#include "rpc_p.h"

/*
 * Private RPC server data.
 *
 * On Apollos, this data with NOT be in the DATA$ section and hence will
 * be per-process when this module is used in a global library.
 */

    /*
     * Interface registry.  Contains info about all interface registered
     * by this server.
     */

#define MAX_IFS 32

typedef struct if_type_info_t {
    uuid_$t type;               /* type of object to which entry applies */
    rpc_$mgr_epv_t mepv;        /* => manager procedures */
    struct if_type_info_t NEAR *link;
} NEAR if_type_info_t;

typedef struct {
    rpc_$if_spec_t ifs;
    boolean generic;            /* T => do operation dispatch on calls to this i/f */
    boolean inuse;              /* T => this entry in use */
    union {                     /* variant depending on generic/specific i/f */
        struct {                /* specific i/f variant */
            rpc_$epv_t epv;         /* => server stubs */
            u_short refcnt;         /* # of times this has been registered */
        } s;
        struct {                /* generic i/f variant */
            rpc_$generic_epv_t epv; /* => server stubs */
            if_type_info_t *ti;     /* linked list of [type, mgr epv] pairs */
        } g;
    } e;
} NEAR ifrgy_elt_t;

internal ifrgy_elt_t ifrgy[MAX_IFS];            /* Array of registered interfaces */

    /*
     * Object registry.  Contains info about all the objects the server
     * handles (and the objects' types).
     */

typedef struct {
    uuid_$t object;
    uuid_$t type;
} NEAR objrgy_elt_t;

#define MAX_OBJS 64

internal objrgy_elt_t objrgy[MAX_OBJS];


    /*
     * Authentication type registry.  Contains the procedure to call
     * instead of who_are_you (way) for each authentication type.
     */

internal rpc_$way_fn_t NEAR * NEAR auth_rgy;

    /*
     * Function to call on authentication failure
     */

internal rpc_$auth_log_fn_t NEAR log_fn = NULL;

    /*
     * Activity states (i.e. states of our information about a remote
     * activity).
     */

typedef enum {
    as_frag,                    /* In the middle of fragment receipt */
    as_in_call_back,            /* Calling back client */
    as_working,                 /* Executing server code */
    as_in_reply,                /* Sending a long reply */
    as_replied,                 /* Have a reply */
    as_idle,                    /* Replied, and reply has been ack'd */
    as_passive                  /* Idle, but holding less information */
} act_state_t;

#define N_STATES (1 + (int) as_passive)


    /*
     * Activity table.  Contains information about remote activities (i.e.
     * tasks, processes) that we are or recently have been talking to.
     */

typedef struct {
    rpc_$pkt_t *opkt;           /* ptr to whole reply */
    u_long len;                 /* total reply length */
    u_short fragnum;            /* # of first frag to send in next blast */
} reply_t;

typedef struct {
    act_state_t state;      /* activity state */
    uuid_$t id;             /* activity UID */
    long prev_seq;          /* sequence # of previous call (current call if working
                               or in frag receipt) */
    u_long atime;           /* time I last heard something from this activity */
    struct active_info_t {  /* This part here only in non-passive state */
        long cb_seq;                /* seq # of pkt that caused us to do a callback */
        rpc_$pkt_t *pkt;            /* request pkt (valid iff in working state) */
        rpc_$sockaddr_t addr;       /* address to resend replies to */
        int sock;                   /* socket I'm talking to this activity over */
        reply_t reply;              /* reply info */
        rpc_$frag_list_t frags;     /* list of inbound fragments */
        struct task_info_t {        /* info about the server task executing request */
            uuid_$t id;                 /* task UID */
#ifdef TASKING
            task_$handle_t handle;      /* task handle */
#endif
            boolean quitable;           /* T => task is currently capable of handling quits */
        } task;
        u_long n_callbacks;         /* # of callbacks made for current call */
        boolean blast_outs;         /* T => outs can be blasted */
        rpc_$server_auth_t *auth;   /* authentication information */
        rpc_$encr_t *encr;          /* encryption info (session keys) */
    } a;
} NEAR activity_t;

#define MAX_ACTIVITIES 128

internal activity_t * NEAR acts[MAX_ACTIVITIES];

#define valid_ahint(pkt) ( \
    (pkt)->hdr.ahint != NO_HINT \
    && acts[(pkt)->hdr.ahint] != NULL \
    && uidequal(acts[(pkt)->hdr.ahint]->id, (pkt)->hdr.actuid) \
)

#define timestamp(xact) { \
    (xact)->atime = time(NULL); \
}

    /*
     * RPC server handle type.  The "hhdr" field must remain first.
     * See comments on "handle_hdr_t" definition in "rpc_p.h".
     */

typedef struct server_handle_t {
    handle_hdr_t hhdr;                  /* common server/client handle header */
    activity_t *act;                    /* pointer to client activity info */
} NEAR server_handle_t;

#ifdef MSDOS
#  define HANDLE_CAST(h) ((server_handle_t *) ((long) (h)))
#else
#  define HANDLE_CAST(h) ((server_handle_t *) h)
#endif

    /*
     * Miscellaneous state variables
     */

internal u_short NEAR n_interfaces;          /* # of registered interfaces */
internal u_short NEAR n_objects;             /* # of registered objects */
internal u_long NEAR boot_time;              /* time server started listening */
internal boolean NEAR faults_are_fatal;      /* T => faults in user routine cause server exit */

#ifdef DEBUG
internal uuid_$string_t NEAR uidstring_buff; /* buffer for uidstring function */
internal char NEAR uidseqstring_buff[50];    /* buffer for seqstring function */
#endif

    /*
     * Cruft for Apollo mark/release mechanism.
     */

#ifdef GLOBAL_LIBRARY

#define MAX_LEVELS 20

typedef struct {
    u_short n_interfaces;
    u_short n_objects;
} level_info_t;

internal level_info_t level_info[MAX_LEVELS];

#endif

#ifdef GLOBAL_LIBRARY
#  include "set_sect.pvt.c"
#endif

internal char *uidstring
    PROTOTYPE((uuid_$t *uid));
internal char *uidseqstring
    PROTOTYPE((rpc_$pkt_t *pkt));
internal char *state_name
    PROTOTYPE((act_state_t state));
internal void init
    PROTOTYPE((void));
internal void send_rejection
    PROTOTYPE((int sock, long rst, rpc_$pkt_t *pkt, rpc_$sockaddr_t *addr));
internal void get_my_activity
    PROTOTYPE((uuid_$t *actuid));
internal ifrgy_elt_t *find_ifrgy_slot
    PROTOTYPE((rpc_$if_spec_t *ifspec, status_$t *st));
internal u_short find_interface
    PROTOTYPE((uuid_$t *iid, u_long if_vers));
internal rpc_$mgr_epv_t find_mgr_epv
    PROTOTYPE((if_type_info_t *ti, uuid_$t *type));
internal boolean inq_object_type
    PROTOTYPE((uuid_$t *object, uuid_$t *type));
internal void free_reply
    PROTOTYPE((activity_t *act));
internal void send_reply
    PROTOTYPE((int sock, activity_t *act));
internal int *set_act_quitable
    PROTOTYPE((activity_t *act, boolean on));
internal u_short find_activity
    PROTOTYPE((uuid_$t *actuid));
internal u_short get_activity
    PROTOTYPE((rpc_$pkt_t *pkt, rpc_$sockaddr_t *from, int sock));
internal void free_activity
    PROTOTYPE((u_short ahint));
internal void passivate_activity
    PROTOTYPE((u_short ahint));
internal activity_t *activate_activity
    PROTOTYPE((u_short ahint, rpc_$sockaddr_t *from, int sock));
internal void scan_activities
    PROTOTYPE((void));
internal void ping_common
    PROTOTYPE((int sock, activity_t *act, rpc_$pkt_t *pkt, rpc_$sockaddr_t *from));
internal void who_are_you
    PROTOTYPE((activity_t *act, status_$t *st, u_long auth_type));
internal rpc_$pkt_t *handle_request_frag
    PROTOTYPE((int sock, rpc_$sockaddr_t *from, u_short ahint, rpc_$linked_pkt_t *pkt, u_long *body_len));
internal void quit_activity
    PROTOTYPE((activity_t *act));
internal void do_request
    PROTOTYPE((int sock, rpc_$linked_pkt_t *pkt, rpc_$sockaddr_t *from, rpc_$cksum_t cksum));
internal void do_ping
    PROTOTYPE((int sock, rpc_$pkt_t *pkt, rpc_$sockaddr_t *from));
internal void do_ack
    PROTOTYPE((int sock, rpc_$pkt_t *pkt, rpc_$sockaddr_t *from));
internal void do_fack
    PROTOTYPE((int sock, rpc_$pkt_t *pkt, rpc_$sockaddr_t *from));
internal void do_quit
    PROTOTYPE((int sock, rpc_$pkt_t *pkt, rpc_$sockaddr_t *from));
internal void do_bad_pkt
    PROTOTYPE((int sock, rpc_$pkt_t *pkt, rpc_$sockaddr_t *from));


/*
 * U I D S T R I N G
 *
 * Return a pointer to a printed UUID.
 *
 */

#ifdef DEBUG

internal char *uidstring(uid)
uuid_$t *uid;
{
    uuid_$encode(uid, uidstring_buff);
    return((char *) uidstring_buff);
}

#else

#define uidstring(junk) ""

#endif


/*
 * U I D S E Q S T R I N G
 *
 * Return a pointer to a printed packet's activity-UID/seq/fragnum string.
 */


#ifdef DEBUG

internal char *uidseqstring(pkt)
rpc_$pkt_t *pkt;
{
    sprintf(uidseqstring_buff, "%s, %lu.%u",
            uidstring(&pkt->hdr.actuid), pkt->hdr.seq, pkt->hdr.fragnum);
    return(uidseqstring_buff);
}

#else

#define uidseqstring(junk) ""

#endif


/*
 * S T A T E _ N A M E
 *
 * Return the text name of an activity state.  This can't simply be a variable
 * because of the vagaries of global libraries.
 */

#ifdef DEBUG

internal char *state_name(state)
act_state_t state;
{
    static char *names[] = {
        "frag",
        "in_call_back",
        "working",
        "in_reply",
        "replied",
        "idle",
        "passive"
    };

    return(names[(int) state]);
}

#else

#define state_name(junk) ""

#endif


/*
 * S E N D _ R E J E C T I O N
 *
 * Send a reject status pkt to someone.
 */

internal void send_rejection(sock, rst, pkt, addr)
int sock;
long rst;
register rpc_$pkt_t *pkt;
rpc_$sockaddr_t *addr;
{
    set_pkt_type(pkt, rpc_$reject);
    rpc_$set_pkt_body_st(pkt, rst);
    pkt->hdr.len = 4;    /* sizeof status_$t on the wire */

    rpc_$sendto(sock, pkt, addr, NULL);
}


/*
 * G E T _ M Y _ A C T I V I T Y
 *
 * Get my "activity" ID.  An activity is a thread of control (task, if we
 * have multiple threads of control per Unix-like process; process, otherwise).
 * An activity can have only one RPC call in progress at a time.
 */

internal void get_my_activity(actuid)
uuid_$t *actuid;
{
#ifdef TASKING

#ifdef apollo
    uid_$t uid;
    if (task_$get_handle() == NULL)
        proc2_$who_am_i(&uid);
    else
        task_$get_uid(&uid);
    uuid_$from_uid(&uid, actuid);
#else
    uuid_$t uuid;
    task_$get_uuid(actuid);
#endif

#else

    static uuid_$t NEAR uuid;
#ifdef UNIX
    static int gpid = -1;
    int mypid = getpid();
#else
    static boolean NEAR gotit = false;
#endif

#ifdef UNIX
    if (mypid != gpid) {
        uuid_$gen(&uuid);
        gpid = mypid; 
    }
#else
    if (! gotit) {
        uuid_$gen(&uuid);
        gotit = true;
    }
#endif

    *actuid = uuid;

#endif
}


/*
 * R P C _ $ S E R V E R _ T O _ C L I E N T _ H A N D L E
 *
 * Allocate and return a client-side binding handle from a server-side
 * handle.  The client-side handle is made to refer to the client/socket
 * whence the call that the server is executing came.  We also return two
 * output parameters:  The activity UID (of the client) and the sequence
 * number to use in the callback.
 */

handle_t rpc_$server_to_client_handle(h, actuid, seq, st)
handle_t h;
uuid_$t *actuid;
u_long *seq;
status_$t *st;
{
    register activity_t *act = HANDLE_CAST(h)->act;

    if (act->state != as_working) {
        st->all = rpc_$not_in_call;
        return(NULL);
    }

    *actuid = act->id;
    *seq = act->a.pkt->hdr.seq + (++act->a.n_callbacks);

    return (rpc_$bind(&act->a.pkt->hdr.object, &act->a.addr.sa, act->a.addr.len, st));
}



/*
 * R P C _ $ G E T _ M Y _ A C T I V I T Y
 *
 * Like (internal) "get_my_activity" except does "alias" processing: i.e. if
 * the task that calls this procedure is running on behalf of a remote client,
 * the client's activity UID is returned instead of returning the activity
 * UID of the server task (i.e. the caller of *this* procedure).
 */

void rpc_$get_my_activity(actuid)
uuid_$t *actuid;
{
    register u_short i;

    get_my_activity(actuid);

#ifdef NOT_DEFINED

    /*
     * THIS CODE IS #IFDEF'D OUT BECAUSE MAKING CALLS THAT ARE RUNNING ON
     * BEHALF OF A REMOTE CALLER USE THAT CALLER'S ACTIVITY UID BUT USING
     * LOCALLY GENERATED SEQUENCE NUMBERS CAUSES A VARIETY OF PROBLEMS WHICH
     * CAN'T BE FIXED RIGHT NOW.
     */

    LOCK(SERVER_MUTEX);

    /*
     * Scan list of client activities for one that we're working on and
     * that this activity is actually doing the work for.  If we find a
     * match, set the UID we'll return to be the activity UID of the client
     * activity.
     */

    for (i = 0; i < MAX_ACTIVITIES; i++) {
        activity_t *act = acts[i];
        if (act != NULL
            && act->state == as_working
            && uidequal(act->a.task.id, *actuid))
        {
            *actuid = act->a.pkt->hdr.actuid;
            break;
        }
    }

    UNLOCK(SERVER_MUTEX);

#endif
}


/*
 * G E T _ I F R G Y _ S L O T
 *
 * Search the ifrgy for a matching ifspec.  If one is found, return a pointer
 * to the slot.  If none is found, find a free slot and return a pointer to it.
 * If there are no free slots, return NULL and set return status.
 */

internal ifrgy_elt_t *find_ifrgy_slot(ifspec, st)
rpc_$if_spec_t *ifspec;
status_$t *st;
{
    register u_short i;
    register ifrgy_elt_t *iface = NULL;

    st->all = status_$ok;

    /*
     * See if this interface has been registered already.
     */

    for (i = 0; i < n_interfaces; i++) {
        register ifrgy_elt_t *xiface = &ifrgy[i];

        if (! xiface->inuse)
            iface = xiface;
        else
            if (uidequal(xiface->ifs.id, ifspec->id) && xiface->ifs.vers == ifspec->vers) {
                iface = xiface;
                break;
            }
    }

    /*
     * If no free slot found, try to extend the table.
     */

    if (iface == NULL) {
        if (n_interfaces >= MAX_IFS) {
            st->all = rpc_$too_many_ifs;
            return (NULL);
        }

        iface = &ifrgy[n_interfaces++];
        iface->inuse = false;
    }

    return (iface);
}


/*
 * R P C _ $ R E G I S T E R
 *
 * Declare that a server supports the specified interface.
 *
 * Use of this procedure implies that the server has only one implementation
 * of the interface.  See "rpc_$register_mgr" for servers that want to
 * have multiple implementations.
 *
 * Note that "rpc_$register" can be called multiple times with the same
 * interface.  However, each such call must have the same EPV (otherwise
 * an error will be returned).  Each valid "re-register" simply increments
 * a references count for the registered interface.  A corresponding number
 * of calls to "rpc_$unregister" are required to actually unregister the
 * interface.
 */

void rpc_$register(ifspec, epv, st)
rpc_$if_spec_t *ifspec;
rpc_$epv_t epv;
status_$t *st;
{
    register ifrgy_elt_t *iface;

    LOCK(SERVER_MUTEX);

    iface = find_ifrgy_slot(ifspec, st);

    if (iface == NULL) {
        UNLOCK(SERVER_MUTEX);
        return;
    }

    /*
     * Store this interface info in the table.
     */

    if (iface->inuse) {
        if (iface->generic || iface->e.s.epv != epv) {
            st->all = rpc_$illegal_register;
            UNLOCK(SERVER_MUTEX);
            return;
        }
    }
    else {
        iface->inuse      = true;
        iface->generic    = false;
        iface->ifs        = *ifspec;
        iface->e.s.epv    = epv;
        iface->e.s.refcnt = 0;
    }

    iface->e.s.refcnt++;
    UNLOCK(SERVER_MUTEX);
}


/*
 * R P C _ $ R E G I S T E R _ M G R
 *
 * Declare that a server supports the specified generic interface.  The
 * server registers a set of procedures ("manager procedures") that implement
 * the specified interface for the specified type.  Servers can call this
 * procedure multiple times with the same interface and generic EPV but
 * with different object types and manager EPVs.  This allows a server
 * to have multiple implementations of the same interface.  See
 * "rpc_$register_object" for more information.
 */

void rpc_$register_mgr(type, ifspec, sepv, mepv, st)
uuid_$t *type;
rpc_$if_spec_t *ifspec;
rpc_$generic_epv_t sepv;
rpc_$mgr_epv_t mepv;
status_$t *st;
{
    register ifrgy_elt_t *iface;
    register if_type_info_t *t;

    LOCK(SERVER_MUTEX);

    iface = find_ifrgy_slot(ifspec, st);

    if (iface == NULL) {
        UNLOCK(SERVER_MUTEX);
        return;
    }

    /*
     * Store this interface info in the table.
     */

    if (iface->inuse) {
        if (! iface->generic || iface->e.g.epv != sepv) {
            st->all = rpc_$illegal_register;
            UNLOCK(SERVER_MUTEX);
            return;
        }
    }
    else {
        iface->inuse   = true;
        iface->generic = true;
        iface->ifs     = *ifspec;
        iface->e.g.epv = sepv;
        iface->e.g.ti  = NULL;
    }

    /*
     * Allocate and fill in a [type, EPV] pair.
     */

    t = (if_type_info_t *) rpc_$nmalloc(sizeof(if_type_info_t));
    t->type = *type;
    t->mepv = mepv;

    /*
     * Thread [type, EPV] pair onto list.
     */

    t->link = iface->e.g.ti;
    iface->e.g.ti = t;

    UNLOCK(SERVER_MUTEX);
}


/*
 * F I N D _ I N T E R F A C E
 *
 * Find the interface associated with the information in the header of the
 * packet passed as an argument to this routine.
 */

internal u_short find_interface(iid, if_vers)
uuid_$t *iid;
u_long if_vers;
{
    register u_short i;

    for (i = 0; i < n_interfaces; i++) {
        register ifrgy_elt_t *iface = &ifrgy[i];
        if (iface->inuse && uidequal(iface->ifs.id, *iid) && iface->ifs.vers == if_vers)
            return(i);
    }

    return(NO_HINT);
}


/*
 * R P C _ $ U N R E G I S T E R
 *
 * Unregister a previously registered interface.
 */

void rpc_$unregister(ifspec, st)
rpc_$if_spec_t *ifspec;
status_$t *st;
{
    register u_short ifno;
    register ifrgy_elt_t *iface;

    st->all = status_$ok;

    LOCK(SERVER_MUTEX);

    ifno = find_interface(&ifspec->id, ifspec->vers);

    if (ifno == NO_HINT) {
        UNLOCK(SERVER_MUTEX);
        st->all = nca_status_$unk_if;
        return;
    }

    iface = &ifrgy[ifno];

    if (! iface->generic) {
        if (--iface->e.s.refcnt == 0)
            iface->inuse = false;
    }
    else {
        /* ... do something here ... */
    }

    UNLOCK(SERVER_MUTEX);
}


/*
 * F I N D _ M G R _ E P V
 *
 * Scan down a ifrgy element's type info (list of [type, EPV] pairs) for
 * and entry that matches the specified type.  Returns the EPV if a match
 * is found, NULL otherwise.
 */

internal rpc_$mgr_epv_t find_mgr_epv(ti, type)
register if_type_info_t *ti;
uuid_$t *type;
{
    while (ti != NULL) {
        if (uidequal(ti->type, *type))
            return (ti->mepv);
        ti = ti->link;
    }

    return (NULL);
}


/*
 * R P C _ $ R E G I S T E R _ O B J E C T
 *
 * Declare that a server supports operations on a particular object and declare
 * the type of that object.  Registering objects is required only when generic
 * interfaces are declared (via "rpc_$register_mgr").  When a server receives
 * a call, the object identified in the call (i.e. the object which the client
 * specified in the "rpc_$bind") is searched for among the objects registered
 * by the server.  If the object is found, the type of the object is used to
 * decide which of the manager EPVs should be used to operate on the object.
 */

void rpc_$register_object(object, type, st)
uuid_$t *object;
uuid_$t *type;
status_$t *st;
{
    register objrgy_elt_t *or;

    if (n_objects >= MAX_OBJS) {
        st->all = rpc_$too_many_objects;
        return;
    }

    or = &objrgy[n_objects++];

    or->object = *object;
    or->type   = *type;

    st->all = status_$ok;
}


/*
 * I N Q _ O B J E C T _ T Y P E
 *
 * Consult the object registry for the specified object's type.  Returns true
 * if object is registered, false otherwise.  If object is not registered, the
 * type returned is uuid_$nil.  The type of the object identified by "uuid_$nil"
 * is defined to be "uuid_$nil".
 */

internal boolean inq_object_type(object, type)
uuid_$t *object;
uuid_$t *type;
{
    register u_short i;

    if (uidequal(uuid_$nil, *object)) {
        *type = uuid_$nil;
        return (true);
    }

    for (i = 0; i < n_objects; i++) {
        register objrgy_elt_t *or = &objrgy[i];

        if (uidequal(or->object, *object)) {
            *type = or->type;
            return (true);
        }
    }

    *type = uuid_$nil;
    return (false);
}


/*
 * F R E E _ R E P L Y
 *
 * Free a reply packet safely.
 */

internal void free_reply(act)
register activity_t *act;
{
    if (act->a.reply.opkt != NULL) {
        rpc_$free_pkt((rpc_$ppkt_p_t) act->a.reply.opkt);
        act->a.reply.opkt = NULL;
    }

    act->state = as_idle;
}


/*
 * S E N D _ R E P L Y
 *
 * Transmit one or more frags of the reply for the specified activity on
 * the specified socket.  The portions of the reply that are sent is
 * determined by the record of the highest in-order reply frags that's
 * been fack'd so far.
 */

internal void send_reply(sock, act)
int sock;
register activity_t *act;
{
    register reply_t *r = &act->a.reply;
    u_short i;
    boolean last_frag_in_blast;
    boolean last_frag_in_reply;
    u_long mtu;
    u_short n_pkts_per_blast;
    status_$t st;
    u_short out_fragnum;

    ASSERT(act->state == as_in_reply || act->state == as_replied);

    mtu = socket_$max_pkt_size((u_long) act->a.addr.sa.family, &st) - PKT_HDR_SIZE;
    if (act->a.encr)
        mtu -= rpc_$encr_overhead(act->a.encr);

    /*
     * If we have a non-large reply, just send it out and return.
     */

    if (r->len <= mtu) {
        ASSERT(act->state = as_replied);
        r->opkt->hdr.len = r->len;
        rpc_$sendto(sock, r->opkt, &act->a.addr, act->a.encr);
        return;
    }

    /*
     * The following logic sends out the frags for a large reply.
     */

    out_fragnum = r->fragnum;
    i = 0;
    n_pkts_per_blast = act->a.blast_outs ? MAX_PKTS_PER_BLAST : 1;

    do {
        rpc_$pkt_t fpkt;

        last_frag_in_reply = rpc_$fill_frag(r->opkt, r->len, out_fragnum, mtu, &fpkt);
        last_frag_in_blast = (i == n_pkts_per_blast - 1) || last_frag_in_reply;

        if (! last_frag_in_blast)
            fpkt.hdr.flags |= PF_NO_FACK;
        rpc_$sendto(sock, &fpkt, &act->a.addr, act->a.encr);

        i++, out_fragnum++;

    } while (! last_frag_in_blast);

    if (last_frag_in_reply)
        act->state = as_replied;
}


/*
 * S E T _ A C T _ Q U I T A B L E
 *
 * Set whether it is OK to deliver quits to a task running on behalf of
 * the specified activity.   Can be set to "not OK" (i.e. false) iff it
 * was previously set to "OK" (i.e. "true").  When setting to "not OK",
 * returns task handle of task that previously set it to "OK"; otherwise,
 * returns NULL.
 */

internal int *set_act_quitable(act, on)
activity_t *act;
boolean on;
{
#ifdef TASKING
    if (on) {
        act->a.task.quitable = true;
        act->a.task.handle = task_$get_handle();
        return (NULL);
    }
    else {
        int *handle = (int *) act->a.task.handle;
        act->a.task.quitable = false;
        act->a.task.handle = 0xeffffff3;
        return (handle);
    }
#else
    act->a.task.quitable = on;
    return (NULL);
#endif
}


/*
 * F I N D _ A C T I V I T Y
 *
 * Find the activity associated with the information in the header of the
 * packet passed as an argument to this routine.
 */

internal u_short find_activity(actuid)
uuid_$t *actuid;
{
    register u_short i;

    /*
     * Search for activity.  Hashing would probably be more appropriate.
     */

    for (i = 0; i < MAX_ACTIVITIES; i++)
        if (acts[i] != NULL && uidequal(acts[i]->id, *actuid)) {
            timestamp(acts[i]);
            return(i);
        }

    return(NO_HINT);
}


/*
 * G E T _ A C T I V I T Y
 *
 * Find the activity (as above), or allocate one if one doesn't exist.
 */

internal u_short get_activity(pkt, from, sock)
register rpc_$pkt_t *pkt;
rpc_$sockaddr_t *from;
int sock;
{
    short usable_entry;
    u_short ahint = pkt->hdr.ahint;
    register activity_t *act;

    /*
     * If call specified an activity hint that's valid, return the hint.
     */

    if (valid_ahint(pkt))
        return(ahint);

    /*
     * Look up the caller's activity in our table of activities, and return
     * its index if we find it.
     */

    ahint = find_activity(&pkt->hdr.actuid);

    if (ahint != NO_HINT)
        return(ahint);

    /*
     * Allocate a slot in our table of activies.
     */

    usable_entry = -1;

    for (ahint = 0; ahint < MAX_ACTIVITIES; ahint++) {
        if (acts[ahint] == NULL)
            break;
        if (acts[ahint]->state == as_idle || acts[ahint]->state == as_passive)
            usable_entry = ahint;
    }

    /*
     * If we didn't find a free entry, we lose.
     */

    if (ahint >= MAX_ACTIVITIES)
        if (usable_entry == -1) {
            eprintf("(get_activity) Can't allocate slot\n");
            return (NO_HINT);
        }
        else {
            if (acts[usable_entry]->state == as_idle)
                passivate_activity(usable_entry);
            free_activity(usable_entry);
            ahint = usable_entry;
        }

    acts[ahint] = act = (activity_t *) rpc_$nmalloc(sizeof *act);

    act->state        = as_idle;
    act->id           = pkt->hdr.actuid;
    act->prev_seq     = -1;
    act->a.reply.opkt = NULL;
    act->a.sock       = sock;
    act->a.frags.head = NULL;
    act->a.task.quitable = false;
#ifdef TASKING
    act->a.task.handle= 0xeffffff1;
#endif

    act->a.blast_outs = ((pkt->hdr.flags & PF_BLAST_OUTS) != 0);    /* See note on PF_BLAST_OUTS in "rpc_p.h" */
    act->a.auth = NULL;
    act->a.encr = NULL;

    copy_sockaddr_r(from, &act->a.addr);

    timestamp(act);

    return(ahint);
}


/*
 * F R E E _ A C T I V I T Y
 */

internal void free_activity(ahint)
u_short ahint;
{
    register activity_t *act;

    act = acts[ahint];
    acts[ahint] = NULL;

    rpc_$nfree(act);
}


/*
 * P A S S I V A T E  _ A C T I V I T Y
 *
 * Put our information about an activity into the passive state -- we remember
 * only the state, activity ID, the previous sequence number, and the timestamp.
 */

internal void passivate_activity(ahint)
u_short ahint;
{
    register activity_t *act, *new_act;

    act = acts[ahint];

    ASSERT(act->state == as_idle);

    /*
     * Allocate a short activity record and copy the fields from the old one.
     */

    acts[ahint] = new_act = (activity_t *) rpc_$nmalloc(sizeof(activity_t) -
                                                         sizeof(struct active_info_t));

    new_act->state    = as_passive;
    new_act->id       = act->id;
    new_act->prev_seq = act->prev_seq;

    timestamp(new_act);
    if (act->a.encr)
        rpc_$encr_destroy(act->a.encr);
    act->a.encr = NULL;
    if (act->a.auth)
        rpc_$sauth_destroy(act->a.auth);
    act->a.auth = NULL;
    rpc_$nfree(act);
}


/*
 * A C T I V A T E  _ A C T I V I T Y
 *
 * Put our information about an activity into the idle state from the passive
 * state.
 */

internal activity_t *activate_activity(ahint, from, sock)
u_short ahint;
rpc_$sockaddr_t *from;
int sock;
{
    register activity_t *act, *new_act;

    act = acts[ahint];

    ASSERT(act->state == as_passive);

    /*
     * Allocate a full activity record and copy the fields from the old one.
     * Give the "active" part some sensible values.
     */

    acts[ahint] = new_act = (activity_t *) rpc_$nmalloc(sizeof(activity_t));
    bzero(new_act, sizeof(activity_t));

    new_act->state        = as_idle;
    new_act->id           = act->id;
    new_act->prev_seq     = act->prev_seq;
    new_act->a.sock       = sock;
    new_act->a.auth       = NULL;
    new_act->a.encr       = NULL;

    copy_sockaddr_r(from, &new_act->a.addr);

    timestamp(new_act);

    rpc_$nfree(act);

    return(new_act);
}


/*
 * S C A N _ A C T I V I T I E S
 *
 * Scan all activities:  retransmit replies and free any old activities.
 */

    /* # of seconds between runs of this procedure */
#define SCAN_ACTIVITIES_INTERVAL    ((u_long) 3)
    /* # of seconds we'll retransmit replies for */
#define REPLY_TIME                  30
    /* # of seconds we'll hold an idle connection before passivating it */
#define IDLE_TIME                   45
    /* # of seconds we'll hold a passive connection before freeing it */
#define PASSIVE_TIME                (5 * 60)

internal void scan_activities()
{
    register u_short i;
    u_long now;

    /*
     * For each activity...
     */

    LOCK(SERVER_MUTEX);

    now = time(NULL);

    for (i = 0; i < MAX_ACTIVITIES; i++) {
        register activity_t *act = acts[i];
        short delta;                /* N.B. NOT unsigned; sometimes now < act->atime */

        if (act == NULL)
            continue;

        delta = ((long) now) - ((long) act->atime);

        switch (act->state) {
            case as_passive:
                if (delta > PASSIVE_TIME) {
                    dprintf("(scan_activities) Freeing passive [%s]\n", uidstring(&act->id));
                    free_activity(i);
                }
                break;

            case as_idle:
                if (delta > IDLE_TIME) {
                    dprintf("(scan_activities) Passivating idle [%s]\n", uidstring(&act->id));
                    passivate_activity(i);
                }
                break;

            case as_in_reply:
            case as_replied:
                if (delta > REPLY_TIME) {
                    dprintf("(scan_activities) Dropping reply [%s]\n",
                            uidseqstring(act->a.reply.opkt));
                    free_reply(act);
                }
                else {
                    dprintf("(scan_activities) Retransmitting reply (state=%s) [%s]\n",
                            state_name(act->state), uidseqstring(act->a.reply.opkt));
                    send_reply(act->a.sock, act);
                }
                break;
        }
    }

    UNLOCK(SERVER_MUTEX);
}


void rpc_$start_activity_scanner()
{
    rpc_$periodically(scan_activities, "RPC activity scanner", SCAN_ACTIVITIES_INTERVAL);
}


/*
 * P I N G _ C O M M O N
 *
 * Routine common to request and ping processing in case we got a packet
 * whose sequence is current.
 */

internal void ping_common(sock, act, pkt, from)
int sock;
register activity_t *act;
register rpc_$pkt_t *pkt;
rpc_$sockaddr_t *from;
{
    switch (act->state) {
        case as_working:
            dprintf("(ping_common) Working (ptype=%s) [%s]\n",
                    rpc_$pkt_name(pkt_type(pkt)), uidseqstring(pkt));
            set_pkt_type(pkt, rpc_$working);
            pkt->hdr.len = 0;
            rpc_$sendto(sock, pkt, from, NULL);
            break;

        case as_in_reply:
        case as_replied:
            dprintf("(ping_common) Resending reply (state=%s, ptype=%s, frag=%u) [%s]\n",
                    state_name(act->state), rpc_$pkt_name(pkt_type(pkt)), act->a.reply.fragnum,
                    uidseqstring(pkt));
            send_reply(sock, act);
            break;

        case as_idle:
        case as_passive:
        case as_frag:
            dprintf("(ping_common) No call (state=%s, ptype=%s) [%s]\n",
                    state_name(act->state), rpc_$pkt_name(pkt_type(pkt)),
                    uidseqstring(pkt));
            set_pkt_type(pkt, rpc_$nocall);
            pkt->hdr.len = 0;
            rpc_$sendto(sock, pkt, from, NULL);
            break;

        default:
            eprintf("(ping_common) Invalid activity state (state=%s, ptype=%s)\n",
                    state_name(act->state), rpc_$pkt_name(pkt_type(pkt)));
            DIE("invalid activity state");
    }
}


/*
 * G E T _ C A L L E R S _ A D D R
 *
 * Return the current activities client side address, used by rpc_$inq_binding
 */

void rpc_$get_callers_addr(h, addr, len)
handle_t h;
socket_$addr_t *addr;
u_long *len;
{
    server_handle_t *p = HANDLE_CAST(h);
    register activity_t *act = p->act;

    if (p->hhdr.cookie == SERVER_COOKIE && act->state != as_passive) {
        copy_sockaddr(&act->a.addr.sa, act->a.addr.len, addr, len);
    }
    else
        *len = 0;
}


/*
 * W H O _ A R E _ Y O U
 *
 * Server's interlude to "rpc_$who_are_you" RPC call.
 */

internal void who_are_you(act, st, auth_type)
register activity_t *act;
status_$t *st;
u_long auth_type;
{
    handle_t h;
    u_long seq;
    status_$t xst;
    pfm_$cleanup_rec crec;
    act_state_t old_state;

    dprintf("(who_are_you) Doing callback\n");

    h = rpc_$bind(&uuid_$nil, &act->a.addr.sa, act->a.addr.len, st);

    if (st->all != status_$ok) {
        dprintf("(who_are_you) Can't bind to client, st=%08lx\n", st->all);
        act->state = as_idle;
        return;
    }

    old_state = act->state;
    act->state = as_in_call_back;

    UNLOCK(SERVER_MUTEX);

    *st = pfm_$p_cleanup(&crec);
    if (st->all != pfm_$cleanup_set) {
        set_act_quitable(act, false);
        LOCK(SERVER_MUTEX);
        act->state = old_state;
        pfm_$enable();
        dprintf("(who_are_you) fault, st=%08lx\n", st->all);
    }
    else {
        set_act_quitable(act, true);
        if (auth_type != 0) {
            if (auth_rgy != NULL &&
                auth_type >= 0 && auth_type <= 255 &&
                auth_rgy[auth_type] != NULL)
            {
                act->a.auth = (*(auth_rgy[auth_type]))
                                    (h, &act->id, boot_time, &seq, st);
            }
            else
                st->all = rpc_$invalid_auth_type;
        }
        else
            (*conv_$client_epv.conv_$who_are_you)(h, &act->id, boot_time, &seq, st);
        set_act_quitable(act, false);
        LOCK(SERVER_MUTEX);
        act->state = old_state;
        pfm_$p_rls_cleanup(&crec, &xst);
    }

    if (st->all == status_$ok)
        act->prev_seq = seq - 1;
    else
        dprintf("(who_are_you) who_are_you failed, st=%08lx\n", st->all);

    rpc_$free_handle(h, &xst);
}


/*
 * H A N D L E _ R E Q U E S T _ F R A G
 *
 * Handle a fragment of a request.  Returns either NULL, if the entire
 * request has not yet arrived, or a pointer to the large (pseudo) packet
 * containing the whole request.  Note that since the returned packet could
 * be so large that the length field in the packet header isn't big enough
 * to hold the length, we return the real body length in the "body_len"
 * output parameter.
 */

internal rpc_$pkt_t *handle_request_frag(sock, from, ahint, lpkt, body_len)
int sock;
rpc_$sockaddr_t *from;
u_short ahint;
rpc_$linked_pkt_t *lpkt;
u_long *body_len;
{
    rpc_$pkt_t *pkt = &lpkt->pkt;
    register activity_t *act = acts[ahint];
    register rpc_$frag_list_t *fl = &act->a.frags;
    rpc_$spkt_t apkt;
    boolean last_frag = pkt->hdr.flags & PF_LAST_FRAG;
    boolean no_fack = (pkt->hdr.flags & PF_NO_FACK) && ! last_frag;
    boolean complete;

    act->state = as_frag;
    act->prev_seq = pkt->hdr.seq;

    /*
     * Insert this frag in the list of incoming frags.  If we have all
     * the frags now, reassemble the frags and return the frags as the
     * one large reassembled packet.  Otherwise, fack (unless told not
     * to) and return NULL.
     */

    rpc_$insert_in_frag_list(fl, lpkt, &complete);

    if (complete) {
        *body_len = fl->len;
        return (rpc_$reassemble_frag_list(fl));
    }
    else {
        if (! no_fack) {
            apkt.hdr = pkt->hdr;
            set_pkt_type(&apkt, rpc_$fack);
            apkt.hdr.len = 0;
            apkt.hdr.ahint = ahint;
            apkt.hdr.fragnum = fl->highest;      /* !!! -1 case is problematic !!! */
            rpc_$sendto(sock, (rpc_$pkt_t *) &apkt, from, act->a.encr);
        }
        *body_len = 0;
        return (NULL);
    }
}


/*
 * Q U I T _ A C T I V I T Y
 *
 * Stop whatever is going on on behalf of some activity.  This routine
 * is called by (1) the logic that handles "quit" packets ("do_quit")
 * and the logic that handles new requests ("do_request").  Calls from
 * the latter result when a new request comes in while we think we're
 * still doing a previous request.  (Probably a "quit" packet got lost.)
 */

internal void quit_activity(act)
register activity_t *act;
{
    switch (act->state) {
        case as_frag:
            rpc_$free_frag_list(&act->a.frags);
            act->state = as_idle;
            break;

        case as_in_call_back:
        case as_working:
            if (act->a.task.quitable) {
                int *handle;
                status_$t st, fst;

                handle = set_act_quitable(act, false);
                fst.all = QUIT_ACTIVITY_FAULT;
#ifdef TASKING
                task_$signal((task_$handle_t) handle, fst, &st);
#else
                pfm_$signal(fst);
#endif
            }
            break;

        case as_in_reply:
        case as_replied:
            free_reply(act);
            break;

        case as_idle:
        case as_passive:
            break;
    }
}



/*
 * D O _ R E Q U E S T
 *
 * Handle a "request" and "idem_request" packets.
 */

internal void do_request(sock, lpkt, from, cksum)
int sock;
rpc_$linked_pkt_t *lpkt;
rpc_$sockaddr_t *from;
rpc_$cksum_t cksum;
{
    register rpc_$pkt_t *pkt = &lpkt->pkt;
    register activity_t *act;
    u_short ahint;
    u_short ihint;
    status_$t st;
    boolean did_call_back = false;
    boolean idem      = ((pkt->hdr.flags & PF_IDEMPOTENT) != 0);
    boolean maybe     = ((pkt->hdr.flags & PF_MAYBE) != 0);
    boolean broadcast = ((pkt->hdr.flags & PF_BROADCAST) != 0);
    ifrgy_elt_t *iface;
    rpc_$pkt_t *opkt;
    rpc_$pkt_t tpkt;
    boolean must_free, got_frag;
    u_long olen;
    struct server_handle_t p;       /* Need non-"near" version here */
    pfm_$cleanup_rec crec;
    u_long mtu;
    boolean large;
    rpc_$drep_t drep;
    u_long body_len;
    status_$t fault_st;
    rpc_$ptype_t opkt_type;

    /*
     * Find interface if it's not specified.
     */

    if (pkt->hdr.ihint == NO_HINT)
        ihint = find_interface(&pkt->hdr.if_id, pkt->hdr.if_vers);
    else
        ihint = pkt->hdr.ihint;

    /*
     * Is interface valid?
     */

    if (ihint == NO_HINT ||
        ! uidequal(ifrgy[ihint].ifs.id, pkt->hdr.if_id) ||   /* interface UIDs match */
        ifrgy[ihint].ifs.vers != pkt->hdr.if_vers)        /* versions match */
    {
        dprintf("(do_request) Unknown interface [%s]\n", uidseqstring(pkt));

        if (! broadcast)
            send_rejection(sock, nca_status_$unk_if, pkt, from);

        return;
    }

    /*
     * Is the request operation number in range?
     */

    if (pkt->hdr.opnum >= ifrgy[ihint].ifs.opcnt) {
        dprintf("(do_request) Opnum out of range [%s]\n", uidseqstring(pkt));

        if (! broadcast)
            send_rejection(sock, nca_status_$op_rng_error, pkt, from);

        return;
    }

    pkt->hdr.ihint = ihint;
    iface = &(ifrgy[pkt->hdr.ihint]);

    /*
     * Get activity info for this call.
     */

    ahint = get_activity(pkt, from, sock);
    if (ahint == NO_HINT)
        return;

    pkt->hdr.ahint = ahint;
    act = acts[ahint];

    /*
     * In a perfect world, we should not get requests while we're in any
     * of the states: working, in_reply, or in_call_back.  In practice,
     * requests are sometimes duplicated and computations are sometimes
     * orphaned (because "quit" packets get lost).  Thus, we can find
     * ourselves with a request when we think we're already handling a
     * request.  So, if we think that's what's happened, here's what we
     * do:
     *
     * Ignore the request for sure if the sequence # is not newer than
     * the most recent one we know about.  (Note that "the most recent"
     * is the callback sequence number if we're doing a callback and the
     * previous sequence # otherwise.)  If it IS newer, quit any processing
     * that we're going with an old request.  Note that in either case
     * we return (and ignore the request).  In the case of the newer request,
     * we need to give the processing of the old request time to quit;
     * the client will surely retransmit the request (right?).
     */

    if (act->state == as_working ||
        act->state == as_in_reply ||
        act->state == as_in_call_back)
    {
        long cseq = (act->state == as_in_call_back ? act->a.cb_seq : act->prev_seq);

        if ((long) pkt->hdr.seq <= cseq)
            dprintf("(do_request) Got inappropriate request (state=%s) [%s]\n",
                    state_name(act->state), uidseqstring(pkt));
        else
            quit_activity(act);
        return;
    }

    if (act->state == as_passive)
        act = activate_activity(ahint, from, sock);

    /*
     * Make sure our current state makes sense.
     */

    if (! (act->state == as_idle || act->state == as_replied || act->state == as_frag)) {
        char buff[100];
        sprintf(buff, "(do_request) Invalid state (state=%s) [%s]",
                state_name(act->state), uidseqstring(pkt));
        DIE(buff);
    }

    /*
     * From here on out, we assume that either we're capable of handling a
     * request or we're in the middle of handling an incoming fragmented
     * request.
     */

    /*
     * Make sure the activity record holds the most recent information about
     * the sockaddr used by the calling activity and the socket we're using.
     * They both may have changed since the last call from the activity.
     */

    copy_sockaddr_r(from, &act->a.addr);
    act->a.sock = sock;

    /*
     * If the request is non-idempotent and we have no sequence number
     * (i.e. this is the first call from the client), we must call the
     * client back to get his sequence number in case this request is
     * actually a retransmission of a request that was actually executed
     * by a previous incarnation of this server.
     */

    if ((! idem) && act->prev_seq == -1) {
        act->a.cb_seq = pkt->hdr.seq;
        who_are_you(act, &st, (u_long) pkt->hdr.auth_type);
        if (st.all != status_$ok) {
            send_rejection(sock, st.all, pkt, from);
            return;
        }
        did_call_back = true;
    }

    /*
     * From this point on, the authentication information is assumed
     * to be set up.
     */

    if (act->a.auth) {
        if (act->a.encr == NULL) {
            act->a.encr = rpc_$sauth_get_encr(act->a.auth, &st);
            if (st.all != status_$ok) {
                dprintf("(do_request) couldn't get encr (status=%08lx)\n", st.all);
                send_rejection(sock, st.all, pkt, from);
                return;
            }
        }

        if (pkt->hdr.auth_type != rpc_$auth_type(act->a.encr))
            send_rejection(sock, rpc_$invalid_auth_type, pkt, from);

        unpack_drep(&drep, pkt->hdr.drep);

        rpc_$encr_recv_xform(act->a.encr, drep,
                             &pkt->body.args[pkt->hdr.len],
                             cksum, &pkt->hdr.seq, &st);
        if (st.all != status_$ok) {
            dprintf("(do_request) couldn't decrypt pkt (status=%08lx)\n", st.all);
            if (log_fn != NULL)
                (*log_fn)(st, &from->sa, from->len);
            send_rejection(sock, st.all, pkt, from);
            return;
        }
    }

    /*
     * Check seq # for this request.  If it is old, ignore the request.
     */

    if ((long) pkt->hdr.seq < act->prev_seq) {
        dprintf("(do_request) Old sequence, previous=%ld [%s]\n",
                act->prev_seq, uidseqstring(pkt));
        return;
    }

    /*
     * If this request matches the previous one we handled (and doesn't
     * match simply because we're handling a multi-fragment request) then
     * consider the possibility of resending any saved reply.  If we're
     * in the "replied" state, then treat the request (a probable duplicate)
     * as a "ping" (which will cause the reply to be resent).  If this
     * is a non-idempotent request we're in trouble -- we must have dropped
     * the reply too soon (or the client did something bad); tell the client
     * the bad news.  If it IS an idempotent request, we'll just fall through
     * and re-execute the call.
     */

    if (pkt->hdr.seq == act->prev_seq && act->state != as_frag) {
        if (act->state == as_replied) {
            ping_common(sock, act, pkt, from);
            return;
        }
        if (! idem) {
            send_rejection(sock, nca_status_$proto_error, pkt, from);
            return;
        }
    }

    /*
     * If we're doing frag reassembly and the current request's seq # doesn't
     * match the previous seq # (i.e. the seq #'s on the frags we have
     * so far), then just drop the frags.  This situation can happen in
     * a fragmented idempotent calls -- stray fragments received after
     * a previous call finished caused us to try to start reassembling
     * the same call again.
     */

    if (act->state == as_frag && pkt->hdr.seq != act->prev_seq) {
        rpc_$free_frag_list(&act->a.frags);
        act->state = as_idle;
    }

    /*
     * If we did a callback and the current pkt's seq is not identical
     * to the result of the callback, someone's screwed up the protocol
     * real bad.  (Note: "who_are_you" fills in the prev_seq field of the
     * activity record to be the result of the callback less one.)
     */

    if (did_call_back && pkt->hdr.seq != act->prev_seq + 1) {
        dprintf("(do_request) Protocol error [%s]\n", uidseqstring(pkt));

        if (! broadcast)
            send_rejection(sock, nca_status_$proto_error, pkt, from);

        return;
    }

    /*
     * Handle fragmentary requests.  "handle_request_frag" does the work and
     * lets us know whether all the frags are now here and assembled by
     * returning a pointer to the assembled packet.  It returns NULL if
     * all the frags are not yet here.  We return now if we don't have
     * all the frags.
     */

    if (pkt->hdr.flags & PF_FRAG) {
        pkt = handle_request_frag(sock, from, ahint, lpkt, &body_len);
        if (pkt == NULL)
            return;
        got_frag = true;
    }
    else {
        body_len = pkt->hdr.len;
        got_frag = false;
    }

    /*
     * ====== We get here if the request is apparently new (and complete) ======
     */


    /*
     * Toss any saved reply (this call is an implicit ack of it).  This puts
     * us in the idle state.
     */

    free_reply(act);

    /*
     * We then switch to the working state.  All premature return paths from now
     * on must transition back to the idle state before returning.
     */

    act->state         = as_working;
    act->prev_seq      = pkt->hdr.seq;
    act->a.n_callbacks = 0;
    act->a.pkt         = pkt;

    get_my_activity(&act->a.task.id);

    /*
     * Call the server routine, send the response to the client and maybe save
     * the response locally.
     */

    UNLOCK(SERVER_MUTEX);

    mtu = socket_$max_pkt_size((u_long) from->sa.family, &st) - PKT_HDR_SIZE;
    if (act->a.encr)
        mtu -= rpc_$encr_overhead(act->a.encr);

    p.hhdr.data_offset = sizeof(rpc_$pkt_hdr_t);
    p.hhdr.cookie      = SERVER_COOKIE;

    p.act = act;

    must_free = false;
    large = false;
    fault_st.all = status_$ok;

    unpack_drep(&drep, pkt->hdr.drep);

    st = pfm_$p_cleanup(&crec);

    if (st.all != pfm_$cleanup_set) {
        fault_st = st;
        set_act_quitable(act, false);
        LOCK(SERVER_MUTEX);
        pfm_$enable();
        dprintf("(do_request) Fault while executing request, st=%08lx\n", fault_st);

        opkt = &tpkt;
        olen = 4;    /* sizeof status_$t on the wire */

        opkt_type = rpc_$fault;
        rpc_$set_pkt_body_st(opkt,
                             (faults_are_fatal || rpc_$termination_fault(fault_st)) ?
                                rpc_$comm_failure :
                                fault_st.all
                            );
    }
    else {
        /*
         * Make the call
         */

#ifndef NO_STATS
        rpc_$stats.calls_in++;
#endif
        set_act_quitable(act, true);

        if (! iface->generic)
            (*(iface->e.s.epv[pkt->hdr.opnum]))(
                (handle_t) &p,
                (rpc_$ppkt_p_t) pkt,        /* pointer to INs */
                body_len,                   /* length of INs */
                (rpc_$ppkt_p_t) &tpkt,      /* pointer to place to put OUTs */
                (u_long) sizeof(tpkt.body), /* size of above place */
                drep,                       /* client's data representation */
                (rpc_$ppkt_p_t *) &opkt,    /* where OUTs really are */
                &olen,                      /* real size of OUTs */
                &must_free,                 /* => OUTs must be freed by caller */
                &st
                );
        else {
            uuid_$t type;
            rpc_$mgr_epv_t mepv;

            inq_object_type(&pkt->hdr.object, &type);

            mepv = find_mgr_epv(iface->e.g.ti, &type);

            if (mepv == NULL)
                raisec(nca_status_$unsupported_type);

            (*(iface->e.g.epv[pkt->hdr.opnum]))(
                (handle_t) &p,
                (rpc_$ppkt_p_t) pkt,        /* pointer to INs */
                body_len,                   /* length of INs */
                (rpc_$ppkt_p_t) &tpkt,      /* pointer to place to put OUTs */
                (u_long) sizeof(tpkt.body), /* size of above place */
                drep,                       /* client's data representation */
                mepv,                       /* manager EPV */
                (rpc_$ppkt_p_t *) &opkt,    /* where OUTs really are */
                &olen,                      /* real size of OUTs */
                &must_free,                 /* => OUTs must be freed by caller */
                &st
                );
        }

        set_act_quitable(act, false);

        LOCK(SERVER_MUTEX);

        if (olen > rpc_$max_body_size) {
            if (must_free)
                rpc_$free_pkt((rpc_$ppkt_p_t) opkt);

            opkt = &tpkt;
            olen = 4;    /* sizeof status_$t on the wire */

            opkt_type = rpc_$reject;
            rpc_$set_pkt_body_st(opkt, nca_status_$out_args_too_big);
        }
        else {
            large = (olen > mtu);
            opkt_type = rpc_$response;
        }

        pfm_$p_rls_cleanup(&crec, &st);
    }

    if (maybe) {
        act->a.reply.opkt = NULL;
        act->state = as_idle;
    }
    else {

        /*
         * For non-maybe requests:  Process the output parameters.  If this
         * is a non-idempotent request or a large response, we must save the
         * packet.  Send the reply packet.
         */

        opkt->hdr = pkt->hdr;

        opkt->hdr.ahint   = ahint;
        opkt->hdr.fragnum = 0;
        opkt->hdr.flags   = 0;

        set_pkt_type(opkt, opkt_type);

        if (idem && ! large) {
            act->a.reply.opkt = NULL;
            act->state = as_idle;
            opkt->hdr.len = olen;
            rpc_$sendto(sock, opkt, from, act->a.encr);
        }
        else {
            u_long alloc_sz = olen;

            act->a.reply.len = olen;
            if (act->a.encr)
                alloc_sz += rpc_$encr_overhead(act->a.encr);
            act->a.reply.opkt = (rpc_$pkt_t *) rpc_$alloc_pkt(alloc_sz);
            bcopy(opkt, act->a.reply.opkt, (int) (sizeof(rpc_$pkt_hdr_t) + olen));
            act->state = large ? as_in_reply : as_replied;
            act->a.reply.fragnum = 0;
            send_reply(sock, act);
        }
    }

    if (got_frag)
        rpc_$free_pkt((rpc_$ppkt_p_t) pkt);

    /*
     * Timestamp the activity since the call might have taken a while and
     * the caller may not have talked to us recently and we don't want the
     * reply we just generated freed up right away.
     */

    timestamp(act);

    if (must_free)
        rpc_$free_pkt((rpc_$ppkt_p_t) opkt);

    act->a.pkt  = NULL;

    /*
     * If we got some non-RPC fault during the server routine and faults
     * are fatal, make sure we blow out now.
     */

    if (faults_are_fatal &&
        fault_st.all != status_$ok &&
        (fault_st.all & 0xffff0000) != rpc_$mod)
    {
#ifdef TASKING
        task_$signal(task_$dt_handle, fault_st, &st);
#else
        pfm_$signal(fault_st);
#endif
    }

    /*
     * If we got a termination fault while executing the server routine,
     * we better re-signal it now.  (Note that for a tasking environment,
     * we do this iff this task is the distinguished task.  Actually, only
     * the d.t. should get this kind of fault, but better safe than sorry.)
     */

    if (rpc_$termination_fault(fault_st)
#ifdef TASKING
        && task_$get_handle() == task_$dt_handle
#endif
        )
    {
        pfm_$signal(fault_st);
    }
}


/*
 * D O _ P I N G
 *
 * Handle a ping packet.
 */

internal void do_ping(sock, pkt, from)
int sock;
register rpc_$pkt_t *pkt;
rpc_$sockaddr_t *from;
{
    register activity_t *act;

    if (pkt->hdr.ahint == NO_HINT)
        pkt->hdr.ahint = find_activity(&pkt->hdr.actuid);

    /*
     * If there's no activity, then say "no call".
     */

    if (! valid_ahint(pkt)) {
        dprintf("(do_ping) No call (no activity for this call) [%s]\n",
                uidseqstring(pkt));
        set_pkt_type(pkt, rpc_$nocall);
        pkt->hdr.len = 0;
        rpc_$sendto(sock, pkt, from, (rpc_$encr_t *) NULL);
        return;
    }

    act = acts[pkt->hdr.ahint];
    timestamp(act);

    /*
     * If there is an activity but the ping's sequence number is greater
     * than the one stored with the activity, then say "no call".  (A packet
     * must have been lost.)
     */

    if ((long) pkt->hdr.seq > act->prev_seq) {
        dprintf("(do_ping) No call (higher numbered ping), ahint=%d, previous=%ld [%s]\n",
                pkt->hdr.ahint, act->prev_seq, uidseqstring(pkt));
        set_pkt_type(pkt, rpc_$nocall);
        pkt->hdr.len = 0;
        rpc_$sendto(sock, pkt, from, (rpc_$encr_t *) NULL);
        return;
    }

    if (pkt->hdr.seq == act->prev_seq) {
        ping_common(sock, act, pkt, from);
        return;
    }

    dprintf("(do_ping) Drop ping [%s]\n", uidseqstring(pkt));
}


/*
 * D O _ A C K
 *
 * Handle an acknowledgment packet.
 */

internal void do_ack(sock, pkt, from)
int sock;
register rpc_$pkt_t *pkt;
rpc_$sockaddr_t *from;
{
    register activity_t *act;

    /*
     * Acks must contains (correct) ahints
     */

    if (! valid_ahint(pkt)) {
        dprintf("(do_ack) No or incorrect ahint in ack, ahint=%d [%s]\n",
                pkt->hdr.ahint, uidseqstring(pkt));
        return;
    }

    act = acts[pkt->hdr.ahint];
    timestamp(act);

    if (act->state == as_replied && (long) pkt->hdr.seq >= act->prev_seq)
        free_reply(act);
}


/*
 * D O _ F A C K
 *
 * Handle a fragment acknowledgment packet.
 */

internal void do_fack(sock, pkt, from)
int sock;
register rpc_$pkt_t *pkt;
rpc_$sockaddr_t *from;
{
    register activity_t *act;

    /*
     * Facks must contains (correct) ahints
     */

    if (! valid_ahint(pkt)) {
        dprintf("(do_fack) No or incorrect ahint in fack, ahint=%d [%s]\n",
                pkt->hdr.ahint, uidseqstring(pkt));
        return;
    }

    act = acts[pkt->hdr.ahint];
    timestamp(act);

    /*
     * Facks are appropriate only if we're in the "in reply" state and if
     * their sequence numbers match.
     */

    if (act->state != as_in_reply || (long) pkt->hdr.seq != act->prev_seq) {
        dprintf("(do_fack) Anomalous fack, state=%s, prev_seq=%lu [%s]\n",
                state_name(act->state), act->prev_seq, uidseqstring(pkt));
        return;
    }

    if (pkt->hdr.fragnum + 1 >= act->a.reply.fragnum)
        act->a.reply.fragnum = pkt->hdr.fragnum + 1;

    send_reply(sock, act);
}


/*
 * D O _ Q U I T
 *
 * Handle a quit packet.
 */

internal void do_quit(sock, pkt, from)
int sock;
register rpc_$pkt_t *pkt;
rpc_$sockaddr_t *from;
{
    register activity_t *act;

    if (pkt->hdr.ahint == NO_HINT)
        pkt->hdr.ahint = find_activity(&pkt->hdr.actuid);

    /*
     * If there's no activity, then say "no call".
     */

    if (! valid_ahint(pkt)) {
        dprintf("(do_quit) No call (no activity for this call) [%s]\n",
                uidseqstring(pkt));
        set_pkt_type(pkt, rpc_$nocall);
        pkt->hdr.len = 0;
        rpc_$sendto(sock, pkt, from, (rpc_$encr_t *) NULL);
        return;
    }

    act = acts[pkt->hdr.ahint];
    timestamp(act);

    /*
     * Let 'em know we heard them.
     */

    set_pkt_type(pkt, rpc_$quack);
    pkt->hdr.len = 0;
    rpc_$sendto(sock, pkt, from, (rpc_$encr_t *)NULL);

    quit_activity(act);
}


/*
 * D O _ B A D _ P K T
 *
 * Handle a bad packet.
 */

internal void do_bad_pkt(sock, pkt, from)
int sock;
register rpc_$pkt_t *pkt;
rpc_$sockaddr_t *from;
{
    dprintf("(do_bad_pkt) Bad pkt, ptype=%s [%s]\n",
            rpc_$pkt_name(pkt_type(pkt)), uidseqstring(pkt));
}


/*
 * R P C _ $ I N T _ L I S T E N _ D I S P A T C H
 *
 * Do server-side processing for one incoming packet.  This routine is useful
 * in case you got an RPC packet by some other means.
 */

void rpc_$int_listen_dispatch(sock, lpkt, cksum, from, from_len, st)
u_long sock;
rpc_$linked_pkt_t *lpkt;
rpc_$cksum_t cksum;
socket_$addr_t *from;
u_long from_len;
status_$t *st;
{
    register rpc_$pkt_t *pkt = &lpkt->pkt;
    rpc_$sockaddr_t addr;
    pfm_$cleanup_rec crec;
    rpc_$ptype_t ptype;
    static void (*pkt_handlers[MAX_PKT_TYPE + 1])() = {
                NULL,           /* rpc_$request -- handled specially */
                do_ping,        /* rpc_$ping           */
                do_bad_pkt,     /* rpc_$response       */
                do_bad_pkt,     /* rpc_$fault          */
                do_bad_pkt,     /* rpc_$working        */
                do_bad_pkt,     /* rpc_$nocall         */
                do_bad_pkt,     /* rpc_$reject         */
                do_ack,         /* rpc_$ack            */
                do_quit,        /* rpc_$quit           */
                do_fack,        /* rpc_$fack           */
                do_bad_pkt      /* rpc_$quack          */
                };

    st->all = status_$ok;
    if (pkt->hdr.ahint >= MAX_ACTIVITIES && pkt->hdr.ahint != NO_HINT) {
        dprintf("(rpc_$int_listen_dispatch) Trash ahint (%u)\n", pkt->hdr.ahint);
        st->all = rpc_$bad_pkt;
        return;
    }

    if (pkt->hdr.ihint >= MAX_IFS && pkt->hdr.ihint != NO_HINT) {
        dprintf("(rpc_$int_listen_dispatch) Trash ihint (%u)\n", pkt->hdr.ihint);
        st->all = rpc_$bad_pkt;
        return;
    }

    copy_sockaddr(from, from_len, &addr.sa, &addr.len);

    LOCK(SERVER_MUTEX);
    if (boot_time == 0)
        boot_time = time(NULL);
    UNLOCK(SERVER_MUTEX);

    /*
     * If the packet has a non-null server boot time and it's not our
     * boot time, tell the caller he's losing (i.e. we crashed).
     */

    if (pkt->hdr.server_boot != 0 && pkt->hdr.server_boot != boot_time) {
        dprintf("(rpc_$int_listen_dispatch) Server boot time mismatch\n");
        send_rejection((int) sock, nca_status_$wrong_boot_time, pkt, &addr);
        return;
    }

    pkt->hdr.server_boot = boot_time;    /* This will go back to the client */

    LOCK(SERVER_MUTEX);     /* Single thread while processing a packet */

    *st = pfm_$p_cleanup(&crec);
    if (st->all != pfm_$cleanup_set) {
        UNLOCK(SERVER_MUTEX);
        pfm_$signal(*st);
    }

    /*
     * Dispatch to the appropriate routine based on packet type.  Note that
     * for simplicity, all the routines other than the "request" pkt type
     * processor get passed a "rpc_$pkt_t *" instead of the "rpc_$linked_pkt_t *"
     * that we were passed.  "do_request" is the only one who might want to
     * link the packet onto a list.
     */

    ptype = pkt_type(pkt);

    if (ptype == rpc_$request)
        do_request((int) sock, lpkt, &addr, cksum);
    else
        (*pkt_handlers[(u_short) ptype])((int) sock, pkt, &addr);

    if (cksum != NULL && rpc_$encr_rgy[pkt->hdr.auth_type] != NULL) {
        rpc_$destroy_prexform(pkt->hdr.auth_type, cksum);
        cksum = NULL;
    }

    pfm_$p_rls_cleanup(&crec, st);

    UNLOCK(SERVER_MUTEX);   /* Resume multi-threading */
}


/*
 * R P C _ $ L I S T E N _ D I S P A T C H
 *
 * Exported version of "rpc_$int_listen_dispatch".  This routine copies the
 * supplied packet into allocated storage, which is where "listen_dispatch"
 * wants it.  Not terribly efficient, but no one who matters calls this
 * routine anyway.
 */

void rpc_$listen_dispatch(sock, _pkt, cksum, from, from_len, st)
u_long sock;
rpc_$ppkt_p_t _pkt;
rpc_$cksum_t cksum;
socket_$addr_t *from;
u_long from_len;
status_$t *st;
{
    register rpc_$pkt_t *pkt = (rpc_$pkt_t *) _pkt;
    register rpc_$linked_pkt_t *lpkt = NULL;
    pfm_$cleanup_rec crec;
    status_$t fst;

    fst = pfm_$p_cleanup(&crec);
    if (fst.all != pfm_$cleanup_set) {
        if (lpkt != NULL)
            rpc_$free_linked_pkt(lpkt);
        pfm_$signal(fst);
    }

    lpkt = rpc_$alloc_linked_pkt(pkt, (u_long) pkt->hdr.len);

    rpc_$int_listen_dispatch(sock, lpkt, cksum, from, from_len, st);

    rpc_$free_linked_pkt(lpkt);
    pfm_$p_rls_cleanup(&crec, st);
}


/*
 * R P C _ $ S E T _ F A U L T _ M O D E
 *
 * Control how faults occurring in user server routines are handled.
 * Calling this procedure with "false" puts it in the default mode:  the
 * fault is reflected back to the client and the server continues processing.
 * Calling this procedure with "true" causes any user server routine fault
 * to cause a "communications failure" fault back to the client and the
 * server to exit.  (In a tasking environment, this means that the DT is
 * signalled.)
 */

ndr_$ulong_int rpc_$set_fault_mode(on)
u_long on;
{
    boolean oldval = faults_are_fatal;

    faults_are_fatal = (boolean) on;
    return ((u_long) oldval);
}


/*
 * R P C _ $ I N Q _ O B J E C T
 *
 * Called by a server routine to find out the ID of the object that the client
 * supplied.
 */

void rpc_$inq_object(h, obj, st)
handle_t h;
uuid_$t *obj;
status_$t *st;
{
    register server_handle_t *p = HANDLE_CAST(h);

    if (p->hhdr.cookie == CLIENT_COOKIE)
        rpc_$inq_object_client(h, obj, st);
    else {
        register activity_t *act = p->act;

        if (act->state != as_working) {
            st->all = rpc_$not_in_call;
            return;
        }

        *obj = act->a.pkt->hdr.object;
        st->all = status_$ok;
    }
}


/*
 * R P C _ $ I N Q _ S A U T H
 *
 * Called by server routine to find handle on caller's identity.
 */

rpc_$server_auth_t *rpc_$inq_sauth(h, st)
handle_t h;
status_$t *st;
{
    register activity_t *act;
    st->all = status_$ok;

    act = (HANDLE_CAST(h))->act;
    if (act->state == as_passive) {
        st->all = rpc_$invalid_handle;
        return NULL;
    }
    else {
        if (act->a.auth == NULL)
            st->all = rpc_$not_authenticated;
        return (act->a.auth);
    }
}


/*
 * R P C _ $ S E T _ S E R V E R _ E N C R
 */

void rpc_$set_server_encr(h, encr, st)
handle_t h;
rpc_$encr_t *encr;
status_$t *st;
{
    register activity_t *act;
    st->all = status_$ok;

    act = (HANDLE_CAST(h))->act;
    if (act->state == as_passive) {
        st->all = rpc_$invalid_handle;
    }
    else {
        if (act->a.encr)
            rpc_$encr_destroy(act->a.encr);
        act->a.encr = encr;
    }
}


/*
 * R P C _ $ I N Q _ S E R V E R _ E N C R
 *
 * Called by rpc_$inq_encr if the handle is a server-side handle.
 */

rpc_$encr_t *rpc_$inq_server_encr(h, st)
handle_t h;
status_$t *st;
{
    register activity_t *act;
    st->all = status_$ok;

    act = (HANDLE_CAST(h))->act;
    if (act->state == as_passive) {
        st->all = rpc_$invalid_handle;
        return NULL;
    }
    else
        return (act->a.encr);
}

/*
 * R P C _ $ R E G I S T E R _ A U T H T Y P E
 *
 * Set the "who_are_you" function for a given authentication type.
 */

void rpc_$register_authtype(auth_type, way, st)
u_long auth_type;
rpc_$way_fn_t way;
status_$t *st;
{
    register int i;

    if (auth_type >= rpc_$max_auth_type) {
        st->all = rpc_$invalid_auth_type;
        return;
    }

    if (auth_rgy == NULL) {
        auth_rgy = (rpc_$way_fn_t NEAR *) rpc_$nmalloc(rpc_$max_auth_type * sizeof(rpc_$way_fn_t));
        for (i = 0; i < rpc_$max_auth_type; i++)
            auth_rgy[i] = NULL;
    }

    auth_rgy[auth_type] = way;
    st->all = status_$ok;
}


/*
 * R P C _ $ S E T _ A U T H _ L O G G E R
 *
 * Sets the function to call when an authentication failure is detected.
 */

void rpc_$set_auth_logger(lproc)
rpc_$auth_log_fn_t lproc;
{
    log_fn = lproc;
}


/*
 * R R P C _ $ I N Q _ I N T E R F A C E S
 *
 * Return a list of interfaces the server currently exports.
 */

void rrpc_$inq_interfaces(h, max_ifs, ifs, l_if, st)
handle_t h;
u_long max_ifs;
rrpc_$interface_vec_t ifs;
long *l_if;
status_$t *st;
{
    register u_short i;

    for (i = 0, *l_if = 0; i < n_interfaces && *l_if < max_ifs; i++)
        if (ifrgy[i].inuse)
            ifs[(*l_if)++] = ifrgy[i].ifs;

    (*l_if)--;

    st->all = status_$ok;
}


#ifdef GLOBAL_LIBRARY

/*
 * R P C _ $ S E R V E R _ M A R K _ R E L E A S E
 *
 * Support for Apollo multiple programs per process ("program levels").
 *
 * This code does what's barely necessary so that you can invoke a server
 * sequentially in the same process.  It doesn't handle the case of a server
 * invoking another server in-process.  This seems pretty unlikely, no?
 *
 */

void rpc_$server_mark_release(is_mark, level, st, is_exec)
boolean *is_mark;
u_short *level;
status_$t *st;
boolean *is_exec;
{
    u_short i;

    st->all = status_$ok;

    if (*is_mark) {
        level_info_t *li = &level_info[*level];

        li->n_interfaces = n_interfaces;
        li->n_objects = n_objects;
    }
    else {
        level_info_t *li = &level_info[*level + 1];

        n_interfaces = li->n_interfaces;
        n_objects = li->n_objects;
        boot_time = 0;
        for (i = 0; i < MAX_ACTIVITIES; i++)
            acts[i] = NULL;
    }
}
#endif

#ifdef FTN_INTERLUDES

void rpc_$register_(ifspec, epv, st)
rpc_$if_spec_t *ifspec;
rpc_$epv_t epv;
status_$t *st;
{
    rpc_$register(ifspec, epv, st);
}

void rpc_$register_mgr_(type, ifspec, sepv, mepv, st)
uuid_$t *type;
rpc_$if_spec_t *ifspec;
rpc_$generic_epv_t sepv;
rpc_$mgr_epv_t mepv;
status_$t *st;
{
    rpc_$register_mgr(type, ifspec, sepv, mepv, st);
}

void rpc_$unregister_(ifspec, st)
rpc_$if_spec_t *ifspec;
status_$t *st;
{
    rpc_$unregister(ifspec, st);
}

void rpc_$register_object_(object, type, st)
uuid_$t *object;
uuid_$t *type;
status_$t *st;
{
    rpc_$register_object(object, type, st);
}

void rpc_$listen_dispatch_(sock, _pkt, cksum, from, from_len, st)
u_long *sock;
rpc_$ppkt_p_t _pkt;
rpc_$cksum_t cksum;
socket_$addr_t *from;
u_long *from_len;
status_$t *st;
{
    rpc_$listen_dispatch(*sock, _pkt, cksum, from, *from_len, st);
}

u_long rpc_$set_fault_mode_(on)
u_long *on;
{
    return (rpc_$set_fault_mode(*on));
}

void rpc_$inq_object_(h, obj, st)
handle_t *h;
uuid_$t *obj;
status_$t *st;
{
    rpc_$inq_object(*h, obj, st);
}

#endif
