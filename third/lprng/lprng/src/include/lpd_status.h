/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-2000, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: lpd_status.h,v 1.1.1.3 2000-03-31 15:48:11 mwhitson Exp $
 ***************************************************************************/



#ifndef _LPD_STATUS_H_
#define _LPD_STATUS_H_ 1

/* PROTOTYPES */
int Job_status( int *sock, char *input );
void Get_queue_status( struct line_list *tokens, int *sock,
	int displayformat, int status_lines, struct line_list *done_list,
	int max_size );
void Print_status_info( int *sock, char *dir, char *file,
	char *prefix, int status_lines, int max_size );
void Get_local_or_remote_status( struct line_list *tokens, int *sock,
	int displayformat, int status_lines, struct line_list *done_list,
	int max_size );

#endif
