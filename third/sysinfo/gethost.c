/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: gethost.c,v 1.1.1.1 1996-10-07 20:16:51 ghudson Exp $";
#endif

/*
 * Get Host name information functions
 */

#include <netdb.h>			/* XXX Avoid buggy AIX 3.2.5 cpp */
#include "defs.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*
 * Get our hostname
 */
extern char *GetHostName()
{
    static char 		Buf[MAXHOSTNAMLEN+1];

    gethostname(Buf, sizeof(Buf));

    return((Buf[0]) ? Buf : (char *) NULL);
}

/*
 * Get aliases for this hostname
 *
 * Note that this won't work on most systems if your
 * gethostbyname() call gets its info from DNS since
 * DNS does not provide this funtionality.
 */
extern char *GetHostAliases()
{
    static char 		Buf[BUFSIZ];
    struct hostent 	       *hp;
    register char	      **pp;
    char 		       *HName;

    if ((HName = GetHostName()) == NULL)
	return((char *) NULL);

    if ((hp = gethostbyname(HName)) == NULL) {
	Error("Cannot find lookup host info for \"%s\": %s", HName, SYSERR);
	return((char *) NULL);
    }

    for (pp = hp->h_aliases, Buf[0] = C_NULL; pp && *pp; ++pp) {
	(void) strcat(Buf, *pp);
	(void) strcat(Buf, " ");
    }

    return((Buf[0]) ? Buf : (char *) NULL);
}

/*
 * Get addresses for this host
 */
extern char *GetHostAddrs()
{
    static char 		Buf[BUFSIZ];
    struct hostent 	       *hp;
    register char	      **pp;
    char 		       *HName;

    if ((HName = GetHostName()) == NULL)
	return((char *) NULL);

    if ((hp = gethostbyname(HName)) == NULL) {
	Error("Cannot find lookup host info for \"%s\": %s", HName, SYSERR);
	return((char *) NULL);
    }

    for (pp = hp->h_addr_list, Buf[0] = C_NULL; pp && *pp; ++pp) {
	if (hp->h_addrtype == AF_INET) {
	    (void) strcat(Buf, (char *) inet_ntoa(*(struct in_addr *)*pp));
	    (void) strcat(Buf, " ");
	}
    }

    return((Buf[0]) ? Buf : (char *) NULL);
}
