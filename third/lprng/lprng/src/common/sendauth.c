/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-2000, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 *
 ***************************************************************************/

 static char *const _id =
"$Id: sendauth.c,v 1.1.1.4 2000-03-31 15:48:04 mwhitson Exp $";

#include "lp.h"
#include "lpd.h"
#include "sendauth.h"
#include "sendjob.h"
#include "globmatch.h"
#include "permission.h"
#include "getqueue.h"
#include "errorcodes.h"
#include "linksupport.h"
#include "krb5_auth.h"
#include "fileopen.h"
#include "child.h"
#if 0
#include "gethostinfo.h"
#endif
/**** ENDINCLUDE ****/

/***************************************************************************
 * Commentary:
 * Patrick Powell Mon Apr 17 05:43:48 PDT 1995
 * 
 * The protocol used to send a secure job consists of the following
 * following:
 * 
 * Client                                   Server
 * \REQ_SECUREprintername C/F user\n - receive a command
 *             0           1   2
 * \REQ_SECUREprintername C/F user controlfile\n - receive a job
 *             0           1   2
 *          
 * 1. Get a temporary file
 * 2. Generate the compressed data files - this has the format
 *      Authentication
 *      \n
 *      \3count cfname\n
 *      [count control file bytes]
 *      \4count dfname\n
 *      [count data file bytes]
 *
 * 3. send the \REQ_SECRemotePrinter_DYN user@RemoteHost_DYN file size\n
 *    string to the remote RemoteHost_DYN, wait for an ACK
 *
 * 4. send the compressed data files - this has the format
 *      wait for an ACK
 ***************************************************************************/

/*
 * Send_auth_transfer
 *  1. we send the command line and wait for ACK of 0
 *  \REQ_SEQUREprinter C/F sender_id authtype [jobsize]
 *  2. if authtype == kerberos we do kerberos
 *      - send a file to the remote end
 *      - get back a file
 *  3. if authtype == pgp we do pgp
 *      - same as kerberos
 *  3. if otherwise,  we start a process with command line options
 *       fd 0 -  sock
 *       fd 1 -  for reports
 *       fd 2 -  for errors
 *    /filter -C -P printer -n sender_id -A authtype -R remote_id -Ttempfile
 *    The tempfile will be sent to the remote end and status
 *     written back on fd 2
 *     - we save this information
 *     - reopen the file and put error messages in it.
 *  RETURN:
 *     0 - no error
 *     !=0 - error
 */

int Send_auth_transfer( int *sock, int transfer_timeout,
	struct job *job, struct job *logjob, char *error, int errlen, char *cmd,
	struct security *security, struct line_list *info )
{
	struct stat statb;
	int ack, len, n, fd;		/* ACME! The best... */
	int status = JFAIL;			/* job status */
	char *secure, *destination, *from, *client, *s;
	char *tempfile;
	char buffer[SMALLBUFFER];
	errno = 0;

	secure = 0;
	fd = Make_temp_fd(&tempfile);

	if( cmd && (s = safestrrchr(cmd,'\n')) ) *s = 0;
	DEBUG1("Send_auth_transfer: cmd '%s'", cmd );

	if(DEBUGL1)Dump_line_list("Send_auth_transfer: info ", info );

	destination = Find_str_value(info, DESTINATION, Value_sep );
	from = Find_str_value(info, FROM, Value_sep );
	client = Find_str_value(info, CLIENT, Value_sep );

	if( safestrcmp(security->config_tag, "kerberos") ){
		Put_in_auth(fd,DESTINATION,destination);
		if( Is_server ) Put_in_auth(fd,SERVER,from);
		Put_in_auth(fd,CLIENT,client);
		if( cmd ){
			Put_in_auth(fd,INPUT,cmd);
		}
	} else {
		if( cmd && (Write_fd_str(fd,cmd) < 0 || Write_fd_str(fd,"\n") < 0) ){
			plp_snprintf(error, errlen, "Send_auth_transfer: '%s' write failed - %s",
				tempfile, Errormsg(errno) );
			goto error;
		}
		if( Is_server && (Write_fd_str(fd,client) < 0 || Write_fd_str(fd,"\n") < 0) ){
			plp_snprintf(error, errlen, "Send_auth_transfer: '%s' write failed - %s",
				tempfile, Errormsg(errno) );
			goto error;
		}
	}

	if( Write_fd_str(fd,"\n") < 0 ){
		plp_snprintf(error, errlen, "Send_auth_transfer: '%s' write failed - %s",
			tempfile, Errormsg(errno) );
		goto error;
	}

	s = Find_str_value(info, CMD, Value_sep );
	if( job ){
        status = Send_normal( &fd, job, logjob, transfer_timeout, fd);
        if( status ) return( status );
		errno = 0;
		if( stat(tempfile,&statb) ){
			Errorcode = JABORT;
			logerr_die( LOG_INFO,"Send_auth_transfer: stat '%s' failed",
				tempfile);
		}
		plp_snprintf( buffer,sizeof(buffer)," %0.0f",(double)(statb.st_size) );
		secure = safestrdup3(s,buffer,"\n",__FILE__,__LINE__);
	} else {
		secure = safestrdup2(s,"\n",__FILE__,__LINE__);
	}
	close( fd ); fd = -1;

	/* send the message */
	DEBUG3("Send_auth_transfer: sending '%s'", secure );
	status = Link_send( RemoteHost_DYN, sock, transfer_timeout,
		secure, strlen(secure), &ack );
	DEBUG3("Send_auth_transfer: status '%s'", Link_err_str(status) );
	if( status ){
		/* open output file */
		if( (fd = Checkwrite(tempfile,&statb,O_WRONLY|O_TRUNC,1,0)) < 0){
			Errorcode = JABORT;
			logerr_die( LOG_INFO, "Send_auth_transfer: open '%s' for write failed",
				tempfile);
		}
		/* we turn off IO from the socket */
		shutdown(*sock,1);
		if( (s = safestrchr(buffer,'\n')) ) *s = 0;
		plp_snprintf( error, errlen,
			"error '%s' sending '%s' to %s@%s\n",
			Link_err_str(status), buffer, RemotePrinter_DYN, RemoteHost_DYN );
		Write_fd_str( fd, error );
		error[0] = 0;
		DEBUG2("Send_auth_transfer: starting read");
		len = 0;
		while( (n = read(*sock,buffer+len,sizeof(buffer)-1-len)) > 0 ){
			buffer[n+len] = 0;
			DEBUG4("Send_auth_transfer: read '%s'", buffer);
			while( (s = strchr(buffer,'\n')) ){
				*s++ = 0;
				DEBUG2("Send_auth_transfer: doing '%s'", buffer);
				plp_snprintf(error,errlen,"%s\n", buffer );
				if( Write_fd_str(fd,error) < 0 ){
					Errorcode = JABORT;
					logerr( LOG_INFO, "Send_auth_transfer: write '%s' failed",
						tempfile );
					goto error;
				}
				memmove(buffer,s,strlen(s)+1);
			}
			len = strlen(buffer);
		}
		if( buffer[0] ){
			DEBUG2("Send_auth_transfer: doing '%s'", buffer);
			plp_snprintf(error,errlen,"%s\n", buffer );
			if( Write_fd_str(fd,error) < 0 ){
				Errorcode = JABORT;
				logerr( LOG_INFO, "Send_auth_transfer: write '%s' failed",
					tempfile );
				goto error;
			}
		}

		close( fd ); fd = -1;
		error[0] = 0;
		goto error;
	}

	/*
     * now we do the protocol dependent exchange
     */

	status = security->send( sock, transfer_timeout, tempfile,
		error, errlen, security, info );

 error:

	/* we are going to put the returned error status in the temp file
	 * as the device to read from
	 */
	if( secure ) free(secure); secure = 0;
	if( error[0] ){
		if( job ){
			setstatus(logjob,"Send_auth_transfer: %s", error );
			Set_str_value(&job->info,ERROR,error);
		}
		if( (fd = Checkwrite(tempfile,&statb,O_WRONLY|O_TRUNC,1,0)) < 0){
			Errorcode = JFAIL;
			logerr_die(LOG_INFO,"Send_auth_transfer: cannot open '%s'", tempfile );
		}
		Write_fd_str(fd,error);
		close( fd ); fd = -1;
		error[0] = 0;
	}
	if( *sock >= 0 ){
		if( (fd = Checkread(tempfile,&statb)) < 0 ){
			Errorcode = JFAIL;
			logerr_die(LOG_INFO,"Send_auth_transfer: cannot open '%s'", tempfile );
		}
		if( dup2( fd, *sock ) == -1 ){
			Errorcode = JFAIL;
			logerr_die(LOG_INFO,"Send_auth_transfer: dup2(%d,%d)", fd, *sock );
		}
		if( fd != *sock ) close(fd); fd = -1;
	}
	Free_line_list(info);
	DEBUG3("Send_auth_transfer: exit status %d, error '%s'",
		status, error );
	return( status );
}


/*
 * 
 * The following routines simply implement the encryption and transfer of
 * the files and/or values
 * 
 * By default, when sending a command,  the file will contain:
 *   key=value lines.
 *   KEY           PURPOSE
 *   client        client or user name
 *   from          originator - server if forwarding, client otherwise
 *   command       command to send
 * 
 */

#if defined(HAVE_KRB5_H)

int Krb5_send( int *sock, int transfer_timeout, char *tempfile,
	char *error, int errlen,
	struct security *security, struct line_list *info )
{
	char *keyfile = 0;
	int status = 0, fd = -1;
	struct stat statb;
	char *principal = 0;
	char *service = 0;
	char *life,  *renew;

	DEBUG1("Krb5_send: tempfile '%s'", tempfile );
	life = renew = 0;
	if( Is_server ){
		if( !(keyfile = Find_str_value(info,"keytab",Value_sep)) ){
			plp_snprintf( error, errlen, "no server keytab file" );
			status = JFAIL;
			goto error;
		}
		DEBUG1("Krb5_send: keyfile '%s'", keyfile );
		if( (fd = Checkread(keyfile,&statb)) == -1 ){
			plp_snprintf( error, errlen,
				"cannot open server keytab file - %s",
				Errormsg(errno) );
			status = JFAIL;
			goto error;
		}
		close(fd);
		if( !(principal = Find_str_value(info,"forward_principal",Value_sep))){
			plp_snprintf( error, errlen, "no server keytab file" );
			status = JFAIL;
			goto error;
		}
	} else {
		if( !(principal = Find_str_value(info,"server_principal",Value_sep))
		 && !(principal = Find_str_value(info,"id",Value_sep)) ){
			plp_snprintf( error, errlen, "no server keytab file" );
			status = JFAIL;
			goto error;
		}
	}
	service = Find_str_value(info, "service", Value_sep );
	life = Find_str_value(info, "life", Value_sep );
	renew = Find_str_value(info, "renew", Value_sep );
	status= client_krb5_auth( keyfile, service,
		RemoteHost_DYN, /* remote host */
		principal,	/* principle name of the remote server */
		0,	/* options */
		life,	/* lifetime of server ticket */
		renew,	/* renewable time of server ticket */
		*sock, error, errlen, tempfile );
	DEBUG1("Krb5_send: client_krb5_auth returned '%d' - error '%s'",
		status, error );
	if( status && error[0] == 0 ){
		plp_snprintf( error, errlen,
		"pgp authenticated transfer to remote host failed");
	}
	if( error[0] ){
		DEBUG2("Krb5_send: writing error to file '%s'", error );
		if( strlen(error) < errlen-2 ){
			memmove( error+1, error, strlen(error)+1 );
			error[0] = ' ';
		}
		if( (fd = Checkwrite(tempfile,&statb,O_WRONLY|O_TRUNC,1,0)) < 0){
			plp_snprintf(error,errlen,
			"Krb5_send: open '%s' for write failed - %s",
				tempfile, Errormsg(errno));
		}
		Write_fd_str(fd,error);
		close( fd ); fd = -1;
		error[0] = 0;
	}
  error:
	return(status);
}
#endif

/*************************************************************
 * PGP Transmission
 * 
 * Configuration:
 *   pgp_id            for client to server
 *   pgp_forward_id    for server to server
 *   pgp_forward_id    for server to server
 *   pgp_path          path to pgp program
 *   pgp_passphrasefile     user passphrase file (relative to $HOME/.pgp)
 *   pgp_server_passphrasefile server passphrase file
 * User ENVIRONMENT Variables
 *   PGPPASS           - passphrase
 *   PGPPASSFD         - passfd if set up
 *   PGPPASSFILE       - passphrase in this file
 *   HOME              - for passphrase relative to thie file
 * 
 *  We encrypt and sign the file,  then send it to the other end.
 *  It will decrypt it, and then send the data back, encrypted with
 *  our public key.
 * 
 *  Keyrings must contain keys for users.
 *************************************************************/

int Pgp_send( int *sock, int transfer_timeout, char *tempfile,
	char *error, int errlen,
	struct security *security, struct line_list *info )
{
	char *pgpfile;
	struct line_list pgp_info;
	char buffer[LARGEBUFFER];
	int status, i, tempfd, len, n, fd;
	struct stat statb;
	char *from, *destination, *s, *t;
	int pgp_exit_code = 0;
	int not_a_ciphertext = 0;

	DEBUG1("Pgp_send: sending on socket %d", *sock );

	len = 0;
	error[0] = 0;
	from = Find_str_value( info, FROM, Value_sep);
	destination = Find_str_value( info, ID, Value_sep );

	tempfd = -1;

	Init_line_list( &pgp_info );
    pgpfile = safestrdup2(tempfile,".pgp",__FILE__,__LINE__); 
    Check_max(&Tempfiles,1);
    Tempfiles.list[Tempfiles.count++] = pgpfile;

	status = Pgp_encode( info, tempfile, pgpfile, &pgp_info,
		buffer, sizeof(buffer), error, errlen, 
        from, destination, &pgp_exit_code );

	if( status ){
		goto error;
	}
	if( !Is_server && Verbose ){
		for( i = 0; i < pgp_info.count; ++i ){
			if( Write_fd_str(1,pgp_info.list[i]) < 0
				|| Write_fd_str(1,"\n") < 0 ) cleanup(0);
		}
	}
	Free_line_list(&pgp_info);

	if( (tempfd = Checkread(pgpfile,&statb)) < 0 ){
		plp_snprintf(error,errlen,
			"Pgp_send: cannot open '%s' - %s", pgpfile, Errormsg(errno) );
		goto error;
	}

	DEBUG1("Pgp_send: encrypted file size '%0.0f'", (double)(statb.st_size) );
	plp_snprintf(buffer,sizeof(buffer),"%0.0f\n",(double)(statb.st_size) );
	Write_fd_str(*sock,buffer);

	while( (len = read( tempfd, buffer, sizeof(buffer)-1 )) > 0 ){
		buffer[len] = 0;
		DEBUG4("Pgp_send: file information '%s'", buffer );
		if( write( *sock, buffer, len) != len ){
			plp_snprintf(error,errlen,
			"Pgp_send: write to socket failed - %s", Errormsg(errno) );
			goto error;
		}
	}

	DEBUG2("Pgp_send: sent file" );
	close(tempfd); tempfd = -1;
	/* we close the writing side */
	shutdown( *sock, 1 );
	if( (tempfd = Checkwrite(pgpfile,&statb,O_WRONLY|O_TRUNC,1,0)) < 0){
		plp_snprintf(error,errlen,
			"Pgp_send: open '%s' for write failed - %s", pgpfile, Errormsg(errno));
		goto error;
	}
	DEBUG2("Pgp_send: starting read");
	len = 0;
	while( (n = read(*sock,buffer,sizeof(buffer)-1)) > 0 ){
		buffer[n] = 0;
		DEBUG4("Pgp_send: read '%s'", buffer);
		if( write(tempfd,buffer,n) != n ){
			plp_snprintf(error,errlen,
			"Pgp_send: write '%s' failed - %s", tempfile, Errormsg(errno) );
			goto error;
		}
		len += n;
	}
	close( tempfd ); tempfd = -1;

	DEBUG2("Pgp_send: total %d bytes status read", len );

	Free_line_list(&pgp_info);

	/* decode the PGP file into the tempfile */
	if( len ){
		status = Pgp_decode( info, tempfile, pgpfile, &pgp_info,
			buffer, sizeof(buffer), error, errlen, from, info,
			&pgp_exit_code, &not_a_ciphertext );
		if( not_a_ciphertext ){
			DEBUG2("Pgp_send: not a ciphertext" );
			if( (tempfd = Checkwrite(tempfile,&statb,O_WRONLY|O_TRUNC,1,0)) < 0){
				plp_snprintf(error,errlen,
				"Pgp_send: open '%s' for write failed - %s",
					tempfile, Errormsg(errno));
			}
			if( (fd = Checkread(pgpfile,&statb)) < 0){
				plp_snprintf(error,errlen,
				"Pgp_send: open '%s' for write failed - %s",
					pgpfile, Errormsg(errno));
			}
			if( error[0] ){
				Write_fd_str(tempfd,error);
				Write_fd_str(tempfd,"\n Contents -\n");
			}
			error[0] = 0;
			len = 0;
			while( (n = read(fd, buffer+len, sizeof(buffer)-len-1)) > 0 ){
				DEBUG2("Pgp_send: read '%s'", buffer );
				while( (s = strchr( buffer, '\n')) ){
					*s++ = 0;
					for( t = buffer; *t; ++t ){
						if( !isprint(cval(t)) ) *t = ' ';
					}
					plp_snprintf(error,errlen,"  %s\n", buffer);
					Write_fd_str(tempfd, error );
					DEBUG2("Pgp_send: wrote '%s'", error );
					memmove(buffer,s,strlen(s)+1);
				}
				len = strlen(buffer);
			}
			DEBUG2("Pgp_send: done" );
			error[0] = 0;
			close(fd); fd = -1;
			close(tempfd); tempfd = -1;
			error[0] = 0;
		}
	}

 error:
	if( error[0] ){
		DEBUG2("Pgp_send: writing error to file '%s'", error );
		if( (tempfd = Checkwrite(tempfile,&statb,O_WRONLY|O_TRUNC,1,0)) < 0){
			plp_snprintf(error,errlen,
			"Pgp_send: open '%s' for write failed - %s",
				tempfile, Errormsg(errno));
		}
		strncpy( buffer, error, sizeof(buffer) -1 );
		buffer[sizeof(buffer)-1 ] = 0;
		while( (s = strchr( buffer, '\n')) ){
			*s++ = 0;
			for( t = buffer; *t; ++t ){
				if( !isprint(cval(t)) ) *t = ' ';
			}
			plp_snprintf(error,errlen,"  %s\n", buffer);
			Write_fd_str(tempfd, error );
			DEBUG2("Pgp_send: wrote '%s'", error );
			memmove(buffer,s,strlen(s)+1);
		}
		close( tempfd ); tempfd = -1;
		error[0] = 0;
	}
	Free_line_list(&pgp_info);
	return(status);
}

int User_send( int *sock, int transfer_timeout, char *tempfile,
	char *error, int errlen,
	struct security *security, struct line_list *info )
{
	char tempbuf[SMALLBUFFER];
	int pid, fd, status;
	int pipe_fd[2], error_fd[2];
	struct line_list args, files;
	char *filter, *id, *esc_id, *from, *esc_from;

	Init_line_list(&args);
	Init_line_list(&files);
	pid = 0;
	fd = pipe_fd[0] = pipe_fd[1] = error_fd[0] = error_fd[1] = -1;
	status = 1;

	filter = Find_str_value(info,"client_filter",Value_sep);
	id = Find_str_value(info,ID,Value_sep);
	esc_id =  Escape(id,0,1);
	Set_str_value(info, ESC_ID, esc_id );
	free(esc_id);
	esc_id = Find_str_value(info,ESC_ID,Value_sep);

	from = Find_str_value(info,FROM,Value_sep);
	esc_from =  Escape(from,0,1);
	Set_str_value(info, "esc_from", esc_from );
	free(esc_from);
	esc_from = Find_str_value(info,"esc_from",Value_sep);

	/* now we use the user specified method */
	DEBUG1("User_send: starting '%s' authentication", Auth_DYN );
	plp_snprintf( tempbuf, sizeof(tempbuf),
		"%s -C -P%s -n$%%%s -A$%%%s -R$%%%s -T%s",
		filter,RemotePrinter_DYN,esc_from,
		Auth_DYN, esc_id, tempfile );
	DEBUG3("User_send: '%s'", tempbuf );

	/* now set up the file descriptors:
	 *   FD  Options Purpose
	 *    0  R/W     sock connection to remote host (R/W)
	 *    1  W       pipe or file descriptor,  for responses to client programs
	 *    2  W       error log
	 */

	if( pipe(pipe_fd) <  0 ){
		plp_snprintf( error, errlen,
			"User_send: pipe failed" );
		goto error;
	}

	if( Is_server ){
		if( pipe(error_fd) <  0 ){
			logerr_die( LOG_INFO,
				"User_send: pipe failed - '%s'", Errormsg(errno) );
			goto error;
		}
		Free_line_list(&args);
		Set_str_value(&args,NAME,"SEND_FILTER");
		Set_str_value(&args,CALL,LOG);
		if( (pid = Start_worker(&args,error_fd[0])) < 0 ){
			Errorcode = JFAIL;
			logerr_die( LOG_INFO,
				"User_send: could not create SEND_FILTER error logging process");
		}
		Free_line_list(&args);
		close(error_fd[0]); error_fd[0] = -1;
		fd = error_fd[1];
	} else {
		fd = 2;
	}

	Free_line_list(&files);
	Check_max(&files,10);
	files.list[files.count++] = Cast_int_to_voidstar(*sock);
	files.list[files.count++] = Cast_int_to_voidstar(pipe_fd[1]);
	files.list[files.count++] = Cast_int_to_voidstar(fd);
	if( (pid = Make_passthrough( tempbuf, 0, &files, 0, 0 )) < 0){
		plp_snprintf( error, errlen,
			"User_send: could not execute '%s'", filter );
		goto error;
	}
	files.count = 0;
	Free_line_list(&files);

	/* now we wait for the status */
	DEBUG3("User_send: sock %d, pipe_fd %d", *sock, pipe_fd[0]);
	if( dup2( pipe_fd[0], *sock ) < 0 ){
		Errorcode = JFAIL;
		plp_snprintf( error, errlen,
			"User_send: dup2 failed" );
		goto error;
	}
	status = 0;

 error:
	if( pipe_fd[0] >= 0 ) close(pipe_fd[0]); pipe_fd[0] = -1;
	if( pipe_fd[1] >= 0 ) close(pipe_fd[1]); pipe_fd[1] = -1;
	if( error_fd[0] >= 0 ) close(error_fd[0]); error_fd[0] = -1;
	if( error_fd[1] >= 0 ) close(error_fd[1]); error_fd[1] = -1;
	DEBUG3("User_send: exit status %d", status );
	Free_line_list(&args);
	Free_line_list(&files);
	return( status );
}

#if defined(HAVE_KRB_H) && defined(MIT_KERBEROS4)
# include <krb.h>
# include <des.h>

#if !defined(HAVE_KRB_AUTH_DEF)
 extern int  krb_sendauth(
  long options,            /* bit-pattern of options */
  int fd,              /* file descriptor to write onto */
  KTEXT ticket,   /* where to put ticket (return) or supplied in case of KOPT_DONT_MK_REQ */
  char *service, char *inst, char *realm,    /* service name, instance, realm */
  u_long checksum,         /* checksum to include in request */
  MSG_DAT *msg_data,       /* mutual auth MSG_DAT (return) */
  CREDENTIALS *cred,       /* credentials (return) */
  Key_schedule schedule,       /* key schedule (return) */
  struct sockaddr_in *laddr,   /* local address */
  struct sockaddr_in *faddr,   /* address of foreign host on fd */
  char *version);           /* version string */
 extern char* krb_realmofhost( char *host );
#endif


 struct krberr{
	int err; char *code;
 } krb4_errs[] = {
	{1, "KDC_NAME_EXP 'Principal expired'"},
	{2, "KDC_SERVICE_EXP 'Service expired'"},
	{2, "K_LOCK_EX 'Exclusive lock'"},
	{3, "KDC_AUTH_EXP 'Auth expired'"},
	{4, "CLIENT_KRB_TIMEOUT 'time between retries'"},
	{4, "KDC_PKT_VER 'Protocol version unknown'"},
	{5, "KDC_P_MKEY_VER 'Wrong master key version'"},
	{6, "KDC_S_MKEY_VER 'Wrong master key version'"},
	{7, "KDC_BYTE_ORDER 'Byte order unknown'"},
	{8, "KDC_PR_UNKNOWN 'Principal unknown'"},
	{9, "KDC_PR_N_UNIQUE 'Principal not unique'"},
	{10, "KDC_NULL_KEY 'Principal has null key'"},
	{20, "KDC_GEN_ERR 'Generic error from KDC'"},
	{21, "GC_TKFIL 'Can't read ticket file'"},
	{21, "RET_TKFIL 'Can't read ticket file'"},
	{22, "GC_NOTKT 'Can't find ticket or TGT'"},
	{22, "RET_NOTKT 'Can't find ticket or TGT'"},
	{26, "DATE_SZ 'RTI date output'"},
	{26, "MK_AP_TGTEXP 'TGT Expired'"},
	{31, "RD_AP_UNDEC 'Can't decode authenticator'"},
	{32, "RD_AP_EXP 'Ticket expired'"},
	{33, "RD_AP_NYV 'Ticket not yet valid'"},
	{34, "RD_AP_REPEAT 'Repeated request'"},
	{35, "RD_AP_NOT_US 'The ticket isn't for us'"},
	{36, "RD_AP_INCON 'Request is inconsistent'"},
	{37, "RD_AP_TIME 'delta_t too big'"},
	{38, "RD_AP_BADD 'Incorrect net address'"},
	{39, "RD_AP_VERSION 'protocol version mismatch'"},
	{40, "RD_AP_MSG_TYPE 'invalid msg type'"},
	{41, "RD_AP_MODIFIED 'message stream modified'"},
	{42, "RD_AP_ORDER 'message out of order'"},
	{43, "RD_AP_UNAUTHOR 'unauthorized request'"},
	{51, "GT_PW_NULL 'Current PW is null'"},
	{52, "GT_PW_BADPW 'Incorrect current password'"},
	{53, "GT_PW_PROT 'Protocol Error'"},
	{54, "GT_PW_KDCERR 'Error returned by KDC'"},
	{55, "GT_PW_NULLTKT 'Null tkt returned by KDC'"},
	{56, "SKDC_RETRY 'Retry count exceeded'"},
	{57, "SKDC_CANT 'Can't send request'"},
	{61, "INTK_W_NOTALL 'Not ALL tickets returned'"},
	{62, "INTK_BADPW 'Incorrect password'"},
	{63, "INTK_PROT 'Protocol Error'"},
	{70, "INTK_ERR 'Other error'"},
	{71, "AD_NOTGT 'Don't have tgt'"},
	{72, "AD_INTR_RLM_NOTGT 'Can't get inter-realm tgt'"},
	{76, "NO_TKT_FIL 'No ticket file found'"},
	{77, "TKT_FIL_ACC 'Couldn't access tkt file'"},
	{78, "TKT_FIL_LCK 'Couldn't lock ticket file'"},
	{79, "TKT_FIL_FMT 'Bad ticket file format'"},
	{80, "TKT_FIL_INI 'tf_init not called first'"},
	{81, "KNAME_FMT 'Bad kerberos name format'"},
	{100, "MAX_HSTNM 'for compatibility'"},
	{512, "CLIENT_KRB_BUFLEN 'max unfragmented packet'"},
	{0,0}
	};

char *krb4_err_str( int err )
{
	int i;
	char *s = 0;
	for( i = 0; (s = krb4_errs[i].code) && err != krb4_errs[i].err; ++i );
	if( s == 0 ){
		static char msg[24];
		plp_snprintf(msg,sizeof(msg),"UNKNOWN %d", err );
		s = msg;
	}
	return(s);
}

#endif


int Send_krb4_auth( struct job *job, int *sock, char **real_host,
	int connect_timeout, char *errmsg, int errlen,
	struct security *security, struct line_list *info  )
{
	int status = JFAIL;
#if defined(HAVE_KRB_H) && defined(MIT_KERBEROS4)
	int ack, i;
	KTEXT_ST ticket;
	char buffer[1], *host;
	char line[LINEBUFFER];
	struct sockaddr_in sinaddr;

	errmsg[0] = 0;
	status = 0;

	sinaddr->sin_family = Host_IP.h_addrtype;
	memmove( &sinaddr->sin_addr, Host_IP.h_addr_list.list[0],
		Host_IP.h_length );

	*sock = Link_open_list( RemoteHost_DYN, real_host, 0, connect_timeout, &sinaddr );
	if( *sock < 0 ){
		/* this is to fix up the error message */
		return(JSUCC);
	}

	host = RemoteHost_DYN;
	if( !safestrcasecmp( host, LOCALHOST ) ){
		host = FQDNHost_FQDN;
	}
	if( !safestrchr( host, '.' ) ){
		if( !(host = Find_fqdn(&LookupHost_IP, host)) ){
			setstatus(logjob, "cannot find FQDN for '%s'", host );
			return JFAIL;
		}
	}
	DEBUG1("Send_krb4_auth: FQND host '%s'", host );
	setstatus(logjob, "sending krb4 auth to %s@%s",
		RemotePrinter_DYN, host);
	plp_snprintf(line, sizeof(line), "%c%s\n", REQ_K4AUTH, RemotePrinter_DYN);
	status = Link_send(host, sock, transfer_timeout, line,
		strlen(line), &ack);
	DEBUG1("Send_krb4_auth: krb4 auth request ACK status %d, ack %d", status, ack );
	if( status ){
		setstatus(logjob, "Printer %s@%s does not support krb4 authentication",
			RemotePrinter_DYN, host);
		return JFAIL;
	}
	status = krb_sendauth(0, *sock, &ticket, KLPR_SERVICE, host,
		krb_realmofhost(host), 0, NULL, NULL,
		NULL, NULL, NULL, "KLPRV0.1");
	DEBUG1("Send_krb4_auth: krb_sendauth status %d, '%s'",
		status, krb4_err_str(status) );
	if( status != KSUCCESS ){
		setstatus(logjob, "krb4 authentication failed to %s@%s - %s",
			RemotePrinter_DYN, host, krb4_err_str(status));
		return JFAIL;
	}
	buffer[0] = 0;
	i = Read_fd_len_timeout(transfer_timeout, *sock, buffer, 1);
	if (i <= 0 || Alarm_timed_out){
		status = LINK_TRANSFER_FAIL;
	} else if(buffer[0]){
		status = LINK_ACK_FAIL;
	}
	if(status){
		setstatus(logjob,
			"krb4 authentication failed to %s@%s",
			RemotePrinter_DYN, host);
	} else {
		setstatus(logjob,
			"krb4 authentication succeeded to %s@%s",
			RemotePrinter_DYN, host);
	}
#endif
	return(status);
}

/***************************************************************************
 * void Fix_send_auth() - get the Use_auth_DYN value for the remote printer
 ***************************************************************************/

 extern struct security SendSecuritySupported[];

/****************************************************************************************
 * struct security *Fix_send_auth( char *name, struct line_list *info
 * 	char *error, int errlen )
 * 
 * Find the information about the encrypt type and then make up the string
 * to send to the server requesting the encryption
 ****************************************************************************************/

struct security *Fix_send_auth( char *name, struct line_list *info,
	struct job *job, char *error, int errlen )
{
	struct security *security = 0;
	char buffer[SMALLBUFFER], *tag, *key, *from, *client, *destination;

	if( name == 0 ){
		if( Is_server ){
			name = Auth_forward_DYN;
		} else {
			name = Auth_DYN;
		}
	}
	DEBUG1("Fix_send_auth: name '%s'", name );
	if( name ){
		for( security = SendSecuritySupported;
			security->name && Globmatch(security->name, name );
			++security );
		DEBUG1("Fix_send_auth: name '%s' matches '%s'", name, security->name );
		if( security->name == 0 ){
			security = 0;
			plp_snprintf(error, errlen,
				"Send_auth_transfer: '%s' security not supported", name );
			goto error;
		}
	} else {
		DEBUG1("Fix_send_auth: no security" );
		return( 0 );
	}

	if( !(tag = security->config_tag) ) tag = security->name;
	plp_snprintf(buffer,sizeof(buffer),"%s_", tag );
	Find_default_tags( info, Pc_var_list, buffer );
	Find_tags( info, &Config_line_list, buffer );
	Find_tags( info, &PC_entry_line_list, buffer );
	if(DEBUGL1)Dump_line_list("Fix_send_auth: found info", info );

	if( !(tag = security->config_tag) ) tag = security->name;
	if( Is_server ){
		/* forwarding */
		key = "F";
		from = Find_str_value(info,ID,Value_sep);
		if(!from)from = Find_str_value(info,"server_principal",Value_sep);
		if( from == 0 ){
			plp_snprintf(error, errlen,
			"Send_auth_transfer: '%s' security missing '%s_id' info", tag, tag );
			goto error;
		}
		Set_str_value(info,FROM,from);
		if( job ){
			client = Find_str_value(&job->info,AUTHINFO,Value_sep);
			Set_str_value(info,CLIENT,client);
		} else {
			client = (char *)Perm_check.auth_client_id;
		}
		if( client == 0 
			&& !(client = Find_str_value(info,"default_client_name",Value_sep)) ){
			plp_snprintf(error, errlen,
			"Send_auth_transfer: security '%s' missing authenticated client", tag );
			goto error;
		}
		Set_str_value(info,CLIENT,client);
		destination = Find_str_value(info,FORWARD_ID,Value_sep);
		if(!destination)destination = Find_str_value(info,"forward_principal",Value_sep);
		if( destination == 0 ){
			plp_snprintf(error, errlen,
			"Send_auth_transfer: '%s' security missing '%s_forward_id' info", tag, tag );
			goto error;
		}
	} else {
		/* from client */
		key = "C";
		from = Logname_DYN;
		Set_str_value(info,FROM,from);
		client = Logname_DYN;
		Set_str_value(info,CLIENT,client);
		destination = Find_str_value(info,ID,Value_sep);
		if(!destination)destination = Find_str_value(info,"server_principal",Value_sep);
		if( destination == 0 ){
			plp_snprintf(error, errlen,
			"Send_auth_transfer: '%s' security missing '%s_id' info", tag, tag );
			goto error;
		}
	}

	Set_str_value(info,DESTINATION,destination);

	DEBUG1("Send_auth_transfer: pr '%s', key '%s', from '%s',"
		" destination '%s'",
		RemotePrinter_DYN,key, from, tag);
	plp_snprintf( buffer, sizeof(buffer),
		"%c%s %s %s %s",
		REQ_SECURE,RemotePrinter_DYN,key, from, tag );
	Set_str_value(info,CMD,buffer);
	DEBUG1("Send_auth_transfer: sending '%s'", buffer );

 error:
	if( error[0] ) security = 0;
	DEBUG1("Fix_send_auth: error '%s'", error );
	if(DEBUGL1)Dump_line_list("Fix_send_auth: info", info );
  
	return(security);
}

void Put_in_auth( int tempfd, const char *key, char *value )
{
	char *v = Escape(value,0,1);
	DEBUG1("Put_in_auth: fd %d, key '%s' value '%s', v '%s'",
		tempfd, key, value, v );
	if(
		Write_fd_str(tempfd,key) < 0
		|| Write_fd_str(tempfd,"=") < 0
		|| Write_fd_str(tempfd,v) < 0
		|| Write_fd_str(tempfd,"\n") < 0
		){
		Errorcode = JFAIL;
		logerr_die(LOG_INFO,"Put_in_auth: cannot write to file" );
	}
	if( v ) free(v); v = 0;
}


/*
 * we include user specified code for authentication
 */

#define SENDING
#include "user_auth.stub"

 struct security SendSecuritySupported[] = {
	/* name,       config_tag, connect,    send,   receive */
#if defined(HAVE_KRB_H) && defined(MIT_KERBEROS4)
	{ "kerberos4", "kerberos", Send_krb4_auth, 0, 0 },
#endif
#if defined(HAVE_KRB5_H)
	{ "kerberos*", "kerberos", 0,           Krb5_send },
#endif
	{ "pgp",       "pgp",      0,           Pgp_send },
	{ "user",      "user",     0,           User_send },
#if defined(USER_SEND)
 USER_SEND
#endif
	{0}
};
