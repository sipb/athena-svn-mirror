/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-2000, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 *
 ***************************************************************************/

 static char *const _id =
"$Id: lpd_secure.c,v 1.2 2000-04-03 19:01:48 mwhitson Exp $";


#include "lp.h"
#include "lpd.h"
#include "lpd_secure.h"
#include "getopt.h"
#include "getqueue.h"
#include "proctitle.h"
#include "permission.h"
#include "linksupport.h"
#include "errorcodes.h"
#include "fileopen.h"
#include "lpd_rcvjob.h"
#include "child.h"
#include "globmatch.h"
#include "lpd_jobs.h"
#include "krb5_auth.h"

/**** ENDINCLUDE ****/

/***************************************************************************
 * Commentary:
 * Patrick Powell Mon Apr 17 05:43:48 PDT 1995
 * 
 * The protocol used to send a secure job consists of the following
 * following:
 * 
 * \REQ_SECUREprintername C/F user authtype\n         - receive a command
 *              0           1   2  3        4
 * \REQ_SECUREprintername C/F user authtype jobsize\n - receive a job
 *              0           1   2  3        4
 * 
 * The server will return an ACK, and then start the authentication
 * process.  See README.security for details.
 * 
 ***************************************************************************/

/*************************************************************************
 * Receive_secure() - receive a secure transfer
 *************************************************************************/
int Receive_secure( int *sock, char *input )
{
	char *printername;
	char error[SMALLBUFFER];	/* error message */
	char *authtype;
	char *cf, *s;
	char *jobsize = 0;
	char *user = 0;
	int tempfd = -1;
	int ack, status, from_server;
	struct line_list args, header_info, info;
	struct stat statb;
	char *tempfile = 0;
	struct security *security = 0;

	Name = "RCVSEC";
	memset( error, 0, sizeof(error));
	ack = 0;
	status = 0;

	DEBUGF(DRECV1)("Receive_secure: input line '%s'", input );
	Init_line_list( &args );
	Init_line_list( &header_info );
	Init_line_list( &info );

	Split(&args,input+1,Whitespace,0,0,0,0,0);
	DEBUGFC(DRECV1)Dump_line_list("Receive_secure - input", &args);
	if( args.count != 5 && args.count != 4 ){
		plp_snprintf( error+1, sizeof(error)-1,
			_("bad command line '%s'"), input );
		ack = ACK_FAIL;	/* no retry, don't send again */
		status = JFAIL;
		goto error;
	}
	Check_max(&args,1);
	args.list[args.count] = 0;

	/*
     * \REQ_SECUREprintername C/F user authtype jobsize\n - receive a job
     *              0           1   2  3        4
	 */
	printername = args.list[0];
	cf = args.list[1];
	user = args.list[2];	/* user is escape encoded */
	Unescape(user);
	authtype = args.list[3];
	Unescape(authtype);
	jobsize = args.list[4];

	setproctitle( "lpd %s '%s'", Name, printername );

	Perm_check.authtype = authtype;
	Perm_check.authfrom = CLIENT;
	from_server = 0;
	if( *cf == 'F' ){
		Perm_check.authfrom = SERVER;
		from_server = 1;
	}

	/* set up the authentication support information */

	if( Clean_name( printername ) ){
		plp_snprintf( error+1, sizeof(error)-1,
			_("bad printer name '%s'"), input );
		ack = ACK_FAIL;	/* no retry, don't send again */
		status = JFAIL;
		goto error;
	}

	Set_DYN(&Printer_DYN,printername);

	if( Setup_printer( printername, error+1, sizeof(error)-1 ) ){
		if( jobsize ){
			plp_snprintf( error+1, sizeof(error)-1,
				_("bad printer '%s'"), printername );
			ack = ACK_FAIL;	/* no retry, don't send again */
			status = JFAIL;
			goto error;
		}
	} else {
		int db, dbf;

		db = Debug;
		dbf = DbgFlag;
		s = Find_str_value(&Spool_control,DEBUG,Value_sep);
		if(!s) s = New_debug_DYN;
		Parse_debug( s, 0 );

		if( !(DRECVMASK & DbgFlag) ){
			Debug = db;
			DbgFlag = dbf;
		} else {
			int tdb, tdbf;
			tdb = Debug;
			tdbf = DbgFlag;
			Debug = db;
			DbgFlag = dbf;
			if( Log_file_DYN ){
				tempfd = Checkwrite( Log_file_DYN, &statb,0,0,0);
				if( tempfd > 0 && tempfd != 2 ){
					dup2(tempfd,2);
					close(tempfd);
				}
				tempfd = -1;
			}
			Debug = tdb;
			DbgFlag = tdbf;
			logDebug("Receive_secure: socket fd %d", *sock);
			Dump_line_list("Receive_secure - input", &args);
		}
		DEBUGF(DRECV1)("Receive_secure: debug '%s', Debug %d, DbgFlag 0x%x",
			s, Debug, DbgFlag );
	}

	if( !(security = Fix_receive_auth(authtype, &info)) ){
		plp_snprintf( error+1, sizeof(error)-1,
			_("unsupported authentication '%s'"), authtype );
		ack = ACK_FAIL;	/* no retry, don't send again */
		status = JFAIL;
		goto error;
	}
	if( !security->receive ){
		plp_snprintf( error+1, sizeof(error)-1,
			_("no receive method supported for '%s'"), authtype );
		ack = ACK_FAIL;	/* no retry, don't send again */
		status = JFAIL;
		goto error;
	}


	if( jobsize ){
		double read_len;
		read_len = strtod(jobsize,0);

		DEBUGF(DRECV2)("Receive_secure: spooling_disabled %d",
			Sp_disabled(&Spool_control) );
		if( Sp_disabled(&Spool_control) ){
			plp_snprintf( error+1, sizeof(error)-1,
				_("%s: spooling disabled"), Printer_DYN );
			ack = ACK_RETRY;	/* retry */
			status = JFAIL;
			goto error;
		}
		if( Max_job_size_DYN > 0 && (read_len+1023)/1024 > Max_job_size_DYN ){
			plp_snprintf( error+1, sizeof(error)-1,
				_("%s: job size %0.0f is larger than %d K"),
				Printer_DYN, read_len, Max_job_size_DYN );
			ack = ACK_RETRY;
			status = JFAIL;
			goto error;
		} else if( !Check_space( read_len, Minfree_DYN, Spool_dir_DYN ) ){
			plp_snprintf( error+1, sizeof(error)-1,
				_("%s: insufficient file space"), Printer_DYN );
			ack = ACK_RETRY;
			status = JFAIL;
			goto error;
		}
	}

	tempfd = Make_temp_fd(&tempfile);
	close(tempfd); tempfd = -1;

	DEBUGF(DRECV1)("Receive_secure: sock %d, user '%s', jobsize '%s'",  
		*sock, user, jobsize );

	status = security->receive( sock, user, jobsize, from_server,
		authtype, &info, error+1, sizeof(error)-1,
		&header_info, tempfile );

 error:
	DEBUGF(DRECV1)("Receive_secure: status %d, ack %d, error '%s'",
		status, ack, error+1 );

	if( status ){
		if( ack == 0 ) ack = ACK_FAIL;
		error[0] = ack;
		DEBUGF(DRECV1)("Receive_secure: sending '%s'", error );
		(void)Link_send( ShortRemote_FQDN, sock,
			Send_query_rw_timeout_DYN, error, strlen(error), 0 );
		Errorcode = JFAIL;
	}

	Free_line_list( &args );
	Free_line_list( &header_info );
	Free_line_list( &info );

	close( *sock ); *sock = -1;
	Remove_tempfiles();

	if( status == 0 && jobsize ){
		/* start a new server */
		DEBUGF(DRECV1)("Receive_secure: starting server");
		if( Server_queue_name_DYN ){
			Do_queue_jobs( Server_queue_name_DYN, 0, 0 );
		} else {
			Do_queue_jobs( Printer_DYN, 0, 0 );
		}
	}
	cleanup(0);
	return(0);
}

int Do_secure_work( int use_line_order, char *jobsize, int from_server,
	char *tempfile, struct line_list *header_info )
{
	int n, len, linecount = 0, done = 0, fd, status = 0;
	char *s, *t;
	char buffer[SMALLBUFFER];
	char error[SMALLBUFFER];
	struct stat statb;

	error[0] = 0;
	if( (fd = Checkread(tempfile,&statb)) < 0 ){ 
		status = JFAIL;
		plp_snprintf( error, sizeof(error),
			"Do_secure_work: reopen of '%s' failed - %s",
				tempfile, Errormsg(errno));
		goto error;
	}

	buffer[0] = 0;
	n = 0;
	done = 0;
	linecount = 0;

	while( !done && n < sizeof(buffer)-1
		&& (len = read( fd, buffer+n, sizeof(buffer)-1-n )) > 0 ){
		buffer[n+len] = 0;
		DEBUGF(DRECV1)("Do_secure_work: read %d - '%s'", len, buffer );
		while( !done && (s = safestrchr(buffer,'\n')) ){
			*s++ = 0;
			if( strlen(buffer) == 0 ){
				done = 1;
				break;
			}
			DEBUGF(DRECV1)("Do_secure_work: line [%d] '%s'", linecount, buffer );
			if( (t = strchr(buffer,'=')) ){
				*t++ = 0;
				Unescape(t);
				Set_str_value(header_info, buffer, t );
			} else {
				switch( linecount ){
					case 0:
						if( jobsize ){
							if( from_server ){
								Set_str_value(header_info,CLIENT,buffer);
							}
							done = 1;
						} else {
							Set_str_value(header_info,INPUT,buffer); break;
						}
						break;
					case 1:
						Set_str_value(header_info,CLIENT,buffer);
						done = 1;
						break;
				}
			}
			++linecount;
			memmove(buffer,s,strlen(s)+1);
			n = strlen(buffer);
		}
	}

	if( fd >= 0 ) close(fd); fd = -1;

	DEBUGFC(DRECV1)Dump_line_list("Do_secure_work - header", header_info );

	if( (status = Check_secure_perms( header_info, from_server, error, sizeof(error))) ){
		goto error;
	}

	buffer[0] = 0;
	if( jobsize ){
		if( (fd = Checkread(tempfile, &statb) ) < 0 ){
			status = JFAIL;
			plp_snprintf( error, sizeof(error),
				"Do_secure_work: reopen of '%s' for read failed - %s",
					tempfile, Errormsg(errno));
			goto error;
		}
		s = Find_str_value(header_info, CLIENT, Value_sep );
		status = Scan_block_file( fd, error, sizeof(error), s );
	} else {
		if( (fd = Checkwrite(tempfile,&statb,O_WRONLY|O_TRUNC,1,0)) < 0 ){
			status = JFAIL;
			plp_snprintf( error, sizeof(error),
				"Do_secure_work: reopen of '%s' for write failed - %s",
					tempfile, Errormsg(errno));
			goto error;
		}
		if( (s = Find_str_value(header_info,INPUT,Value_sep)) ){
			Dispatch_input( &fd, s );
		}
	}

 error:

	if( fd >= 0 ) close(fd); fd = -1;
	DEBUGF(DRECV1)("Do_secure_work: status %d, error '%s'", status, error );
	if( error[0] ){
		logmsg(LOG_INFO,"Do_secure_work: error '%s'", status, error );
		if( (fd = Checkwrite(tempfile,&statb,O_WRONLY|O_TRUNC,1,0)) < 0 ){
			Errorcode = JFAIL;
			logerr_die(LOG_INFO, "Do_secure_work: reopen of '%s' for write failed",
				tempfile );
		}
		Write_fd_str(fd,error);
		close(fd);
	}
	return( status );
}



 extern struct security ReceiveSecuritySupported[];

/***************************************************************************
 * void Fix_auth() - get the Use_auth_DYN value for the remote printer
 ***************************************************************************/

struct security *Fix_receive_auth( char *name, struct line_list *info )
{
	struct security *s;

	if( name == 0 ){
		if( Is_server ){
			name = Auth_forward_DYN;
		} else {
			name = Auth_DYN;
		}
	}

	for( s = ReceiveSecuritySupported; s->name && Globmatch(s->name, name ); ++s );
	DEBUG1("Fix_receive_auth: name '%s' matches '%s'", name, s->name );
	if( s->name == 0 ){
		s = 0;
	} else {
		char buffer[64], *str;
		if( !(str = s->config_tag) ) str = s->name;
		plp_snprintf(buffer,sizeof(buffer),"%s_", str );
		Find_default_tags( info, Pc_var_list, buffer );
		Find_tags( info, &Config_line_list, buffer );
		Find_tags( info, &PC_entry_line_list, buffer );
	}
	if(DEBUGL1)Dump_line_list("Fix_receive_auth: info", info );
	return(s);
}

#if defined(HAVE_KRB5_H)
int Krb5_receive( int *sock, char *user, char *jobsize, int from_server,
	char *authtype, struct line_list *info,
	char *error, int errlen, struct line_list *header_info, char *tempfile )
{
	int status = 0;
	char *from = 0;
	char *keytab = 0;
	char *service = 0;

	error[0] = 0;
	DEBUGF(DRECV1)("Krb5_receive: starting, jobsize '%s'", jobsize );
	keytab = Find_str_value(info,"keytab",Value_sep);
	service = Find_str_value(info,"service",Value_sep);
	if( Write_fd_len( *sock, "", 1 ) < 0 ){
		status = JABORT;
		plp_snprintf( error, errlen, "Krb5_receive: ACK 0 write error - %s",
			Errormsg(errno) );
	} else if( (status = server_krb5_auth( keytab, service, *sock,
		&from, error, errlen, tempfile )) ){
		plp_snprintf( error, errlen, "Krb5_receive: receive error '%s'\n", error );
	} else {
		DEBUGF(DRECV1)("Krb5_receive: from '%s'", from );
		if( from ){
			Set_str_value( header_info, FROM, from );
		}
		status = Do_secure_work( 1, jobsize, from_server, tempfile, header_info );
		if( server_krb5_status( *sock, error, errlen, tempfile ) ){
			plp_snprintf( error, errlen, "Krb5_receive: status send failed - '%s'",
				error );
		}
	}
	if( error ){
		logmsg(LOG_INFO,"%s", error );
	}
	if( from ) free(from); from = 0;
	return(status);
}
#endif

int Pgp_receive( int *sock, char *user, char *jobsize, int from_server,
	char *authtype, struct line_list *info,
	char *error, int errlen, struct line_list *header_info, char *tempfile )
{
	char *pgpfile;
	int tempfd, status, n;
	char buffer[LARGEBUFFER];
	struct stat statb;
	struct line_list pgp_info;
	double len;
	char *id = Find_str_value( info, ID, Value_sep );
	char *from = 0;
	int pgp_exit_code = 0;
	int not_a_ciphertext = 0;

	Init_line_list(&pgp_info);
	tempfd = -1;
	error[0] = 0;

	pgpfile = safestrdup2(tempfile,".pgp",__FILE__,__LINE__);
	Check_max(&Tempfiles,1);
	Tempfiles.list[Tempfiles.count++] = pgpfile;

	if( id == 0 ){
		status = JABORT;
		plp_snprintf( error, errlen, "Pgp_receive: no pgp_id or auth_id value");
		goto error;
	}

	if( Write_fd_len( *sock, "", 1 ) < 0 ){
		status = JABORT;
		plp_snprintf( error, errlen, "Pgp_receive: ACK 0 write error - %s",
			Errormsg(errno) );
		goto error;
	}


	if( (tempfd = Checkwrite(pgpfile,&statb,O_WRONLY|O_TRUNC,1,0)) < 0 ){
		status = JFAIL;
		plp_snprintf( error, errlen,
			"Pgp_receive: reopen of '%s' for write failed - %s",
			pgpfile, Errormsg(errno) );
		goto error;
	}
	DEBUGF(DRECV4)("Pgp_receive: starting read from %d", *sock );
	while( (n = read(*sock, buffer,1)) > 0 ){
		/* handle old and new format of file */
		buffer[n] = 0;
		DEBUGF(DRECV4)("Pgp_receive: remote read '%d' '%s'", n, buffer );
		if( isdigit(cval(buffer)) ) continue;
		if( isspace(cval(buffer)) ) break;
		if( write( tempfd,buffer,1 ) != 1 ){
			status = JFAIL;
			plp_snprintf( error, errlen,
				"Pgp_receive: bad write to '%s' - '%s'",
				tempfile, Errormsg(errno) );
			goto error;
		}
		break;
	}
	while( (n = read(*sock, buffer,sizeof(buffer)-1)) > 0 ){
		buffer[n] = 0;
		DEBUGF(DRECV4)("Pgp_receive: remote read '%d' '%s'", n, buffer );
		if( write( tempfd,buffer,n ) != n ){
			status = JFAIL;
			plp_snprintf( error, errlen,
				"Pgp_receive: bad write to '%s' - '%s'",
				tempfile, Errormsg(errno) );
			goto error;
		}
	}
	if( n < 0 ){
		status = JFAIL;
		plp_snprintf( error, errlen,
			"Pgp_receive: bad read from socket - '%s'",
			Errormsg(errno) );
		goto error;
	}
	close(tempfd); tempfd = -1;
	DEBUGF(DRECV4)("Pgp_receive: end read" );

	status = Pgp_decode(info, tempfile, pgpfile, &pgp_info,
		buffer, sizeof(buffer), error, errlen, id, header_info,
		&pgp_exit_code, &not_a_ciphertext );
	if( status ) goto error;

	DEBUGFC(DRECV1)Dump_line_list("Pgp_receive: header_info", header_info );

	from = Find_str_value(header_info,FROM,Value_sep);
	if( from == 0 ){
		status = JFAIL;
		plp_snprintf( error, errlen,
			"Pgp_receive: no 'from' information" );
		goto error;
	}

	status = Do_secure_work( 0, jobsize, from_server, tempfile, header_info );

	Free_line_list( &pgp_info);
 	status = Pgp_encode(info, tempfile, pgpfile, &pgp_info,
		buffer, sizeof(buffer), error, errlen,
		id, from, &pgp_exit_code );
	if( status ) goto error;

	/* we now have the encoded output */
	if( (tempfd = Checkread(pgpfile,&statb)) < 0 ){
		status = JFAIL;
		plp_snprintf( error, errlen,
			"Pgp_receive: reopen of '%s' for read failed - %s",
			tempfile, Errormsg(errno) );
		goto error;
	}
	len = statb.st_size;
	DEBUGF(DRECV1)( "Pgp_receive: return status encoded size %0.0f",
		len);
	while( (n = read(tempfd, buffer,sizeof(buffer)-1)) > 0 ){
		buffer[n] = 0;
		DEBUGF(DRECV4)("Pgp_receive: sending '%d' '%s'", n, buffer );
		if( write( *sock,buffer,n ) != n ){
			status = JFAIL;
			plp_snprintf( error, errlen,
				"Pgp_receive: bad write to socket - '%s'",
				Errormsg(errno) );
			goto error;
		}
	}
	if( n < 0 ){
		status = JFAIL;
		plp_snprintf( error, errlen,
			"Pgp_receive: read '%s' failed - %s",
			tempfile, Errormsg(errno) );
		goto error;
	}

 error:
	if( tempfd>=0) close(tempfd); tempfd = -1;
	Free_line_list(&pgp_info);
	return(status);
}


int User_receive( int *sock, char *user, char *jobsize, int from_server,
	char *authtype, struct line_list *info,
	char *error, int errlen, struct line_list *header_info, char *tempfile )
{
	char *s, *t;
	int tempfd;
	char buffer[SMALLBUFFER];
	int pipe_fd[2], report_fd[2], error_fd[2], pid, error_pid;
	int status, n, len;
	struct line_list args, files, options;
	struct stat statb;
	char *receive_filter, *id, *esc_id;

	Init_line_list(&args);
	Init_line_list(&files);
	Init_line_list(&options);
	tempfd = -1;


	receive_filter = Find_str_value(info, "receive_filter", Value_sep );
	id = Find_str_value(info,ID,Value_sep);
	esc_id =  Escape(id,0,1);
	Set_str_value(info, ESC_ID, esc_id );
	free(esc_id);
	esc_id = Find_str_value(info,ESC_ID,Value_sep);

	status = pid = error_pid = 0;
	pipe_fd[0] = pipe_fd[1] = report_fd[0] = report_fd[1]
		= error_fd[0] = error_fd[1] = -1;

	plp_snprintf( buffer, sizeof(buffer),
		"%s -S -P%s -n%s -A%s -R%s -T%s",
		receive_filter, Printer_DYN, esc_id,
		authtype, user, tempfile );

	DEBUGF(DRECV1)( "User_receive: rcv authenticator '%s'", buffer);

	/* now set up the file descriptors:
	 *   FD Options Purpose
	 *   0  R/W     socket connection to remote host (R/W)
	 *   1  W       for status report about authentication
	 *   2  W       error log
	 *   3  R       for server status to be sent to client
	 */

	if( pipe(pipe_fd) == -1 || pipe(report_fd) == -1 || pipe(error_fd) == -1 ){
		Errorcode = JFAIL;
		logerr_die( LOG_INFO, _("User_receive: pipe failed") );
	}

	Free_line_list(&options);
	Set_str_value(&options,PRINTER,Printer_DYN);
	Set_str_value(&options,NAME,"RCVSEC_ERR");
	Set_str_value(&options,CALL,LOG);
	if( (error_pid = Start_worker(&options,error_fd[0])) < 0 ){
		Errorcode = JFAIL;
		logerr_die(LOG_INFO,"User_receive: fork failed");
	}
	Free_line_list(&options);

	files.count = 0;
	Check_max(&files,10);
	files.list[files.count++] = Cast_int_to_voidstar(*sock);
	files.list[files.count++] = Cast_int_to_voidstar(pipe_fd[1]);
	files.list[files.count++] = Cast_int_to_voidstar(error_fd[1]);
	files.list[files.count++] = Cast_int_to_voidstar(report_fd[0]);

	/* create the secure process */
	if((pid = Make_passthrough( buffer, 0, &files, 0, 0 )) < 0 ){
		Errorcode = JFAIL;
		logerr_die( LOG_INFO, _("User_receive: fork failed") );
	}
	files.count = 0;
	Free_line_list(&files);

	/* we now send all information to the authenticator */
	DEBUGF(DRECV1)("User_receive: report_fd %d dup to socket %d",
		report_fd[1], *sock );
	if( dup2( report_fd[1], *sock ) == -1 ){
		int err;
		err = errno;
		plp_snprintf( error, errlen,
			_("User_receive: dup of %d to %d failed - %s"),
			report_fd[1], *sock, Errormsg(err));
		status = JFAIL;
		goto error;
	}
	close( pipe_fd[1] ); pipe_fd[1] = -1;
	close( error_fd[1] ); error_fd[1] = -1;
	close( error_fd[0] ); error_fd[0] = -1;
	close( report_fd[0] ); report_fd[0] = -1;
	close( report_fd[1] ); report_fd[1] = -1;

	/* now we wait for the authentication info */

	n = 0;
	while( n < sizeof(buffer)-1
		&& (len = read( pipe_fd[0], buffer+n, sizeof(buffer)-1-n )) > 0 ){
		buffer[n+len] = 0;
		DEBUGF(DRECV1)("User_receive: read authentication '%s'", buffer );
		while( (s = safestrchr(buffer,'\n')) ){
			*s++ = 0;
			if( strlen(buffer) == 0 ){
				break;
			}
			if( (t = strchr(buffer,'=')) ){
				Unescape(t+1);
				Add_line_list(&options,buffer,Value_sep,1,1);
			}
			memmove(buffer,s,strlen(s)+1);
		}
	}
	close(pipe_fd[0]); pipe_fd[0] = -1;
	if( (tempfd = Checkread(tempfile, &statb) ) < 0 ){
		int err;
		err = errno;
		plp_snprintf( error, errlen,
			_("User_receive: reopen of '%s' failed - %s"),
			tempfile, Errormsg(err));
		status = JFAIL;
		goto error;
	}
	n = 0;
	while( n < sizeof(buffer)-1
		&& (len = read( tempfd, buffer+n, sizeof(buffer)-1-n )) > 0 ){
		buffer[n+len] = 0;
		DEBUGF(DRECV1)("User_receive: from file '%s'", buffer );
		while( (s = safestrchr(buffer,'\n')) ){
			*s++ = 0;
			if( strlen(buffer) == 0 ){
				break;
			}
			if( (t = strchr(buffer,'=')) ){
				Unescape(t+1);
				Add_line_list(&options,buffer,Value_sep,1,1);
			}
			memmove(buffer,s,strlen(s)+1);
		}
	}
	close(tempfd); tempfd = -1;

	DEBUGFC(DRECV1)Dump_line_list("User_receive - received", &options );

	if( (status = Check_secure_perms( &options, from_server, error, errlen )) ){
		DEBUGF(DRECV1)("User_receive: no permission");
		goto error;
	}

	/* now we do the dirty work */
	if( (s = Find_str_value(&options,INPUT,Value_sep)) ){
		DEBUGF(DRECV1)("User_receive: undecoded command '%s'", s );
		Dispatch_input( sock, s );
		status = 0;
		goto error;
	} else if( jobsize ){
		if( (tempfd = Checkread(tempfile, &statb) ) < 0 ){
			int err;
			err = errno;
			plp_snprintf( error, errlen,
				_("User_receive: reopen of '%s' failed - %s"),
				tempfile, Errormsg(err));
			status = JFAIL;
			goto error;
		}
		s = Find_str_value(header_info, CLIENT, Value_sep );
		if( (status = Scan_block_file( tempfd, error, errlen-4, s )) ){
			goto error;
		}
	}

 error:

	if( pipe_fd[1] >=0 )close( pipe_fd[1]  ); pipe_fd[1] = -1;
	if( pipe_fd[0] >=0 )close( pipe_fd[0]  ); pipe_fd[0] = -1;
	if( error_fd[1] >=0 )close( error_fd[1]  ); error_fd[1] = -1;
	if( error_fd[0] >=0 )close( error_fd[0]  ); error_fd[0] = -1;
	if( report_fd[1] >=0 )close( report_fd[1]  ); report_fd[1] = -1;
	if( report_fd[0] >=0 )close( report_fd[0]  ); report_fd[0] = -1;

	while( pid > 0 || error_pid >0 ){
		plp_status_t procstatus;
		memset(&procstatus,0,sizeof(procstatus));
		n = plp_waitpid(-1,&procstatus,0);
		if( n == -1 ){
			int err = errno;
			DEBUGF(DRECV1)("User_receive: waitpid(%d) returned %d, err '%s'",
				pid, n, Errormsg(err) );
			if( err == EINTR ) continue; 
			Errorcode = JABORT;
			logerr_die( LOG_ERR, "User_receive: waitpid(%d) failed", pid);
		}
		DEBUGF(DRECV1)("User_receive: pid %d, exit status '%s'",
			n, Decode_status(&procstatus) );
		if( n == pid ) pid = 0;
		else if( n == error_pid ) error_pid = 0;
	}
	Free_line_list(&args);
	Free_line_list(&files);
	Free_line_list(&options);

	return( status );
}


int Check_secure_perms( struct line_list *options, int from_server,
	char *error, int errlen )
{
	/*
	 * line 1 - CLIENT=xxxx   - client authentication
	 * line 2 - SERVER=xxxx   - server authentication
	 * ...    - FROM=xxxx     - from
	 * line 3 - INPUT=\00x  - command line
	 */
	char *received_id, *client_id;
	received_id = Find_str_value(options,FROM,Value_sep);
	client_id = Find_str_value(options,CLIENT,Value_sep);
	if( !from_server ){
		if( !client_id ){
			client_id = received_id;
		}
		if( !received_id ){
			received_id = client_id;
		}
	}
	Perm_check.auth_from_id = received_id;
	Perm_check.auth_client_id = client_id;
	if( !client_id ){
		plp_snprintf( error, errlen, "Printer %s@%s: missing authentication client id",
			Printer_DYN,Report_server_as_DYN?Report_server_as_DYN:ShortHost_FQDN );
		return( JABORT );
	}
	return(0);
}

/***************************************************************************
 * Commentary:
 * MIT Athena extension  --mwhitson@mit.edu 12/2/98
 * 
 * The protocol used to send a krb4 authenticator consists of:
 * 
 * Client                                   Server
 * kprintername\n - receive authenticator
 *                                          \0  (ack)
 * <krb4_credentials>
 *                                          \0
 * 
 * The server will note validity of the credentials for future service
 * requests in this session.  No capacity for TCP stream encryption or
 * verification is provided.
 * 
 ***************************************************************************/

/* A bunch of this has been ripped from the Athena lpd, and, as such,
 * isn't as nice as Patrick's code.  I'll clean it up eventually.
 *   -mwhitson
 *   Run it through the drunk tank and detox center,
 *      and cleaned the dog's teeth.
 *     -papowell ("sigh...") Powell
 */

#if defined(HAVE_KRB_H) && defined(MIT_KERBEROS4)
# include <krb.h>
# include <des.h>
#if !defined(HAVE_KRB_AUTH_DEF)
 extern int krb_recvauth(
	long options,            /* bit-pattern of options */
	int fd,              /* file descr. to read from */
	KTEXT ticket,            /* storage for client's ticket */
	char *service,           /* service expected */
	char *instance,          /* inst expected (may be filled in) */
	struct sockaddr_in *faddr,   /* address of foreign host on fd */
	struct sockaddr_in *laddr,   /* local address */
	AUTH_DAT *kdata,         /* kerberos data (returned) */
	char *filename,          /* name of file with service keys */
	Key_schedule schedule,       /* key schedule (return) */
	char *version);           /* version string (filled in) */
#endif

#endif

int Receive_k4auth( int *sock, char *input )
{
	int status = 0;
	char error_msg[LINEBUFFER];
	char cmd[LINEBUFFER];
	struct line_list values;
	int ack = ACK_SUCCESS;
	int k4error=0;

#if defined(HAVE_KRB_H) && defined(MIT_KERBEROS4)

	int sin_len = sizeof(struct sockaddr_in);
	struct sockaddr_in faddr;
	int len;
	uid_t euid;
	KTEXT_ST k4ticket;
	AUTH_DAT k4data;
	char k4principal[ANAME_SZ];
	char k4instance[INST_SZ];
	char k4realm[REALM_SZ];
	char k4version[9];
	char k4name[ANAME_SZ + INST_SZ + REALM_SZ + 3];

	Init_line_list(&values);
	error_msg[0] = '\0';
	DEBUG1("Receive_k4auth: doing '%s'", ++input);

	k4principal[0] = k4realm[0] = '\0';
	memset(&k4ticket, 0, sizeof(k4ticket));
	memset(&k4data, 0, sizeof(k4data));
	if (getpeername(*sock, (struct sockaddr *) &faddr, &sin_len) <0) {
	  	status = JFAIL;
		plp_snprintf( error_msg, sizeof(error_msg), 
			      "Receive_k4auth: couldn't get peername" );
		goto error;
	}
    DEBUG1("Receive_k4auth: remote host IP '%s'",
        inet_ntoa( faddr.sin_addr ) );
	status = Link_send( ShortRemote_FQDN, sock,
			Send_query_rw_timeout_DYN,"",1,0 );
	if( status ){
		ack = ACK_FAIL;
		plp_snprintf( error_msg, sizeof(error_msg),
			      "Receive_k4auth: sending ACK 0 failed" );
		goto error;
	}
	strcpy(k4instance, "*");
	euid = geteuid();
	if( Is_server ) To_root();  /* Need root here to read srvtab */
	k4error = krb_recvauth(0, *sock, &k4ticket, KLPR_SERVICE,
			      k4instance,
			      &faddr,
			      (struct sockaddr_in *)NULL,
			      &k4data, "", NULL,
			      k4version);
	if( Is_server ) (void)To_daemon();
	DEBUG1("Receive_k4auth: krb_recvauth returned %d, '%s'",
		k4error, krb_err_txt[k4error] );
	if (k4error != KSUCCESS) {
		/* erroring out here if the auth failed. */
	  	status = JFAIL;
		plp_snprintf( error_msg, sizeof(error_msg),
		    "kerberos 4 receive authentication failed - '%s'",
			krb_err_txt[k4error] );
	  	goto error;
	}

	strncpy(k4principal, k4data.pname, ANAME_SZ);
	strncpy(k4instance, k4data.pinst, INST_SZ);
	strncpy(k4realm, k4data.prealm, REALM_SZ);

	/* Okay, we got auth.  Note it. */

	if (k4instance[0]) {
		plp_snprintf( k4name, sizeof(k4name), "%s.%s@%s", k4principal,
			      k4instance, k4realm );
	} else {
		plp_snprintf( k4name, sizeof(k4name), "%s@%s", k4principal, k4realm );
	}
	DEBUG1("Receive_k4auth: auth for %s", k4name);
	Perm_check.authtype = "kerberos4";
	Set_str_value(&values,FROM,k4name);
	/* we will only use this for client to server authentication */
	if( (status = Check_secure_perms( &values, 0, error_msg, sizeof(error_msg)) )){
		DEBUGF(DRECV1)("Receive_k4auth: Check_secure_perms failed - '%s'",
			error_msg );
		goto error;
	}

	/* ACK the credentials  */
	status = Link_send( ShortRemote_FQDN, sock,
			Send_query_rw_timeout_DYN,"",1,0 );
	if( status ){
		ack = ACK_FAIL;
		plp_snprintf(error_msg, sizeof(error_msg),
			     "Receive_k4auth: sending ACK 0 failed" );
		goto error;
	}
	len = sizeof(cmd)-1;
    status = Link_line_read(ShortRemote_FQDN,sock,
        Send_job_rw_timeout_DYN,cmd,&len);
	if( len >= 0 ) cmd[len] = 0;
    DEBUG1( "Receive_k4auth: read status %d, len %d, '%s'",
        status, len, cmd );
    if( len == 0 ){
        DEBUG3( "Receive_k4auth: zero length read" );
		cleanup(0);
    }
    if( status ){
        logerr_die( LOG_DEBUG, "Service_connection: cannot read request" );
    }
    if( len < 3 ){
        fatal( LOG_INFO, "Service_connection: bad request line '%s'", cmd );
    }
	Free_line_list(&values);
    Dispatch_input(sock,cmd);

	status = ack = 0;

#else
	/* not supported */
	Init_line_list(&values);
	ack = ACK_FAIL;
	plp_snprintf(error_msg,sizeof(error_msg),
		"kerberos 4 not supported");
	goto error;
#endif

 error:	
	DEBUG1("Receive_k4auth: error - status %d, ack %d, k4error %d, error '%s'",
		status, ack, k4error, error_msg );
	if( status || ack ){
		cmd[0] = ack;
		if(ack) Link_send( ShortRemote_FQDN, sock,
			Send_query_rw_timeout_DYN, cmd, 1, 0 );
		if( status == 0 ) status = JFAIL;
		/* shut down reception from the remote file */
		if( error_msg[0] ){
			safestrncat( error_msg, "\n" );
			Write_fd_str( *sock, error_msg );
		}
	}

	DEBUG1("Receive_k4auth: done");
	Free_line_list(&values);
	return(status);
}

#define RECEIVE 1
#include "user_auth.stub"

 struct security ReceiveSecuritySupported[] = {
	/* name, config_tag, connect, send, receive */
#if defined(HAVE_KRB_H) && defined(MIT_KERBEROS4)
	{ "kerberos4", "kerberos",  0, 0, 0 },
#endif
#if defined(HAVE_KRB5_H)
	{ "kerberos*", "kerberos",   0, 0, Krb5_receive },
#endif
	{ "pgp",       "pgp",   0, 0, Pgp_receive, },
	{ "user", "user",       0, 0, User_receive },
#if defined(USER_RECEIVE)
/* this should have the form of the entries above */
 USER_RECEIVE
#endif
	{0}
};
