/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-2000, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: checkpc.h,v 1.1.1.3 2000-03-31 15:48:12 mwhitson Exp $
 ***************************************************************************/



#ifndef _CHECKPC_H_
#define _CHECKPC_H_ 1

/* PROTOTYPES */
int main( int argc, char *argv[], char *envp[] );
void Show_info(void);
void Scan_printer(void);
void Make_write_file( char *dirpath, char *name, char *printer );
void usage(void);
int getage( char *age );
int getk( char *age );
void Clean_log( int trunc, char  *dpath, char *logfile );
int Check_file( char  *path, int fix, int age, int rmflag );
int Fix_create_dir( char  *path, struct stat *statb );
int Fix_owner( char *path );
int Fix_perms( char *path, int perms );
int Check_spool_dir( char *path, int owner );
void Test_port(int ruid, int euid, char *serial_line );
void Fix_clean( char *s, int no, struct line_list *files );
int Start_worker( struct line_list *l, int fd );

#endif
