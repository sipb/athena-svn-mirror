/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/AL/get_phost.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/lib/AL/get_phost.c,v 4.1 1987-09-01 16:22:04 steiner Exp $
 */

#ifndef lint
static char *rcsid_phost_c = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/AL/get_phost.c,v 4.1 1987-09-01 16:22:04 steiner Exp $";
#endif lint

#include <stdio.h>
#include <ctype.h>
#include <netdb.h>

char *index();

/*
 * The convention established by the Kerberos-authenticated rcmd
 * services (rlogin, rsh, rcp) is that the principal host name is
 * all lower case characters.  Therefore, we can get this name from
 * an alias by taking the official, fully qualified hostname, stripping off
 * the domain info (ie, take everything up to but excluding the
 * '.') and translating it to lower case.  For example, if "menel" is an
 * alias for host officially named "menelaus" (in /etc/hosts), for 
 * the host whose official name is "MENELAUS.MIT.EDU", the user could
 * give the command "menel echo foo" and we will resolve it to "menelaus".
 */

char *
PrincipalHostname( alias )
char *alias;
{
    struct hostent *h;
    char *phost = alias;
    if ( (h=gethostbyname(alias)) != (struct hostent *)NULL ) {
	char *p = index( h->h_name, '.' );
	if (p) *p = NULL;
	p = phost = h->h_name;
	do {
	    if (isupper(*p)) *p=tolower(*p);
	} while (*p++);
    }
    return( phost );
}
