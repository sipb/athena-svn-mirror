/*	ls.pvt.h, dds/ls, pato, 11/24/86
 *	Location Server - Client Agent - private data types and shared information
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
 *
 */

/*
** Useful macros
*/

#ifndef NULL
#define NULL 0
#endif

extern boolean uuid_$equal();

#define STATUS_OK(s) ((s)==NULL || (s)->all == status_$ok)
#define SET_STATUS(s,val) (s)->all = val
#define STATUS(s) (s)->all
#define FLAG_SET(v,f) (((v) & (f)) == (f))
#define EQ_UID(a,b) (uuid_$equal(&(a), &(b)))
#define IS_UIDNIL(a) (EQ_UID((a), uuid_$nil))

#define FALSE 0
#define TRUE !FALSE

#define EQ_OBJ(f,a,b,exact) \
   (f.f1 || EQ_UID(a->object,b->object) || (!exact && IS_UIDNIL(b->object))) && \
   (f.f2 || EQ_UID(a->obj_type,b->obj_type) || \
       (!exact && IS_UIDNIL(b->object))) && \
   (f.f3 || EQ_UID(a->obj_interface,b->obj_interface) || \
       (!exact && IS_UIDNIL(b->object)))

#define EQ_SERVICE(f,a,b,exact,st) \
   (EQ_OBJ(f,a,b,exact) && \
   (f.f4 || socket_$equal(&(a->saddr),(a->saddr_len),&(b->saddr),(b->saddr_len),socket_$eq_netaddr,st)))

#define NORMAL_CASE if (STATUS(&fst) == pfm_$cleanup_set)

extern uuid_$t uuid_$nil;


