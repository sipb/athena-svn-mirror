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
 * BSD/OS specific functions
 */

#include "defs.h"
#include "myscsi.h"
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/sysctl.h>
#include <sys/device.h>
#include <sys/time.h>
#include <sys/types.h>
#include "/sys/dev/scsi/scsi_ioctl.h"	/* Need for SCSI INQUIRY */

/*
 * Top level device tree
 */
static char			        AllDevsSym[] = "_alldevs";

/*
 * For consistancy
 */
typedef struct device		        BSDdev_t;
static kvm_t			       *kd;

/*
 * Local globals
 */
static int				FloppyCtlrFlags;

/*
 * Platform Specific Interfaces
 */
extern char			       *GetModelSysinfo();
extern char			       *GetModelSysCtl();
PSI_t GetModelPSI[] = {			/* Get system Model */
    { GetModelSysinfo },
    { GetModelSysCtl },
    { NULL },
};
PSI_t GetSerialPSI[] = {		/* Get Serial Number */
    { NULL },
};
PSI_t GetRomVerPSI[] = {		/* Get ROM Version */
    { NULL },
};
PSI_t GetKernArchPSI[] = {		/* Get Kernel Architecture */
    { GetKernArchUname },
    { NULL },
};
PSI_t GetAppArchPSI[] = {		/* Get Application Architecture */
    { GetKernArchUname },		/* Not a typo */
    { NULL },
};
PSI_t GetCpuTypePSI[] = {		/* Get CPU Type */
    { GetModelSysCtl },			/* Yes, this is correct */
    { NULL },
};
extern char			       *GetNcpuSysCtl();
PSI_t GetNumCpuPSI[] = {		/* Get Number of CPU's */
    { GetNcpuSysCtl },
    { NULL },
};
extern char			       *GetKernVerSysCtl();
PSI_t GetKernVerPSI[] = {		/* Get Kernel Version */
    { GetKernVerSysCtl },
    { NULL },
};
PSI_t GetOSNamePSI[] = {		/* Get OS Name */
    { GetOSNameUname },
    { NULL },
};
extern char			       *GetOSDistSysCtl();
PSI_t GetOSDistPSI[] = {		/* Get OS Dist */
    { GetOSDistSysCtl },
    { NULL },
};
PSI_t GetOSVerPSI[] = {			/* Get OS Version */
    { GetOSVerUname },
    { NULL },
};
PSI_t GetManShortPSI[] = {		/* Get Short Man Name */
    { GetManShortSysinfo },
    { GetManShortDef },
    { NULL },
};
PSI_t GetManLongPSI[] = {		/* Get Long Man Name */
    { GetManLongSysinfo },
    { GetManLongDef },
    { NULL },
};
extern char			       *GetMemorySysCtl();
PSI_t GetMemoryPSI[] = {		/* Get amount of memory */
    { GetMemorySysCtl },
    { NULL },
};
extern char			       *GetVirtMemSysCtl();
PSI_t GetVirtMemPSI[] = {		/* Get amount of virtual memory */
    { GetVirtMemSysCtl },
    { NULL },
};
extern char			       *GetBootTimeSysCtl();
PSI_t GetBootTimePSI[] = {		/* Get System Boot Time */
    { GetBootTimeSysCtl },
    { NULL },
};

/*
 * Use sysctl() to get MODEL.
 */
extern char *GetModelSysinfo()
{
    static int			Speed;
    static char			SpeedSYM[] = "_cpuspeed";
    kvm_t		       *kd;
    nlist_t		       *nlPtr;

    if (!(kd = KVMopen()))
	return((char *) NULL);

    if ((nlPtr = KVMnlist(kd, SpeedSYM, (struct nlist *)NULL, 0)) == NULL)
	return((char *) NULL);

    if (CheckNlist(nlPtr))
	return((char *) NULL);

    if (KVMget(kd, (KVMaddr_t) nlPtr->n_value, (void *) &Speed, 
	       sizeof(Speed), KDT_DATA)) {
	SImsg(SIM_GERR, "Cannot read CPU speed from kernel.");
	return((char *) NULL);
    }

    KVMclose(kd);

    printf("CPU speed = %d\n", Speed);

    return(NULL);
}

/*
 * Issue a SCSI command and return results
 */
extern int ScsiCmd(ScsiCmd)
     ScsiCmd_t		       *ScsiCmd;
{
    static char			Buff[SCSI_BUF_LEN];
    ScsiCdbG0_t		       *CdbPtr = NULL;
    static scsi_user_cdb_t	Cmd;

    if (!ScsiCmd || !ScsiCmd->Cdb || !ScsiCmd->CdbLen || 
	ScsiCmd->DevFD < 0 || !ScsiCmd->DevFile) {
	SImsg(SIM_DBG, "ScsiCmd: Bad parameters.");
	return(-1);
    }

    (void) memset(Buff, 0, sizeof(Buff));
    (void) memset(&Cmd, 0, sizeof(Cmd));

    (void) memcpy(Cmd.suc_cdb, ScsiCmd->Cdb, ScsiCmd->CdbLen);
    Cmd.suc_cdblen = ScsiCmd->CdbLen;
    Cmd.suc_data = (caddr_t) Buff;
    Cmd.suc_datalen = sizeof(Buff);
    Cmd.suc_timeout = MySCSI_CMD_TIMEOUT;	/* suc_timeout == seconds */
    Cmd.suc_flags = SUC_READ;

    /*
     * Issue command to device
     */
    errno = 0;
    CdbPtr = (ScsiCdbG0_t *) ScsiCmd->Cdb;
    (void) ioctl(ScsiCmd->DevFD, SCSIRAWCDB, &Cmd);

    if (Cmd.suc_sus.sus_status != 0 || errno > 0)  {
	SImsg(SIM_GERR, "%s: ioctl SCSI cmd 0x%x failed: %s", 
	      ScsiCmd->DevFile, CdbPtr->cmd, SYSERR);
	return(-1);
    }

    ScsiCmd->Data = (void *) Buff;
    return(0);
}

/*
 * Get the root (top level) device.
 */
static BSDdev_t *DevGetRoot()
{
    struct nlist	       *nlPtr;
    static BSDdev_t		Root;
    static BSDdev_t	       *RootPtr;
    KVMaddr_t			Addr;

    if (RootPtr)
	return(&Root);

    if (!kd)
	if (!(kd = KVMopen()))
	    return((BSDdev_t *) NULL);

    if ((nlPtr = KVMnlist(kd, AllDevsSym, (struct nlist *)NULL, 0)) == NULL)
	return((BSDdev_t *) NULL);

    if (CheckNlist(nlPtr))
	return((BSDdev_t *) NULL);

    /*
     * Read pointer from kernel
     */
    Addr = nlPtr->n_value;
    if (KVMget(kd, Addr, (void *) &RootPtr, 
	       sizeof(BSDdev_t *), KDT_DATA)) {
	SImsg(SIM_GERR, 
	      "Cannot read Device Root Location (%s) root from kernel", 
	      AllDevsSym);
	return((BSDdev_t *) NULL);
    }

    if (KVMget(kd, (KVMaddr_t)RootPtr, (void *)&Root, 
	       sizeof(BSDdev_t), KDT_DATA)) {
	SImsg(SIM_GERR, "Cannot read root device info from kernel");
	return((BSDdev_t *) NULL);
    }

    return(&Root);
}

/*
 * Get the cfdata by reading into CFdata from Addr.
 */
static struct cfdata *DevGetCFdata(DevName, Addr, CFdata)
     char		       *DevName;
     struct cfdata	       *Addr;
     struct cfdata	       *CFdata;
{
    static char			DataName[128];

    if (!Addr) {
	SImsg(SIM_GERR, "No cfdata available from kernel for %s", DevName);
	return((struct cfdata *) NULL);
    }

    if (KVMget(kd, (KVMaddr_t) Addr, (void *) CFdata,
	       sizeof(struct cfdata), KDT_DATA)) {
	SImsg(SIM_GERR, "Cannot read device cfdata for %s.", DevName);
	return((struct cfdata *) NULL);
    }

    return(CFdata);
}

/*
 * Get the cfdriver by reading into CFdriver from Addr.
 */
static struct cfdriver *DevGetCFdriver(DevName, Addr, CFdriver)
     char		       *DevName;
     struct cfdriver	       *Addr;
     struct cfdriver	       *CFdriver;
{
    static char			DriverName[128];

    if (!Addr) {
	SImsg(SIM_GERR, "No cfdriver available from kernel for %s", DevName);
	return((struct cfdriver *) NULL);
    }

    if (KVMget(kd, (KVMaddr_t) Addr, (void *) CFdriver,
	       sizeof(struct cfdriver), KDT_DATA)) {
	SImsg(SIM_GERR, "Cannot read cfdriver for %s.", DevName);
	return((struct cfdriver *) NULL);
    }

    if (!CFdriver->cd_name) {
	SImsg(SIM_GERR, "No cfdriver.cd_name available from kernel for %s", 
	      DevName);
	return((struct cfdriver *) NULL);
    }
    if (KVMget(kd, (KVMaddr_t) CFdriver->cd_name, (void *) DriverName,
				sizeof(DriverName), KDT_STRING)) {
	SImsg(SIM_GERR, "Cannot read cfdriver.cd_name from kernel for %s",
	      DevName);
	return((struct cfdriver *) NULL);
    }
    CFdriver->cd_name = strdup(DriverName);

    return(CFdriver);
}

/*
 * Check a device and obtain all needed info and the call ProbeDevice.
 */
static void DevCheckDevice(BSDdevice, Parent, TreePtr, SearchNames)
    BSDdev_t		       *BSDdevice;
    BSDdev_t		       *Parent;
    DevInfo_t 		      **TreePtr;
    char		      **SearchNames;
{
    DevInfo_t		       *DevInfo;
    static DevData_t		DevData;
    char		       *DevName;
    static struct cfdata	CFdata;
    static struct cfdriver      CFdriver;

    if (!BSDdevice)
	return;

    DevName = BSDdevice->dv_xname;

    (void) memset((void *) &DevData, 0, sizeof(DevData));
    DevData.NodeID = (int) BSDdevice;	/* Fake up a NodeID */
    DevData.Slave = -1;
    DevData.DevNum = -1;
    DevData.Flags |= DD_IS_ALIVE;

    /*
     * Get the Device basename by looking up the driver entry.
     */
    if (!DevGetCFdata(DevName, BSDdevice->dv_cfdata, &CFdata))
	return;
    if (!DevGetCFdriver(DevName, CFdata.cf_driver, &CFdriver))
	return;
    DevData.DevName = strdup(CFdriver.cd_name);
    DevData.DevUnit = BSDdevice->dv_unit;
    DevData.OSflags = CFdata.cf_flags;

    /*
     * Now get and set our Parent's data
     */
    if (Parent) {
	/*
	 * Get the Parent's Device basename by looking up the driver entry.
	 */
	if (!DevGetCFdata(Parent->dv_xname, Parent->dv_cfdata, &CFdata))
	    return;
	if (!DevGetCFdriver(Parent->dv_xname, CFdata.cf_driver, 
			   &CFdriver))
	    return;
	DevData.CtlrName = strdup(CFdriver.cd_name);
	DevData.CtlrUnit = Parent->dv_unit;
    }

    SImsg(SIM_DBG, 
          "\tDevCheckDevice: <%s> name = <%s> unit = %d Parent=<%s> PaUnit=%d",
	  DevName,
	  DevData.DevName, DevData.DevUnit, 
	  (DevData.CtlrName) ? DevData.CtlrName : "", DevData.CtlrUnit);

    /* Probe and add device */
    if (TreePtr && (DevInfo = (DevInfo_t *) 
		    ProbeDevice(&DevData, TreePtr, SearchNames, NULL))) {
	AddDevice(DevInfo, TreePtr, SearchNames);
    }
}

/*
 * Recursively traverse and descend the KDT
 */
static int DevTraverse(BSDdevice, Parent, TreePtr, SearchNames)
    BSDdev_t		       *BSDdevice;
    BSDdev_t		       *Parent;
    DevInfo_t 		      **TreePtr;
    char		      **SearchNames;
{
    char		       *DevName;
    BSDdev_t		       *DevParent = NULL;
    BSDdev_t		       *Sibling;

    if (!BSDdevice)
	return(-1);

    DevName = BSDdevice->dv_xname;

    /*
     * Read in the Device's Parent
     */
    if (BSDdevice->dv_parent) {
	DevParent = (BSDdev_t *) xcalloc(1, sizeof(BSDdev_t));
	if (KVMget(kd, (KVMaddr_t) BSDdevice->dv_parent, (void *) DevParent,
		   sizeof(BSDdev_t), KDT_DATA))
	    SImsg(SIM_GERR, "Cannot read parent device for %s: %s", 
		  DevName, SYSERR);
    }

    SImsg(SIM_DBG, "FOUND <%s> on <%s> class=%d unit=%d flags=0x%x",
	  DevName, (DevParent) ? DevParent->dv_xname : "NONE",
	  BSDdevice->dv_class, BSDdevice->dv_unit, BSDdevice->dv_flags);

    /*
     * Check the device
     */
    DevCheckDevice(BSDdevice, DevParent, TreePtr, SearchNames);

    /*
     * If this node has a sibling, read the sibling data from kernel space
     * and TRAVERSE.
     */
    if (BSDdevice->dv_next) {
	Sibling = (BSDdev_t *) xcalloc(1, sizeof(BSDdev_t));
	if (KVMget(kd, (KVMaddr_t) BSDdevice->dv_next, (void *) Sibling,
		   sizeof(BSDdev_t), KDT_DATA)) {
	    SImsg(SIM_GERR, "Cannot read sibling data for %s.", DevName);
	} else {
	    DevTraverse(Sibling, Parent, TreePtr, SearchNames);
	}
	(void) free(Sibling);
    }
    (void) free(DevParent);

    return(0);
}

/*
 * Build device tree and place in TreePtr.
 */
extern int BuildDevices(TreePtr, SearchNames)
    DevInfo_t 		       **TreePtr;
    char		       **SearchNames;
{
    BSDdev_t		       *Root;

    if (Root = DevGetRoot())
	return(DevTraverse(Root, NULL, TreePtr, SearchNames));

    return(0);
}

/*
 * Probe a Floppy Controller
 */
extern DevInfo_t *ProbeFloppyCtlr(ProbeData)
     ProbeData_t	       *ProbeData;
{
    /*
     * All we need is the OSflags values for use by ProbeFloppy()
     */
    if (ProbeData && ProbeData->DevData)
	FloppyCtlrFlags = ProbeData->DevData->OSflags;

    return DeviceCreate(ProbeData);
}

/*
 * Probe a Floppy Drive
 */
extern DevInfo_t *ProbeFloppy(ProbeData)
     ProbeData_t	       *ProbeData;
{
    DevInfo_t		       *DevInfo;
    Define_t		       *Define;
    int				Type = 0;
    int				Capacity;
    int				Mask;
    static char			Buff[128];

    DevInfo = DeviceCreate(ProbeData);

    if (FloppyCtlrFlags) {
	/* See floppy(4) for where we get Mask */
	if (DevInfo->Unit == 0)
	    Mask = 0x0f;
	else
	    Mask = 0xf0;
	Type = FloppyCtlrFlags & Mask;
    }
    if (Define = DefGet("FloppyTypes", NULL, Type, 0)) {
	DevInfo->Model = Define->ValStr2;
	Capacity = atoi(Define->ValStr1);
	if (Capacity < 1000)
	    (void) snprintf(Buff, sizeof(Buff), "%d KB", Capacity);
	else
	    (void) snprintf(Buff, sizeof(Buff), "%.02f MB", 
			    (float) ((float) Capacity / (float) 1000));
	DevInfo->ModelDesc = strdup(Buff);
    } else
	SImsg(SIM_DBG, "Unknown FloppyTypes value %d", Type);

    return(DevInfo);
}

/*
 * Initialize the OS specific parts of the Device Types table
 */
void DevTypesInit()
{
    register int		i;

    for (i = 0; DevTypes[i].Name; ++i)
	switch (DevTypes[i].Type) {
	case DT_DISKDRIVE:	DevTypes[i].Probe = ProbeDiskDrive;	break;
	case DT_CDROM:		DevTypes[i].Probe = ProbeDiskDrive;	break;
	case DT_TAPEDRIVE:	DevTypes[i].Probe = ProbeTapeDrive;	break;
	case DT_NETIF:		DevTypes[i].Probe = ProbeNetif;		break;
	case DT_FLOPPY:		DevTypes[i].Probe = ProbeFloppy;	break;
	case DT_FLOPPYCTLR:	DevTypes[i].Probe = ProbeFloppyCtlr;	break;
#ifdef notdef
	case DT_FRAMEBUFFER:	DevTypes[i].Probe = ProbeFrameBuffer;	break;
	case DT_KEYBOARD:	DevTypes[i].Probe = ProbeKbd;		break;
#endif
	}
}

#ifdef notdef
static u_short GetNetIfIndex(DevName)
     char		       *DevName;
{
    static struct ifnet	        IfNet;
    struct ifnet	       *IfNetPtr;
    struct ifnet	       *Ptr;
    kvm_t		       *kd;
    nlist_t		       *nlPtr;
    char		        IfNetSYM[] = "_ifnet";
    char			IfName[128];
    u_short			IfIndex = 0;

    if (!(kd = KVMopen()))
	return(0);

    if ((nlPtr = KVMnlist(kd, IfNetSYM, (struct nlist *)NULL, 0)) == NULL)
	return(0);

    if (CheckNlist(nlPtr))
	return(0);

    if (KVMget(kd, (KVMaddr_t) nlPtr->n_value, (void *) &IfNetPtr, 
	       sizeof(struct ifnet *), KDT_DATA)) {
	SImsg(SIM_GERR, "Cannot read ifnet pointer from kernel.");
	return(0);
    }

    while (IfNetPtr) {
	if (KVMget(kd, (KVMaddr_t) IfNetPtr, (void *) &IfNet, 
		   sizeof(IfNet), KDT_DATA)) {
	    SImsg(SIM_GERR, "Cannot read ifnet from kernel.");
	    break;
	}
	if (KVMget(kd, (KVMaddr_t) IfNet.if_name, (void *) &IfName, 
		   sizeof(IfName), KDT_STRING)) {
	    SImsg(SIM_GERR, "Cannot read if_name from kernel.");
	    break;
	}
	(void) strcat(IfName, itoa(IfNet.if_unit));

	if (EQ(IfName, DevName)) {
	    IfIndex = IfNet.if_index;
	    break;
	}

	IfNetPtr = IfNet.if_next;
    }

    KVMclose(kd);

    return(IfIndex);
}
#endif

extern void SetMacInfoBSDOS(DevInfo, NetIf)
     DevInfo_t		       *DevInfo;
     NetIF_t 		       *NetIf;
{
#ifdef notdef
    static char			Ether[128];
    int				Query[3];
    static struct sockaddr_dl	SockAddrDL;
    struct sockaddr_dl	       *sdl;
    size_t			Len = sizeof(SockAddrDL);
    register char	       *cp;
    register int		i;
    u_short			IfIndex;

    if ((IfIndex = GetNetIfIndex(DevInfo->Name)) == 0) {
	SImsg(SIM_GERR, "%s: Cannot find interface index number.",
	      DevInfo->Name);
	return;
    }
    Query[0] = CTL_NET;
    Query[1] = PF_LINK;
    Query[2] = IfIndex;

    if (sysctl(Query, sizeof(Query), &SockAddrDL, &Len, NULL, 0) == -1) {
	SImsg(SIM_GERR, "%s: sysctl PF_LINK failed: %s", 
	      DevInfo->Name, SYSERR);
	return;
    }

    sdl = &SockAddrDL;
    cp = (char *) LLADDR(sdl);
    if (sdl->sdl_alen > 0 && sdl->sdl_type == IFT_ETHER) {
	(void) snprintf(Ether, sizeof(Ether), "%2x:%2x:%2x:%2x:%2x:%2x",
			cp[0] & 0xff, cp[1] & 0xff, cp[2] & 0xff, 
			cp[3] & 0xff, cp[4] & 0xff, cp[5] & 0xff);
	NetIf->MACaddr = strdup(Ether);
	return;
    }
#endif
}
