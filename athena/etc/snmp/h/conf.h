/*
 * $Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/h/conf.h,v 1.1 1994-09-18 12:57:16 cfields Exp $
 *
 * $Log: not supported by cvs2svn $
 */

/*
 * THIS SOFTWARE IS THE CONFIDENTIAL AND PROPRIETARY PRODUCT OF PERFORMANCE
 * SYSTEMS INTERNATIONAL, INC. ANY UNAUTHORIZED USE, REPRODUCTION, OR TRANSFER 
 * OF THIS SOFTWARE IS STRICTLY PROHIBITED.  COPYRIGHT (C) 1990 PSI, INC.  
 * (SUBJECT TO LIMITED DISTRIBUTION AND RESTRICTED DISCLOSURE ONLY.) 
 * ALL RIGHTS RESERVED.
 */
/*
 * THIS SOFTWARE IS THE CONFIDENTIAL AND PROPRIETARY PRODUCT OF NYSERNET,
 * INC.  ANY UNAUTHORIZED USE, REPRODUCTION, OR TRANSFER OF THIS SOFTWARE
 * IS STRICTLY PROHIBITED.  (C) 1989 NYSERNET, INC.  (SUBJECT TO 
 * LIMITED DISTRIBUTION AND RESTRICTED DISCLOSURE ONLY.)  ALL RIGHTS RESERVED.
 */
/* for machine and OS dependencies */
#define BSD	43			/* running any form of BSD */
#undef NAMED				/* same netdb.h as BIND */
#undef INGRES
#undef SVR3WIN				/* SVR3 with WIN/3B */

/* display system */
#undef XV10				/* X10 Release 4 */
#define XV11				/* X11 Release 2 */
#define MONO				/* monochrome graphics */
#undef COLOR				/* Color graphics */
#define UNK_BELL			/* ring bell on unknown status event */
