/*	llb_p.h
 *	Location Broker - Private types/macros for LLB client agent
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

/*
** Private interfaces
*/

extern void llb_ca_$insert(
#ifdef __STDC__
        socket_$addr_t *location,
        u_long location_len,
        lb_$entry_t  *xentry,
        status_$t    *status
#endif
);

extern void llb_ca_$delete(
#ifdef __STDC__
        socket_$addr_t *location,
        u_long location_len,
        lb_$entry_t  *xentry,
        status_$t    *status 
#endif
);

extern void llb_ca_$lookup(
#ifdef __STDC__
        uuid_$t          *object,
        uuid_$t          *obj_type,
        uuid_$t          *obj_interface,
        socket_$addr_t   *location,
        u_long           location_len,
        lb_$lookup_handle_t  *entry_handle,
        u_long           max_num_results,
        u_long           *num_results,
        lb_$entry_t      *result_entries,
        status_$t        *status 
#endif
);

