/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	"notice.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <ctype.h>

struct sockaddr_in *
resolve_host(name)

/* Resolve the specified host name into an internet address.  The "name" may
 * be either a character string name, or an address in the form a.b.c.d where
 * the pieces are octal, decimal, or hex numbers.  Returns a pointer to a
 * sockaddr_in struct (note this structure is statically allocated and must
 * be copied), or NULL if the name is unknown.
 */

register char *name;
{
	register struct hostent *fhost;
	static struct sockaddr_in sa;

	
	if ((sa.sin_addr.s_addr = inet_addr(name)) != -1) {
		sa.sin_family = AF_INET;	/* grot */
		sa.sin_port = 0;
	} else if ((fhost = gethostbyname(name)) != NULL) {
		sa.sin_family = fhost->h_addrtype;
		sa.sin_port = 0;
		memcpy(&sa.sin_addr, fhost->h_addr, fhost->h_length);
	} else
			return(NULL);
	return(&sa);
}


/*
 * The convention established by the Kerberos-authenticated
 * services is that the principal host name is
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
