/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
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
 *
 * RETURNS (MCSIOP_CREATE)
 * Query->Out = String containing hostname
 * Query->OutSize = Length of Out.
 */
extern int GetHostName(Query)
     MCSIquery_t	       *Query;
{
    static char			HName[MAXHOSTNAMELEN];

    if (Query->Op == MCSIOP_CREATE) {
	if (gethostname(HName, sizeof(HName)) == 0) {
	    Query->Out = strdup(HName);
	    Query->OutSize = strlen(HName);
	    return 0;
	}
    } else if (Query->Op == MCSIOP_DESTROY) {
	if (Query->Out && Query->OutSize)
	    (void) free(Query->Out);
	return 0;
    }

    return -1;
}

/*
 * Get aliases for this hostname
 *
 * Note that this won't work on most systems if your
 * gethostbyname() call gets its info from DNS since
 * DNS does not provide this funtionality.
 *
 * RETURNS
 * If Query->Flags & MCSIF_STRING:
 * Query->Out = String of space seperated values
 * Query->OutSize = Length of Out
 * Else:
 * Query->Out = Array of NULL terminated values
 * Query->OutSize = Number of enteries in Query->Out
 */
extern int GetHostAliases(Query)
     MCSIquery_t	       *Query;
{
    static char			HName[MAXHOSTNAMELEN];
    char		      **List = NULL;
    char		       *Buffer = NULL;
    int				ListCnt = 0;
    struct hostent 	       *hp;
    register char	      **pp;
    register int		Len;

    /*
     * Destroy and return if commanded 
     */
    if (Query->Op == MCSIOP_DESTROY) {
	if (FLAGS_ON(Query->Flags, MCSIF_STRING)) {
	    if (Query->Out && Query->OutSize)
		(void) free(Query->Out);
	} else {
	    for (pp = (char **) Query->Out; pp && *pp; ++pp)
		if (*pp)
		    (void) free(*pp);
	    if (Query->Out)
		(void) free(Query->Out);
	}
	return 0;
    }
    /* else MCSIOP_CREATE */

    if (gethostname(HName, sizeof(HName)) != 0)
	return -1;

    if ((hp = gethostbyname(HName)) == NULL) {
	SImsg(SIM_GERR, "Cannot find lookup host info for \"%s\": %s", 
	      HName, SYSERR);
	return -1;
    }

    if (FLAGS_ON(Query->Flags, MCSIF_STRING)) {
	/* Calculate buf size */
	for (Len = 0, pp = hp->h_aliases; pp && *pp; ++pp)
	    Len += strlen(*pp) + 1;
	Buffer = xcalloc(1, Len + 2);
    }

    for (pp = hp->h_aliases; pp && *pp; ++pp) {
	if (FLAGS_ON(Query->Flags, MCSIF_STRING)) {
	    if (*Buffer)
		strcat(Buffer, " ");
	    strcat(Buffer, *pp);
	} else {
	    if (!List)
		List = (char **) xcalloc(2, sizeof(char *));
	    else
		List = (char **) xrealloc(List, (ListCnt+1) * 
					  sizeof(char *));
	    List[ListCnt] = strdup(*pp);
	    List[++ListCnt] = NULL;
	}
    }

    if (FLAGS_ON(Query->Flags, MCSIF_STRING)) {
	Query->Out = Buffer;
	Query->OutSize = strlen(Buffer);
    } else {
	Query->Out = List;
	Query->OutSize = ListCnt;
    }

    return (Query->OutSize) ? 0 : -1;
}

/*
 * Get addresses for this host
 *
 * RETURNS
 * Query->Out = Array of NULL terminated values
 * Query->OutSize = Number of enteries in Query->Out
 */
extern int GetHostAddrs(Query)
     MCSIquery_t	       *Query;
{
    static char			HName[MAXHOSTNAMELEN];
    register char	      **pp;
    char		      **List = NULL;
    char		       *Buffer = NULL;
    int				ListCnt = 0;
    char 		       *AddrStr;
    struct hostent 	       *hp;

    /*
     * Destroy and return if commanded 
     */
    if (Query->Op == MCSIOP_DESTROY) {
	if (FLAGS_ON(Query->Flags, MCSIF_STRING)) {
	    if (Query->Out && Query->OutSize)
		(void) free(Query->Out);
	} else {
	    for (pp = (char **) Query->Out; pp && *pp; ++pp)
		if (*pp)
		    (void) free(*pp);
	    if (Query->Out)
		(void) free(Query->Out);
	}
	return 0;
    }
    /* else MCSIOP_CREATE */

    if (gethostname(HName, sizeof(HName)) != 0)
	return -1;

    if ((hp = gethostbyname(HName)) == NULL) {
	SImsg(SIM_GERR, "Cannot find lookup host info for \"%s\": %s", 
	      HName, SYSERR);
	return -1;
    }

    for (pp = hp->h_addr_list; pp && *pp; ++pp) {
	if (hp->h_addrtype == AF_INET) {
	    AddrStr = (char *) inet_ntoa(*(struct in_addr *)*pp);
	    if (!AddrStr)
		continue;

	    if (FLAGS_ON(Query->Flags, MCSIF_STRING)) {
		if (!Buffer)
		    Buffer = (char *) xcalloc(1, strlen(AddrStr) + 2);
		else
		    Buffer = (char *) xrealloc(Buffer, 
					       strlen(Buffer)+
					       strlen(AddrStr)+2);
		if (*Buffer)
		    strcat(Buffer, " ");
		strcat(Buffer, AddrStr);
	    } else {
		if (!List)
		    List = (char **) xcalloc(2, sizeof(char *));
		else
		    List = (char **) xrealloc(List, 
					      (ListCnt+1) * sizeof(char *));
		List[ListCnt] = strdup(AddrStr);
		List[++ListCnt] = NULL;
	    }
	}
    }

    if (FLAGS_ON(Query->Flags, MCSIF_STRING)) {
	Query->Out = Buffer;
	Query->OutSize = strlen(Buffer);
    } else {
	Query->Out = List;
	Query->OutSize = ListCnt;
    }

    return (Query->OutSize) ? 0 : -1;
}
