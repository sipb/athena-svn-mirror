/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1999, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: fileopen.h,v 1.2 2001-03-07 01:20:05 ghudson Exp $
 ***************************************************************************/



#ifndef _FILEOPEN_H_
#define _FILEOPEN_H_ 1

/*****************************************************************
 * File open functions
 * These perform extensive checking for permissions and types
 *  see fileopen.c for details
 *****************************************************************/

/* PROTOTYPES */

int Checkread( const char *file, struct stat *statb );
int Checkwrite( const char *file, struct stat *statb, int rw, int create, int del );
void Remove_files( void *p );
int Checkwrite_timeout(int timeout,
	const char *file, struct stat *statb, int rw, int create, int delay );

#endif
