/*
 * Copyright (c) 1992-1998 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not
 * sold for profit or used in part or in whole for commercial gain
 * without prior written agreement, and the author is credited
 * appropriately.
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * sysctl(3) functions common to BSD implimentations (BSD/OS, FreeBSD, etc.)
 */

#include "defs.h"
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/socket.h>		/* BSD/OS 3.1 */
#include <net/if.h>
#include <net/if_dl.h>
#include <net/route.h>
#if	defined(freebsd)
#include <net/ethernet.h>
#endif	/* freebsd */
#if	defined(VM_SWAPSTATS)
#include <vm/swap_pager.h>
#endif	/* VM_SWAPSTATS */

/*
 * Use sysctl() to get MODEL.
 */
extern char *GetModelSysCtl()
{
    int				Query[2];
    static char			Model[128];
    size_t			Length = sizeof(Model);

    Query[0] = CTL_HW;
    Query[1] = HW_MODEL;
    if (sysctl(Query, 2, Model, &Length, NULL, 0) == -1) {
	SImsg(SIM_GERR, "sysctl CTL_HW.HW_MODEL failed: %s", SYSERR);
	return((char *) NULL);
    }

    return(Model);
}

/*
 * Use sysctl() to get Number of CPUs
 */
extern char *GetNcpuSysCtl()
{
    int				Query[2];
    int				NumCpu = 0;
    size_t			Length = sizeof(NumCpu);

    Query[0] = CTL_HW;
    Query[1] = HW_NCPU;
    if (sysctl(Query, 2, &NumCpu, &Length, NULL, 0) == -1) {
	SImsg(SIM_GERR, "sysctl CTL_HW.HW_NCPU failed: %s", SYSERR);
	return((char *) NULL);
    }

    return(itoa(NumCpu));
}

/*
 * Use sysctl() to get the Kernel Version
 */
extern char *GetKernVerSysCtl()
{
    int				Query[2];
    static char			Version[256];
    size_t			Length = sizeof(Version);

    Query[0] = CTL_KERN;
    Query[1] = KERN_VERSION;
    if (sysctl(Query, 2, Version, &Length, NULL, 0) == -1) {
	SImsg(SIM_GERR, "sysctl CTL_KERN.KERN_MODEL failed: %s", SYSERR);
	return((char *) NULL);
    }

    return(Version);
}

/*
 * Use sysctl() to get the OS Distribution
 */
extern char *GetOSDistSysCtl()
{
    int				Query[2];
    int				Dist = 0;
    size_t			Length = sizeof(Dist);

    Query[0] = CTL_KERN;
    Query[1] = KERN_OSREV;
    if (sysctl(Query, 2, &Dist, &Length, NULL, 0) == -1) {
	SImsg(SIM_GERR, "sysctl CTL_KERN.KERN_OSREV failed: %s", SYSERR);
	return((char *) NULL);
    }

    return(itoa(Dist));
}

/*
 * Use sysctl() to get the Memory
 */
extern char *GetMemorySysCtl()
{
    int				Query[2];
    int				Memory = 0;
    Large_t			Amount;
    size_t			Length = sizeof(Memory);

    Query[0] = CTL_HW;
    Query[1] = HW_PHYSMEM;
    if (sysctl(Query, 2, &Memory, &Length, NULL, 0) == -1) {
	SImsg(SIM_GERR, "sysctl CTL_HW.HW_PHYSMEM failed: %s", SYSERR);
	return((char *) NULL);
    }

    Amount = DivRndUp(Memory, (Large_t)MBYTES);

    return(GetMemoryStr(Amount));
}

/*
 * Use sysctl() to get the Virtual Memory
 */
extern char *GetVirtMemSysCtl()
{
#if	defined(VM_SWAPSTATS)
    int				Query[2];
    struct swapstats		Swap;
    size_t			Length = sizeof(Swap);
    Large_t			Amount;

    Query[0] = CTL_VM;
    Query[1] = VM_SWAPSTATS;
    if (sysctl(Query, 2, &Swap, &Length, NULL, 0) == -1) {
	SImsg(SIM_GERR, "sysctl CTL_Vm.VM_SWAPSTATS failed: %s", SYSERR);
	return((char *) NULL);
    }

    Amount = DivRndUp(Swap.swap_total * 512, (Large_t)MBYTES);

    return(GetMemoryStr(Amount));
#else
    return((char *) NULL);
#endif	/* VM_SWAPSTATS */
}

/*
 * Use sysctl() to get the Boot Time
 */
extern char *GetBootTimeSysCtl()
{
    int				Query[2];
    struct timeval		TimeVal;
    time_t			Time;
    size_t			Length = sizeof(TimeVal);
    char		       *TimeStr;
    register char	       *cp;

    Query[0] = CTL_KERN;
    Query[1] = KERN_BOOTTIME;
    if (sysctl(Query, 2, &TimeVal, &Length, NULL, 0) == -1) {
	SImsg(SIM_GERR, "sysctl CTL_KERN.KERN_BOOTTIME failed: %s", SYSERR);
	return((char *) NULL);
    }

    Time = (time_t) TimeVal.tv_sec;
    TimeStr = ctime(&Time);
    if (cp = strchr(TimeStr, '\n'))
	*cp = CNULL;

    return(TimeStr);
}

/*
 * Use sysctl(3) to retrieve the MAC Info for a network interface.
 * This is based on code from FreeBSD 2.2.6 ifconfig(8).
 * This is totally convoluted!  Yuck!
 */
extern void SetMacInfoSysCtl(DevInfo, NetIf)
     DevInfo_t		       *DevInfo;
     NetIF_t		       *NetIf;
{
    static char			Ether[128];
    static char			EtherName[128];
#if	defined(HAVE_ETHER_NTOHOST)
    struct ether_addr		EtherAddr;
#endif	/* HAVE_ETHER_NTOHOST */
    register char	       *cp;
    register int		n;
    register int		Len;
    struct if_msghdr 	       *ifm, *nextifm;
    struct ifa_msghdr 	       *ifam;
    struct sockaddr_dl 	       *sdl;
    char		       *buf, *lim, *next;
    size_t 			needed;
    int 			mib[6];

    mib[0] = CTL_NET;
    mib[1] = PF_ROUTE;
    mib[2] = 0;
    mib[3] = AF_INET;	/* address family */
    mib[4] = NET_RT_IFLIST;
    mib[5] = 0;
    if (sysctl(mib, 6, NULL, &needed, NULL, 0) < 0) {
	SImsg(SIM_GERR, "%s: sysctl get iflist estimate failed: %s",
	      DevInfo->Name, SYSERR);
	return;
    }
    buf = (char *) xmalloc(needed);
    if (sysctl(mib, 6, buf, &needed, NULL, 0) < 0) {
	SImsg(SIM_GERR, "%s: sysctl retrieve iflist table failed: %s",
	      DevInfo->Name, SYSERR);
	return;
    }
    lim = buf + needed;
    
    next = buf;
    while (next < lim) {
	
	ifm = (struct if_msghdr *)next;
	
	if (ifm->ifm_type == RTM_IFINFO) {
	    sdl = (struct sockaddr_dl *)(ifm + 1);
	} else {
	    SImsg(SIM_GERR, 
	    "%s: Cannot get MAC info: out of sync parsing NET_RT_IFLIST.", 
		  DevInfo->Name);
	    return;
	}
	
	next += ifm->ifm_msglen;
	ifam = NULL;
	while (next < lim) {
	    
	    nextifm = (struct if_msghdr *)next;
	    
	    if (nextifm->ifm_type != RTM_NEWADDR)
		break;
	    
	    if (ifam == NULL)
		ifam = (struct ifa_msghdr *)nextifm;
	    
	    next += nextifm->ifm_msglen;
	}

	if (EQN(DevInfo->Name, sdl->sdl_data, sdl->sdl_nlen)) {
	    /* Found it */
	    cp = (char *)LLADDR(sdl);
	    if ((n = sdl->sdl_alen) > 0) {
		Ether[0] = CNULL;
             	while (--n >= 0) {
		    Len = strlen(Ether);
		    (void) snprintf(&Ether[Len], sizeof(Ether)-Len,
				    "%02x%s", *cp++ & 0xff, n > 0 ? ":" : "");
		}

		NetIf->MACaddr = strdup(Ether);
#if	defined(HAVE_ETHER_NTOHOST)
		memcpy(&EtherAddr.octet, LLADDR(sdl), ETHER_ADDR_LEN);
		if (ether_ntohost(EtherName, &EtherAddr) == 0)
		    NetIf->MACname = strdup(EtherName);
#endif	/* HAVE_ETHER_NTOHOST */
	    }
	    break;
	}
    }
    (void) free(buf);
}
