/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1999, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: sendjob.h,v 1.2 2001-03-07 01:20:23 ghudson Exp $
 ***************************************************************************/



#ifndef _SENDJOB_1_
#define _SENDJOB_1_ 1

/* PROTOTYPES */
int Send_job( struct job *job,
	int connect_timeout_len, int connect_interval, int max_connect_interval,
	int transfer_timeout );
int Send_normal( int *sock, struct job *job, int transfer_timeout, int block_fd );
int Send_control( int *sock, struct job *job, int transfer_timeout,
	int block_fd );
int Send_data_files( int *sock, struct job *job, int transfer_timeout,
	int block_fd );
int Send_block( int *sock, struct job *job, int transfer_timeout );
int Send_secure_block( int *sock, struct job *job, int transfer_timeout );

#endif
