/*	glbd.c
 *	Location Broker - NON-REPLICATED global broker - main program
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
 */

#include "std.h"

#ifdef DSEE
#  include "$(lb.idl).h"
#  include "$(socket.idl).h"
#  include "$(glb.idl).h"
#else
#  include "lb.h"
#  include "socket.h"
#  include "glb.h"
#endif

#include "pfm.h"
#include "lb_p.h"

#define CHECK_STATUS(val) \
        if (STATUS(&status) != status_$ok) { \
                rpc_$status_print("GLBD: ", status); \
                exit(val); \
        }

static void register_server(glb_uid, glb_type_uid)
    uuid_$t *glb_uid;
    uuid_$t *glb_type_uid;
{
    struct {
        socket_$addr_t saddr;
        u_long saddr_len;
    } af[socket_$num_families];
    lb_$entry_t glb_entry;
    status_$t status;
    extern uuid_$t uuid_$nil;
    int i;
    u_long num_active_families = 0;
    socket_$addr_family_t supported_families[socket_$num_families];
    u_long num_families = socket_$num_families;

    socket_$valid_families(&num_families, supported_families, &status);

    /*
     * Get sockaddrs for the families that we will be supporting. 
     */
    for (i = 0; i < num_families; i++) {
        rpc_$use_family((u_long) supported_families[i], &af[i].saddr,
                        &af[i].saddr_len, &status);
        if (STATUS_OK(&status)) {
            supported_families[num_active_families] =
                supported_families[i];
            af[num_active_families] = af[i];
            num_active_families++;
        }
        else {
            ndr_$char family_name[20];
            u_long namelen = sizeof(family_name);

            strcpy(family_name, "<unknown>");
            socket_$family_to_name((u_long) supported_families[i], family_name,
                                   &namelen, &status);
            printf("GLBD: unable to listen on %.*s address family\n",
                   (int) namelen, family_name);
        }
    }
    if (num_active_families == 0) {
        printf("GLBD: unable to listen on any address families\n");
        exit(1);
    }

    /*
     * Register our interface with the rpc runtime 
     */
    rpc_$register(&glb_$if_spec, glb_$server_epv, &status);
    CHECK_STATUS(2);


    /*
     * Register with the local location service - deriving a name that
     * might be meaningful to a human reader. 
     */

#ifndef EMBEDDED_LLBD
    for (i = 0; i < num_active_families; i++) {
        lb_$register(glb_uid, glb_type_uid, &glb_$if_spec.id,
                  lb_$server_flag_local, (ndr_$char *) "non-replicated GLB",
                     &af[i].saddr, af[i].saddr_len, &glb_entry, &status);
        CHECK_STATUS(3);
    }
#else
    glb_entry.object = *glb_uid;
    glb_entry.obj_type = *glb_type_uid;
    glb_entry.obj_interface = glb_$if_spec.id;
    glb_entry.flags = 0;
    glb_entry.saddr = af[0].saddr;
    glb_entry.saddr_len = af[0].saddr_len;

    strcpy(glb_entry.annotation, "non-replicated glb");

    llb_$insert(NULL, &glb_entry, &status);
    CHECK_STATUS(3);
#endif
} 

#define version  "nrglbd/nck version 1.5.1"

main(argc, argv)
    int argc;
    char *argv[];
{
    status_$t status;
    extern boolean rpc_$debug;
    uuid_$t glb_uid;
    uuid_$t glb_type_uid;

    lb_$process_args_i(argc, argv, &glb_uid, &glb_type_uid, version);

    if (!rpc_$debug) {
        int pid;
        int i;

#ifdef UNIX
        /* Run the daemon in the background */
        pid = fork();
        if (pid > 0)
            exit(0);
#endif

#ifdef SYS5
        setpgrp();
#endif

#ifdef apollo
        {
            std_$call pm_$set_my_name();
            static char name[] = "glbd";
            short namelen = sizeof(name) - 1;

            pm_$set_my_name(name, namelen, status);
        }
#endif
    }

    pfm_$init((unsigned long) pfm_$init_signal_handlers);

    register_server(&glb_uid, &glb_type_uid);

#ifdef MSDOS
    rpc_$allow_remote_shutdown((u_long) true, NULL, &status);
#endif

    printf("Listening...\n");

    rpc_$listen(1l, &status);

    if (STATUS(&status) != status_$ok)
        rpc_$status_print("GLBD: ", status);
}
