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
 * Portions of code found in this file are based on the 4.3BSD 
 * netstat(8) program.
 */

/*
 * Network Interface routines
 */

#include <stdio.h>
#include "os.h"

#include <sys/types.h>
#include <sys/socket.h>
#if	defined(NEED_SOCKIO)
#include <sys/sockio.h>
#endif	/* NEED_SOCKIO */
#include <sys/param.h>
#include <sys/errno.h>
#include <netdb.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#if	defined(HAVE_IN_IFADDR)
#include <netinet/in_var.h>
#endif	/* HAVE_IN_IFADDR */

#include "defs.h"

#if	GETNETIF_TYPE == GETNETIF_IFNET
/*
 * Network Interface symbol
 */
char 				NetifSYM[] = "_ifnet";
#endif	/* GETNETIF_IFNET */

/*
 * Interface Address union
 */
union {
    struct ifaddr 	ifaddr;
#if	defined(HAVE_IN_IFADDR)
    struct in_ifaddr 	in_ifaddr;
#endif	/* HAVE_IN_IFADDR */
} 				ifaddress;

/*
 * Create a DevInfo_t for a network interface.
 */
static DevInfo_t *CreateNetif(ProbeData, FullName, IfNet)
     ProbeData_t	       *ProbeData;
     char		       *FullName;
     struct ifnet              *IfNet;
{
    DevInfo_t		       *DevInfo;

    /* There's no guarentee this is already set */
    if (!ProbeData->DevDefine)
	ProbeData->DevDefine = DevDefGet(NULL, DT_NETIF, 0);
    else
	ProbeData->DevDefine->Type = DT_NETIF;
    if (FullName)
	ProbeData->DevName = FullName;

    /* Create the device */
    if (!(DevInfo = DeviceCreate(ProbeData)))
	return((DevInfo_t *) NULL);

    if (IfNet)
	DevInfo->Unit 	= IfNet->if_unit;

    if (!DevInfo->Model) {
#if	defined(HAVE_IF_VERSION)
	if (IfNet->if_version && IfNet->if_version[0])
	    DevInfo->Model 	= strdup(IfNet->if_version);
	else
#endif	/* HAVE_IF_VERSION */
	    if (ProbeData->DevDefine)
		DevInfo->Model 	= ProbeData->DevDefine->Model;
    }

    if (!DevInfo->ModelDesc && ProbeData->DevDefine)
	DevInfo->ModelDesc = ProbeData->DevDefine->Desc;

    return(DevInfo);
}

/*
 * Return the netent of the network whose address is given.
 * The address is assumed to be that of a net or subnet, not a host.
 */
static struct netent *GetNet(inaddr, mask)
    u_long 			inaddr;
    u_long 			mask;
{
    u_long 			net;
    register u_long 		i, netaddr;
    int 			subnetshift;
    static struct in_addr 	in_addr;

    if (in_addr.s_addr = inaddr) {
	i = in_addr.s_addr;
	if (mask == 0) {
	    if (IN_CLASSA(i)) {
		mask = IN_CLASSA_NET;
		subnetshift = 8;
	    } else if (IN_CLASSB(i)) {
		mask = IN_CLASSB_NET;
		subnetshift = 8;
	    } else {
		mask = IN_CLASSC_NET;
		subnetshift = 4;
	    }
	    /*
	     * If there are more bits than the standard mask
	     * would suggest, subnets must be in use.
	     * Guess at the subnet mask, assuming reasonable
	     * width subnet fields.
	     */
	    while (in_addr.s_addr &~ mask)
		mask = (long)mask >> subnetshift;
	}
	net = in_addr.s_addr & mask;
	while ((mask & 1) == 0)
	    mask >>= 1, net >>= 1;
	netaddr = net;
    } else {
	netaddr = inaddr;
    }

    return(getnetbyaddr(netaddr, AF_INET));
}

/*
 * Get NetIF_t for an Internet address
 */
extern NetIF_t *GetNetifINET(AddrFam, hostaddr, maskaddr)
    AddrFamily_t	       *AddrFam;
    struct sockaddr_in	       *hostaddr;
    struct sockaddr_in	       *maskaddr;
{
    struct in_addr		in_addr;
    struct netent 	       *np;
    struct hostent 	       *hp;
    struct sockaddr_in 	       *sin;
    char		       *inet_ntoa();
    NetIF_t		       *ni;

    ni = NewNetif(NULL);

    if (hostaddr && maskaddr) {
	np = GetNet((u_long) hostaddr->sin_addr.s_addr, 
		    (u_long) maskaddr->sin_addr.s_addr);
	in_addr.s_addr = hostaddr->sin_addr.s_addr & 
	  maskaddr->sin_addr.s_addr;
	ni->NetAddr = strdup(inet_ntoa(in_addr));
	sin = hostaddr;
    } else {
#if	defined(HAVE_IN_IFADDR)
	np = GetNet((u_long) ifaddress.in_ifaddr.ia_subnet, 
		    (u_long) ifaddress.in_ifaddr.ia_subnetmask);
	in_addr.s_addr = ifaddress.in_ifaddr.ia_subnet;
	ni->NetAddr = strdup(inet_ntoa(in_addr));
	sin = (struct sockaddr_in *) &ifaddress.in_ifaddr.ia_addr;
#endif	/* HAVE_IN_IFADDR */
    }

    ni->HostAddr = strdup(inet_ntoa(sin->sin_addr));
    hp = gethostbyaddr((char *) &(sin->sin_addr),
		       sizeof(struct in_addr), AF_INET);

    if (hp)
	ni->HostName = strdup(hp->h_name);

    if (np)
	ni->NetName = strdup(np->n_name);
	    
    if (AddrFam)
	ni->TypeName = AddrFam->Name;

    return(ni);
}

/*
 * Get NetIF_t for an unknown address type
 */
extern NetIF_t *GetNetifUnknown(AddrFam)
    AddrFamily_t	       *AddrFam;
{
    NetIF_t		       *ni = NULL;

    ni = NewNetif(NULL);
    ni->HostAddr = "<unknown>";
    if (AddrFam)
	ni->TypeName = AddrFam->Name;

    return(ni);
}

/*
 * Get an Address Family table entry
 */
static AddrFamily_t *GetAddrFamily(type)
    int				type;
{
    extern AddrFamily_t		AddrFamily[];
    register int		i;

    for (i = 0; AddrFamily[i].Name; ++i)
	if (AddrFamily[i].Type == type)
	    return(&AddrFamily[i]);

    return((AddrFamily_t *) NULL);
}

/*
 * See if IfName matches any names in ProbeData and return name
 * found.
 */
static int IFmatch(ProbeData, IfName)
     ProbeData_t	       *ProbeData;
     char		       *IfName;
{
    register char	      **cpp;

    if (!ProbeData)
	return(FALSE);

    if (EQ(ProbeData->DevName, IfName))
	return(TRUE);

    for (cpp = ProbeData->AliasNames; cpp && *cpp; ++cpp)
	if (EQ(*cpp, IfName))
	    return(TRUE);

    return(FALSE);
}

#if	GETNETIF_TYPE == GETNETIF_IFNET
/*
 * Get a linked list of NetIF_t's for each address starting at 'startaddr'.
 */
static NetIF_t *GetNetifAddrs(kd, startaddr, FullName, DevInfo)
    kvm_t		       *kd;
    off_t			startaddr;
    char		       *FullName;
    DevInfo_t		       *DevInfo;
{
    u_long			addr;
    NetIF_t		       *base = NULL;
    register NetIF_t	       *ni, *pni;
    AddrFamily_t	       *AddrFamPtr;

    for (addr = startaddr; addr; addr = (u_long) ifaddress.ifaddr.ifa_next) {
	/*
	 * Read the ifaddr structure from kernel space
	 */
	if (KVMget(kd, addr, (char *) &ifaddress, 
		   sizeof(ifaddress), KDT_DATA)) {
	    SImsg(SIM_GERR, "cannot read if address");
	    continue;
	}

	/*
	 * Now get and call the Address Family specific routine
	 * to extract a NetIF_t.
	 */
	if (AddrFamPtr = GetAddrFamily(ifaddress.ifaddr.ifa_addr.sa_family)) {
	    if (ni = (*AddrFamPtr->GetNetIF)(AddrFamPtr, NULL, NULL))
		SetMacInfo(DevInfo, ni);
	} else {
	    SImsg(SIM_DBG, "Address family %d is not defined.", 
			     ifaddress.ifaddr.ifa_addr.sa_family);
	    continue;
	}

	/*
	 * Add the new NetIF_t to the base of the linked list.
	 */
	if (base) {
	    for (pni = base; pni && pni->Next; pni = pni->Next);
	    pni->Next = ni;
	} else {
	    base = ni;
	}
    }

    return(base);
}

/*
 * Query/find network interface devices and add them to devicelist
 */
extern DevInfo_t *ProbeNetif(ProbeData)
     ProbeData_t	       *ProbeData;
{
    DevInfo_t 		       *DevInfo = NULL;
    static struct ifnet         ifnet;
    static char 		ifname[16], FullName[17];
    nlist_t		       *nlptr;
    register char	       *p;
    u_long 		        ifnetaddr;
    kvm_t 		       *kd;
    char 		       *DevName;
    DevData_t 		       *DevData;
    DevDefine_t	 	       *DevDefine;
    int				NameMatch = FALSE;

    if (!ProbeData || !ProbeData->DevName)
	return((DevInfo_t *) NULL);

    DevName = ProbeData->DevName;
    DevData = ProbeData->DevData;
    DevDefine = ProbeData->DevDefine;
    SImsg(SIM_DBG, "ProbeNetif(%s)", DevName);

    if (!(kd = KVMopen()))
	return((DevInfo_t *) NULL);

    if ((nlptr = KVMnlist(kd, NetifSYM, (nlist_t *)NULL, 0)) == NULL)
	return((DevInfo_t *) NULL);

    if (CheckNlist(nlptr))
	return((DevInfo_t *) NULL);

    /*
     * Read address of ifnet structure from kernel space
     */
    if (KVMget(kd, nlptr->n_value, (char *) &ifnetaddr, 
		sizeof(ifnetaddr), KDT_DATA)) {
	SImsg(SIM_GERR, "kvm_read ifnetaddr failed");
	KVMclose(kd);
	return((DevInfo_t *) NULL);
    }

    /*
     * Read and then check each ifnet entry we found.
     */
    for (; ifnetaddr; ifnetaddr = (off_t) ifnet.if_next) {
	/*
	 * Read the ifnet structure
	 */
	if (KVMget(kd, ifnetaddr, (char *)&ifnet, sizeof(ifnet), KDT_DATA)) {
	    SImsg(SIM_GERR, "kvm_read ifnetaddr ifnet failed");
	    continue;
	}

	/*
	 * Read if_name from kernel space
	 */
	if (KVMget(kd, (u_long)ifnet.if_name, ifname, 
		   sizeof(ifname), KDT_STRING)) {
	    SImsg(SIM_GERR, "kvm_read ifname failed");
	    continue;
	}

	/*
	 * Copy if_name to 'FullName' and add unit number
	 */
	(void) snprintf(FullName, sizeof(FullName),  "%s%d", ifname, ifnet.if_unit);

	/*
	 * Check to see if this is the interface we want.
	 */
	if (IFmatch(ProbeData, FullName)) {
	    NameMatch = TRUE;
	    break;
	}
    }

    if (!NameMatch) {
	/* Device Not Found */
	SImsg(SIM_DBG, "%s: Could not find netif in ifconf table.", DevName);
	return((DevInfo_t *) NULL);
    }

    /*
     * Create and set device
     */
    DevInfo = CreateNetif(ProbeData, FullName, &ifnet);

    /*
     * Get and set address info
     */
    if (ifnet.if_addrlist) {
	NetIF_t 	       *ni;
	
	if (ni = GetNetifAddrs(kd, (off_t) ifnet.if_addrlist, 
			       FullName, DevInfo))
	    DevInfo->DevSpec = (caddr_t *) ni;
    }

    KVMclose(kd);

    return(DevInfo);
}
#endif	/* GETNETIF_IFNET */

#if	GETNETIF_TYPE == GETNETIF_IFCONF
/*
 * Set network address data.
 */
static int SetNetifAddrs(ifname, sock, hostaddr, maskaddr)
    char		       *ifname;
    int				sock;
    struct sockaddr_in	       *hostaddr;
    struct sockaddr_in	       *maskaddr;
{
    struct ifreq		ifr;

    (void) strcpy(ifr.ifr_name, ifname);

    /*
     * Get address info
     */
    if (ioctl(sock, SIOCGIFADDR, (char *) &ifr) < 0) {
	SImsg(SIM_GERR, "%s: ioctl SIOCGIFADDR failed: %s.", ifname, SYSERR);
	return(-1);
    }
    memcpy((char *)hostaddr, (char *)&ifr.ifr_addr, 
	   sizeof(struct sockaddr_in));

    /*
     * Get the network mask
     */
    if (ioctl(sock, SIOCGIFNETMASK, (char *) &ifr) < 0) {
	SImsg(SIM_GERR, "%s: ioctl SIOCGIFNETMASK failed: %s.", 
			 ifname, SYSERR);
	return(-1);
    }
    memcpy((char *)maskaddr, (char *)&ifr.ifr_addr, 
	   sizeof(struct sockaddr_in));

    return(0);
}

/*
 * Query/find network interface devices and add them to devicelist
 */
extern DevInfo_t *ProbeNetif(ProbeData)
     ProbeData_t	       *ProbeData;
{
    DevInfo_t 		       *DevInfo = NULL;
    NetIF_t	 	       *ni;
    struct ifreq	       *ifreq;
    static struct ifconf	ifconf;
    static struct sockaddr_in	HostAddr;
    static struct sockaddr_in	MaskAddr;
    static char			ReqBuff[BUFSIZ];
    static int			SockDesc = -1;
    register int		n;
    register char	      **cpp;
    register char 	       *Alias = NULL;
    AddrFamily_t	       *AddrFamPtr;
    int				NameMatch = FALSE;
    char 		       *DevName;
    DevData_t 		       *DevData;
    DevDefine_t	 	       *DevDefine;

    if (!ProbeData || !ProbeData->DevName)
	return((DevInfo_t *) NULL);

    DevName = ProbeData->DevName;
    DevData = ProbeData->DevData;
    DevDefine = ProbeData->DevDefine;
    SImsg(SIM_DBG, "ProbeNetif(%s)", DevName);

    if (SockDesc < 0) {
	/*
	 * Get list of all interfaces
	 */
	ifconf.ifc_len = sizeof(ReqBuff);
	ifconf.ifc_buf = ReqBuff;

	if ((SockDesc = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	    SImsg(SIM_GERR, "Cannot create socket: %s.", SYSERR);
	    return((DevInfo_t *)NULL);
	}

	if (ioctl(SockDesc, SIOCGIFCONF, (char *) &ifconf) < 0) {
	    SImsg(SIM_GERR, "%s: ioctl SIOCGIFCONF failed: %s.", 
			     DevName, SYSERR);
	    return((DevInfo_t *)NULL);
	}
    }

    /*
     * Iterate over all known interfaces
     */
    for (ifreq = ifconf.ifc_req, n = ifconf.ifc_len/sizeof(struct ifreq);
	 --n >= 0; ifreq++) {

	/*
	 * Compare the hardware DevName as passed to us, the config file name
	 * and all config file aliases.
	 * Check to see if this is the interface we want.
	 */
	if (IFmatch(ProbeData, ifreq->ifr_name)) {
	    NameMatch = TRUE;
	    break;
	}

	if (DevDefine && DevData && !NameMatch) {
	    /* Check config file name */
	    Alias = MkDevName(DevDefine->Name, DevData->DevUnit,
			      DevDefine->Type, DevDefine->Flags);
	    if (EQ(Alias, ifreq->ifr_name)) {
		NameMatch = TRUE;
		break;
	    }
	  
	    /* Check all name aliases */
	    for (cpp = DevDefine->Aliases; !NameMatch && cpp && *cpp; ++cpp) {
		Alias = MkDevName(*cpp, DevData->DevUnit,
				  DevDefine->Type, DevDefine->Flags);
		if (EQ(Alias, ifreq->ifr_name)) {
		    NameMatch = TRUE;
		    break;
		}
	    }
	}
    }

    if (!NameMatch) {
	/* Device Not Found */
	SImsg(SIM_DBG, "%s: Could not find netif in ifconf table.", DevName);
	return((DevInfo_t *) NULL);
    }

    /* Use the system's canonical name */
    ProbeData->DevName = ifreq->ifr_name;
    if (!(DevInfo = DeviceCreate(ProbeData)))
	return((DevInfo_t *) NULL);

    DevInfo->Type = DT_NETIF;

    /*
     * Set address info
     */
    if (SetNetifAddrs(ifreq->ifr_name, SockDesc, &HostAddr, &MaskAddr) == 0) {
	/*
	 * Now get and call the Address Family specific routine
	 * to extract a NetIF_t.
	 */
	if (AddrFamPtr = GetAddrFamily(HostAddr.sin_family)) {
	    if (ni = (*AddrFamPtr->GetNetIF)(AddrFamPtr, &HostAddr, 
					     &MaskAddr)) {
		SetMacInfo(DevInfo, ni);
		DevInfo->DevSpec = (caddr_t *) ni;
	    }
	} else
	    SImsg(SIM_DBG, "Address family %d is not defined.", 
		  HostAddr.sin_family);
    }

    return(ProbeData->RetDevInfo = DevInfo);
}
#endif	/* GETNETIF_IFCONF */
