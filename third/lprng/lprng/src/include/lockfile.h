/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-2000, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: lockfile.h,v 1.1.1.4 2000-03-31 15:48:10 mwhitson Exp $
 ***************************************************************************/



#ifndef _LOCKFILE_H_
#define _LOCKFILE_H_ 1

/* PROTOTYPES */

int Do_lock( int fd, int block );
int LockDevice(int fd, int block);

#endif