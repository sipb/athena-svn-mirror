/* psspool.h
 *
 * Copyright (C) 1985,1987 Adobe Systems Incorporated. All rights reserved.
 * GOVERNMENT END USERS: See notice of rights in Notice file in TranScript
 * library directory -- probably /usr/lib/ps/Notice
 * RCSID: $Id: psspool.h,v 1.2 1999-01-22 23:11:30 ghudson Exp $
 *
 * nice constants for printcap spooler filters
 *
 */

#define PS_INT	'\003'
#define PS_EOF	'\004'
#define PS_STATUS '\024'

/* error exit codes for lpd-invoked processes */
#define TRY_AGAIN 1
#define THROW_AWAY 2

