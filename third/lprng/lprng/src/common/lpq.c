/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1999, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 *
 ***************************************************************************/

 static char *const _id =
"$Id: lpq.c,v 1.1.1.1 1999-05-04 18:06:59 danw Exp $";


/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1997, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 *
 ***************************************************************************
 * MODULE: lpq.c
 * PURPOSE:
 **************************************************************************/

/***************************************************************************
 * SYNOPSIS
 *      lpq [ -PPrinter_DYN ]
 *    lpq [-Pprinter ]*[-a][-s][-l][+[n]][-Ddebugopt][job#][user]
 * DESCRIPTION
 *   lpq sends a status request to lpd(8)
 *   and reports the status of the
 *   specified jobs or all  jobs  associated  with  a  user.  lpq
 *   invoked  without  any arguments reports on the printer given
 *   by the default printer (see -P option).  For each  job  sub-
 *   mitted  (i.e.  invocation  of lpr(1)) lpq reports the user's
 *   name, current rank in the queue, the names of files compris-
 *   ing  the job, the job identifier (a number which may be sup-
 *   plied to lprm(1) for removing a specific job), and the total
 *   size  in  bytes.  Job ordering is dependent on the algorithm
 *   used to scan the spooling directory and is  FIFO  (First  in
 *   First Out), in order of priority level.  File names compris-
 *   ing a job may be unavailable (when lpr(1) is used as a  sink
 *   in  a  pipeline)  in  which  case  the  file is indicated as
 *   ``(stdin)''.
 *    -P printer
 *         Specifies a particular printer, otherwise  the  default
 *         line printer is used (or the value of the PRINTER vari-
 *         able in the environment).  If PRINTER is  not  defined,
 *         then  the  first  entry in the /etc/printcap(5) file is
 *         reported.  Multiple printers can be displayed by speci-
 *         fying more than one -P option.
 *
 *   -a   All printers listed in the  /etc/printcap(5)  file  are
 *        reported.
 *
 *   -l   An alternate  display  format  is  used,  which  simply
 *        reports the user, jobnumber, and originating host.
 *
 *   [+[n]]
 *        Forces lpq to periodically display  the  spool  queues.
 *        Supplying  a  number immediately after the + sign indi-
 *        cates that lpq should sleep n seconds in between  scans
 *        of the queue.
 *        Note: the screen will be cleared at the start of each
 *        display using the 'curses.h' package.
 ****************************************************************************
 *
 * Implementation Notes
 * Patrick Powell Tue May  2 09:58:29 PDT 1995
 * 
 * The LPD server will be returning the formatted status;
 * The format can be the following:
 * 
 * SHORT:
 * Warning: lp is down: lp is ready and printing
 * Warning: no daemon present
 * Rank   Owner      Job  Files                                 Total Size
 * active root       30   standard input                        5 bytes
 * 2nd    root       31   standard input                        5 bytes
 * 
 * LONG:
 * 
 * Warning: lp is down: lp is ready and printing
 * Warning: no daemon present
 * 
 * root: 1st                                [job 030taco]
 *         standard input                   5 bytes
 * 
 * root: 2nd                                [job 031taco]
 *         standard input                   5 bytes
 * 
 */

#include "lp.h"

#include "child.h"
#include "getopt.h"
#include "getprinter.h"
#include "getqueue.h"
#include "initialize.h"
#include "linksupport.h"
#include "patchlevel.h"
#include "sendreq.h"
#include "termclear.h"

/**** ENDINCLUDE ****/


#undef EXTERN
#undef DEFINE
#define EXTERN
#define DEFINE(X) X
#include "lpq.h"
/**** ENDINCLUDE ****/

 struct line_list Lpq_options;

#define MAX_SHORT_STATUS 6

/***************************************************************************
 * main()
 * - top level of LPQ
 *
 ****************************************************************************/

int main(int argc, char *argv[], char *envp[])
{
	int i;
	struct line_list l, options;

	Init_line_list(&l);
	Init_line_list(&options);

	/* set signal handlers */
	(void) plp_signal (SIGHUP, cleanup_HUP);
	(void) plp_signal (SIGINT, cleanup_INT);
	(void) plp_signal (SIGQUIT, cleanup_QUIT);
	(void) plp_signal (SIGTERM, cleanup_TERM);

	/*
	 * set up the user state
	 */

#ifndef NODEBUG
	Debug = 0;
#endif

	Longformat = 1;
	Status_line_count = 0;
	Displayformat = REQ_DLONG;

	Initialize(argc, argv, envp);
	Setup_configuration();
	Get_parms(argc, argv );      /* scan input args */
	if( LP_mode ){
		Displayformat = REQ_LPSTAT;
	}

	if( All_printers || (Printer_DYN && strcasecmp(Printer_DYN,ALL) == 0 ) ){
		Get_all_printcap_entries();
		if(DEBUGL1)Dump_line_list("lpq- All_line_list", &All_line_list );
	}
	/* we do the individual printers */
	if( Displayformat == REQ_DLONG && Longformat ){
		Status_line_count = (1 << (Longformat-1));
	}
	do {
		Free_line_list(&Printer_list);
		if( Clear_scr ){
			Term_clear();
			Write_fd_str(1,Time_str(0,0));
			Write_fd_str(1,"\n");
		}
		if( All_printers ){
			DEBUG1("lpq: all printers");
			for( i = 0; i < All_line_list.count; ++i ){
				Set_DYN(&Printer_DYN,All_line_list.list[i] );
				Show_status(argv);
			}
		} else {
			Show_status(argv);
		}
		DEBUG1("lpq: done");
		Remove_tempfiles();
		DEBUG1("lpq: tempfiles removed");
		if( Interval > 0 ){
			plp_sleep( Interval );
		}
		/* we check to make sure that nobody killed the output */
	} while( Interval > 0 );
	DEBUG1("lpq: after loop");
	/* if( Clear_scr ){ Term_finish(); } */
	Errorcode = 0;
	DEBUG1("lpq: cleaning up");
	cleanup(0);
	return(0);
}

void Show_status(char **argv)
{
	int fd;
	char msg[LINEBUFFER];

	DEBUG1("Show_status: start");
	/* set up configuration */
	Get_printer();
	Fix_Rm_Rp_info();

	if( LP_mode == 0 && Displayformat != REQ_DSHORT
		&& safestrcasecmp(Printer_DYN, RemotePrinter_DYN) ){
		plp_snprintf( msg, sizeof(msg), _("Printer: %s is %s@%s\n"),
			Printer_DYN, RemotePrinter_DYN, RemoteHost_DYN );
		DEBUG1("Show_status: '%s'",msg);
		if(  Write_fd_str( 1, msg ) < 0 ) cleanup(0);
	}
	if( Check_for_rg_group( Logname_DYN ) ){
		plp_snprintf( msg, sizeof(msg),
			"Printer: %s - cannot use printer, not in privileged group\n" );
		if(  Write_fd_str( 1, msg ) < 0 ) cleanup(0);
		return;
	}
	fd = Send_request( 'Q', Displayformat,
		&argv[Optind], Connect_timeout_DYN,
		Send_query_rw_timeout_DYN, 1 );
	if( fd >= 0 ){
		if( Read_status_info( RemoteHost_DYN, fd,
			1, Send_query_rw_timeout_DYN, Displayformat,
			Status_line_count ) ){
			cleanup(0);
		}
		close(fd);
	}
	DEBUG1("Show_status: end");
}


/***************************************************************************
 *int Read_status_info( int ack, int fd, int timeout );
 * ack = ack character from remote site
 * sock  = fd to read status from
 * char *host = host we are reading from
 * int output = output fd
 *  We read the input in blocks,  split up into lines,
 *  and then pass the lines to a lower level routine for processing.
 *  We run the status through the plp_snprintf() routine,  which will
 *   rip out any unprintable characters.  This will prevent magic escape
 *   string attacks by users putting codes in job names, etc.
 ***************************************************************************/

int Read_status_info( char *host, int sock,
	int output, int timeout, int displayformat,
	int status_line_count )
{
	int i, n, status, count;
	char buffer[LARGEBUFFER];
	char *s, *t;
	struct line_list l;
	int look_for_pr = 0;

	Init_line_list(&l);

	status = count = 0;
	buffer[0] = 0;
	/* long status - trim lines */
	DEBUG1("Read_status_info: output %d, timeout %d, dspfmt %d",
		output, timeout, displayformat );
	DEBUG1("Read_status_info: status_line_count %d", status_line_count );
	do {
		n = sizeof(buffer)-count-1;
		if( n <= 0 ){
			break;
		}
		status = Link_read( host, &sock, timeout,
			buffer+count, &n );
		DEBUG1("Read_status_info: Link_read status %d, read %d", status, n );
		if( status || n == 0 ){
			status = 1;
		} else {
			buffer[count+n] = 0;
		}
		DEBUG3("Read_status_info: got '%s'", buffer );
		/* now we have to split line up */
		if( displayformat == REQ_VERBOSE
			|| displayformat == REQ_LPSTAT ){
			if( Write_fd_str( output, buffer ) < 0 ) return(1);
			buffer[0] = 0;
			count = 0;
			continue;
		}
		if( (s = strrchr(buffer,'\n')) ){
			*s++ = 0;
			/* add the lines */
			Split(&l,buffer,Line_ends,0,0,0,0,0);
			memmove(buffer,s,strlen(s)+1);
			count = strlen(buffer);
		}
		if( displayformat == REQ_DSHORT ){
			for( i = 0; i < l.count; ++i ){
				t = l.list[i];
				if( t && !Find_exists_value(&Printer_list,t,0) ){
					if( Write_fd_str( output, t ) < 0
						|| Write_fd_str( output, "\n" ) < 0 ) return(1);
					Add_line_list(&Printer_list,t,0,1,0);
				}
			}
			Free_line_list(&l);
			continue;
		}
		if( status ){
			if( count ){
				Add_line_list(&l,buffer,0,0,0);
			}
			Check_max(&l,1);
			l.list[l.count++] = 0;
		}
 again:
		DEBUG3("Read_status_info: look_for_pr '%d'", look_for_pr );
		if( DEBUGL3 )Dump_line_list("Read_status_info - starting, Printer_list",
			&Printer_list);
		while( look_for_pr && l.count ){
			s = l.list[0];
			if( s == 0 || isspace(cval(s)) || !(t = strstr(s,"Printer:"))
				|| Find_exists_value(&Printer_list,t,0) ){
				Remove_line_list(&l,0);
			} else {
				look_for_pr = 0;
			}
		}
		if( l.count == 0 ) continue;
		if( status_line_count ){
			/* we only print the last status_line_count that
			 * are different
			 */
			if(DEBUGL3)Dump_line_list("Read_status_info- pruning", &l );
			if( (look_for_pr = Remove_excess( &l, status_line_count, output )) ){
				goto again;
			}
		} else {
			while( l.count > 0 ){
				s = l.list[0];
				DEBUG3("Read_status_info: checking '%s', total %d", s, l.count );
				if( s && !isspace(cval(s)) && (t = strstr(s,"Printer:"))
					&& Find_exists_value(&Printer_list,t,0) ){
					look_for_pr = 1;
					goto again;
				}
				if( Write_fd_str( output, s ) < 0
					|| Write_fd_str( output, "\n" ) < 0 ) return(1);
				Remove_line_list( &l, 0 );
			}
		}
	} while( status == 0 );
	Free_line_list(&l);
	DEBUG1("Read_status_info: done" );
	return(0);
}

/* VARARGS2 */
#ifdef HAVE_STDARGS
 void setstatus (struct job *job,char *fmt,...)
#else
 void setstatus (va_alist) va_dcl
#endif
{
#ifndef HAVE_STDARGS
    struct job *job;
    char *fmt;
#endif
	char msg[LARGEBUFFER];
    VA_LOCAL_DECL

    VA_START (fmt);
    VA_SHIFT (job, struct job * );
    VA_SHIFT (fmt, char *);

	msg[0] = 0;
	if( Verbose ){
		(void) plp_vsnprintf( msg, sizeof(msg)-2, fmt, ap);
		strcat( msg,"\n" );
		if( Write_fd_str( 2, msg ) < 0 ) cleanup(0);
	}
	VA_END;
	return;
}

 void send_to_logger (struct job *job,const char *header, char *fmt){;}
/* VARARGS2 */
#ifdef HAVE_STDARGS
 void setmessage (struct job *job,const char *header, char *fmt,...)
#else
 void setmessage (va_alist) va_dcl
#endif
{
#ifndef HAVE_STDARGS
    struct job *job;
    char *fmt, *header;
#endif
	char msg[LARGEBUFFER];
    VA_LOCAL_DECL

    VA_START (fmt);
    VA_SHIFT (job, struct job * );
    VA_SHIFT (header, char * );
    VA_SHIFT (fmt, char *);

	msg[0] = 0;
	if( Verbose ){
		(void) plp_vsnprintf( msg, sizeof(msg)-2, fmt, ap);
		strcat( msg,"\n" );
		if( Write_fd_str( 2, msg ) < 0 ) cleanup(0);
	}
	VA_END;
	return;
}


int Remove_excess( struct line_list *l, int status_line_count, int output )
{
	char *s, *t;
	int i, j, n;
	/* now we might need to prune these */
	if(DEBUGL3)Dump_line_list("Remove_excess- starting", l );
	while(l->count){
		if( (s = l->list[0]) == 0 ){
			Remove_line_list(l,0);
			return(0);
		}
		DEBUG3("Remove_excess: checking '%s', total %d", s, l->count );
		i = 0;
		if( !isspace(cval(s)) ){
			if( (t = strstr( s, "Printer:" )) ){
				DEBUG3("Remove_excess: looking for '%s'", t );
				if( Find_exists_value(&Printer_list,t,0) ){
					DEBUG1("Remove_excess: already done '%s'", t );
					return(1);
				}
				DEBUG1("Remove_excess: not done '%s'", t );
				if( l->count > 1 ){
					DEBUG1("Remove_excess: adding to done '%s'", t );
					Add_line_list( &Printer_list, t, 0, 0, 0);
				}
			} else {
				i = 1;
			}
		}
		/* we search for differences */
		if( i == 0 ){
			if( (t = strpbrk(s,":")) ){
				n = t - s;
			} else {
				n = strlen(s);
			}
			DEBUG2("Remove_excess: checking '%s', length %d", s, n );
			for( i = 1; i < l->count && (t=l->list[i]) && !strncmp(s,t,n); ++i );
		}
		DEBUG2("Remove_excess: found difference at %d", i );
		n = i - status_line_count;
		if( i < l->count ){
			n = i - status_line_count;
			if( n < 0 ) n = 0;
			DEBUG2("Remove_excess: skipping %d, doing up to %d", n, i );
			for( j = n; j < i; ++j ){
				if( Write_fd_str( output, l->list[j] ) < 0
					|| Write_fd_str( output, "\n" ) < 0 ) return(1);
			}
			DEBUG2("Remove_excess: removing %d", i );
			for( j = 0; j < i; ++j ){
				Remove_line_list(l,0);
			}
		} else if( n > 0 ){
			DEBUG2("Remove_excess: removing surplus %d", n );
			for( j = 0; j < n; ++j ){
				Remove_line_list(l,0);
			}
		} else {
			break;
		}
	}
	if(DEBUGL3)Dump_line_list("Remove_excess- now have", l );
	return(0);
}


/***************************************************************************
 * void Get_parms(int argc, char *argv[])
 * 1. Scan the argument list and get the flags
 * 2. Check for duplicate information
 ***************************************************************************/

 extern char *next_opt;

 char LPQ_optstr[]    /* LPQ options */
 = "D:P:VacLlst:v" ;

void Get_parms(int argc, char *argv[] )
{
	int option, i;
	char *name, *t, *s;

	if( argv[0] && (name = strrchr( argv[0], '/' )) ) {
		++name;
	} else {
		name = argv[0];
	}
/*
SYNOPSIS
     lpstat [ -d ] [ -r ] [ -R ] [ -s ] [ -t ] [ -a [list] ]
          [ -c [list] ] [ -f [list] [ -l ] ] [ -o [list] ]
          [ -p [list] [ -D ] [ -l ] ] [ -P ] [ -S [list] [ -l ] ]
          [ -u [login-ID-list] ] [ -v [list] ]
*/
	/* check to see if we simulate (poorly) the LP options */
	if( name && strcmp( name, "lpstat" ) == 0 ){
		LP_mode = 1;
		Opterr = 0;
		for( i = 0; i < argc; ++i ){
			s = argv[i];
			Add_line_list(&Lpq_options,s,0,0,0);
			if( s[0] == '-' && strchr("acfopSuv",s[1]) ){
				if( (s = argv[i+1]) ){
					if( s[0] != '-' ){
						/* we have a list here */
						++i;
						t = s;
						while( (t = strpbrk(t,Whitespace)) ){
							*t = ',';
						}
						Add_line_list(&Lpq_options,s,0,0,0);
					}
				}
			}
		}
	} else {
		/* scan the input arguments, setting up values */
		while ((option = Getopt (argc, argv, LPQ_optstr )) != EOF) {
			switch (option) {
			case 'D':
				Parse_debug(Optarg,1);
				break;
			case 'P': if( Optarg == 0 ) usage();
				Set_DYN(&Printer_DYN,Optarg);
				break;
			case 'V': ++Verbose; break;
			case 'a': Set_DYN(&Printer_DYN,"all"); ++All_printers; break;
			case 'c': Clear_scr = 1; break;
			case 'l': ++Longformat; break;
			case 'L': Longformat = 0; break;
			case 's': Longformat = 0;
						Displayformat = REQ_DSHORT;
						break;
			case 't': if( Optarg == 0 ) usage();
						Interval = atoi( Optarg );
						break;
			case 'v': Longformat = 0; Displayformat = REQ_VERBOSE; break;
			default:
				usage();
			}
		}
	}
	if( Verbose ) {
		fprintf( stderr, _("Version %s\n"), PATCHLEVEL );
		if( Verbose > 1 ) Printlist( Copyright, stderr );
	}
}

 char *lpq_msg = 
"usage: %s [-aAclV] [-Ddebuglevel] [-Pprinter] [-tsleeptime]\n\
  -a           - all printers\n\
  -c           - clear screen before update\n\
  -l           - increase (lengthen) detailed status information\n\
                 additional l flags add more detail.\n\
  -Ddebuglevel - debug level\n\
  -Pprinter    - specify printer\n\
  -s           - short (summary) format\n\
  -tsleeptime  - sleeptime between updates\n\
  -V           - print version information\n";

 char *lpstat_msg = 
"usage: %s [-d] [-r] [-R] [-s] [-t] [-a [list]]\n\
  [-c [list]] [-f [list] [-l]] [-o [list]]\n\
  [-p [list]] [-P] [-S [list]] [list]\n\
  [-u [login-ID-list]] [-v [list]]\n\
 list is a list of print queues\n\
 -a [list] destination status *\n\
 -c [list] class status *\n\
 -f [list] forms status *\n\
 -o [list] job or printer status *\n\
 -p [list] printer status *\n\
 -P        paper types - ignored\n\
 -r        scheduler status\n\
 -s        summary status information - short format\n\
 -S [list] character set - ignored\n\
 -t        all status information - long format\n\
 -u [joblist] job status information\n\
 -v [list] printer mapping *\n\
 * - long status format produced\n";

void usage(void)
{
	if( LP_mode ){
		fprintf( stderr, lpstat_msg, Name );
	} else {
		fprintf( stderr, lpq_msg, Name );
	}
	exit(1);
}

 int Start_worker( struct line_list *args, int fd )
{
	return(1);
}

#if 0

#include "permission.h"
#include "lpd.h"
#include "lpd_status.h"
 int Send_request(
	int class,					/* 'Q'= LPQ, 'C'= LPC, M = lprm */
	int format,					/* X for option */
	char **options,				/* options to send */
	int connect_timeout,		/* timeout on connection */
	int transfer_timeout,		/* timeout on transfer */
	int output					/* output on this FD */
	)
{
	int i, n;
	int socket = 1;
	char cmd[SMALLBUFFER];

	cmd[0] = format;
	cmd[1] = 0;
	plp_snprintf(cmd+1, sizeof(cmd)-1, "%s", RemotePrinter_DYN);
	for( i = 0; options[i]; ++i ){
		n = strlen(cmd);
		plp_snprintf(cmd+n,sizeof(cmd)-n," %s",options[i] );
	}
	Perm_check.remoteuser = "papowell";
	Perm_check.user = "papowell";
	Is_server = 1;
	Job_status(&socket,cmd);
	return(-1);
}

#endif
