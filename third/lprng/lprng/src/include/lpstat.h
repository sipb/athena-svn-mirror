/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-2000, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: lpstat.h,v 1.1.1.3 2000-03-31 15:48:11 mwhitson Exp $
 ***************************************************************************/



#ifndef _LPQ_H_
#define _LPQ_H_ 1
EXTERN int LP_mode;			/* LP mode */
EXTERN int Longformat;      /* Long format */
EXTERN int Displayformat;   /* Display format */
EXTERN int All_printers;    /* show all printers */
EXTERN int Status_line_count; /* number of status lines */
EXTERN int Clear_scr;       /* clear screen */
EXTERN int Interval;        /* display interval */
EXTERN int Mail_fd;

/* PROTOTYPES */
int main(int argc, char *argv[], char *envp[]);
void Show_status(char **argv, int display_format);
int Read_status_info( char *host, int sock,
	int output, int timeout, int display_format,
	int status_line_count );
int Add_val( char **var, char *val );
void Get_parms(int argc, char *argv[] );
void usage(void);

#endif