/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1999, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: lpd_rcvjob.h,v 1.1.1.2 1999-10-27 20:10:14 mwhitson Exp $
 ***************************************************************************/



#ifndef _LPD_RCVJOB_H_
#define _LPD_RCVJOB_H_ 1

/* PROTOTYPES */
int Receive_job( int *sock, char *input );
int Receive_block_job( int *sock, char *input );
int Scan_block_file( int fd, char *error, int errlen );
int Read_one_line( int fd, char *buffer, int maxlen );
int Check_space( double jobsize, int min_space, char *pathname );
int Do_perm_check( struct job *job, char *error, int errlen );
int Check_for_missing_files( struct job *job, struct line_list *files,
	char *error, int errlen );
int Find_non_colliding_job_number( struct job *job, char *dpath );
int Get_route( struct job *job, char *error, int errlen );

#endif
