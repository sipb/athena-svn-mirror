/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: netif.c,v 1.1.1.1 1996-10-07 20:16:53 ghudson Exp $";
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
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/if_ether.h>
#include <netdb.h>

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
    struct in_ifaddr 	in_ifaddr;
} 				ifaddress;

/*
 * Create a DevInfo_t for a network interface.
 */
static DevInfo_t *CreateNetif(FullName, IfNet, DevData, DevDefine)
    char		       *FullName;
    struct ifnet               *IfNet;
    DevData_t		       *DevData;
    DevDefine_t		       *DevDefine;
{
    DevInfo_t		       *DevInfo;

    DevInfo = NewDevInfo(NULL);

    DevInfo->Name 	= strdup(FullName);
    DevInfo->Type 	= DT_NETIF;
    if (IfNet)
	DevInfo->Unit 	= IfNet->if_unit;
    if (DevData) {
	DevInfo->NodeID = DevData->NodeID;
	DevInfo->Master = MkMasterFromDevData(DevData);
    }

#if	defined(HAVE_IF_VERSION)
    if (IfNet->if_version && IfNet->if_version[0])
	DevInfo->Model 	= strdup(IfNet->if_version);
    else
#endif	/* HAVE_IF_VERSION */
	if (DevDefine)
	    DevInfo->Model 	= DevDefine->Model;

    if (DevDefine)
	DevInfo->ModelDesc = DevDefine->Desc;

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

    if (in_addr.s_addr = ntohl(inaddr)) {
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
	np = GetNet((u_long) htonl(hostaddr->sin_addr.s_addr), 
		    (u_long) htonl(maskaddr->sin_addr.s_addr));
	in_addr.s_addr = ntohl(hostaddr->sin_addr.s_addr & 
			       maskaddr->sin_addr.s_addr);
	ni->NetAddr = strdup(inet_ntoa(in_addr));
	sin = hostaddr;
    } else {
	np = GetNet((u_long) htonl(ifaddress.in_ifaddr.ia_subnet), 
		    (u_long) ifaddress.in_ifaddr.ia_subnetmask);
	in_addr.s_addr = ntohl(ifaddress.in_ifaddr.ia_subnet);
	ni->NetAddr = strdup(inet_ntoa(in_addr));
	sin = (struct sockaddr_in *) &ifaddress.in_ifaddr.ia_addr;
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
	    Error("cannot read if address");
	    continue;
	}

	/*
	 * Now get and call the Address Family specific routine
	 * to extract a NetIF_t.
	 */
	if (AddrFamPtr = GetAddrFamily(ifaddress.ifaddr.ifa_addr.sa_family)) {
	    if (ni = (*AddrFamPtr->GetNetIF)(AddrFamPtr, NULL, NULL))
		SetMacInfo(FullName, ni, DevInfo);
	} else {
	    if (Debug) Error("Address family %d is not defined.", 
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
extern DevInfo_t *ProbeNetif(name, DevData, DevDefine)
    char 		       *name;
    DevData_t 		       *DevData;
    DevDefine_t	 	       *DevDefine;
{
    DevInfo_t 		       *DevInfo = NULL;
    static struct ifnet         ifnet;
    static char 		ifname[16], FullName[17];
    nlist_t		       *nlptr;
    register char	       *p;
    u_long 		        ifnetaddr;
    kvm_t 		       *kd;

    if (Debug) printf("ProbeNetif() '%s'\n", name);

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
	if (Debug) Error("kvm_read ifnetaddr failed");
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
	    if (Debug) Error("kvm_read ifnetaddr ifnet failed");
	    continue;
	}

	/*
	 * Read if_name from kernel space
	 */
	if (KVMget(kd, (u_long)ifnet.if_name, ifname, 
		   sizeof(ifname), KDT_STRING)) {
	    if (Debug) Error("kvm_read ifname failed");
	    continue;
	}

	/*
	 * Copy if_name to 'FullName' and add unit number
	 */
	(void) sprintf(FullName, "%s%d", ifname, ifnet.if_unit);

	/*
	 * Check to see if this is the interface we want.
	 */
	if (!EQ(FullName, name))
	    continue;

	/*
	 * Create and set device
	 */
	DevInfo = CreateNetif(FullName, &ifnet, DevData, DevDefine);

	/*
	 * Get and set address info
	 */
	if (ifnet.if_addrlist) {
	    NetIF_t 	       *ni;

	    if (ni = GetNetifAddrs(kd, (off_t) ifnet.if_addrlist, 
				   FullName, DevInfo))
		DevInfo->DevSpec = (caddr_t *) ni;
	}
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
	if (Debug) Error("%s: ioctl SIOCGIFADDR failed: %s.", ifname, SYSERR);
	return(-1);
    }
    memcpy((char *)hostaddr, (char *)&ifr.ifr_addr, 
	   sizeof(struct sockaddr_in));

    /*
     * Get the network mask
     */
    if (ioctl(sock, SIOCGIFNETMASK, (char *) &ifr) < 0) {
	if (Debug) Error("%s: ioctl SIOCGIFNETMASK failed: %s.", 
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
extern DevInfo_t *ProbeNetif(name, DevData, DevDefine)
    char 		       *name;
    DevData_t 		       *DevData;
    DevDefine_t	 	       *DevDefine;
{
    DevInfo_t 		       *DevInfo = NULL;
    NetIF_t	 	       *ni;
    struct ifreq	       *ifreq;
    static struct ifconf	ifconf;
    static struct sockaddr_in	hostaddr;
    static struct sockaddr_in	maskaddr;
    static char			reqbuf[BUFSIZ];
    static char			buff[BUFSIZ];
    static int			sock = -1;
    register int		n;
    AddrFamily_t	       *AddrFamPtr;

    if (Debug) printf("ProbeNetif() '%s'\n", name);

    if (sock < 0) {
	/*
	 * Get list of all interfaces
	 */
	ifconf.ifc_len = sizeof(reqbuf);
	ifconf.ifc_buf = reqbuf;

	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	    if (Debug) Error("Cannot create socket: %s.", SYSERR);
	    return((DevInfo_t *)NULL);
	}

	if (ioctl(sock, SIOCGIFCONF, (char *) &ifconf) < 0) {
	    if (Debug) Error("%s: ioctl SIOCGIFCONF failed: %s.", name,SYSERR);
	    return((DevInfo_t *)NULL);
	}
    }

    /*
     * Iterate over all known interfaces
     */
    for (ifreq = ifconf.ifc_req, n = ifconf.ifc_len/sizeof(struct ifreq);
	 --n >= 0; ifreq++) {

	/*
	 * Check to see if this is the interface we want.
	 */
	if (!EQ(name, ifreq->ifr_name))
	    continue;

	/*
	 * Create and set device
	 */
	DevInfo 		= NewDevInfo(NULL);
	DevInfo->Name 		= strdup(ifreq->ifr_name);
	DevInfo->Type 		= DT_NETIF;
	if (DevData) {
	    DevInfo->NodeID 	= DevData->NodeID;
	    DevInfo->Unit 	= DevData->DevUnit;
	    DevInfo->Master 	= MkMasterFromDevData(DevData);
	}
	if (DevDefine) {
	    DevInfo->Model 	= DevDefine->Model;
	    DevInfo->ModelDesc 	= DevDefine->Desc;
	}

	/*
	 * Set address info
	 */
	if (SetNetifAddrs(ifreq->ifr_name, sock, &hostaddr, &maskaddr) == 0) {
	    /*
	     * Now get and call the Address Family specific routine
	     * to extract a NetIF_t.
	     */
	    if (AddrFamPtr = GetAddrFamily(hostaddr.sin_family)) {
		if (ni = (*AddrFamPtr->GetNetIF)(AddrFamPtr, &hostaddr, 
						 &maskaddr)) {
		    SetMacInfo(name, ni, DevInfo);
		    DevInfo->DevSpec = (caddr_t *) ni;
		}
	    } else
		if (Debug) Error("Address family %d is not defined.", 
				 hostaddr.sin_family);
	}

	return(DevInfo);
    }

    return((DevInfo_t *)NULL);
}
#endif	/* GETNETIF_IFCONF */
