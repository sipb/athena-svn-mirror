/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-2000, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: lpd_remove.h,v 1.1.1.3 2000-03-31 15:48:12 mwhitson Exp $
 ***************************************************************************/



#ifndef _LPD_REMOVE_H_
#define _LPD_REMOVE_H_ 1

/* PROTOTYPES */
int Job_remove( int *sock, char *input );
void Get_queue_remove( char *user, int *sock, struct line_list *tokens,
	struct line_list *done_list );
void Get_local_or_remote_remove( char *user, int *sock,
	struct line_list *tokens, struct line_list *done_list );
int Remove_file( char *openname );
int Remove_job( struct job *job );

#endif