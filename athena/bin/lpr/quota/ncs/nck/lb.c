/*  lb.c
 *  Location Broker - LLB/GLB client Agent - Accesses both local and global databases
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

#include "std.h"


#ifdef DSEE
#  include "$(nbase.idl).h"
#  include "$(lb.idl).h"
#  include "$(uuid.idl).h"
#else
#  include "nbase.h"
#  include "lb.h"
#  include "uuid.h"
#endif

#include "lb_p.h"
#include "llb_p.h"
#include "glb_p.h"

/*
** lb_$register - register the specified entry, if lb_$server_flag_local in "flags"
**              then make this entry local, if lb_$server_flag_startable
**              then the forwarding agent will start this service on demand.
**		(startable: Not Yet Implemented)
*/
void lb_$register(object, obj_type, obj_interface, flags, annotation,
                        saddr, saddr_len, xentry, status)
   uuid_$t              *object;
   uuid_$t              *obj_type;
   uuid_$t              *obj_interface;
   lb_$server_flag_t    flags;
   unsigned char        annotation[];
   socket_$addr_t       *saddr;
   u_long               saddr_len;
   lb_$entry_t          *xentry;
   status_$t            *status;
{
	status_$t	local_status;
	lb_$entry_t	local_entry;

	if (status == NULL)
		status = &local_status;
	if (xentry == NULL) {
		xentry = &local_entry;
	}

	xentry->object = *object;
	xentry->obj_type = *obj_type;
	xentry->obj_interface = *obj_interface;
	xentry->flags = flags;
	bcopy(annotation, xentry->annotation, (int) sizeof(xentry->annotation));
	xentry->saddr_len = saddr_len;
	bcopy(saddr, &xentry->saddr, (int) saddr_len);

	SET_STATUS(status, status_$ok);

	llb_ca_$insert(NULL, 0L, xentry, status);
	if (STATUS_OK(status) && !FLAG_SET(xentry->flags, lb_$server_flag_local)) {
		glb_ca_$insert(xentry, status);
	}
}

/*
** lb_$unregister - unregister the specified service.
*/
void lb_$unregister(xentry, status)
   lb_$entry_t	*xentry;
   status_$t	*status;
{
	status_$t		local_status;

	if (status == NULL)
		status = &local_status;

	SET_STATUS(status, status_$ok);

	if (!FLAG_SET(xentry->flags, lb_$server_flag_local)) {
		glb_ca_$delete(xentry, status);
	}
	if (STATUS_OK(status)) {
		llb_ca_$delete(NULL, 0L, xentry, status);
	}
}


/*
** lb_$lookup_range - lookup matching entries.  if any of the uuid_$t fields
**              are uuid_$nil, then that field will match
**              any value in the database.  This allows lookup by any combination
**              of entry fields.  "location" is a pointer to a socket_$addr_t,
**              if the socket address is empty (location_len == 0), then the
**		request is sent to a global
**              location server, otherwise it is sent to the local location server
**              running at the node specified by the "location" parameter.
**
**              "entry_handle" is a handle describing where to
**              begin the search, and is set to describe where to continue
**              the search from.  A handle value of lb_$default_lookup_handle on
**              input indicates the search should begin at the beginning of the
**              data, and a value of lb_$default_lookup_handle on output
**              indicates that all available data has been delivered.
**
**              NOTE: It is possible that serial invokations of this
**              function will return duplicate or incomplete information.
*/

void lb_$lookup_range(object, obj_type, obj_interface, location, location_len,
				entry_handle, max_num_results, num_results,
				result_entries, status)
   uuid_$t          *object;
   uuid_$t          *obj_type;
   uuid_$t          *obj_interface;
   socket_$addr_t	*location;
   u_long		    location_len;
   lb_$lookup_handle_t  *entry_handle;
   u_long		    max_num_results;
   u_long		    *num_results;
   lb_$entry_t		result_entries[];
   status_$t		*status;
{
	status_$t	local_status;
	u_long		local_num_results;
	lb_$lookup_handle_t local_continue_handle;

	if (status == NULL)
		status = &local_status;
	if (num_results == NULL)
		num_results = &local_num_results;
	if (entry_handle == NULL) {
		entry_handle = &local_continue_handle;
		*entry_handle = lb_$default_lookup_handle;
	}
	SET_STATUS(status, status_$ok);

	if (location_len != 0) {
		llb_ca_$lookup(object, obj_type, obj_interface, location,
				location_len, entry_handle, max_num_results,
				num_results, result_entries, status);
	} else {
		glb_ca_$lookup(object, obj_type, obj_interface, entry_handle,
				max_num_results, num_results,
				result_entries, status);
	}
}

/*
** lb_$lookup_object - search for the specified object in the global database.
**              This call implies wildcards for the object type and obj_interface.
*/
void lb_$lookup_object(object, entry_handle, max_num_results, num_results,
				result_entries, status)
   uuid_$t      	*object;
   lb_$lookup_handle_t  *entry_handle;
   u_long		max_num_results;
   u_long		*num_results;
   lb_$entry_t		result_entries[];
   status_$t		*status;
{
	lb_$lookup_range(object, &uuid_$nil, &uuid_$nil, (socket_$addr_t *) NULL, 0L, entry_handle,
				max_num_results, num_results,
				result_entries, status);
}

/*
** lb_$lookup_object_local - search for the specified object at the specified site.
**              This call implies wildcards for the object type and obj_interface.
*/
void lb_$lookup_object_local(object, location, location_len, entry_handle,
				max_num_results, num_results, result_entries,
				status)
   uuid_$t              *object;
   socket_$addr_t	*location;
   u_long		location_len;
   lb_$lookup_handle_t  *entry_handle;
   u_long		max_num_results;
   u_long		*num_results;
   lb_$entry_t		result_entries[];
   status_$t		*status;
{
	lb_$lookup_range(object, &uuid_$nil, &uuid_$nil, location, location_len,
				entry_handle, max_num_results, num_results,
				result_entries, status);
}

/*
** lb_$lookup_type - search for the specified type in the global database.
**              This call implies wildcards for the object and obj_interface.
*/
void lb_$lookup_type(obj_type, entry_handle, max_num_results, num_results,
				result_entries, status)
   uuid_$t              *obj_type;
   lb_$lookup_handle_t  *entry_handle;
   u_long		max_num_results;
   u_long		*num_results;
   lb_$entry_t		result_entries[];
   status_$t		*status;
{
	lb_$lookup_range(&uuid_$nil, obj_type, &uuid_$nil, (socket_$addr_t *) NULL, 0L, entry_handle,
				max_num_results, num_results,
				result_entries, status);
}

/*
** lb_$lookup_interface - search for the specified obj_interface in the global
**              database.  This call implies wildcards for the object and
**              object type.
*/
void lb_$lookup_interface(obj_interface, entry_handle, max_num_results, num_results,
				result_entries, status)
   uuid_$t              *obj_interface;
   lb_$lookup_handle_t  *entry_handle;
   u_long		max_num_results;
   u_long		*num_results;
   lb_$entry_t		result_entries[];
   status_$t		*status;
{
	lb_$lookup_range(&uuid_$nil, &uuid_$nil, obj_interface, (socket_$addr_t *) NULL, 0L,
				entry_handle, max_num_results, num_results,
				result_entries, status);
}

/*
**  lb_$use_short_timeouts - use short rpc timeouts for location broker operations
*/
u_long lb_$use_short_timeouts (flag)
    u_long    flag;
{
	(void) llb_ca_$set_short_timeout(flag);
	return glb_ca_$set_short_timeout(flag);
}

/*
** lb_$process_args - common command line processing for getting at internal
**		debugging.
*/  


void lb_$process_args_i(argc, argv, object_uid, type_uid, version)
   int		argc;
   char		*argv[];
   uuid_$t	*object_uid;
   uuid_$t	*type_uid;                                     
   char         *version;
{
	extern boolean rpc_$debug;
	uuid_$t dummy_uid;
	status_$t status;

	if (object_uid == NULL)
		object_uid = &dummy_uid;
	if (type_uid == NULL)
		type_uid = &dummy_uid;
                       
        glb_ca_$get_object_uuid(object_uid);
	uuid_$decode((ndr_$char *) "333b91de0000.0d.00.00.87.84.00.00.00", type_uid, &status);

	argv++;
	while (--argc > 0) {
		if (**argv == '-') {
			switch (*(*argv+1)) {
				case '1':
				    uuid_$decode(
					(ndr_$char *) "333b91de0000.0d.00.00.87.84.00.00.00",
					object_uid, &status);
				    break;

				case '2':
				    uuid_$decode(
					(ndr_$char *) "333b91eb0000.0d.00.00.87.84.00.00.00",
					object_uid, &status);
				    break;

				case '3':
				    uuid_$decode(
					(ndr_$char *) "333b91f70000.0d.00.00.87.84.00.00.00",
					object_uid, &status);
				    break;

                                case 'v':
                                    printf("%s\n", version);
                                    exit(0);

				case 'd':
				case 'D':
					switch (*(*argv+2)) {
						case 'l':
						case 'L':
							rpc_$debug = true;
							break;

						case '\0':
#ifdef apollo
							ddslib_$init();
#endif
							break;

						default:
							break;
					}
					break;
			}
		}
		argv++;
	}
}

void lb_$process_args(argc, argv, object_uid, type_uid)
   int		argc;
   char		*argv[];
   uuid_$t	*object_uid;
   uuid_$t	*type_uid;                                     
{
    lb_$process_args_i(argc, argv, object_uid, type_uid, "");
}

#ifdef FTN_INTERLUDES

void lb_$register_(object, obj_type, obj_interface, flags, annotation,
                        saddr, saddr_len, xentry, status)
   uuid_$t              *object;
   uuid_$t              *obj_type;
   uuid_$t              *obj_interface;
   lb_$server_flag_t    *flags;
   unsigned char        annotation[];
   socket_$addr_t       *saddr;
   u_long               *saddr_len;
   lb_$entry_t          *xentry;
   status_$t            *status;
{
    lb_$register(object, obj_type, obj_interface, *flags, annotation,
                        saddr, *saddr_len, xentry, status);
}

void lb_$unregister_(xentry, status)
   lb_$entry_t	*xentry;
   status_$t	*status;
{
    lb_$unregister(xentry, status);
}

void lb_$lookup_range_(object, obj_type, obj_interface, location, location_len,
				entry_handle, max_num_results, num_results,
				result_entries, status)
   uuid_$t          *object;
   uuid_$t          *obj_type;
   uuid_$t          *obj_interface;
   socket_$addr_t	*location;
   u_long		    *location_len;
   lb_$lookup_handle_t  *entry_handle;
   u_long		    *max_num_results;
   u_long		    *num_results;
   lb_$entry_t		result_entries[];
   status_$t		*status;
{
    lb_$lookup_range(object, obj_type, obj_interface, location, *location_len,
				entry_handle, *max_num_results, num_results,
				result_entries, status);
}

void lb_$lookup_object_(object, entry_handle, max_num_results, num_results,
				result_entries, status)
   uuid_$t      	*object;
   lb_$lookup_handle_t  *entry_handle;
   u_long		*max_num_results;
   u_long		*num_results;
   lb_$entry_t		result_entries[];
   status_$t		*status;
{
    lb_$lookup_object(object, entry_handle, *max_num_results, num_results,
				result_entries, status);
}

void lb_$lookup_object_local_(object, location, location_len, entry_handle,
				max_num_results, num_results, result_entries,
				status)
   uuid_$t              *object;
   socket_$addr_t	*location;
   u_long		*location_len;
   lb_$lookup_handle_t  *entry_handle;
   u_long		*max_num_results;
   u_long		*num_results;
   lb_$entry_t		result_entries[];
   status_$t		*status;
{
    lb_$lookup_object_local(object, location, *location_len, entry_handle,
				*max_num_results, num_results, result_entries,
				status);
}

void lb_$lookup_type_(obj_type, entry_handle, max_num_results, num_results,
				result_entries, status)
   uuid_$t              *obj_type;
   lb_$lookup_handle_t  *entry_handle;
   u_long		*max_num_results;
   u_long		*num_results;
   lb_$entry_t		result_entries[];
   status_$t		*status;
{
    lb_$lookup_type(obj_type, entry_handle, *max_num_results, num_results,
				result_entries, status);
}

void lb_$lookup_interface_(obj_interface, entry_handle, max_num_results, num_results,
				result_entries, status)
   uuid_$t              *obj_interface;
   lb_$lookup_handle_t  *entry_handle;
   u_long		*max_num_results;
   u_long		*num_results;
   lb_$entry_t		result_entries[];
   status_$t		*status;
{
    lb_$lookup_interface_(obj_interface, entry_handle, *max_num_results, num_results,
				result_entries, status);
}

#endif

