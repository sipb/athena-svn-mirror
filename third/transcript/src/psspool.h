/* psspool.h
 *
 * Copyright (C) 1985,1987,1992 Adobe Systems Incorporated. All rights
 *  reserved. 
 * GOVERNMENT END USERS: See notice of rights in Notice file in TranScript
 * library directory -- probably /usr/lib/ps/Notice
 * RCSID: $Header: /afs/dev.mit.edu/source/repository/third/transcript/src/psspool.h,v 1.1.1.1 1996-10-07 20:25:52 ghudson Exp $
 *
 * nice constants for printcap spooler filters
 *
 * RCSLOG:
 * $Log: not supported by cvs2svn $
 * Revision 3.3  1992/08/21  16:26:32  snichols
 * Release 4.0
 *
 * Revision 3.2  1992/07/14  22:12:29  snichols
 * Updated copyrights.
 *
 * Revision 3.1  1992/06/01  21:34:01  snichols
 * conflicting definitions of TRY_AGAIN when compling qmscomm, because
 * of including netdb.h.
 *
 * Revision 3.0  1991/06/17  16:50:45  snichols
 * Release 3.0
 *
 * Revision 2.2  1987/11/17  16:52:34  byron
 * *** empty log message ***
 *
 * Revision 2.2  87/11/17  16:52:34  byron
 * Release 2.1
 * 
 * Revision 2.1.1.2  87/11/12  13:42:02  byron
 * Changed Government user's notice.
 * 
 * Revision 2.1.1.1  87/04/23  10:26:50  byron
 * Copyright notice.
 * 
 * Revision 2.2  86/11/02  14:13:37  shore
 * Product Update
 * 
 * Revision 2.1  85/11/24  11:51:12  shore
 * Product Release 2.0
 * 
 * Revision 1.2  85/05/14  11:26:54  shore
 * 
 * 
 *
 */

#define PS_INT	'\003'
#define PS_EOF	'\004'
#define PS_STATUS '\024'

/* error exit codes for lpd-invoked processes */

/* because the qmscomm module include netdb, which has it's own idea of
   what TRY_AGAIN is, use TRYAGAIN instead */
#ifdef QMS
#define TRYAGAIN 1
#else
#define TRY_AGAIN 1
#endif /* QMS */
#define THROW_AWAY 2

