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
 *
 * This header file exists because "nbase.h" (derived from "nbase.idl")
 * doesn't have all the stuff that the Apollo-specific ".h" files (included
 * when building for Apollos only) need.
 * 
 * On non-Apollos, this file simply includes "nbase.h" (i.e. the includer
 * might just as well have included "nbase.h".)
 * 
 * On Apollos, this file (a) includes "/us/include/apollo/base.h" (which
 * defines some of the same things as "nbase.idl", some more things not
 * in "nbase.idl", but lacks some things that are in "nbase.idl, and (b)
 * defines those missing pieces (since we're not using "nbase.h").
 *
 * Note that to make things mesh a little better, we include "timebase"
 * here as well.
 */

#ifndef apollo

#include "nbase.h"
#include "timebase.h"

#define time_$high32(t)    ((t).high)
#define time_$low16(t)     ((t).low)

#else

#include <apollo/base.h>
#define base__included      /* <apollo/base.h> is a superset of the "base.h" from "base.idl" */

#define time_$high32(t)    ((t).c1.high)
#define time_$low16(t)     ((t).c1.low)

#define nbase__included     /* The following are the "missing pieces" */

typedef struct uuid_$t {
    unsigned long time_high;
    unsigned short time_low;
    unsigned short reserved;
    char family;
    char host[7];      
} uuid_$t;

extern uuid_$t uuid_$nil;

#define socket_$unspec_port 0

#define socket_$unspec (short) 0x0
#define socket_$unix (short) 0x1
#define socket_$internet (short) 0x2
#define socket_$implink (short) 0x3
#define socket_$pup (short) 0x4
#define socket_$chaos (short) 0x5
#define socket_$ns (short) 0x6
#define socket_$nbs (short) 0x7
#define socket_$ecma (short) 0x8
#define socket_$datakit (short) 0x9
#define socket_$ccitt (short) 0xa
#define socket_$sna (short) 0xb
#define socket_$unspec2 (short) 0xc
#define socket_$dds (short) 0xd

typedef short socket_$addr_family_t;

#define socket_$num_families 32

#define socket_$sizeof_family 2
#define socket_$sizeof_data 14 
#define socket_$sizeof_ndata 12
#define socket_$sizeof_hdata 12

typedef struct socket_$addr_t { 
    socket_$addr_family_t family;
    unsigned char (data)[14];
} socket_$addr_t;

typedef struct socket_$net_addr_t {
    socket_$addr_family_t family;
    unsigned char (data)[12];
} socket_$net_addr_t;

typedef struct socket_$host_id_t { 
    socket_$addr_family_t family;
    unsigned char (data)[12];
} socket_$host_id_t;

#endif
