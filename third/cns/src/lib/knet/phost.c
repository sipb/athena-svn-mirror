/*
 * Copyright 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <netdb.h>

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
	char *p = strchr( h->h_name, '.' );
	if (p) *p = 0;
	p = phost = h->h_name;
	do {
	    if (isupper(*p)) *p=tolower(*p);
	} while (*p++);
    }
    return( phost );
}
