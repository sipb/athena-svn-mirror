/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1999, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: printjob.h,v 1.1.1.2 1999-05-04 18:07:11 danw Exp $
 ***************************************************************************/



#ifndef _PRINTJOB_H_
#define _PRINTJOB_H_ 1

/* PROTOTYPES */
int Wait_for_pid( int of_pid, char *name, int suspend, int timeout );
void Print_job( int output, struct job *job, int timeout );
char *Fix_str( char *str );
void Print_banner( char *name, char *pgm, struct job *job );
int Write_outbuf_to_OF( struct job *job, char *title,
	int of_pid, int of_fd, int of_error,
	char *buffer, int outlen,
	char *msg, int msgmax,
	int timeout, int suspend, int max_wait );

#endif
