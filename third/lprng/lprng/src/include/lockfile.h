/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1999, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: lockfile.h,v 1.2 2001-03-07 01:20:11 ghudson Exp $
 ***************************************************************************/



#ifndef _LOCKFILE_H_
#define _LOCKFILE_H_ 1

/* PROTOTYPES */

int Do_lock( int fd, int block );
int LockDevice(int fd, int block);

#endif
