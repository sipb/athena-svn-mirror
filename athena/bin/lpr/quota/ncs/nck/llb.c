/*      llb.c
 *      Location Broker - LLB client agent
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

#ifdef DSEE
#  include "$(nbase.idl).h"
#  include "$(lb.idl).h"
#  include "$(socket.idl).h"
#  include "$(llb.idl).h"
#else
#  include "nbase.h"
#  include "lb.h"
#  include "socket.h"
#  include "llb.h"
#endif

#include "lb_p.h"
#include "llb_p.h"

#include "pfm.h"

#ifdef __STDC__
static handle_t get_binding(socket_$addr_t *ilocation, u_long ilocation_len, status_$t *st);
#endif


/*
** Useful macros
*/

#define EXCEPTION_RANGE \
    { \
    pfm_$cleanup_rec crec; \
    status_$t fst; \
    fst = pfm_$p_cleanup(&crec);

#define EXCEPTION_END pfm_$p_rls_cleanup(&crec, &fst); }

#define EXCEPTION_CASE \
    if (STATUS(&fst) != pfm_$cleanup_set) { \
        if ((STATUS(&fst) & 0xffff0000) != rpc_$mod) \
            pfm_$signal(fst); \
        else \
            STATUS(status) = STATUS(&fst);\
        pfm_$enable(); \
    }

static u_long   use_short_timeouts = 0;

/*
** Shared Static data
*/
#ifdef apollo
#  include "set_sect.pvt.c"
#endif

/*
** Return a binding handle for an LLBD.  If the input location is non-NULL,
** then just make a binding using that location (using the LLB's well-known
** port though).  Otherwise, use the local machine's location.
*/

static handle_t get_binding ( ilocation, ilocation_len, st )
    socket_$addr_t  *ilocation;
    u_long          ilocation_len;
    status_$t       *st;
{
    socket_$addr_t  location;
    u_long          location_len;
    handle_t        handle;
    status_$t       lst;

    if (ilocation != NULL) {
        location = *ilocation;
        location_len = ilocation_len;
    }
    else {
        socket_$net_addr_t naddr;
        u_long nlen = sizeof naddr;
#ifndef apollo
        socket_$addr_family_t families[socket_$num_families];
        u_long num_families = socket_$num_families;
#endif

#ifdef apollo
        location.family = socket_$dds;
#else
        socket_$valid_families(&num_families, families, st);
        location.family = families[0];
#endif
        location_len = sizeof location;

        socket_$inq_my_netaddr((u_long) location.family, &naddr, &nlen, st);
        socket_$set_netaddr(&location, &location_len, &naddr, nlen, st);
    }

    socket_$set_wk_port(&location, &location_len, (u_long) socket_$wk_fwd, st);

    handle = rpc_$bind(&uuid_$nil, &location, location_len, st);

    if (st->all == status_$ok && use_short_timeouts) {
        rpc_$set_short_timeout(handle, use_short_timeouts, &lst);
    }

    return (handle);
}


u_long llb_ca_$set_short_timeout ( flag )
    u_long    flag;
{
    u_long    old_flag;

    old_flag = use_short_timeouts;
    use_short_timeouts = flag;

    return old_flag;
}

/*
** Client agent for "llb_$insert".
*/

void llb_ca_$insert(location, location_len, xentry, status)
    socket_$addr_t *location;
    u_long location_len;
    lb_$entry_t  *xentry;
    status_$t    *status;
{
    handle_t llb_handle;
    status_$t st;

#ifdef EMBEDDED_LLBD
    if (location == NULL) {
        (*llb_$manager_epv.llb_$insert)(NULL, xentry, status);
        return;
    }
#endif

    llb_handle = get_binding(location, location_len, status);
    if (!STATUS_OK(status)) {
        return;
    }

    SET_STATUS(status, lb_$server_unavailable);

    EXCEPTION_RANGE {
        EXCEPTION_CASE;
        NORMAL_CASE {
            (*llb_$client_epv.llb_$insert)(llb_handle, xentry, status);
        }
    } EXCEPTION_END;

    rpc_$free_handle(llb_handle, &st);
}


/*
** Client agent for "llb_$delete".
*/

void llb_ca_$delete(location, location_len, xentry, status)
    socket_$addr_t *location;
    u_long location_len;
    lb_$entry_t  *xentry;
    status_$t    *status;
{
    handle_t llb_handle;
    status_$t st;

#ifdef EMBEDDED_LLBD
    if (location == NULL) {
        (*llb_$manager_epv.llb_$delete)(NULL, xentry, status);
        return;
    }
#endif

    llb_handle = get_binding(location, location_len, status);
    if (!STATUS_OK(status)) {
        return;
    }

    SET_STATUS(status, lb_$server_unavailable);

    EXCEPTION_RANGE {
        EXCEPTION_CASE;
        NORMAL_CASE {
            (*llb_$client_epv.llb_$delete)(llb_handle, xentry, status);
        }
    } EXCEPTION_END;

    rpc_$free_handle(llb_handle, &st);
}

/*
** Client agent for "llb_$lookup".
*/

void llb_ca_$lookup(object, obj_type, obj_interface, location, location_len,
                entry_handle, max_num_results, num_results,
                result_entries, status)
   uuid_$t          *object;
   uuid_$t          *obj_type;
   uuid_$t          *obj_interface;
   socket_$addr_t   *location;
   u_long           location_len;
   lb_$lookup_handle_t  *entry_handle;
   u_long           max_num_results;
   u_long           *num_results;
   lb_$entry_t      *result_entries;
   status_$t        *status;
{
#define MIN(A,B) (A < B ? A : B)
    u_long num_entries = 0;
    u_long lookup_amount;
    status_$t st;
    handle_t llb_handle;

#ifdef EMBEDDED_LLBD
    if (location == NULL) {
        (*llb_$manager_epv.llb_$lookup)(
                        NULL,
                        object, obj_type, obj_interface,
                        entry_handle,
                        max_num_results,
                        &num_entries,
                        result_entries,
                        status);
        return;
    }
#endif

    SET_STATUS(status, lb_$server_unavailable);
    *num_results = 0;

    llb_handle = get_binding(location, location_len, &st);
    if (!STATUS_OK(&st)) {
        return;
    }

    EXCEPTION_RANGE {
        EXCEPTION_CASE;
        NORMAL_CASE {
            do {
                lookup_amount = MIN(max_num_results, llb_$max_lookup_results);
                (*llb_$client_epv.llb_$lookup)(
                                llb_handle,
                                object, obj_type, obj_interface,
                                entry_handle,
                                lookup_amount,
                                &num_entries,
                                result_entries,
                                status);
                result_entries += llb_$max_lookup_results;
                *num_results += num_entries;
                max_num_results -= num_entries;
            } while ((*entry_handle != lb_$default_lookup_handle) &&
                 (max_num_results > 0));
        }
    } EXCEPTION_END;

    rpc_$free_handle(llb_handle, &st);
    if (STATUS(status) == lb_$not_registered && *num_results > 0) {
        SET_STATUS(status, status_$ok);
    }
}
