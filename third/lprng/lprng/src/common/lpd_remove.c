/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1999, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 *
 ***************************************************************************/

 static char *const _id =
"$Id: lpd_remove.c,v 1.1.1.1 1999-05-04 18:06:59 danw Exp $";


#include "lp.h"
#include "lpd_remove.h"
#include "getqueue.h"
#include "getprinter.h"
#include "gethostinfo.h"
#include "getopt.h"
#include "permission.h"
#include "child.h"
#include "proctitle.h"
#include "fileopen.h"
#include "sendreq.h"
/**** ENDINCLUDE ****/

/***************************************************************************
 * Commentary:
 * Patrick Powell Tue May  2 09:32:50 PDT 1995
 * 
 * Remove a Job.
 * This is very similar to the status program.
 * 
 * 1. We check for permissions first
 *    - first we check to see if the remote host has permissions
 *    - next we check to see if the user has permissions
 * Note:
 *   if we have control permissions,  then we can remove any job.
 *   Normally,  the options passed are 'user jobnumber' and we
 *   use the user name and/or job number to select them.  When we have
 *   control permissions,  we are not restricted to our own jobs.
 *     so: if we have control permissions,  AND pass an option, we
 *         do not check for our name.
 *         if we have control permissions AND we do not pass an option,
 *         we check for our name.
 *
 *  we have \006printer user key key key
 *              0       1    2 ...        index
 ***************************************************************************/

int Job_remove( int *sock, char *input )
{
	char error[LINEBUFFER];
	int i;
	char *name, *s, *user = 0;
	struct line_list tokens, done_list;

	Init_line_list(&tokens);
	Init_line_list(&done_list);
	Name = "Job_remove";

	/* get the options */
	++input;
	DEBUG1("Job_remove: input '%s'", input );
	Split(&tokens,input,Whitespace,0,0,0,0,0);
	if(DEBUGL2)Dump_line_list("Job_remove: input", &tokens );

	/* check printername for characters, underscore, digits */

	if( tokens.count < 2 ){
		plp_snprintf( error, sizeof(error),
			_("missing user or printer name"));
		goto error;
	}
	name = tokens.list[0];

	DEBUG1("Job_remove: checking '%s'", name );
	if( (s = Clean_name( name )) ){
		plp_snprintf( error, sizeof(error),
			_("printer '%s' has illegal character at '%s' in name"), name, s );
		goto error;
	}
	DEBUG1("Job_remove: result '%s'", name );
	Set_DYN(&Printer_DYN,name);

	user = safestrdup(tokens.list[1],__FILE__,__LINE__);
	Perm_check.user = user;
	Perm_check.remoteuser = user;

	/* remove the first two tokens */
	Remove_line_list(&tokens,1);
	Remove_line_list(&tokens,0);
	Check_max(&tokens,1);
	tokens.list[tokens.count] = 0;

	if( strcmp( Printer_DYN, ALL ) ){
		DEBUG2( "Job_remove: checking printcap entry '%s'",  Printer_DYN );
		Set_DYN(&Printer_DYN, Printer_DYN );
		Get_queue_remove( user, sock, &tokens, &done_list );
	} else {
		Get_all_printcap_entries();
		for( i = 0; i < All_line_list.count; ++i ){
			Set_DYN(&Printer_DYN, All_line_list.list[i]);
			Get_queue_remove( user, sock, &tokens, &done_list );
		}
	}
	goto done;

 error:
	logmsg( LOG_INFO, _("Job_remove: error '%s'"), error );
	DEBUG2("Job_remove: error msg '%s'", error );
	safestrncat(error,"\n");
	if( Write_fd_str( *sock, error ) < 0 ) cleanup(0);
 done:
	DEBUG2( "Job_remove: done" );
	if( user ) free(user); user = 0;
	Free_line_list(&done_list);
	Free_line_list(&tokens);
	return( 0 );
}

/***************************************************************************
 * void Get_queue_remove
 *  - find and remove the spool queue entries
 ***************************************************************************/

void Get_queue_remove( char *user, int *sock, struct line_list *tokens,
	struct line_list *done_list )
{
	char msg[SMALLBUFFER], header[SMALLBUFFER];
	int control_perm, permission, count, removed, status,
		i, c = 0, pid, fd, active_pid;
	char *s, *identifier, *path;
	struct stat statb;
	struct line_list info;
	struct job job;

	Init_line_list(&info);
	Init_job(&job);

	/* set printer name and printcap variables */
	if(DEBUGL2)Dump_line_list("Get_queue_remove - tokens", tokens );
	DEBUG2( "Get_queue_remove: user '%s', printer '%s'",
		user, Printer_DYN );

	Errorcode = 0;

	setproctitle( "lpd LPRM '%s'", Printer_DYN );
	/* first check to see if you have control permissions */

	status = Setup_printer( Printer_DYN, msg, sizeof(msg) );
	if( status ){
		DEBUG2("Get_queue_remove: no spool queue '%s'", Printer_DYN);
		return;
	}

	/* set up status */
	if( Find_exists_value(done_list,Printer_DYN,Value_sep ) ){
		return;
	}
	Add_line_list(done_list,Printer_DYN,Value_sep,1,1);

	/* check for permissions */

	Free_line_list(&Perm_line_list);
	Merge_line_list(&Perm_line_list,&RawPerm_line_list,0,0,0);
	if( Perm_filters_line_list.count ){
		Filterprintcap( &Perm_line_list, &Perm_filters_line_list,
			Printer_DYN);
	}
	Perm_check.service = 'C';
	Perm_check.printer = Printer_DYN;
	Perm_check.host = 0;
	Perm_check.user = 0;

	control_perm = Perms_check( &Perm_line_list, &Perm_check, 0 );
	DEBUG2("Job_status: permission '%s'", perm_str(control_perm));

	if( control_perm != P_ACCEPT ) control_perm = 0;

	plp_snprintf( msg, sizeof(msg), _("Printer %s@%s:\n"),
		Printer_DYN, ShortHost_FQDN );
	Write_fd_str( *sock, msg );

	Free_line_list( &Sort_order );
	Scan_queue( Spool_dir_DYN, &Spool_control, &Sort_order,0,0,0,0 );
	DEBUG2("Get_queue_remove: total files %d", Sort_order.count );

	/* scan the files to see if there is one which matches */
	removed = 0;
	active_pid = 0;
	if(DEBUGL3)Dump_line_list("Get_queue_remove - tokens", tokens );
	for( count = 0; count < Sort_order.count; ++count ){
		Free_job(&job);
		s = Sort_order.list[count];
		if( (s = strchr(Sort_order.list[count],';')) ){
			Split(&job.info,s+1,";",1,Value_sep,1,1,1);
		}

		if(DEBUGL3)Dump_job("Get_queue_remove - info",&job);
        if( tokens->count && Patselect( tokens, &job.info, 0) ){
			continue;
        }


		identifier = Find_str_value(&job.info,IDENTIFIER,Value_sep);
		if( !identifier ) identifier
			= Find_str_value(&job.info,TRANSFERNAME,Value_sep);

		DEBUG3("Get_queue_remove: matched '%s'", identifier );
		plp_snprintf( msg, sizeof(msg), _("  checking perms '%s'\n"),
			identifier );
		Write_fd_str( *sock, msg );

		/* we check to see if we can remove this one if we are the user */

		if( control_perm == 0 ){
			/* now we get the user name and IP address */
			Perm_check.user = Find_str_value(&job.info,LOGNAME,Value_sep);
			Perm_check.host = 0;
			if( (s = Find_str_value(&job.info,FROMHOST,Value_sep)) 
				&& Find_fqdn( &PermHost_IP, s ) ){
				Perm_check.host = &PermHost_IP;
			}
			Perm_check.service = 'M';
			permission = Perms_check( &Perm_line_list, &Perm_check, &job );
			if( permission == P_REJECT ){
				plp_snprintf( msg, sizeof(msg), _("  no permissions '%s'\n"),
					identifier );
				Write_fd_str( *sock, msg );
				continue;
			}
		}

		/* log this to the world */
		DEBUG4("Get_queue_remove: removing '%s'", identifier );
		plp_snprintf( msg, sizeof(msg), _("  dequeued '%s'\n"),
			identifier );
		Write_fd_str( *sock, msg );

		setmessage( &job, "LPRM", "start" );
		if( Remove_job( &job ) ){
			setmessage( &job, "LPRM", "fail" );
			plp_snprintf( msg, sizeof(msg),
				_("error: could not remove '%s'"), identifier ); 
			Write_fd_str( *sock, msg );
			goto error;
		}
		setmessage( &job, "LPRM", "success" );
		if( (pid = Find_flag_value(&job.info,SERVER,Value_sep)) ){
			DEBUG4("Get_queue_remove: killing %d", pid );
			active_pid = pid;
		}
		++removed;
		if( tokens->count == 0 ) break;
	}
	Free_line_list(&info);
	Free_job(&job);
	Free_line_list( &Sort_order );
	if( removed ){
		/* kill spooler process */
		path = Make_pathname( Spool_dir_DYN, Printer_DYN );
		pid = 0;
		if( (fd = Checkread( path, &statb ) ) >= 0 ){
			pid = Read_pid( fd, (char *)0, 0 );
			close( fd );
		}
		DEBUG2("Get_queue_status: checking server pid %d", pid );
		free(path);
		/* kill active spooler */
		if( active_pid && active_pid != pid ){
			kill( active_pid, SIGINT );
			kill( pid, SIGCONT );
		}
		if( pid > 0 ){
			kill( pid, SIGUSR2 );
			/* sigh ... yes, you may need to start it */
			kill( pid, SIGCONT );
		}
	}

	if( Server_names_DYN ){
		Free_line_list(&info);
		Split(&info, Server_names_DYN, File_sep, 0,0,0,0,0);
		for( i = 0; i < info.count; ++i ){
			DEBUG2("Get_queue_status: getting subserver status '%s'", 
				info.list[i] );
			Set_DYN(&Printer_DYN,info.list[i]);
			Get_local_or_remote_remove( user, sock, tokens, done_list );
			DEBUG2("Get_queue_status: finished subserver status '%s'", 
				info.list[i] );
		}
	} else if( Destinations_DYN ){
		Free_line_list(&info);
		Split(&info, Destinations_DYN, File_sep, 0,0,0,0,0);
		for( i = 0; i < info.count; ++i ){
			DEBUG2("Get_queue_status: getting destination status '%s'", 
				info.list[i] );
			Set_DYN(&Printer_DYN,info.list[i]);
			Get_local_or_remote_remove( user, sock, tokens, done_list );
			DEBUG2("Get_queue_status: finished destination status '%s'", 
				info.list[i] );
		}
	} else if( Bounce_queue_dest_DYN ){
		DEBUG2("Get_queue_status: getting bouncequeue dest status '%s'", 
			Bounce_queue_dest_DYN);
		Set_DYN(&Printer_DYN,Bounce_queue_dest_DYN);
		Get_local_or_remote_remove( user, sock, tokens, done_list );
		DEBUG2("Get_queue_status: finished subserver status '%s'", 
			Bounce_queue_dest_DYN );
	} else if( RemoteHost_DYN ){
		fd = Send_request( 'M', REQ_REMOVE, tokens->list, Connect_timeout_DYN,
			Send_query_rw_timeout_DYN, *sock );
		if( fd >= 0 ){
			while( (c = read(fd,msg,sizeof(msg))) > 0 ){
				Write_fd_len(*sock,msg,c);
			}
			close(fd);
		}
	}

	DEBUG2("Get_queue_remove: finished '%s'", Printer_DYN );
	goto done;

 error:
	DEBUG2("Get_queue_remove: error msg '%s'", msg );
	plp_snprintf(header, sizeof(header), "Printer: %s", Printer_DYN );
	safestrncpy( header, _(" ERROR: ") );
	safestrncat( header, msg );
	safestrncat( header, "\n" );
	Write_fd_str( *sock, header );
 done:
	Free_line_list(&info);
	Free_job(&job);
	return;
}

void Get_local_or_remote_remove( char *user, int *sock,
	struct line_list *tokens, struct line_list *done_list )
{
	char msg[LARGEBUFFER];
	int fd, n;

	/* we have to see if the host is on this machine */

	if( !strchr(Printer_DYN,'@') ){
		Get_queue_remove( user, sock, tokens, done_list );
		return;
	}
	Fix_Rm_Rp_info();
	/* now we look at the remote host */
	if( Find_fqdn( &LookupHost_IP, RemoteHost_DYN )
		&& ( !Same_host(&LookupHost_IP,&Host_IP )
			|| !Same_host(&LookupHost_IP,&Host_IP )) ){
		Get_queue_remove( user, sock, tokens, done_list );
		return;
	}
	fd = Send_request( 'M', REQ_REMOVE, tokens->list, Connect_timeout_DYN,
		Send_query_rw_timeout_DYN, *sock );
	if( fd >= 0 ){
		while( (n = read(fd,msg,sizeof(msg))) > 0 ){
			Write_fd_len(*sock,msg,n);
		}
		close(fd);
	}
}

int Remove_file( char *openname )
{
	struct stat statb;
	int fail = 0;
	if( openname ){
		DEBUG3("Remove_file: removing '%s'", openname );
		unlink( openname );
		if( stat( openname, &statb ) == 0 ){
			logerr( LOG_INFO, "Remove_file: unlink did not remove '%s'",
				openname);
			fail = 1;
		}
	}
	return(fail);
}

int Remove_job( struct job *job )
{
	int i;
	int fail = 0;
	char *identifier, *openname, *hf_name;
	struct line_list *datafile;

	Setup_cf_info( Spool_dir_DYN, 0, job );

	if(DEBUGL3)Dump_job("Remove_job",job);
	identifier = Find_str_value(&job->info,IDENTIFIER,Value_sep);
	setmessage( job, TRACE, "remove START" );
	if( !identifier ){
		identifier = Find_str_value(&job->info,TRANSFERNAME,Value_sep);
	}

	DEBUG3("Remove_job: identifier '%s'",identifier);
	fail = 0;
	for( i = 0; i < job->datafiles.count; ++i ){
		datafile = (void *)job->datafiles.list[i];
		openname = Find_str_value(datafile,OPENNAME,Value_sep);
		if( openname ) fail |= Remove_file( openname );
	}
	openname = Find_str_value(&job->info,OPENNAME,Value_sep);
	if( openname ) fail |= Remove_file( openname );
	hf_name = Find_str_value(&job->info,HF_NAME,Value_sep);
	if( hf_name ) fail |= Remove_file( hf_name );

	if( fail == 0 ){
		setmessage( job, TRACE, "remove SUCCESS" );
	} else {
		setmessage( job, TRACE, "remove FAILED" );
	}
	return( fail );
}
