/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1997, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 *
 ***************************************************************************
 * MODULE: lpd_control.c
 * PURPOSE: return status
 **************************************************************************/

static char *const _id =
"lpd_control.c,v 3.21 1998/03/24 02:43:22 papowell Exp";

#include "lp.h"
#include "control.h"
#include "printcap.h"
#include "cleantext.h"
#include "decodestatus.h"
#include "fileopen.h"
#include "getqueue.h"
#include "gethostinfo.h"
#include "jobcontrol.h"
#include "killchild.h"
#include "linksupport.h"
#include "pathname.h"
#include "patselect.h"
#include "permission.h"
#include "serverpid.h"
#include "setstatus.h"
#include "setupprinter.h"
#include "waitchild.h"
#include "getprinter.h"
/**** ENDINCLUDE ****/

/***************************************************************************

The control (LPC) interface is sort of a catch-all for hacking.
I have tried to provide the following functionality.

1. Spool Queues have a 'control.printer' file that is read/written by
   the Get_spool_control and Set_spool_control routines.  These routines
   will happily put out the various control strings you need.
	USED BY: start/stop, enable/disable, debug, forward, holdall

2. Individual jobs have a 'hold file' that is read/written by
   the Get_ job_control and Set_ job_control routines.  These also
   will read/write various control strings.
   USED by topq, hold, release

 ***************************************************************************/

static char status_header[] = "%-18s %8s %8s %4s %7s %7s %8s %s%s";

void Do_printer_work( char *user, int action, int *socket,
	int tokencount, struct token *tokens, char *error, int errorlen );
void Do_queue_server( char *user, int action, int *socket,
	int tokencount, struct token *tokens, char *error, int errorlen );
void Do_queue_control( char *user, int action, int *socket,
	int tokencount, struct token *tokens, char *error, int errorlen );
int Do_control_file( char *user, int action, int *socket,
	int tokencount, struct token *tokens, char *error, int errorlen );
int Do_control_lpq( char *user, int action, int *socket,
	int tokencount, struct token *tokens, char *error, int errorlen );
int Do_control_status( char *user, int action, int *socket,
	int tokencount, struct token *tokens, char *error, int errorlen );
int Do_control_redirect( char *user, int action, int *socket,
	int tokencount, struct token *tokens, char *error, int errorlen );
int Do_control_class( char *user, int action, int *socket,
	int tokencount, struct token *tokens, char *error, int errorlen );
int Do_control_printcap( struct printcap_entry *pc, int action, int *socket,
	int tokencount, struct token *tokens, char *error, int errorlen );
int Do_control_debug( char *user, int action, int *socket,
	int tokencount, struct token *tokens, char *error, int errorlen );
int Do_control_defaultq( int *socket );

static char *Redirect_str;
static char *Action = "updated";
static int Control_allowed;
static int User_action;

int Job_control( int *socket, char *input, int maxlen )
{
	struct token tokens[20];
	char error[LINEBUFFER];
	int tokencount;
	int i, action;
	char *name, *user, *s, *end;
	int permission, reject;

	/* get the format */

	error[0] = 0;
	Name = "Job_control";
	++input;
	if( (s = strchr(input, '\n' )) ) *s = 0;
	DEBUG0("Job_control: doing '%s'", input );

	/* check printername for characters, underscore, digits */
	tokencount = Crackline(input, tokens, sizeof(tokens)/sizeof(tokens[0]));

	if( tokencount < 3 ){
		plp_snprintf( error, sizeof(error),
			_("bad control command '%s'"), input );
		goto error;
	}

	for( i = 0; i < tokencount; ++i ){
		tokens[i].start[tokens[i].length] = 0;
	}

	if(DEBUGL3 ){
		logDebug("Job_control: tokencount '%d'", tokencount );
		for( i = 0; i < tokencount; ++i ){
			logDebug("Job_control: token [%d] '%s'", i, tokens[i].start );
		}
	}

	/* get the name for the printer */
	/* it is either the default or user specified */

	name = tokens[0].start;
	if( tokencount > 3 ){
		name = tokens[3].start;
	}

	if( (s = Clean_name( name )) ){
		plp_snprintf( error, sizeof(error),
			_("printer '%s' has illegal char '%c' in name"), name, *s );
		goto error;
	}
	proctitle( "lpd %s %s", Name, Printer );
	Printer = name;
	user = tokens[1].start;

	s = tokens[2].start;
	action = Get_controlword( s );
	if( action == 0 ){
		plp_snprintf( error, sizeof(error),
			_("%s: unknown control request '%s'"), Printer, s );
		goto error;
	}

	/* check the permissions for the action */

	Perm_check.printer = Printer;
	Perm_check.remoteuser = user;
	Perm_check.user = user;
	Perm_check.service = 'C';
	switch( action ){
		case DEFAULTQ:
		case PRINTCAP:
		case LPD:
		case STATUs:
			User_action = 'S';
			break;
		case TOPQ:
		case ABORT:
		case KILL:
		case MOVE:
		case HOLD:
		case RELEASE:
		case REDO:
			if( User_lpc ){
				safestrncpy(error, User_lpc );
				for( s = error; s; s = end ){
					while( isspace(*s) ) ++*s;
					end = strpbrk( s, ",; \t");
					if( end ) *end++ = 0;
					if( action == Get_controlword( s ) ){
						User_action = 'U';
						break;
					}
				}
			}
			break;
	}

	DEBUG1( "Job_control: checking USER='%s' SERVICE='%c', PRINTER='%s'",
		user, Perm_check.service, Printer );
	DEBUG1( "Job_control: user action '%c'", User_action?User_action:'?' );

	Perm_check.service = 'C';
	Init_perms_check();
	permission = Perms_check( &Perm_file, &Perm_check, Cfp_static );
	DEBUG1( "Job_control: checked for C,  permission %s", perm_str(permission) );
	reject = 0;
	if( permission == ACCEPT ){
		Control_allowed = 1;
	} else {
		reject = ( (permission == REJECT)
			|| (permission == 0 && Last_default_perm == REJECT));
	}
	if( reject && User_action == 'S' ){
		Perm_check.service = User_action;
		Init_perms_check();
		permission = Perms_check( &Perm_file, &Perm_check, Cfp_static );
		reject = 0;
		if( permission == ACCEPT ){
			Control_allowed = 1;
		} else {
			reject = ( (permission == REJECT)
				|| (permission == 0 && Last_default_perm == REJECT));
		}
	}
	if( reject && User_action != 'U' ){
		if( Perm_check.service == 'S' ){
			plp_snprintf( error, sizeof(error),
				_("%s: no permission to get status"), Printer );
		} else {
			plp_snprintf( error, sizeof(error),
				_("%s: no permission to control queue"), Printer );
		}
		goto error;
	}
	DEBUG1( "Job_control: checked for C, Control_allowed %d", Control_allowed );

	switch( action ){
		case REREAD:
			DEBUG1( "Job_control: sending pid %d SIGHUP", Server_pid );
			plp_snprintf( error, sizeof(error), "lpd server pid %d on %s, sending SIGHUP\n",
				Server_pid, FQDNHost );
			(void)kill(Server_pid,SIGHUP);
			if( Write_fd_str( *socket, error ) < 0 ) cleanup(0);
			goto done;

		case LPD:
			DEBUG1( "Job_control: lpd pid %d", Server_pid );
			plp_snprintf( error, sizeof(error), "lpd server pid %d on %s\n",
				Server_pid, FQDNHost );
			if( Write_fd_str( *socket, error ) < 0 ) cleanup(0);
			goto done;

		case DEFAULTQ:
			Do_control_defaultq ( socket );
			goto done;
		case STATUs:
			/* we put out a space at the start to make PCNFSD happy */
			plp_snprintf( error, sizeof(error), status_header,
				" Printer", "Printing", "Spooling", "Jobs",
				"Server", "Slave", "Redirect", "Status/Debug","" );
			safestrncat(error,"\n");
			if( Write_fd_str( *socket, error ) < 0 ) cleanup(0);
		case STOP:
		case START:
		case DISABLE:
		case ENABLE:
		case ABORT:
		case UP:
		case DOWN:
		case HOLDALL:
		case NOHOLDALL:
			/* control line is 'Xprinter user action arg1 arg2
             *                    t[0]   t[1]  t[2]  t[3]
			 */
			if( tokencount > 4 ){
				/* we have a list of printers to use */
				for( i = 3; i < tokencount; ++i ){
					Printer = tokens[i].start;
					DEBUG1( "Job_control: doing printer '%s'", Printer );
					Do_printer_work( user, action, socket,
						tokencount, tokens, error, sizeof(error) );
				}
				goto done;
			}
			break;
		case MOVE:
			/* we have Nprinter user move jobid* target */
			if( tokencount < 5 ){
				plp_snprintf( error, sizeof(error),
					_("Use: MOVE printer (user|jobid)* target") );
				goto error;
			}
			break;
	}
	Do_printer_work( user, action, socket,
		tokencount, tokens, error, sizeof(error) );
done:
	DEBUG3( "Job_control: DONE" );
	return(0);

error:
	DEBUG2("Job_control: error msg '%s'", error );
	safestrncat(error,"\n");
	if( Write_fd_str( *socket, error ) < 0 ) cleanup(0);
	DEBUG3( "Job_control: done" );
	return(0);
}

void Do_printer_work( char *user, int action, int *socket,
	int tokencount, struct token *tokens, char *error, int errorlen )
{
	int c, i;

	if( strcmp( Printer, "all" ) ){
		DEBUG3( "Job_control: checking printcap entry '%s'",  Printer );
		Do_queue_control( user, action, socket,
			tokencount-4, &tokens[4], error, errorlen );
	} else {
		/* we work our way down the printcap list, checking for
			ones that have a spool queue */
		/* note that we have already tried to get the 'all' list */
		if( All_list.count ){
			char **line_list;
			DEBUG4("checkpc: using the All_list" );
			line_list = All_list.list;
			c = All_list.count;
			for( i = 0; i < c; ++i ){
				Printer = line_list[i];
				DEBUG4("checkpc: list entry [%d of %d] '%s'",
					i, c,  Printer );
				Do_queue_control( user, action, socket,
					tokencount-4, &tokens[4], error, errorlen);
			}
		} else {
			struct printcap_entry *entries, *entry;
			DEBUG4("checkpc: using the printcap list" );
			entries = (void *)Expanded_printcap_entries.list;
			c = Expanded_printcap_entries.count;
			for( i = 0; i < c; ++i ){
				entry = &entries[i];
				Printer = entry->names[0];
				DEBUG4("checkpc: printcap entry [%d of %d] '%s'",
					i, c,  Printer );
				Do_queue_control( user, action, socket,
					tokencount-4, &tokens[4], error, errorlen);
			}
		}
	}
}

/***************************************************************************
 * Do_queue_control()
 * do the actual queue control operations
 * - start, stop, enable, disable are simple
 * - others are more complex, and are handled in Do_control_file
 * We have tokens:
 *   user printer action p1 p2 p3 -> p1 p2 p3
 ***************************************************************************/

void Do_queue_control( char *user, int action, int *socket,
	int tokencount, struct token *tokens, char *error, int errorlen )
{
	struct control_file *cfp = 0, **cfpp;	/* control files */
	int i;
	pid_t serverpid;			/* server pid to kill off */
	struct stat statb;			/* status of file */
	int fd;						/* file descriptor */
	char line[LINEBUFFER];
	char msg[LINEBUFFER];
	char *pname;
	int status;
	int permission, reject;
	struct printcap_entry *pc = 0;
	int user_mod = 0;
	/* first get the printer name */

	/* process the job */
	Name = "Do_queue_control";
	error[0] = 0;

	/*
	 * some of the commands allow a list of printers to be
	 * specified, others only take a single printer
	 * We need to put in the list stuff for the ones that take a list
	 */

	pname = Printer;
	switch( action ){
	case LPQ:
		if( Do_control_lpq( user, action, socket,
			tokencount, tokens, error, errorlen ) ){
			goto error;
		}
		return;
	case PRINTCAP:
		Full_printer_vars( Printer, &pc );
		Do_control_printcap( pc, action, socket,
			tokencount, tokens, error, errorlen );
		return;
	}

	pc = 0;

	status = Setup_printer( Printer, error, errorlen, 0, 1, (void *) 0, &pc );

	if( status ){
		char pr[LINEBUFFER];
		if( Printer == 0 ) Printer = pname;
		switch( action ){
		case STATUs:
			plp_snprintf( pr, sizeof(pr), "%s@%s", Printer,
				Report_server_as?Report_server_as:ShortHost );
			if( status != 2 ){
				plp_snprintf( msg, sizeof(msg), _("%-18s WARNING %s\n"),
					pr, error );
			} else {
				if( RemotePrinter == 0 && RemoteHost == 0 ){
					plp_snprintf( error, errorlen,
						_(" printer %s@%s not in printcap"),
						Printer, Report_server_as?Report_server_as:ShortHost );
				} else {
					if( RemoteHost == 0 ) RemoteHost = Default_remote_host;
					if( RemotePrinter == 0 ) RemotePrinter = Printer;
					plp_snprintf( error, errorlen,
						_(" no spooling, forwarding directly to %s@%s"),
						RemotePrinter, RemoteHost );
				}
				plp_snprintf( msg, sizeof(msg), "%-18s - %s\n",
					pr, error );
			}
			if( Write_fd_str( *socket, msg ) < 0 ) cleanup(0);
			return;
		default:
			goto error;
		}
	}

	if(DEBUGL3 ){
		logDebug("Do_queue_control: tokencount '%d'", tokencount );
		for( i = 0; i < tokencount; ++i ){
			logDebug("Do_queue_control: token [%d] '%s'", i, tokens[i].start );
		}
	}

	DEBUG1("Do_queue_control: Control_allowed %d", Control_allowed );
	if( !Control_allowed ){
		Perm_check.service = 'C';
		Init_perms_check();
		permission = Perms_check( &Perm_file, &Perm_check, Cfp_static );
		reject = 0;
		if( permission == ACCEPT ){
			Control_allowed = 1;
		} else {
			reject = ( (permission == REJECT)
				|| (permission == 0 && Last_default_perm == REJECT));
		}
		if( reject && User_action == 'S' ){
			Perm_check.service = User_action;
			Init_perms_check();
			permission = Perms_check( &Perm_file, &Perm_check, Cfp_static );
			reject = 0;
			if( permission == ACCEPT ){
				Control_allowed = 1;
			} else {
				reject = ( (permission == REJECT)
					|| (permission == 0 && Last_default_perm == REJECT));
			}
		}
		if( reject && User_action != 'U' ){
			if( Perm_check.service == 'S' ){
				plp_snprintf( error, sizeof(error),
					_("%s: no permission to get status"), Printer );
			} else {
				plp_snprintf( error, sizeof(error),
					_("%s: no permission to control queue"), Printer );
			}
			goto error;
		}
	}

	/* set the conditions */

	switch( action ){
	case STOP: Printing_disabled = 1; break;
	case START: Printing_disabled = Printing_aborted = 0; break;
	case DISABLE: Spooling_disabled = 1; break;
	case ENABLE: Spooling_disabled = 0; break;
	case ABORT: case KILL: break;
	case UP: Printing_disabled = Printing_aborted = Spooling_disabled = 0; break;
	case DOWN: Printing_disabled = Spooling_disabled = 1; break;
	case HOLDALL: Hold_all = 1; break;
	case NOHOLDALL: Hold_all = 0; break;

	case RELEASE: case REDO: case TOPQ:
		Printing_disabled = Printing_aborted = 0;
	case HOLD:
		if( Do_control_file( user, action, socket,
			tokencount, tokens, error, errorlen ) ){
			goto error;
		}
		break;

	case MOVE:
		Redirect_str = tokens[tokencount-1].start;
		--tokencount;
		if( strlen( Redirect_str ) >=
			sizeof( ((struct control_file *)0)->hold_info.redirect ) - 2 ){
			plp_snprintf( error, errorlen,
				_("%s: destination printer too long '%s'"),
				Printer, Redirect_str );
			goto error;
		}
		if( Do_control_file( user, action, socket,
			tokencount, tokens, error, errorlen ) ){
			goto error;
		}
		break;

	case LPRM:
		if( Do_control_lpq( user, action, socket,
			tokencount, tokens, error, errorlen ) ){
			goto error;
		}
		break;
		
	case STATUs:
		if( Do_control_status( user, action, socket,
			tokencount, tokens, error, errorlen ) ){
			goto error;
		}
		break;

	case REDIRECT:
		if( Do_control_redirect( user, action, socket,
			tokencount, tokens, error, errorlen ) ){
			goto error;
		}
		break;

	case CLAss:
		if( Do_control_class( user, action, socket,
			tokencount, tokens, error, errorlen ) ){
			goto error;
		}
		break;

	case DEBUG:
		if( Do_control_debug( user, action, socket,
			tokencount, tokens, error, errorlen ) ){
			goto error;
		}
		break;
		
	default:
		plp_snprintf( error, errorlen, _("not implemented yet") );
		goto error;
	}

	/* modify the control file to force rescan of queue */

	switch( action ){
	case STATUs:
	case LPD:
		break;
	default:
		Set_spool_control( (void *)0 );
	}

	/* kill off the server */
	switch( action ){
	/* case STOP: - you do NOT want to kill server*/
	case ABORT:
	case KILL:
	case DOWN:
		pname = Add_path( CDpathname, Printer );
		serverpid = 0;
		if( (fd = Checkread( pname, &statb ) ) >= 0 ){
			serverpid = Read_pid( fd, (char *)0, 0 );
			close( fd );
			if( kill( serverpid, 0 ) ){
				serverpid = 0;
			}
		}

		/* get jobs */
		Scan_queue(1,1);

		cfpp = (void *)C_files_list.list;
		for( i = 0; status == 0 && i < C_files_list.count; ++i ){
			cfp = cfpp[i];
			if( (cfp->hold_info.server
				&& kill( cfp->hold_info.server, 0 ) == 0)
				|| i == 0 ){
				/* now we need to find out if this is the active job */
				if( !Control_allowed && User_action == 'U' ){
					/* now we get the user name and IP address */
					if( cfp->LOGNAME && cfp->LOGNAME[1] ){
						Perm_check.user = cfp->LOGNAME+1;
					} else {
						Perm_check.user = 0;
					}
					if( cfp->FROMHOST && cfp->FROMHOST[1]
						&& Find_fqdn( &PermcheckHostIP,cfp->FROMHOST+1, 0 ) ){
						Perm_check.host = &PermcheckHostIP;
					} else {  
						Perm_check.host = 0;
					}
					Perm_check.service = 'U';
					Init_perms_check();
					reject = ((permission = Perms_check( &Perm_file, &Perm_check,
							Cfp_static )) == REJECT
						|| (permission == 0 && Last_default_perm == REJECT));
					if( reject ){
						plp_snprintf( msg, sizeof(msg), _("no perms '%s'\n"),
								cfp->identifier+1);
						if( Write_fd_str( *socket, msg ) < 0 ) cleanup(0);
						continue;
					}
				}
				user_mod = 1;
				setmessage( cfp, "QUEUE", "%s@%s: %s",
					Printer, FQDNHost, "terminating active printing" );
			}
		}
		cfp = 0;
		DEBUG4("Do_queue_control: server pid %d", serverpid );
		if( serverpid > 0 && (Control_allowed || user_mod) ){
			DEBUG4("Do_queue_control: kill(pid %d, SIGINT)", serverpid );
			if( kill( serverpid, SIGINT ) == 0 ){
				setmessage( 0, "QUEUE", "%s@%s: %s %d",
					Printer, FQDNHost, "terminating active server - PID ",
					serverpid );
				plp_snprintf(msg,sizeof(msg),_("killing server PID %d\n"),
					serverpid );
				Write_fd_str(*socket,msg);
				while( kill(serverpid, SIGINT ) == 0 ){
					plp_sleep(2);
					if( kill( serverpid, SIGINT ) == 0 ){
						plp_snprintf(msg,sizeof(msg),
							_("killing server PID %d again\n"),
							serverpid );
						Write_fd_str(*socket,msg);
						DEBUG4("Do_queue_control: server %d still active",
						serverpid );
					}
				}
			}
			serverpid = 0;
		}
		break;
	}

	/* start the server if necessary */
	switch( action ){
	case KILL:
	case TOPQ:
	case RELEASE:
	case REDO:
	case HOLD:
	case UP:
	case START:
	case REDIRECT:
	case MOVE:
	case NOHOLDALL:
		if( Server_queue_name && *Server_queue_name ){
			plp_snprintf( line, sizeof(line), "!%s\n", Server_queue_name );
		} else {
			plp_snprintf( line, sizeof(line), "!%s\n", Printer );
		}
		DEBUG3("Do_queue_control: sending '%s' to LPD", line );
		if( Write_fd_str( Lpd_pipe[1], line ) < 0 ){
			logerr_die( LOG_ERR, _("Do_queue_control: write to pipe '%d' failed"),
				Lpd_pipe[1] );
		}
	}

	switch( action ){
	case STATUs:	Action = 0; break; /* no message */
	case UP:		Action = _("enabled and started"); break;
	case DOWN:		Action = _("disabled and stopped"); break;
	case STOP:		Action = _("stopped"); break;
	case RELEASE: case REDO: case TOPQ:
	case START:		Action = _("started"); break;
	case DISABLE:	Action = _("disabled"); break;
	case ENABLE:	Action = _("enabled"); break;
	case REDIRECT:	Action = _("redirected"); break;
	case HOLDALL:	Action = _("holdall on"); break;
	case NOHOLDALL:	Action = _("holdall off"); break;
	case MOVE:		Action = _("move done"); break;
	case CLAss:		Action = _("class updated"); break;
	case KILL:      Action = _("killed job"); break;
	case ABORT:		Action = _("aborted job"); break;
	}
	if( Action ){
		if( tokencount > 0 ){
			setmessage( cfp, "QUEUE", "%s@%s: %s %s",
			Printer, FQDNHost, Action, tokens[0].start );
		} else {
			setmessage( cfp, "QUEUE", "%s@%s: %s",
			Printer, FQDNHost, Action );
		}
		plp_snprintf( line, sizeof(line), "%s@%s: %s\n",
			Printer, FQDNHost, Action );
		if( Write_fd_str( *socket, line ) < 0 ) cleanup(0);
	}

	return;

error:
	DEBUG2("Do_queue_control: error msg '%s'", error );
	if( strchr(error, '\n') == 0 ){
		error[errorlen-2] = 0;
		strcat(error,"\n");
	}
	if( Write_fd_str( *socket, error ) < 0 ) cleanup(0);
	DEBUG3( "Do_queue_control: done" );
	return;
}


/***************************************************************************
 * Do_control_file:
 *  perform a suitable operation on a control file
 * 1. get the control files
 * 2. check to see if the control file has been selected
 * 3. update the hold file for the control file
 ***************************************************************************/

int Do_control_file( char *user, int action, int *socket,
	int tokencount, struct token *tokens, char *error, int errorlen )
{
	int i, j;						/* ACME! Nothing but the best */
	int status;						/* status of last IO op */
	char msg[SMALLBUFFER];			/* message field */
	struct control_file *cfp, **cfpp;	/* control files */
	int select;						/* select this file */
	struct token token;
	struct destination *destination;
	int permission, reject;

	/* set up default token */
	token.start = user;
	token.length = strlen( user );

	/* get the job files */
	Scan_queue( 1, 1 );

	DEBUG4("Do_control_file: total files %d, tokencount %d",
		C_files_list.count, tokencount );

	/* scan the files to see if there is one which matches */

	status = 0;
	cfpp = (void *)C_files_list.list;
	for( i = 0; status == 0 && i < C_files_list.count; ++i ){
		cfp = cfpp[i];

		/*
		 * check to see if this entry matches any of the patterns
		 */

		DEBUG4("Do_control_file: checking '%s'", cfp->transfername );

		select = 1;
		destination = 0;

next_destination:
		if( tokencount > 0 ){
			for( j = 0; select && j < tokencount; ++j ){
				select = Patselect( &tokens[j], cfp, &destination );
			}
		} else {
			select = Patselect( &token, cfp, &destination );
		}
		if( !select ) continue;

		/* now we check for user permissions */
		if( !Control_allowed ){
			/* now we get the user name and IP address */
			if( cfp->LOGNAME && cfp->LOGNAME[1] ){
				Perm_check.user = cfp->LOGNAME+1;
			} else {
				Perm_check.user = 0;
			}
			if( cfp->FROMHOST && cfp->FROMHOST[1]
				&& Find_fqdn( &PermcheckHostIP,cfp->FROMHOST+1, 0 ) ){
				Perm_check.host = &PermcheckHostIP;
			} else {  
				Perm_check.host = 0;
			}
			Perm_check.service = 'U';
			Init_perms_check();
			reject = ((permission = Perms_check( &Perm_file, &Perm_check,
					Cfp_static )) == REJECT
				|| (permission == 0 && Last_default_perm == REJECT));
			if( reject ){
				plp_snprintf( msg, sizeof(msg), _("no perms '%s'\n"),
					destination?
						(destination->identifier+1):(cfp->identifier+1));
				if( Write_fd_str( *socket, msg ) < 0 ) cleanup(0);
				continue;
			}
		}

		DEBUG4("Do_control_file: selected '%s', id '%s', destination '%s'",
			cfp->transfername, cfp->identifier+1, destination->destination );
		/* we report this job being selected */
		plp_snprintf( msg, sizeof(msg), _("%s: selected '%s'\n"),
			Printer, destination?
				(destination->identifier+1):(cfp->identifier+1));
		if( Write_fd_str( *socket, msg ) < 0 ) cleanup(0);
		switch( action ){
		case HOLD:
			if( destination ){
				destination->hold_time = time( (void *)0 );
			} else {
				cfp->hold_info.hold_time = time( (void *)0 );
				destination = (void *)cfp->destination_list.list;
				for( j =0; j < cfp->destination_list.count; ++j ){
					destination[j].hold_time = cfp->hold_info.hold_time;
				}
				destination = 0;
			}
			setmessage( cfp, "TRACE", "%s@%s: job held", Printer, FQDNHost );
			break;
		case TOPQ: cfp->hold_info.priority_time = time( (void *)0 );
			if( destination ){
				cfp->hold_info.hold_time = 0;
				destination->hold_time = 0;
			} else {
				cfp->hold_info.hold_time = 0;
				destination = (void *)cfp->destination_list.list;
				for( j =0; j < cfp->destination_list.count; ++j ){
					destination[j].hold_time = 0;
				}
				destination = 0;
			}
			setmessage( cfp, "TRACE", "%s@%s: job topq", Printer, FQDNHost );
			break;
		case MOVE:
			strcpy( cfp->hold_info.redirect, Redirect_str );
			/* and we update the priority to put it at head of queue */
			cfp->hold_info.priority_time = time( (void *)0 );
			cfp->hold_info.hold_time = 0;
			setmessage( cfp, "TRACE", "%s@%s: job moved", Printer, FQDNHost );
			break;
		case RELEASE:
		case REDO:
			cfp->hold_info.hold_time = 0;
			cfp->hold_info.attempt = 0;
			if( action == REDO ){
				cfp->hold_info.done_time = 0;
				/* cfp->hold_info.not_printable = 0; */
			}
			if( destination ){
				destination->hold_time = 0;
				if( action == REDO ){
					destination->done_time = 0;
					destination->copy_done = 0;
				}
			} else {
				destination = (void *)cfp->destination_list.list;
				for( j =0; j < cfp->destination_list.count; ++j ){
					destination[j].hold_time = 0;
					destination[j].attempt = 0;
					destination[j].ignore = 0;
					if( action == REDO ){
						destination[j].done_time = 0;
						destination[j].copy_done = 0;
					}
				}
				destination = 0;
			}
			setmessage( cfp, "TRACE", "%s@%s: job %s", Printer, FQDNHost,
				(action == RELEASE) ? "released" : "redone" );
			break;
		}
		cfp->hold_info.done_time = 0;
		cfp->error[0] = 0;
		if( destination ){
			destination->error[0] = 0;
			destination->done_time = 0;
		} else {
			destination = (void *)cfp->destination_list.list;
			for( j =0; j < cfp->destination_list.count; ++j ){
				destination[j].done_time = 0;
				destination[j].error[0] = 0;
			}
			destination = 0;
		}
		Set_job_control( cfp, (void *)0 );
		if( tokencount <= 0 ){
			DEBUG4("Do_control_file: finished '%s'", cfp->transfername );
			break;
		}
		if( destination ) goto next_destination;
	}
	return( 0 );
}



/***************************************************************************
 * Do_control_lpq:
 *  forward an LPQ or LPRM
 ***************************************************************************/

int Do_control_lpq( char *user, int action, int *socket,
	int tokencount, struct token *tokens, char *error, int errorlen )
{
	char msg[LINEBUFFER];			/* message field */
	int i = 0;

	/* synthesize an LPQ or LPRM line */ 
	msg[sizeof(msg)-1] = 0;
	switch( action ){
	case LPQ:  i = REQ_DSHORT; break;
	case LPRM: i = REQ_REMOVE; break;
	}

	plp_snprintf( msg, sizeof(msg), "%c%s", i, Printer );
	switch( action ){
		case LPRM: 
			safestrncat( msg, " " );
			safestrncat( msg, user );
			break;
	}
	for( i = 0; i < tokencount; ++i ){
		safestrncat( msg, " " );
		safestrncat( msg, tokens[i].start );
	}
	safestrncat( msg, "\n" );
	DEBUG3("Do_control_lpq: sending '%s'", msg );

	switch( action ){
	case LPQ: Job_status( socket,  msg, sizeof(msg) ); break;
	case LPRM: Job_remove( socket,  msg, sizeof(msg) ); break;
	}
	return(0);
}

/***************************************************************************
 * Do_control_status:
 *  report current status
 ***************************************************************************/

int Do_control_status( char *user, int action, int *socket,
	int tokencount, struct token *tokens, char *error, int errorlen )
{
	char msg[SMALLBUFFER];			/* message field */
	char pr[LINEBUFFER];
	char pr_status[LINEBUFFER];
	char count[32];
	char server[32];
	char spooler[32];
	char forward[LINEBUFFER];
	int serverpid, unspoolerpid;	/* server and unspooler */
	int i, fd;
	struct stat statb;
	char *path;
	int cnt, hold_cnt;
	int control_status = 0;
	struct control_file *cfp, **cfpp;	/* pointer to control file */
	struct destination *dest;

	/* get the job files */
	Scan_queue( 1, 1 );

	DEBUG4("Do_control_status: total files %d, tokencount %d",
		C_files_list.count, tokencount );

	cnt = 0;
	hold_cnt = 0;
	cfpp = (void *)C_files_list.list;
	for( i = 0; i < C_files_list.count; ++i ){
		cfp = cfpp[i];
		if( cfp->hold_info.hold_time ){
			++hold_cnt;
		} else if( cfp->hold_info.server > 0 && kill(cfp->hold_info.server,0) == 0 ){
			/* active */
			++cnt;
		} else if( Job_printable_status( cfp, &dest, msg, sizeof(msg) ) == 0 ){ 
			++cnt;
		}
	}
	DEBUG0( "Do_control_status: printable jobs %d, held jobs %d", cnt, hold_cnt );

	/* now check to see if there is a server and unspooler process active */
	path = Add_path( CDpathname, Printer );
	serverpid = 0;
	if( (fd = Checkread( path, &statb ) ) >= 0 ){
		serverpid = Read_pid( fd, (char *)0, 0 );
		close( fd );
	}
	DEBUG4("Get_queue_status: server pid %d", serverpid );
	if( serverpid > 0 ){
		if( kill( serverpid, 0 ) < 0 ){
			DEBUG4("Get_queue_status: server %d not active", serverpid );
			serverpid = 0;
		}
	} /**/

	path = Add2_path( CDpathname, "unspooler.", Printer );
	unspoolerpid = 0;
	if( (fd = Checkread( path, &statb ) ) >= 0 ){
		unspoolerpid = Read_pid( fd, (char *)0, 0 );
		close( fd );
	}

	DEBUG4("Get_queue_status: unspooler pid %d", unspoolerpid );
	if( unspoolerpid > 0 ){
		if( kill( unspoolerpid, 0 ) < 0 ){
			DEBUG4("Get_queue_status: unspooler %d not active", unspoolerpid );
			unspoolerpid = 0;
		}
	} /**/
	close(fd);


	plp_snprintf( pr, sizeof(pr), "%s@%s", Printer,
		Report_server_as?Report_server_as:ShortHost );
	pr_status[0] = 0;
	if( Bounce_queue_dest ){
		int len;
		if( control_status++ == 0 ){
			strncat( pr_status, "(", sizeof(pr_status) );
		} else {
			strncat( pr_status, " ", sizeof(pr_status) );
		}
		len = strlen(pr_status);
		plp_snprintf( pr_status+len, sizeof(pr_status)-len,
			"bq->%s", Bounce_queue_dest );
	}
	if( Hold_all ){
		int len;
		if( control_status++ == 0 ){
			strncat( pr_status, "(", sizeof(pr_status) );
		} else {
			strncat( pr_status, " ", sizeof(pr_status) );
		}
		len = strlen(pr_status);
		plp_snprintf( pr_status+len, sizeof(pr_status)-len, _("holdall") );
	}
	if( Auto_hold ){
		int len;
		if( control_status++ == 0 ){
			strncat( pr_status, "(", sizeof(pr_status) );
		} else {
			strncat( pr_status, " ", sizeof(pr_status) );
		}
		len = strlen(pr_status);
		plp_snprintf( pr_status+len, sizeof(pr_status)-len, _("autohold") );
	}
	if( control_status ) strncat( pr_status, ") ", sizeof(pr_status) );
	plp_snprintf( count, sizeof(count), "%d", cnt );
	strcpy( server, "none" );
	strcpy( spooler, "none" );
	if( serverpid ) plp_snprintf( server, sizeof(server),"%d",serverpid );
	if( unspoolerpid ) plp_snprintf( spooler, sizeof(spooler),"%d",unspoolerpid );

	forward[0] = 0;
	if( Forwarding ){
		plp_snprintf( forward, sizeof( forward ), "%s", Forwarding );
	}

	plp_snprintf( msg, sizeof(msg),
		status_header,
		pr,
		Printing_disabled?"disabled":(Printing_aborted?"aborted":"enabled"),
		Spooling_disabled? "disabled" : "enabled",
		count, server, spooler, forward, pr_status, Control_debug?Control_debug:"" );
	trunc_str( msg );
	safestrncat(msg,"\n");
	if( Write_fd_str( *socket, msg ) < 0 ) cleanup(0);
	return( 0 );
}


/***************************************************************************
 * Do_control_redirect:
 *  perform a suitable operation on a control file
 * 1. get the control files
 * 2. if no options, report redirect name
 * 3. if option = none, remove redirect file
 * 4. if option = printer@host, specify name
 ***************************************************************************/

int Do_control_redirect( char *user, int action, int *socket,
	int tokencount, struct token *tokens, char *error, int errorlen )
{
	char forward[LINEBUFFER];
	char *s;

	/* get the spool entries */
	DEBUG4("Do_control_redirect: tokencount %d", tokencount );

	error[0] = 0;
	forward[0] = 0;
	switch( tokencount ){
	case -1:
	case 0:
		break;
	case 1:
		s = tokens[0].start;
		DEBUG4("Do_control_redirect: redirect to '%s'", s );
		if( strcasecmp( s, "off" ) == 0 ){
			Forwarding = 0;
		} else {
			if( strpbrk( s, ":; \t;" ) ){
				plp_snprintf( error, errorlen,
					_("forward format is printer@host, not '%s'"), s );
				goto error;
			}
			Forwarding = s;
		}
		break;

	default:
		mystrncpy( error, _("too many arguments"), errorlen );
		goto error;
	}

	if( Forwarding ){
		plp_snprintf( forward, sizeof(forward), _("forwarding to '%s'"),
			Forwarding );
	} else {
		plp_snprintf( forward, sizeof(forward), _("forwarding off") );
	}

	if( forward[0] ){
		safestrncat(forward,"\n");
		if( Write_fd_str( *socket, forward ) < 0 ) cleanup(0);
	}
	return( 0 );

error:
	return( 1 );
}


/***************************************************************************
 * Do_control_class:
 *  perform a suitable operation on a control file
 * 1. get the control files
 * 2. if no options, report class name
 * 3. if option = none, remove class file
 * 4. if option = printer@host, specify name
 ***************************************************************************/

int Do_control_class( char *user, int action, int *socket,
	int tokencount, struct token *tokens, char *error, int errorlen )
{
	char forward[LINEBUFFER];
	char *s;

	/* get the spool entries */
	DEBUG4("Do_control_class: tokencount %d", tokencount );

	error[0] = 0;
	forward[0] = 0;
	switch( tokencount ){
	case -1:
	case 0:
		break;
	case 1:
		s = tokens[0].start;
		DEBUG4("Do_control_class: class to '%s'", s );
		if( strcasecmp( s, "off" ) == 0 ){
			Classes = 0;
		} else {
			Classes = s;
		}
		break;

	default:
		mystrncpy( error, _("too many arguments"), errorlen );
		goto error;
	}

	if( Classes ){
		plp_snprintf( forward, sizeof(forward), _("classes printed '%s'"),
			Classes );
	} else {
		plp_snprintf( forward, sizeof(forward), _("all classes printed") );
	}

	if( forward[0] ){
		safestrncat(forward,"\n");
		if( Write_fd_str( *socket, forward ) < 0 ) cleanup(0);
	}
	return( 0 );

error:
	return( 1 );
}

/***************************************************************************
 * Do_control_debug:
 *  perform a suitable operation on a control file
 * 1. get the control files
 * 2. if no options, report debug name
 * 3. if option = none, remove debug file
 * 4. if option = printer@host, specify name
 ***************************************************************************/

int Do_control_debug( char *user, int action, int *socket,
	int tokencount, struct token *tokens, char *error, int errorlen )
{
	char debugging[LINEBUFFER];
	char *s;

	/* get the spool entries */
	DEBUG4("Do_control_debug: tokencount %d", tokencount );

	error[0] = 0;
	debugging[0] = 0;
	switch( tokencount ){
	case -1:
	case 0:
		break;
	case 1:
		s = tokens[0].start;
		DEBUG4("Do_control_debug: debug to '%s'", s );
		if( strcasecmp( s, "off" ) == 0 ){
			Control_debug = 0;
		} else {
			Control_debug = s;
		}
		break;

	default:
		mystrncpy( error, _("too many arguments"), errorlen );
		goto error;
	}

	if( Control_debug ){
		plp_snprintf( debugging, sizeof(debugging),
			_("debugging override set to '%s'"),
			Control_debug );
	} else {
		plp_snprintf( debugging, sizeof(debugging), _("debugging override off") );
	}

	if( debugging[0] ){
		safestrncat(debugging,"\n");
		if( Write_fd_str( *socket, debugging ) < 0 ) cleanup(0);
	}
	return( 0 );

error:
	return( 1 );
}


/***************************************************************************
 * Do_control_printcap:
 *  get the LPD status
 * 1. get the printcap PID
 * 2. if no options, report PID
 * 3. if option = HUP, send signal
 ***************************************************************************/

int Do_control_printcap( struct printcap_entry *pc, int action, int *socket,
	int tokencount, struct token *tokens, char *error, int errorlen )
{
	char *printcap;

	/* get the spool entries */

	DEBUG4("Do_control_printcap: tokencount %d", tokencount );
	printcap = Linearize_pc_list( pc, (char *)0 );

	if( printcap == 0 || *printcap == 0 ){
		printcap = "\n";
	}
	if( Write_fd_str( *socket, printcap ) < 0 ) cleanup(0);
	return(0);
}

int Do_control_defaultq( int *socket )
{
	char msg [LINEBUFFER];

	Printer = 0;
	/* get the default queue */
	Get_printer(0);
	plp_snprintf( msg, sizeof(msg), "%s\n", Printer );
	if ( Write_fd_str( *socket, msg ) < 0 ) cleanup(0);

	return(0);
}