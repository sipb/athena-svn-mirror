/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1999, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: lpd_secure.h,v 1.1.1.1 1999-05-04 18:07:15 danw Exp $
 ***************************************************************************/



#ifndef _LPD_SECURE_H_
#define _LPD_SECURE_H_ 1

/* PROTOTYPES */
int Receive_secure( int *sock, char *input );
int Pgp_receive( int *sock, char *user, char *jobsize, int from_server );
int Krb5_receive( int *sock, char *authtype, char *user,
	char *jobsize, int from_server );
int Check_secure_perms( struct line_list *options, int from_server );
void Wait_for_child( void *p );
int Receive_k4auth( int *sock, char *input );

#endif
