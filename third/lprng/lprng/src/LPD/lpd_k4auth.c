/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1997, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 *
 ***************************************************************************
 * MODULE: lpd_k4auth.c
 * PURPOSE: receive and decode kerberos 4 credentials (MIT klpr protocol)
 **************************************************************************/

static char *const _id =
"lpd_k4auth.c,v 3.14 1998/03/24 02:43:22 mwhitson Exp";

#include "lp.h"
#include "errorcodes.h"
#include "linksupport.h"
#include "permission.h"
#include "setuid.h"

/**** ENDINCLUDE ****/

/***************************************************************************
Commentary:
MIT Athena extension  --mwhitson@mit.edu 12/2/98

The protocol used to send a krb4 authenticator consists of:

Client                                   Server
kprintername\n - receive authenticator
                                         \0  (ack)
<krb4_credentials>
                                         \0

The server will note validity of the credentials for future service
requests in this session.  No capacity for TCP stream encryption or
verification is provided.

 ***************************************************************************/


/* A bunch of this has been ripped from the Athena lpd, and, as such,
 * isn't as nice as Patrick's code.  I'll clean it up eventually.
 *   -mwhitson
 */

int Receive_k4auth( int *sock, char *input, int maxlen )
{
	int sin_len = sizeof(struct sockaddr_in);
	int status = 0;
	int ack = ACK_SUCCESS;
	struct sockaddr_in faddr;
	char error_msg[LINEBUFFER];
	uid_t euid;
	KTEXT_ST k4ticket;
	AUTH_DAT k4data;
	char k4principal[ANAME_SZ];
	char k4instance[INST_SZ];
	char k4realm[REALM_SZ];
	char k4version[9];
	int k4error=0;

	error_msg[0] = '\0';
	DEBUG0("Receive_k4auth: doing '%s'", ++input);

	k4principal[0] = k4realm[0] = '\0';
	memset(&k4ticket, 0, sizeof(KTEXT_ST));
	memset(&k4data, 0, sizeof(AUTH_DAT));
	if (getpeername(*sock, (struct sockaddr *) &faddr, &sin_len) <0) {
	  	status = JFAIL;
		plp_snprintf( error_msg, sizeof(error_msg), 
			      "Receive_k4auth: couldn't get peername" );
		goto error;
	}
	status = Link_ack( ShortRemote, sock, 0, 0x100, 0 );
	if( status ){
		ack = ACK_FAIL;
		plp_snprintf( error_msg, sizeof(error_msg),
			      "Receive_k4auth: sending ACK 0 failed" );
		goto error;
	}
	strcpy(k4instance, "*");
	euid = geteuid();
	if( UID_root ) (void)To_root();  /* Need root here to read srvtab */
	k4error = krb_recvauth(0L, *sock, &k4ticket, KLPR_SERVICE,
			      k4instance,
			      &faddr,
			      (struct sockaddr_in *)NULL,
			      &k4data, "", NULL,
			      k4version);
	if( UID_root ) (void)To_uid( euid );
	if (k4error != KSUCCESS) {
		/* erroring out here if the auth failed. */
	  	status = JFAIL;
		plp_snprintf( error_msg, sizeof(error_msg),
			      "Receive_k4auth: authentication failed" );
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
	DEBUG0("Receive_k4auth: auth for %s", k4name);

	k4flag = 1;

	/* Perhaps this should go somewhere else... */
	Auth_from = 1;
	Perm_check.authtype = "krb4";

	/* ACK the credentials  */
	status = Link_ack( ShortRemote, sock, 0, 0x100, 0 );
	if( status ){
		ack = ACK_FAIL;
		plp_snprintf(error_msg, sizeof(error_msg),
			     "Receive_k4auth: sending ACK 0 failed" );
		goto error;
	}
	goto done;

error:	
	DEBUG0("Receive_k4auth: error - status %d, ack %d, k4error %d, error '%s'",
		status, ack, k4error, error_msg );
	if( status || ack ){
		if( ack ) (void)Link_ack( ShortRemote, sock, 0, ack, 0 );
		if( status == 0 ) status = JFAIL;
		DEBUG0("Receive_k4auth: sending ACK %d, msg '%s'",
			ack, error_msg );
		/* shut down reception from the remote file */
		if( error_msg[0] ){
			safestrncat( error_msg, "\n" );
			Write_fd_str( *sock, error_msg );
		}
	}

done:
	DEBUG0("Receive_k4auth: done - status %d, ack %d, principal %s",
		status, ack, k4name );
	return(status);
}
