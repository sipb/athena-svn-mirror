/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1999, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: lpd_jobs.h,v 1.1.1.1 1999-05-04 18:07:07 danw Exp $
 ***************************************************************************/



#ifndef _LPD_JOBS_H_
#define _LPD_JOBS_H_ 1

/* PROTOTYPES */
void Update_spool_info( struct line_list *sp );
int cmp_server( const void *left, const void *right );
int Get_subserver_pc( char *printer, struct line_list *l, int done_time );
void Dump_subserver_info( char *title, struct line_list *l);
void Get_subserver_info( struct line_list *order,
	char *list, char *old_order);
int Copy_or_link( char *srcfile, char *destfile );
int Do_queue_jobs( char *name, int subserver );
int Remote_job( struct job *job, char *id );
int Local_job( struct job *job, char *id );
int Fork_subserver( struct line_list *server_info, int use_subserver,
	struct line_list *parms );
void Wait_for_subserver( struct line_list *servers, struct line_list *order );
int Decode_transfer_failure( int attempt, struct job *job );
void Update_status( struct job *job, int status );
int Check_print_perms( struct job *job );
void Setup_user_reporting( struct job *job );
void Service_worker( struct line_list *args );
int Printer_open( char *lp_device, struct job *job,
	int max_attempts, int interval, int max_interval, int grace,
	int connect_tmout, int *filterpid, int *errorpid );

#endif
