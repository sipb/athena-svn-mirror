/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-2000, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: krb5_auth.h,v 1.1.1.4 2000-03-31 15:48:10 mwhitson Exp $
 ***************************************************************************/



#ifndef _KRB5_AUTH_H
#define _KRB5_AUTH_H 1

/* PROTOTYPES */
int server_krb5_auth( char *keytabfile, char *service, int sock,
	char **auth, char *err, int errlen, char *file );
int server_krb5_status( int sock, char *err, int errlen, char *file );
int server_krb5_status( int sock, char *err, int errlen, char *file );
int client_krb5_auth( char *keytabfile, char *service, char *host,
	char *server_principal,
	int options, char *life, char *renew_time,
	int sock, char *err, int errlen, char *file );
int remote_principal_krb5( char *service, char *host, char *err, int errlen );

#endif
