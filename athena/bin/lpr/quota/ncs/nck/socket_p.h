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

typedef struct {
    u_long (*inq_port)();             
    void (*set_port)();            
    void (*set_wk_port)();         
    void (*from_name)();           
    void (*to_numeric_name)();             
    void (*to_name)();             
    u_long (*max_pkt_size)();         
    void (*inq_my_netaddr)();      
    void (*inq_netaddr)();         
    void (*set_netaddr)();         
    void (*inq_hostid)();          
    void (*set_hostid)();          
    boolean (*eq_network)();
    void (*inq_broad_addrs)();
    void (*to_local_rep)();
    void (*from_local_rep)();
} socket_$handler_rec_t;

#define internal static

#ifndef NULL
#define NULL 0
#endif

#ifdef GLOBAL_LIBRARY
#ifdef INET
#define inet_$socket_handler inet_$socket_handler_pure_data$
#endif
#ifdef DDS
#define dds_$socket_handler  dds_$socket_handler_pure_data$
#endif
#endif

#ifdef INET
globalref socket_$handler_rec_t inet_$socket_handler;
#endif

#ifdef DDS
globalref socket_$handler_rec_t dds_$socket_handler;
#endif


