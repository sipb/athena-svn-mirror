/*
 * kadm_etxt.c
 *
 * Copyright 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include "mit-copyright.h"

#define	DEFINE_SOCKADDR		/* Ask krb.h for struct sockaddr, netdb, etc */
#include "krb.h"
#include "kadm.h"
#include "kadm_err.h"
#include "krb_err.h"

/*
 * This file contains an array of error text strings.
 * The associated error codes (which are defined in "kadm_err.h")
 * follow the string in the comments at the end of each line.
 */

static const
/* Some C compilers (like ThinkC when producing a driver) can't grok
   initialized multimentional arrays! */
#ifdef UNIDIMENSIONAL_ARRAYS
 char kadm_err_txt[34][100] = {  
#else 
 char *const kadm_err_txt [34] = { 
#endif
	 "$Header: /afs/dev.mit.edu/source/repository/third/cns/src/lib/kadm/kadm_etx.c,v 1.1.1.1 1996-09-06 00:47:28 ghudson Exp $",
	 "Cannot fetch local realm",
	 "Unable to fetch credentials",
	 "Bad key supplied",
	 "Can't encrypt data",
	 "Cannot encode/decode authentication info",
	 "Principal attemping change is in wrong realm",
	 "Packet is too large",
	 "Version number is incorrect",
	 "Checksum does not match",
	 "Unsealing private data failed",
	 "Unsupported operation",
	 "Could not find administrating host",
	 "Administrating host name is unknown",
	 "Could not find service name in services database",
	 "Could not create socket",
	 "Could not connect to server",
	 "Could not fetch local socket address",
	 "Could not fetch master key",
	 "Could not verify master key",
	 "Entry already exists in database",
	 "Database store error",
	 "Database read error",
	 "Insufficient access to perform requested operation",
	 "Data is available for return to client",
	 "No such entry in the database",
	 "Memory exhausted",
	 "Could not fetch system hostname",
	 "Could not bind port",
	 "Length mismatch problem",
	 "Illegal use of wildcard",
	 "Database locked or in use",
	 "Insecure password rejected",
	 "Cleartext password and DES key did not match",
};


/*
 * FIXME: This rountine translates error codes based on the
 * fact that we only of 16bit ints on some machines.
 */
const char * INTERFACE
kadm_get_err_text(errno)
    int errno;
{
	int erridx;

	erridx = errno & 0xFF;

    if ((errno & 0xFFFFFF00) == (ERROR_TABLE_BASE_kadm & 0xFFFFFF00))
    	return kadm_err_txt[erridx];
    else
	    return krb_get_err_text(erridx);
}
