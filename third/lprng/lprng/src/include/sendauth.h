/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1999, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: sendauth.h,v 1.2 2001-03-07 01:20:22 ghudson Exp $
 ***************************************************************************/



#ifndef _SENDAUTH_H_
#define _SENDAUTH_H_ 1

/* PROTOTYPES */
void Put_in_auth( int tempfd, const char *key, char *value );
void Setup_auth_info( int tempfd, char *cmd );
int Send_auth_transfer( int *sock, int transfer_timeout, char *tempfile, int printjob );
int Pgp_send( int *sock, int transfer_timeout, char *tempfile );
char *krb4_err_str( int err );
int Send_krb4_auth(int *sock, struct job *job, int transfer_timeout );

#endif
