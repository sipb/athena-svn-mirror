/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/AL/get_phost.c,v $
 * $Author: cfields $
 *
 * Copyright 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#ifndef lint
static char *rcsid_phost_c =
"$Header: /afs/dev.mit.edu/source/repository/athena/lib/AL/get_phost.c,v 4.10 1994-08-21 19:30:27 cfields Exp $";
#endif /* lint */

#include <mit-copyright.h>
#include <stdio.h>
#include <ctype.h>
#include <netdb.h>
#include <string.h>

/*
 * This routine takes an alias for a host name and returns the first
 * field, lower case, of its domain name.  For example, if "menel" is
 * an alias for host officially named "menelaus" (in /etc/hosts), for
 * the host whose official name is "MENELAUS.MIT.EDU", the name "menelaus"
 * is returned.
 *
 * This is done for historical Athena reasons: the Kerberos name of
 * rcmd servers (rlogin, rsh, rcp) is of the form "rcmd.host@realm"
 * where "host"is the lowercase for of the host name ("menelaus").
 * This should go away: the instance should be the domain name
 * (MENELAUS.MIT.EDU).  But for now we need this routine...
 *
 * Then you have names like test.dialup.mit.edu, where the key you want
 * is really "rcmd.pesto@ATHENA.MIT.EDU".  It's necessary to reverse-
 * resolve the IP address.
 *
 * A pointer to the name is returned, if found, otherwise a pointer
 * to the original "alias" argument is returned.
 */

char * krb_get_phost(alias)
    char *alias;
{
    struct hostent *h;
    static char phost[100];
    char *p;
    extern int krb_ReverseResolve;

    strcpy(phost, alias);
    if ((h=gethostbyname(phost)) != (struct hostent *)0 ) {
        /* copy the new name */
        strcpy(phost, h->h_name);

	/* reverse-resolve the address */
	if (krb_ReverseResolve) {
	  if ((h=gethostbyaddr(h->h_addr, h->h_length, h->h_addrtype))
	      != (struct hostent *)0)
	    strcpy(phost, h->h_name);
	}

        /* cut off the domain name */
        p = strchr( phost, '.' );
        if (p) *p = '\0';

	/* convert to lowercase */
	p = phost;
	do {
		if (isupper(*p)) *p=tolower(*p);
	} while  (*p++);
    }
    return(phost);
}
