/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1999, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 *
 ***************************************************************************/

 static char *const _id =
"$Id: lpstat.c,v 1.1.1.1 1999-05-04 18:06:48 danw Exp $";


/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1997, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 *
 ***************************************************************************
 * MODULE: lpstat.c
 * PURPOSE:
 **************************************************************************/

/***************************************************************************
 * SYNOPSIS
 *   lpstat  [-d]  [-r]  [-R]  [-s]  [-t]  [-a  [list
 *  ]  ]  [-c  [list] ]  [-f  [list]  [-l] ]  [-o  [
 *   list] ]  [-p   [list]  [-D]  [-l] ]  [-S  [list
 *  ]   [-l] ]  [-u  [login- ID -list ] ]  [-v  [list]
 *  ]
 *
 *    lpstat 
 * DESCRIPTION
 *   lpstat sends a status request to lpd(8)
 * See lpstat for details on the way that status is gathered.
 *   The lpstat version simulates the lpstat operation,
 * using the information returned by LPRng in response to a verbose query.
 * 
 ****************************************************************************
 *
 */

#include "lp.h"

#include "child.h"
#include "getopt.h"
#include "getprinter.h"
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
#include "lpstat.h"
/**** ENDINCLUDE ****/

 int P_flag, R_flag, S_flag, a_flag, c_flag, d_flag, f_flag, l_flag,
	o_flag, p_flag, r_flag, s_flag, t_flag, u_flag, v_flag;
 char *S_val, *a_val, *c_val, *f_val, *o_val, *p_val, *u_val, *v_val;
 struct line_list S_list, a_list, c_list, f_list, o_list, p_list, u_list, v_list;

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
	struct line_list l, options, request_list;
	char msg[SMALLBUFFER], *s;

	Init_line_list(&l);
	Init_line_list(&options);
	Init_line_list(&request_list);

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

	Displayformat = REQ_DLONG;

	Initialize(argc, argv, envp);
	Setup_configuration();
	Get_parms(argc, argv );      /* scan input args */

	/* set up configuration */
	Get_printer();
	Fix_Rm_Rp_info();
	Get_all_printcap_entries();

	/* check on printing scheduler is running */
	if( r_flag || t_flag ){
		Write_fd_str(1,"scheduler is running\n");
	}
	if( d_flag || t_flag ){
		if( Printer_DYN == 0 ){
			Write_fd_str(1,"no system default destination\n");
		} else {
			plp_snprintf(msg,sizeof(msg), "system default destination: %s\n", Printer_DYN);
			Write_fd_str(1,msg);
		}
	}
	if( s_flag || t_flag || v_flag ){
		for( i = 0; i < All_line_list.count; ++i ){
			Set_DYN(&Printer_DYN,All_line_list.list[i] );
			Fix_Rm_Rp_info();
			plp_snprintf(msg,sizeof(msg), "system for %s: %s\n", Printer_DYN, RemoteHost_DYN);
			Write_fd_str(1,msg);
		}
	}

	Merge_line_list( &request_list, &Printer_list,0,1,1);
	Free_line_list( &Printer_list );
	if( t_flag || All_printers ){
		Merge_line_list( &request_list, &All_line_list,0,1,1);
	}

	DEBUG1("lpstat: all printers");
	for( i = 0; i < request_list.count; ++i ){
		s = request_list.list[i];
		if( Find_exists_value( &All_line_list, s, 0 ) ){
			Add_line_list( &l, s, 0, 1, 1 );
		} else {
			Add_line_list( &options, s, 0, 1, 1 );
		}
	}
	Check_max(&options,2);
	options.list[options.count] = 0;
	if( options.count ){
		for( i = options.count; i > 0 ; --i ){
			options.list[i] = options.list[i-1];
		}
		options.list[0] = safestrdup(Logname_DYN,__FILE__,__LINE__);
		++options.count;
	}
	options.list[options.count] = 0;
	if(DEBUGL1)Dump_line_list("lpstat - printers", &l);
	if(DEBUGL1)Dump_line_list("lpstat - options", &options);
	if( options.count && l.count == 0 ){
		Merge_line_list( &l, &All_line_list,0,1,1);
	}
	for( i = 0; i < l.count; ++i ){
		s = l.list[i];
		Set_DYN(&Printer_DYN,s );
		Show_status(options.list);
	}

	DEBUG1("lpstat: done");
	Remove_tempfiles();
	DEBUG1("lpstat: tempfiles removed");
	if( Interval > 0 ){
		plp_sleep( Interval );
	}

	Errorcode = 0;
	DEBUG1("lpstat: cleaning up");
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

	if( s_flag || t_flag ){
		plp_snprintf(msg,sizeof(msg), "printer %s unkown state. enabled since %s. available\n",
			Printer_DYN, Pretty_time(0));
		Write_fd_str(1,msg);
	}

	if( Check_for_rg_group( Logname_DYN ) ){
		plp_snprintf( msg, sizeof(msg),
			"Printer: %s - cannot use printer, not in privileged group\n" );
		if(  Write_fd_str( 1, msg ) < 0 ) cleanup(0);
		return;
	}
	fd = Send_request( 'Q', Displayformat,
		0, Connect_timeout_DYN, Send_query_rw_timeout_DYN, 1 );
	if( fd >= 0 ){
		Read_status_info( RemoteHost_DYN, fd,
			1, Send_query_rw_timeout_DYN, 0,
			0 );
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
	int n, status, count;
	char buffer[LARGEBUFFER];
	char *s;
	struct line_list l;

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
		if( (s = strrchr(buffer,'\n')) ){
			*s++ = 0;
			/* add the lines */
			Split(&l,buffer,Line_ends,0,0,0,0,0);
			memmove(buffer,s,strlen(s)+1);
			count = strlen(buffer);
		}
	} while( status == 0 );
	Check_max(&l,1);
	l.list[l.count] = 0;
	if(DEBUGL3)Dump_line_list("Read_status_info - raw status", &l);
	Remove_excess( &l, 3, 1 );
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


int Add_val( char **var, char *val )
{
	int c = 0;
	if( val && cval(val) != '-' ){
		c = 1;
		if( *var ){
			*var = safeextend3(*var,",",val,__FILE__,__LINE__);
		} else {
			*var = safestrdup(val,__FILE__,__LINE__);
		}
	}
	return(c);
}

/***************************************************************************
 * void Get_parms(int argc, char *argv[])
 * 1. Scan the argument list and get the flags
 * 2. Check for duplicate information
 ***************************************************************************/


void Get_parms(int argc, char *argv[] )
{
	int i, n;
	char *name, *s;

	if( argv[0] && (name = strrchr( argv[0], '/' )) ) {
		++name;
	} else {
		name = argv[0];
	}
	Name = name;

/*
SYNOPSIS
     lpstat [ -d ] [ -r ] [ -R ] [ -s ] [ -t ] [ -a [list] ]
          [ -c [list] ] [ -f [list] [ -l ] ] [ -o [list] ]
          [ -p [list] [ -D ] [ -l ] ] [ -P ] [ -S [list] [ -l ] ]
          [ -u [login-ID-list] ] [ -v [list] ]
*/
	n = 0;
	for( i = 1; i < argc; ++i ){
		s = argv[i];
		if( cval(s) == '-' ){
			switch( cval(s+1) ){
			case 'd': n=1; d_flag = 1; if(cval(s+2)) usage(); break;
			case 'r': n=1; r_flag = 1; if(cval(s+2)) usage(); break;
			case 'R': n=1; Rflag = 1; if(cval(s+2)) usage(); break;
			case 's': n=1; s_flag = 1; if(cval(s+2)) usage(); break;
			case 't': n=1; t_flag = 1; if(cval(s+2)) usage(); break;
			case 'l': n=1; l_flag = 1; if(cval(s+2)) usage(); break;
			case 'P': n=1; Pflag = 1; if(cval(s+2)) usage(); break;
			case 'a': n=1; a_flag = 1; if( cval(s+2) ) Add_val(&a_val,s+2); else { i += Add_val(&a_val,argv[i+1]); } break;
			case 'c': n=1; c_flag = 1; if( cval(s+2) ) Add_val(&c_val,s+2); else { i += Add_val(&c_val,argv[i+1]); } break;
			case 'f': n=1; f_flag = 1; if( cval(s+2) ) Add_val(&f_val,s+2); else { i += Add_val(&f_val,argv[i+1]); } break;
			case 'o': n=1; o_flag = 1; if( cval(s+2) ) Add_val(&o_val,s+2); else { i += Add_val(&o_val,argv[i+1]); } break;
			case 'p': n=1; p_flag = 1; if( cval(s+2) ) Add_val(&p_val,s+2); else { i += Add_val(&p_val,argv[i+1]); } break;
			case 'S': n=1; Sflag = 1; if( cval(s+2) ) Add_val(&S_val,s+2); else { i += Add_val(&S_val,argv[i+1]); } break;
			case 'u': n=1; u_flag = 1; if( cval(s+2) ) Add_val(&u_val,s+2); else { i += Add_val(&u_val,argv[i+1]); } break;
			case 'v': n=1; v_flag = 1; if( cval(s+2) ) Add_val(&v_val,s+2); else { i += Add_val(&v_val,argv[i+1]); } break;
			case 'T': Parse_debug( s+2, 1 ); break;
			}
		}
	}
	if( n == 0 ){
		o_flag = 1;
	}
	if( a_flag && a_val == 0 ) All_printers = 1; Split(&Printer_list,a_val,", ",1,0,1,1,0);
	if( c_flag && c_val == 0 ) All_printers = 1; Split(&Printer_list,c_val,", ",1,0,1,1,0);
	if( f_flag && f_val == 0 ) f_val = "all"; Split(&f_list,f_val,", ",1,0,1,1,0);
	if( o_flag && o_val == 0 ) All_printers = 1; Split(&Printer_list,o_val,", ",1,0,1,1,0);
	if( S_flag && S_val == 0 ) S_val = "all"; Split(&S_list,S_val,", ",1,0,1,1,0);
	if( u_flag && u_val == 0 ) u_val = "all"; Split(&u_list,u_val,", ",1,0,1,1,0);
	if( v_flag && v_val == 0 ) All_printers = 1; Split(&v_list,v_val,", ",1,0,1,1,0);
	if( Verbose ) {
		fprintf( stderr, _("Version %s\n"), PATCHLEVEL );
		if( Verbose > 1 ) Printlist( Copyright, stderr );
	}
	if(DEBUGL1){
		logDebug("d_flag %d, r_flag %d, R_flag %d, s_flag %d, t_flag %d, l_flag %d, P_flag %d",
			d_flag, r_flag, R_flag, s_flag, t_flag, l_flag, P_flag );
		logDebug("a_flag %d, a_val '%s'", a_flag, a_val );
		logDebug("c_flag %d, c_val '%s'", c_flag, c_val );
		logDebug("f_flag %d, f_val '%s'", f_flag, f_val );
		logDebug("o_flag %d, o_val '%s'", o_flag, o_val );
		logDebug("p_flag %d, p_val '%s'", p_flag, p_val );
		logDebug("S_flag %d, S_val '%s'", S_flag, S_val );
		logDebug("u_flag %d, u_val '%s'", u_flag, u_val );
		logDebug("v_flag %d, v_val '%s'", v_flag, v_val );
	}
}

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
	fprintf( stderr, lpstat_msg, Name );
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
	plp_snprintf(cmd+1, sizeof(cmd)-1, RemotePrinter_DYN);
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

