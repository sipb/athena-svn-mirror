/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1999, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: lpstat.h,v 1.1.1.1 1999-05-04 18:07:16 danw Exp $
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

/* PROTOTYPES */
int Pflag, Rflag, Sflag, aflag, cflag, dflag, fflag, lflag,
	oflag, pflag, rflag, sflag, tflag, uflag, vflag;
char *S_val, *a_val, *c_val, *f_val, *o_val, *p_val, *u_val, *v_val;

 struct line_list Lpq_options;

#define MAX_SHORT_STATUS 6

/***************************************************************************
 * main()
 * - top level of LPQ
 *
 ****************************************************************************/

int main(int argc, char *argv[], char *envp[]);
void Show_status(char **argv);
int Read_status_info( char *host, int sock,
	int output, int timeout, int displayformat,
	int status_line_count );
int Remove_excess( struct line_list *l, int status_line_count, int output );
void Get_parms(int argc, char *argv[] );
void usage(void);

#endif
