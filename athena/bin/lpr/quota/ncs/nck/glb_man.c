/*	glb_man.c
 *	Location Broker - NON-REPLICATED global broker - implementation of glb_ interface
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
#  include "$(llb.idl).h"
#else
#  include "nbase.h"
#  include "lb.h"
#  include "llb.h"
#endif

#include "lb_p.h"
#include "llb_man.h"

static char *gdb;

#ifdef vms
#  define glb_$database_pathname "ncs$exe:glbdbase.dat"
#endif
#ifdef UNIX
#  define glb_$database_pathname "/etc/ncs/glbdbase.dat"
#endif
#ifdef MSDOS
#  define glb_$database_pathname "c:\\ncs\\glbdbase.dat"
#endif

#define glb_$max_entries 500


static int init(status)
    status_$t *status;
{
    if (gdb != NULL)
        return 1;
    else {
        gdb = (char *) lbdb_$init(glb_$database_pathname, glb_$max_entries, status);
        return (status->all == status_$ok);
    }
}

void glb_$find_server(handle)
   handle_t    handle;
{
}

void glb_$insert(handle, xentry, status)
   handle_t    handle;
   lb_$entry_t *xentry;
   status_$t   *status;
{
    if (! init(status))
        return;

    lbdb_$insert((struct db *) gdb, xentry, status);
}

void glb_$delete(handle, xentry, status)
   handle_t    handle;
   lb_$entry_t *xentry;
   status_$t   *status;
{
    if (! init(status))
        return;

    lbdb_$delete((struct db *) gdb, xentry, status);
}

void glb_$lookup(handle, object, obj_type, obj_interface, entry_handle,
                max_num_results, num_results,
                result_entries, status)
   handle_t     handle;
   uuid_$t      *object;
   uuid_$t      *obj_type;
   uuid_$t      *obj_interface;
   lb_$lookup_handle_t    *entry_handle;
   u_long       max_num_results;
   u_long       *num_results;
   lb_$entry_t  result_entries[];
   status_$t    *status;
{
    if (! init(status))
        return;

    lbdb_$lookup((struct db *) gdb, object, obj_type, obj_interface, entry_handle,
            max_num_results, true, num_results, result_entries, status);
}
