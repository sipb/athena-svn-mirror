/*      glb.c
 *      Location Broker - GLB client agent
 *
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
 */

#include "std.h"

#include "pbase.h"

#ifdef DSEE
#  include "$(lb.idl).h"
#  include "$(glb.idl).h"
#  include "$(uuid.idl).h"
#  include "$(socket.idl).h"
#else
#  include "lb.h"
#  include "glb.h"
#  include "uuid.h"
#  include "socket.h"
#endif

#include "pfm.h"

#define GLB_MODULE

#include "lb_p.h"
#include "glb_p.h"

#ifdef __STDC__
static boolean has_server_list(void);
static void open_server_list(status_$t *st);
static void close_server_list(void);
static void find_server_nbrd(handle_t h, u_long family, status_$t *st);
static int check_binding( status_$t *status);
static void glb_register_ops(void (*op)(), lb_$entry_t *xentry, status_$t *status);
static sort_entries(int num, lb_$entry_t *entries);
extern void glb_ca_$get_object_uuid (uuid_$t *obj);
#endif

#ifdef apollo
#  define INVALIDATE_GLOBAL_BROKER_ADDR \
        u_long slen;\
        status_$t ndummy; \
        socket_$addr_t addr;\
        \
        rpc_$inq_binding(glb_$handle, &addr, &slen, &ndummy);\
        valid_binding = FALSE; \
        invalidate_global_broker_address(&addr);
#else
#  define INVALIDATE_GLOBAL_BROKER_ADDR \
        valid_binding = FALSE;
#endif

#define EXCEPTION_RANGE \
        { \
        pfm_$cleanup_rec crec; \
        status_$t fst; \
        fst = pfm_$p_cleanup(&crec);

#define EXCEPTION_END \
        if (STATUS(&fst) != pfm_$cleanup_set) \
            pfm_$enable(); \
        else \
            pfm_$p_rls_cleanup(&crec, &fst); \
        }

#define EXCEPTION_CASE \
        if (STATUS(&fst) != pfm_$cleanup_set) { \
                INVALIDATE_GLOBAL_BROKER_ADDR; \
                if ((STATUS(&fst) & 0xffff0000) != rpc_$mod) { \
                        pfm_$signal(fst); \
                } \
        } if (STATUS(&fst) != pfm_$cleanup_set)


#ifdef DEBUG_GLB
#  define print_rpc_error(st) rpc_$status_print("GLB: ", *(st));
#else
#  define print_rpc_error(st)
#endif

/*
** Static data - per process private data
*/
static int      valid_binding = 0;
static u_long   use_short_timeouts = 0;
static struct {
        u_long          saddr_len;
        socket_$addr_t  saddr;
        boolean         valid;
        short           pad;
} af[socket_$num_families];   /* init to 0, valid == false */

#ifdef GLOBAL_LIBRARY
#   define glb_$default_obj glb_$default_obj_pure_data$
#   define glb_$default_type glb_$default_type_pure_data$
#endif

uuid_$t glb_$default_obj = { 0x333b91c5, 0x0000, 0x0000, 0x0d,
                                { 0x00, 0x00, 0x87, 0x84, 0x00, 0x00, 0x00 } };

uuid_$t glb_$default_type = { 0x333b91de, 0x0000, 0x0000, 0x0d,
                                { 0x00, 0x00, 0x87, 0x84, 0x00, 0x00, 0x00 } };

/*
** Shared Static data
*/
#ifdef apollo
#  include "set_sect.pvt.c"
#endif


/*
 * goop for those how can't (or won't) broadcast
 */

#ifndef GLB_SITES
#  ifdef UNIX
#    define GLB_SITES       "/etc/ncs/glb_site.txt"
#  endif
#  ifdef vms
#    define GLB_SITES       "ncs$exe:glb_site.txt"
#  endif
#  ifdef MSDOS
#    define GLB_SITES       "c:\\ncs\\glb_site.txt"
#  endif
#endif

#ifndef GLB_OBJ_FILE
#  ifdef UNIX
#    define GLB_OBJ_FILE    "/etc/ncs/glb_obj.txt"
#  endif
#  ifdef vms
#    define GLB_OBJ_FILE    "ncs$exe:glb_obj.txt"
#  endif
#  ifdef MSDOS
#    define GLB_OBJ_FILE    "c:\\ncs\\glb_obj.txt"
#  endif
#endif

static boolean use_server_list = false;
static int sitefd = -1;
static long siteoff = 0;
static socket_$string_t glb_location;
static u_long glb_loc_size;

extern boolean rpc_$debug;
#define dprintf if (rpc_$debug) printf

static void get_default_object ( obj )
    uuid_$t *obj;
{
    int             fd;
    uuid_$string_t  uuid_string;
    status_$t       st;

    st.all = -1;

    fd = open(GLB_OBJ_FILE, O_RDONLY, 0);
    if (fd > -1) {
        if (read(fd, uuid_string, sizeof(uuid_string)) == sizeof(uuid_string)) {
            uuid_$decode(uuid_string, obj, &st);
        }
    }

    if (st.all != status_$ok) {
        *obj = glb_$default_obj;
    }
}

static boolean has_server_list()
{
    return(access(GLB_SITES, 0) != -1 ? true : false);
}

static void open_server_list(st)
status_$t       *st;
{
    st->all = status_$ok;
    if (sitefd != -1) {
        close(sitefd);
        sitefd = -1;
    }

    siteoff = 0;
    if ((sitefd = open(GLB_SITES, O_RDONLY)) == -1) {
        dprintf("Can't open '%s', errno=%d\n", GLB_SITES, errno);
        st->all = rpc_$comm_failure;  /* look like a GLB comm failure */
    }
}

static void close_server_list()
{
    if (sitefd == -1)
        return;

    close(sitefd);
    sitefd = -1;
}

/*
 * This routine's job is to bind the handle to a *working* glb
 * (of the specified family) in the case that a broadcast
 * capability is unavailable.
 */
static void find_server_nbrd(h, family, st)
handle_t        h;
u_long          family;
status_$t       *st;
{
    socket_$addr_t saddr;
    u_long slen = sizeof saddr;
    int cc, i;
    u_long nresults = 0;
    boolean firstpass = true;
    u_long glb_port;
    lb_$entry_t results[socket_$num_families], *rp;
    lb_$lookup_handle_t lookup_handle;
    pfm_$cleanup_rec crec;
    status_$t dummy, fst;
    char *cp, *lp;
    char buf[100];
    socket_$string_t location;

    /*
     * Get the next potential site name from the file
     *
     * File format:
     *    one site name per line of the form:  <family>:<host address>
     *          e.g.    ip:site_foo
     *                  ip:#192.9.29.1
     *    empty lines and lines which have a '#' or space in column 0
     *          are treated as comment lines
     */

NEXTSITE:

    lseek(sitefd, siteoff, 0);

    do
        cc = read(sitefd, buf, sizeof buf);
    while (cc == -1 && errno == EINTR);

    if (cc == -1) {
        dprintf("(find_server_nbrd) Error reading '%s', errno=%d\n", GLB_SITES, errno);
        st->all = rpc_$comm_failure;
        return;
    }

    if (cc == 0) {
        if (firstpass)
            dprintf("(find_server_nbrd) Error reading '%s' - file is empty\n", GLB_SITES);
        else
            dprintf("(find_server_nbrd) no more potential sites\n");
        st->all = rpc_$comm_failure;
        return;
    }

    firstpass = false;

    /* get first line of text and null terminate it */
    for (cp=buf, lp=(char *)location; cc-- && *cp != '\n'; ) {
        *lp++ = *cp++;
        siteoff++;
    }

    *lp = 0;

    if (cc != -1)       /* must have stopped due to newline */
        siteoff++;

    if (location[0] == '#' || location[0] == 0 || location[0] == ' ') {
        goto NEXTSITE;
    }

    /*
     * convert the site name to an address and see if it's the right family
     */

    dprintf("(find_server_nbrd) trying site '%s'\n", location);
    socket_$from_name((u_long) socket_$unspec, location, (u_long) strlen((char *) location),
        (u_long) socket_$unspec_port, &saddr, &slen, st);

    if (st->all != status_$ok) {
        dprintf("(find_server_nbrd) error converting to sockaddr (0x%lx)\n", st->all);
        goto NEXTSITE;
    }

    if (saddr.family != family) {
        dprintf("(find_server_nbrd) family mismatch '%s' (%d) and %d\n", location,
            saddr.family, family);
        goto NEXTSITE;
    }

    /*
     * See if the site has a running LLB and a registered GLB.
     * If so, set the binding to that GLB and see if it's alive.
     */

    if (IS_UIDNIL(glb_$uid)) {
        get_default_object(&glb_$uid);
    }

    lookup_handle = lb_$default_lookup_handle;
    lb_$lookup_range(
            &glb_$uid,            /* object */
            &uuid_$nil,           /* type */
            &glb_$if_spec.id,     /* interface */
            &saddr, slen,         /* LLB address */
            &lookup_handle,
            (u_long) socket_$num_families, /* max results */
            &nresults,
            results,
            st);

    if (st->all != status_$ok) {
        dprintf("(find_server_nbrd) '%s' LLB lookup failure (0x%lx)\n",
            location, st->all);
        goto NEXTSITE;
    }

    if (nresults == 0) {
        dprintf("(find_server_nbrd) '%s' LLB doesn't have a GLB registered (0x%lx)\n",
            location, st->all);
        goto NEXTSITE;
    }

    /* we need a glb registered on an appropriate family */
    for (i = 1, rp = results; i <= nresults; i++, rp++) {
        if (socket_$valid_family((u_long) rp->saddr.family, st))
            break;
        dprintf("(find_server_nbrd) LLB lookup result %d, invalid family (%d)\n",
            i, rp->saddr.family);
    }

    if (i > nresults) {
        dprintf("(find_server_nbrd) LLB doesn't have GLB registered with valid family\n");
        goto NEXTSITE;
    }

    glb_loc_size = sizeof glb_location;
    socket_$to_name(&rp->saddr, rp->saddr_len,
        glb_location, &glb_loc_size, &glb_port, st);
    if (st->all != status_$ok) {
        strcpy((char *) glb_location,"'couldn't convert name'");
        glb_port = 0;
    }
    dprintf("(find_server_nbrd) GLB registered at '%s[%ld]'\n", glb_location, glb_port);

    /*
     * bind to the registered GLB's full address
     */

    rpc_$set_binding(h, &rp->saddr, rp->saddr_len, st);
    if (st->all != status_$ok) {
        dprintf("(find_server_nbrd) couldn't set glb binding (0x%lx)\n", st->all);
        goto NEXTSITE;
    }

    /*
     * Try a GLB lookup (note, this particular lookup shouldn't find anything).
     * If we don't get a RPC runtime fault, it must be alive.
     */

    fst = pfm_$p_cleanup(&crec);
    if (fst.all != pfm_$cleanup_set) {
        dprintf("(find_server_nbrd) RPC runtime failure on glb lookup(0x%lx)\n", fst.all);
        goto NEXTSITE;
    }

    /*
     * we can't really mimic the short timeout mode of the standard glb binding
     * function because the runtime only does this short timeout/fault stuff
     * for broadcast RPC (which this isn't)! But just in case this changes, ...
     */

    if (use_short_timeouts)
        rpc_$set_short_timeout(h, (u_long) use_short_timeouts, &dummy);

    lookup_handle = lb_$default_lookup_handle;
    (*glb_$client_epv.glb_$lookup)(
            h,
            &glb_$uid, &uuid_$nil, &uuid_$nil,
            &lookup_handle,
            1l, /* max results */
            &nresults,
            results,
            &dummy);
    pfm_$p_rls_cleanup(&crec, &dummy);

    /* at last! */
    st->all = status_$ok;
}

static int check_binding(status)
   status_$t *status;
{
    int i;
    status_$t dummy;
    socket_$addr_t  saddr;
    u_long len;
    u_long favored_family = socket_$unspec;
    static u_long wk_family[] = {
        socket_$dds,
        socket_$internet,
    };
#define MAX_FAMILIES (sizeof(wk_family)/sizeof(u_long))

#ifdef DDS
    favored_family = socket_$dds;
#else
    favored_family = socket_$internet;
#endif

    if (valid_binding && glb_$handle != NULL)
        return TRUE;

    if (glb_$handle == NULL) {
        if (IS_UIDNIL(glb_$uid)) {
            get_default_object(&glb_$uid);
        }
    }

#ifdef apollo
    if (check_global_broker_address(&saddr, &len)) {
        if (glb_$handle != NULL) {
            rpc_$set_binding(glb_$handle, &saddr, len, &dummy);
        } else {
            glb_$handle = rpc_$bind(&glb_$uid, &saddr, len, &dummy);
        }
        if (use_short_timeouts) {
            rpc_$set_short_timeout(glb_$handle,
                                    (u_long) use_short_timeouts,
                                    &dummy);
        }
        if (STATUS_OK(&dummy)) {
            valid_binding = TRUE;
            return valid_binding;
        }
    }
#endif

    valid_binding = FALSE;

    if (has_server_list()) {
        use_server_list = true;
        open_server_list(status);
        if (status->all != status_$ok)
            pfm_$signal(*status);
    }

    for (i = 0; i < MAX_FAMILIES; i++) {
        if (glb_$handle != NULL) {
            rpc_$free_handle(glb_$handle, &dummy);
        }
        if (! socket_$valid_family(wk_family[i], &dummy)) {
            continue;
        }
        glb_$handle = rpc_$alloc_handle(&glb_$uid, wk_family[i], &dummy);
        if (!STATUS_OK(&dummy)) {
            print_rpc_error(status);
            break;
        }
        if (use_short_timeouts) {
            rpc_$set_short_timeout(glb_$handle,
                                    (u_long) use_short_timeouts,
                                    &dummy);
        }

        EXCEPTION_RANGE {
            EXCEPTION_CASE {
                if (favored_family == socket_$unspec
                    ||  favored_family == wk_family[i])
                STATUS(status) = STATUS(&fst);
                rpc_$free_handle(glb_$handle, &dummy);
                glb_$handle = NULL;
                if (use_server_list)
                    close_server_list();
            } NORMAL_CASE {
                if (!use_server_list) {
                    (*glb_$client_epv.glb_$find_server)(glb_$handle);
                } else {
                    find_server_nbrd(glb_$handle, wk_family[i], &dummy);
                    if (dummy.all != status_$ok)
                        pfm_$signal(dummy);
                }
                valid_binding = TRUE;
            }
        } EXCEPTION_END;

        if (valid_binding) {
#ifdef apollo
            rpc_$inq_binding(glb_$handle, &saddr, &len, &dummy);
            if (STATUS_OK(&dummy))
                set_global_broker_address(&saddr, len);
#endif
            break;
        }
    }

    if (use_server_list)
        close_server_list();

    return valid_binding;
}

u_long glb_ca_$set_short_timeout(flag)
    u_long    flag;
{
    u_long    old_flag;

    old_flag = use_short_timeouts;
    use_short_timeouts = flag;

    return old_flag;
}


/*
**  get_object_uuid
**      determine the location broker object uuid for this machine.
*/

void glb_ca_$get_object_uuid ( obj )
    uuid_$t *obj;
{
    if (IS_UIDNIL(glb_$uid)) {
        get_default_object(&glb_$uid);
    }

    if (obj != NULL)
        *obj = glb_$uid;
}


/*
**  get_server_address
**      determine the address of the GLB we are connected to (forcing a
**  connection if one hasn't been establshed yet)
*/

void glb_ca_$get_server_address ( saddr, len, st )
    socket_$addr_t  *saddr;
    u_long          *len;
    status_$t       *st;
{
    st->all = status_$ok;
    if (check_binding(st)) {
        rpc_$inq_binding(glb_$handle, saddr, len, st);
    }
}

/*
**  set_server_address
**      force a connection to the chosen server.
*/
void glb_ca_$set_server_address ( saddr, len, st )
    socket_$addr_t  *saddr;
    u_long          len;
    status_$t       *st;
{
    glb_ca_$get_object_uuid(NULL);

    if (glb_$handle != NULL) {
        rpc_$set_binding(glb_$handle, saddr, len, st);
    } else {
        glb_$handle = rpc_$bind(&glb_$uid, saddr, len, st);
    }
    if (st->all == status_$ok)
        valid_binding = TRUE;
}


static void glb_register_ops(op, xentry, status)
   void         (*op)();
   lb_$entry_t  *xentry;
   status_$t    *status;
{
        int done = FALSE;

        SET_STATUS(status, lb_$server_unavailable);

        while (check_binding(status) && !done) {
                EXCEPTION_RANGE {
                        EXCEPTION_CASE {
                                print_rpc_error(&fst);
                        } NORMAL_CASE {
                                (*op)(glb_$handle, xentry, status);
                                done = TRUE;
                        }
                } EXCEPTION_END;
        }
}

void glb_ca_$insert(xentry, status)
   lb_$entry_t  *xentry;
   status_$t    *status;
{
        glb_register_ops(glb_$client_epv.glb_$insert, xentry, status);
}

#ifdef GLB_DEBUG
dump_xentry(xentry)
lb_$entry_t  *xentry;
{
        char cbuf[100];
        int  cbuf_size = sizeof cbuf;
        unsigned port = 0;
        status_$t st;

        printf("dump_xentry:\n");
        uuid_$encode(&xentry->object, cbuf);
        printf("    object:   %s\n", cbuf);
        uuid_$encode(&xentry->obj_type, cbuf);
        printf("    obj_type: %s\n", cbuf);
        uuid_$encode(&xentry->obj_interface, cbuf);
        printf("    obj_intf: %s\n", cbuf);
        printf("    flags:    0x%x\n", xentry->flags);
        printf("    annot:    %s\n", xentry->annotation);
        printf("    slen:     %d\n", xentry->saddr_len);
        socket_$to_numeric_name(&xentry->saddr, xentry->saddr_len, cbuf, &cbuf_size, &port, &st);
        if (st.all != status_$ok)
                strcpy(cbuf, "couldn't convert saddr");
        printf("    saddr:    %s[%d]\n", cbuf, port);
}
#endif

void glb_ca_$delete(xentry, status)
   lb_$entry_t  *xentry;
   status_$t    *status;
{
        glb_register_ops(glb_$client_epv.glb_$delete, xentry, status);
}

/*
** glb_ca_$lookup:  this routine may need to make a series of remote calls in
**              order to fill its output data buffer.  If the remote server
**              fails, then the current buffer is flushed and the request
**              begins anew at a new server.  This is due to entry_handles
**              not being valid across servers - related to this, we check
**              to see if we have a valid binding as soon as the routine
**              is entered, and if not we replace the entry handle with
**              the default handle right away.  (If you don't have a valid
**              connection, then the handle is meaningless, so don't use it)
*/
void glb_ca_$lookup(object, obj_type, obj_interface, entry_handle, max_num_results,
                                num_results, result_entries, status)
   uuid_$t              *object;
   uuid_$t              *obj_type;
   uuid_$t              *obj_interface;
   lb_$lookup_handle_t  *entry_handle;
   u_long               max_num_results;
   u_long               *num_results;
   lb_$entry_t          *result_entries;
   status_$t            *status;
{
#define MIN(A,B) (A < B ? A : B)
        u_long num_entries = 0;
        u_long lookup_amount;
        int done = FALSE;
        lb_$entry_t *results;
        int count = 0;
        int max_count;
        lb_$lookup_handle_t     handle;


        SET_STATUS(status, lb_$server_unavailable);
        *num_results = 0;
        handle = *entry_handle;

        if (!valid_binding) {
                handle = lb_$default_lookup_handle;
        }
        while (check_binding(status) && !done) {
            EXCEPTION_RANGE {
                EXCEPTION_CASE {
                        print_rpc_error(&fst);
                        handle = lb_$default_lookup_handle;
                } NORMAL_CASE {
                    count = 0;
                    max_count = max_num_results;
                    results = result_entries;
                    do {
                        lookup_amount = MIN(max_count, glb_$max_lookup_results);
                        (*glb_$client_epv.glb_$lookup)(
                                                glb_$handle,
                                                object, obj_type, obj_interface,
                                                &handle,
                                                lookup_amount,
                                                &num_entries,
                                                results,
                                                status);
                        results += num_entries;
                        count += num_entries;
                        max_count -= num_entries;
                    } while ((handle != lb_$default_lookup_handle) &&
                             (max_count > 0));
                    done = TRUE;
                }
            } EXCEPTION_END;
        }
        if (done) {
                *num_results = count;
                *entry_handle = handle;
                if (count > 0) {
                        sort_entries(count, result_entries);
                }
        }
        if (STATUS(status) == lb_$not_registered && *num_results > 0) {
                SET_STATUS(status, status_$ok);
        }
}

static sort_entries(num, entries)
   int          num;
   lb_$entry_t  *entries;
{
        int         i;
        int         cur;
        status_$t   st;
        lb_$entry_t *e;
        u_long      family;

        cur = 0;
        for (i = 0, e = entries; i < num; i++, e++) {
                family = e->saddr.family;
                if (!socket_$valid_family(family, &st))
                    continue;

                if (!af[family].valid) {
                        socket_$from_name(family, (ndr_$char *) "", (u_long) 0L,
                                            (u_long) socket_$unspec_port,
                                            &af[family].saddr,
                                            &af[family].saddr_len, &st);
                        if (st.all == status_$ok)
                                af[family].valid = true;
                }

                if (af[family].valid
                    && socket_$equal(&af[family].saddr, af[family].saddr_len,
                                        &e->saddr, e->saddr_len,
                                        (u_long) socket_$eq_network, &st)) {
                        if (cur != i) {
                                lb_$entry_t temp;

                                temp = entries[cur];
                                entries[cur] = *e;
                                *e = temp;
                        }
                        cur++;
                }
        }
}


#ifdef apollo
/*
** Store the global broker address in shared memory so that a given node
** will continue to use the same broker until that broker becomes unavailable.
** (Instead of having each process potentially talk to a different broker)
*/

#include <apollo/mutex.h>
#include <apollo/sfcb.h>

typedef struct {
        sfcb_$header_t  hdr;
        short           pad;
        int             len;
        socket_$addr_t  addr;
} socket_$addr_info_t;

socket_$addr_info_t *global_broker_address;

#define LOCK_GBA_SFCB(foo) { \
    time_$clock_t wait_forever; \
    time_$high32(wait_forever) = time_$low16(wait_forever) = -1; \
    mutex_$lock(&global_broker_address->hdr.slock, wait_forever); \
}

#define UNLOCK_GBA_SFCB(foo) \
    mutex_$unlock(&global_broker_address->hdr.slock)

#define MAX_LEVELS 10
    static struct level_info_t {
        handle_t        handle;
        boolean         valid_binding;
        char            pad[3];
    } level_info[MAX_LEVELS];

get_global_broker_sfcb()
{
        xoid_$t obj_xoid;
        uid_$t  type_uid;
        status_$t st;
        short size;

        uuid_$to_uid(&glb_$default_obj, &obj_xoid.uid, &st);
        obj_xoid.rfu1 = 0;
        obj_xoid.rfu2 = 0;

        uuid_$to_uid(&glb_$default_type, &type_uid, &st);

        size = sizeof(socket_$addr_info_t);
        sfcb_$get(type_uid, obj_xoid, size, &global_broker_address, &st);
        if (st.all != status_$ok)
                global_broker_address = NULL;
        else {
                if (global_broker_address->hdr.use_count == 1) {
                        global_broker_address->len = 0;
                }
                UNLOCK_GBA_SFCB();
        }
}

check_global_broker_address(addr, len)
   socket_$addr_t       *addr;
   int                  *len;
{
        int     retcode = FALSE;
        status_$t st;

        if (global_broker_address == NULL) {
                get_global_broker_sfcb();
        }
        if (global_broker_address != NULL) {
                LOCK_GBA_SFCB();
                if (global_broker_address->len != 0) {
                        *len = global_broker_address->len;
                        bcopy(&global_broker_address->addr, addr, (int) *len);
                        retcode = TRUE;
                }
                UNLOCK_GBA_SFCB();
        }

        return retcode;
}

set_global_broker_address(addr, len)
    socket_$addr_t *addr;
    int len;
{
        status_$t st;

        if (global_broker_address == NULL) {
                get_global_broker_sfcb();
        }
        if (global_broker_address != NULL) {
                LOCK_GBA_SFCB();
                global_broker_address->len = len;
                bcopy(addr, &global_broker_address->addr, (int) len);
                UNLOCK_GBA_SFCB();
        }
}

invalidate_global_broker_address(addr)
    socket_$addr_t *addr;
{
        if (global_broker_address != NULL) {
            LOCK_GBA_SFCB();
            if (global_broker_address->len != 0) {
                if (!bcmp(&global_broker_address->addr, addr, (int) global_broker_address->len))
                        global_broker_address->len = 0;
            }
            UNLOCK_GBA_SFCB();
        }
}

/*
** M A R K _ R E L E A S E
**      We are assuming that users of the location server library will NOT
**      be calling pgm_$invoke() or invoke().  We install this mark release
**      handler so that subsequent programs executed by an inprocess shell will
**      not use stale information, BUT this is not enough to let a program
**      continue to use the library after an invoke() call.
*/

void lb_$mark_release(is_mark, level, st, is_exec)
    boolean     *is_mark;
    pinteger    *level;
    status_$t   *st;
    boolean     *is_exec;
{
    struct level_info_t *li = &level_info[*level];

    SET_STATUS(st, status_$ok);

    if (*is_mark) {
        li->handle          = glb_$handle;
        li->valid_binding   = valid_binding;
        valid_binding       = false;
    } else {
        glb_$handle         = li->handle;
        valid_binding       = li->valid_binding;
    }
}

#endif
