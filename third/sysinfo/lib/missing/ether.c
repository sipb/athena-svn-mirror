/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

#if	defined(HAVE_AUTOCONFIG_H)
#include "autoconfig.h"
#endif	/* HAVE_AUTOCONFIG_H */

#include "defs.h"
#include "ether.h"

#if	!defined(HAVE_ETHER_NTOHOST)
/*
 * Convert an ether_addr into an ASCII string
 */
char *ether_ntoa(eaddr)
     struct ether_addr	       *eaddr;
{
    static char		        Buff[64];

    (void) snprintf(Buff, sizeof(Buff), "%2X:%2X:%2X:%2X:%2X:%2X",
		    eaddr->ether_addr_octet[0],
		    eaddr->ether_addr_octet[1],
		    eaddr->ether_addr_octet[2],
		    eaddr->ether_addr_octet[3],
		    eaddr->ether_addr_octet[4],
		    eaddr->ether_addr_octet[5]);

    return Buff;
}
#endif	/* !HAVE_ETHER_NTOA */

#if	!defined(HAVE_ETHER_NTOHOST)

static char EthersFile[]	= _PATH_ETHERS;

/*
 * Map ethernet address (Eaddr) to a host name and place it in Hostname.
 * Currently we only look at /etc/ethers.  We should look in NIS also.
 */
int ether_ntohost(Hostname, Eaddr)
     char		       *Hostname;
     struct ether_addr		Eaddr;
{
    FILE		       *FH;
    static char			Buff[512];
    char		       *AddrStr;
    char		       *cp;

    FH = fopen("r", EthersFile);
    if (!FH) {
	return -1;
    }

    AddrStr = ether_ntoa(Eaddr);
    if (!AddrStr)
	return -1;

    while (fgets(Buff, sizeof(Buff), FH)) {
	if (Buff[0] == '#')
	    continue;
	/* Find end of addr */
	for (cp = Buff; cp && *cp && *cp != ' ' && *cp != '\t'; ++cp);
	if (!cp || !*cp)
	    continue;
	if (strncmp(AddrStr, Buff, cp-Buff) == 0) {
	    fclose(FH);
	    /* Find start of hostname */
	    for ( ; cp && *cp && !isalpha(*cp); ++cp);
	    if (!cp || !*cp)
		return -1;
	    strcpy(Hostname, cp);
	    return 0;
	}
    }

    fclose(FH);

    return -1;
}
#endif	/* HAVE_ETHER_NTOHOST */
