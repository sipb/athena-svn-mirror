/*  DDS_INIT, /us/lib/ddslib, vlv, 04/22/86
 *   Initialization procedure for ddslib
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

 Changes:
 
    08/01/88 mishkin    removed task stuff (now in pm)
    12/24/87 pato       removed rgy sub_system
    07/13/87 pato       added rgy sub_system
    05/02/87 mishkin    Change ddslib_$init to not be entry point but instead be called
                        from various modules.  Page fault minimization!
    01/26/87 pato       ls names changed to lb.
    12/19/86 pato       call ls_$lib_init.
    06/10/86 mishkin    call rpc_$lib_init.
    06/06/86 mishkin    dds_$init_ddslib => ddslib_$init.
    04/22/86 vlv        Original coding
*/


#nolist
#include "/us/ins/ubase.ins.c"
#include "/us/ins/pm.ins.c"
#include "/us/ins/pfm.ins.c"
#list


boolean ddslib_$initialized;

extern void rpc_$client_mark_release(), rpc_$server_mark_release(),
            rpc_$lsn_mark_release(), lb_$mark_release();

static void (*mr_handlers[])() = {
    rpc_$client_mark_release,
    rpc_$server_mark_release,
    rpc_$lsn_mark_release,
    lb_$mark_release,
};

#define N_MR_HANDLERS 4


/*
 * S T A T I C _ C L E A N U P
 * 
 */

static void static_cleanup(is_mark, level, st, is_exec)
boolean *is_mark;
short *level;
status_$t *st;
boolean *is_exec;
{
    short i;

    for (i = N_MR_HANDLERS - 1; i >= 0; i--)
        (*mr_handlers[i])(is_mark, level, st, is_exec);

    ddslib_$initialized = false;
}


/*
 * D D S L I B _ $ I N I T
 *
 * This procedure is called from various modules in "ddslib" to ensure that the
 * initialization associated with Apollo "program levels" is handled.
 */

void ddslib_$init()
{
    status_$t st;
    short i;
    extern short pm_$level;
    boolean xtrue = true, xfalse = false;

    if (ddslib_$initialized)
        return;

    pfm_$static_cleanup(static_cleanup, st);

    ddslib_$initialized = true;

    for (i = 0; i <= N_MR_HANDLERS - 1; i++) {
        pm_$add_mark_release(mr_handlers[i], st);
        (*mr_handlers[i])(&xtrue, &pm_$level, &st, &xfalse);
    }
}
