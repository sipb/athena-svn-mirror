/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1999, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: lpd_logger.h,v 1.1.1.3.2.1 2001-03-07 01:42:42 ghudson Exp $
 ***************************************************************************/



#ifndef _LPD_LOGGER_H_
#define _LPD_LOGGER_H_ 1

/* PROTOTYPES */
void Free_file_info( struct file_info *io );
void Hex_dump( void *p, int len );
void Dump_file_info_sub( char *title, struct file_info *io );
void Dump_file_info( char *title, struct file_info *io );
void Dump_file_info_contents( char *title, struct file_info *io );
void Init_file_info( struct file_info *io, char *path, int max_size );
void Read_rec( struct file_info *io, char *s, int start, int reccount );
void Write_rec( struct file_info *io, char *s, int start, int reccount );
char *Get_record( struct file_info *io, int start, int *len );
int Put_record( struct file_info *io, int start, char *buf );
void Remove_first_record( struct file_info *io );
void Add_record( struct file_info *io, char *buf );
void Dump_queue_status(void);
void Logger( struct line_list *args );

#endif
