/*	glb_p.h
 *	Location Broker - Private types/macros for GLB client agent
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
 *	This file contains hard tabs - use DM commmand: ts 1 9 -r
 */

/* glb - local locating server */

extern void glb_ca_$insert(
#ifdef __STDC__
        lb_$entry_t *,
        status_$t *
#endif
        );

extern void glb_ca_$delete(
#ifdef __STDC__
        lb_$entry_t *,
        status_$t *
#endif
        );

extern void glb_ca_$lookup(
#ifdef __STDC__
	uuid_$t	*,
	uuid_$t	*,
	uuid_$t	*,
	lb_$lookup_handle_t *,
	u_long,
	u_long *,
	lb_$entry_t *,
	status_$t *
#endif
        );

extern u_long glb_ca_$set_short_timeout(
#ifdef __STDC__
	u_long
#endif
	);

extern void glb_ca_$get_object_uuid (
#ifdef __STDC__
        uuid_$t *
#endif
        );

extern void glb_ca_$get_server_address (
#ifdef __STDC__
        socket_$addr_t *,
        u_long *,
        status_$t *
#endif
        );

extern void glb_ca_$set_server_address (
#ifdef __STDC__
        socket_$addr_t *,
        u_long,
        status_$t *
#endif
        );

#ifdef GLB_MODULE
handle_t	glb_$handle = NULL;
uuid_$t		glb_$uid = { 0, 0 };
uuid_$t		glb_$type_uid = { 0, 0 };
#else
extern handle_t		glb_$handle;
extern uuid_$t		glb_$uid;
extern uuid_$t		glb_$type_uid;
#endif

