/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: macinfo.c,v 1.1.1.1 1996-10-07 20:16:51 ghudson Exp $";
#endif

/*
 * Media Access Control (MAC) info routines
 */

#include "defs.h"

#if	GETMAC_TYPE == GETMAC_IFREQ_ENADDR

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <net/if.h>

/*
 * Find and set the MAC info using ifreq.ifr_enaddr
 */
extern void SetMacInfo(DevName, NetIf)
    char 		       *DevName;
    NetIF_t 		       *NetIf;
{
    struct ifreq	        ifreq;
    struct ether_addr	       *EtherAddr;
    struct ether_addr	       *ether_aton();
    static char			Buff[BUFSIZ];
    static int			Sock = -1;

    if (!NetIf)
	return;

    Sock = socket(AF_INET, SOCK_STREAM, 0);
    if (Sock < 0) {
	if (Debug) Error("Create AF_INET SOCK_STREAM failed: %s", SYSERR);
	return;
    }

    strncpy(ifreq.ifr_name, DevName, sizeof(ifreq.ifr_name));
    if (ioctl(Sock, SIOCGENADDR, &ifreq) < 0) {
	if (Debug) Error("%s: ioctl SIOCGENADDR failed: %s.", 
			 ifreq.ifr_name, SYSERR);
	close(Sock);
	return;
    }
    close(Sock);

    NetIf->MACaddr = strdup(ifreq.ifr_enaddr);
    EtherAddr = ether_aton(ifreq.ifr_enaddr);
    if (EtherAddr && ether_ntohost(Buff, EtherAddr) == 0)
	NetIf->MACname = strdup(Buff);
}
#endif	/* HAVE_IFREQ_ENADDR */

#if	GETMAC_TYPE == GETMAC_NIT

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/nit_if.h>

/*
 * Find and set the MAC info using the Network Interface Tap (NIT)
 */
extern void SetMacInfo(DevName, NetIf)
    char 		       *DevName;
    NetIF_t 		       *NetIf;
{
    register struct sockaddr   *SockAddr;
    struct ifreq 	        ifreq;
    char 		       *ether_ntoa(), Buf[MAXHOSTNAMLEN+1];
    int 		        Desc;

    if (!NetIf)
	return;

    if ((Desc = open("/dev/nit", O_RDONLY)) == SYSFAIL) {
	if (Debug) Error("open /dev/nit failed");
	return;
    }

    /*
     * Bind to NIT for DevName
     */
    strncpy(ifreq.ifr_name, DevName, sizeof ifreq.ifr_name);
    if (ioctl(Desc, NIOCBIND, (caddr_t) &ifreq) < 0) {
	if (Debug) Error("ioctl:  NIOCBIND");
	return;
    }

    /*
     * Get address
     */
    if (ioctl(Desc, SIOCGIFADDR, (caddr_t)&ifreq) < 0) {
	if (Debug) Error("ioctl (SIOCGIFADDR)");
	return;
    }

    (void) close(Desc);

    SockAddr = (struct sockaddr *)&ifreq.ifr_addr;
    NetIf->MACaddr = strdup(ether_ntoa((struct ether_addr *) 
					  SockAddr->sa_data));

    if (ether_ntohost(Buf, (struct ether_addr *) SockAddr->sa_data) == 0)
	NetIf->MACname = strdup(Buf);
}
#endif	/* HAVE_NIT */

#if	GETMAC_TYPE == GETMAC_DLPI

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/dlpi.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include "dlpi.h"

/*
 * Find and set the MAC info using the Data Link Provider Interface (DLPI)
 */
extern void SetMacInfo(DevName, NetIf, DevInfo)
    char 		       *DevName;
    NetIF_t 		       *NetIf;
    DevInfo_t		       *DevInfo;
{
    char 		        buff[MAXHOSTNAMLEN+1];
    int 		        fd;
    long			dlbuf[MAXDLBUF];
    static char			devname[MAXPATHLEN];
    static struct ether_addr	ether_addr;
    register char	       *cp;
    union DL_primitives	       *dlp;
    u_char			addr[MAXDLADDR];

    if (!NetIf)
	return;

    dlp = (union DL_primitives *) dlbuf;
    (void) sprintf(devname, "%s/%s", _PATH_DEV, DevName);
    /*
     * Remove unit part of name from the device name.
     */
    if (cp = strrchr(devname, '/')) {
	++cp;
	while (!isdigit(*cp) && ++cp);
	if (isdigit(*cp))
	    *cp = CNULL;
    }

    if ((fd = open(devname, O_RDWR)) == SYSFAIL) {
	if (Debug) Error("Cannot open %s: %s.", devname, SYSERR);
	return;
    }

    /*
     * Setup
     */
    dlattachreq(fd, DevInfo->Unit);
    dlokack(fd, dlbuf);
    dlbindreq(fd, 0, 0, DL_CLDLS, 0, 0);
    dlbindack(fd, dlbuf);

    /*
     * Get current physical address
     */
    dlphysaddrreq(fd, DL_CURR_PHYS_ADDR);
    dlphysaddrack(fd, dlbuf);

    addrtostring(OFFADDR(dlp, dlp->physaddr_ack.dl_addr_offset),
		 dlp->physaddr_ack.dl_addr_length, addr);

    NetIf->MACaddr = strdup((char *)addr);

    memcpy((char *) ether_addr.ether_addr_octet, 
	   (char *) OFFADDR(dlp, dlp->physaddr_ack.dl_addr_offset), 
	   dlp->physaddr_ack.dl_addr_length);
    if (ether_ntohost(buff, &ether_addr) == 0)
	NetIf->MACname = strdup(buff);

    /*
     * Get factory physical address
     */
    dlphysaddrreq(fd, DL_FACT_PHYS_ADDR);
    dlphysaddrack(fd, dlbuf);

    addrtostring(OFFADDR(dlp, dlp->physaddr_ack.dl_addr_offset),
		 dlp->physaddr_ack.dl_addr_length, addr);

    NetIf->FacMACaddr = strdup((char *)addr);

    memcpy((char *) ether_addr.ether_addr_octet, 
	   (char *) OFFADDR(dlp, dlp->physaddr_ack.dl_addr_offset), 
	   dlp->physaddr_ack.dl_addr_length);
    if (ether_ntohost(buff, &ether_addr) == 0)
	NetIf->FacMACname = strdup(buff);

    /*
     * We're done
     */
    dlunbindreq(fd);
    dldetachreq(fd);
    (void) close(fd);
}
#endif	/* HAVE_DLPI */

#if	defined(HAVE_PACKETFILTER)

#include <sys/time.h>
#include <net/pfilt.h>

#include <fcntl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>

#if	defined(NEED_ETHER_ADDR)
/*
 * This didn't appear in <netinet/if_ether.h> until Ultrix 4.2 (4.1?)
 */
struct ether_addr {
	u_char	ether_addr_octet[6];
};
#endif	/* NEED_ETHER_ADDR */

/*
 * Get network type information
 */
static char *GetNetType(type)
    int				type;
{
    register int		i;
    Define_t		       *Define;

    Define = DefGet(DL_NETTYPE, NULL, (long) type, 0);
    if (Define && Define->ValStr1)
	return(Define->ValStr1);

    return((char *) NULL);
}

/*
 * Find and set the MAC info using the Packet Filter
 */
extern void SetMacInfo(DevName, Netif, DevInfo)
     char 		       *DevName;
     NetIF_t 		       *Netif;
     DevInfo_t		       *DevInfo;
{
    struct endevp		endevp;
    struct ether_addr		ether_addr;
    char 		       *ether_ntoa(), HostBuf[MAXHOSTNAMLEN+1];
    char 		       *p;
    int 		        Desc;

    if (!DevName || !Netif)
	return;

    /*
     * Open this device using the packet filter
     */
    if ((Desc = pfopen(DevName, O_RDONLY)) < 0) {
	if (Debug) Error("pfopen %s failed: %s.", DevName, SYSERR);
	return;
    }

    /*
     * Retrieve info
     */
    if (ioctl(Desc, EIOCDEVP, &endevp) < 0) {
	if (Debug) Error("ioctl EIOCDEVP of %s failed: %s.", DevName, SYSERR);
	return;
    }

    close(Desc);

    /*
     * Convert address into ethers(5) format
     */
    memcpy((char *) ether_addr.ether_addr_octet, (char *) endevp.end_addr,
	   endevp.end_addr_len);

    /*
     * Set what we now know.
     */
    if (p = ether_ntoa(&ether_addr))
	Netif->MACaddr = strdup(p);

    if (ether_ntohost(HostBuf, &ether_addr) == 0)
	Netif->MACname = strdup(HostBuf);

    if (DevInfo && (p = GetNetType(endevp.end_dev_type)))
	AddDevDesc(DevInfo, p, NULL, DA_INSERT|DA_PRIME);
}
#endif	/* HAVE_PACKETFILTER */

#if	!defined(GETMAC_TYPE)
extern void SetMacInfo(DevName, NetIf)
    /*ARGSUSED*/
    char 		       *DevName;
    NetIF_t 		       *NetIf;
{
    /* Do Nothing */
}
#endif	/* !GETMAC_TYPE */
