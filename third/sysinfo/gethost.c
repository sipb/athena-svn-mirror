/*
 * Copyright (c) 1992-1998 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not
 * sold for profit or used in part or in whole for commercial gain
 * without prior written agreement, and the author is credited
 * appropriately.
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.3 $";
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
    static char 		Buf[256];
    struct hostent 	       *hp;
    register char	      **pp;
    char 		       *HName;
    register int		Len;

    if ((HName = GetHostName()) == NULL)
	return((char *) NULL);

    if ((hp = gethostbyname(HName)) == NULL) {
	SImsg(SIM_GERR, "Cannot find lookup host info for \"%s\": %s", 
	      HName, SYSERR);
	return((char *) NULL);
    }

    for (pp = hp->h_aliases, Buf[0] = C_NULL; pp && *pp; ++pp) {
	Len = strlen(Buf);
	if ((Len + strlen(*pp) + 1) >= (sizeof(Buf) - Len - 2))
	    break;
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
    static char 		Buf[256];
    struct hostent 	       *hp;
    register char	      **pp;
    char 		       *HName;
    char 		       *AddrStr;
    register int		Len;

    if ((HName = GetHostName()) == NULL)
	return((char *) NULL);

    if ((hp = gethostbyname(HName)) == NULL) {
	SImsg(SIM_GERR, "Cannot find lookup host info for \"%s\": %s", 
	      HName, SYSERR);
	return((char *) NULL);
    }

    for (pp = hp->h_addr_list, Buf[0] = C_NULL; pp && *pp; ++pp) {
	if (hp->h_addrtype == AF_INET) {
	    AddrStr = (char *) inet_ntoa(*(struct in_addr *)*pp);
	    if (!AddrStr)
		continue;

	    Len = strlen(Buf);
	    if ((Len + strlen(AddrStr) + 1) >= (sizeof(Buf) - Len - 2))
		break;

	    (void) strcat(Buf, AddrStr);
	    (void) strcat(Buf, " ");
	}
    }

    return((Buf[0]) ? Buf : (char *) NULL);
}
