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
 */

#include "/us/ins/ubase.ins.c"
#include "/us/ins/asknode.ins.c"
#include "/us/ins/uid_list.ins.c"

void get_node_root(node, network, uid, st)
int node;
int network;
uid_$t *uid;
status_$t *st;
{ 
    asknode_$reply_t reply;
    asknode_$rqst_data_t rqst;

    asknode_$internet_info(ask_diskless,
                           node, network, 
                           rqst, (short) sizeof rqst, reply, *st);
    if (st->all != 0)
        return;

    if (reply.ar_u.ar1.diskless) {
        *uid = diskless_$uid;
        uid->low = node;
    }
    else {
        asknode_$internet_info(ask_node_root,
                           node, network, 
                           rqst, (short) sizeof rqst, reply, *st);
        if (st->all != 0)
            return;
        *uid = reply.ar_u.uid;
    }
}
