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
 * Kernel Device Tree routines
 */

#include "defs.h"
#include "sunos-kdt.h"
#include "sunos-obp.h"

extern char		 	KDTsymbol[];
static kvm_t		       *kd = NULL;
static int			KDTtraverse();

/*
 * Get the root (top) node from the KDT
 */
static KDTdevinfo_t *KDTgetRoot()
{
    struct nlist	       *nlptr;
    static KDTdevinfo_t 	Root, *PtrRoot = NULL;
    u_long 			Addr;

    /*
     * See if we've already gotten Root
     */
    if (PtrRoot)
	return(&Root);

    if (!kd)
	if (!(kd = KVMopen()))
	    return((KDTdevinfo_t *) NULL);

    if ((nlptr = KVMnlist(kd, KDTsymbol, (struct nlist *)NULL, 0)) == NULL)
	return((KDTdevinfo_t *) NULL);

    if (CheckNlist(nlptr))
	return((KDTdevinfo_t *) NULL);

    /*
     * Read pointer to "top_devinfo" from kernel
     */
    Addr = nlptr->n_value;
    if (KVMget(kd, Addr, (char *) &PtrRoot, 
	       sizeof(struct dev_info *), KDT_DATA)) {
	SImsg(SIM_GERR, "Cannot read `%s' root from kernel", KDTsymbol);
	return((KDTdevinfo_t *) NULL);
    }

    if (KVMget(kd, (u_long)PtrRoot, (char *)&Root, 
	       sizeof(KDTdevinfo_t), KDT_DATA)) {
	SImsg(SIM_GERR, "Cannot read dev_info root device info from kernel");
	return((KDTdevinfo_t *) NULL);
    }

    return(&Root);
}

/*
 * Build device tree by reading the Kernel Device Tree.
 */
extern int KDTbuild(TreePtr, SearchNames)
    DevInfo_t 		      **TreePtr;
    char		      **SearchNames;
{
    KDTdevinfo_t	       *Root;

    if (!(Root = KDTgetRoot()))
	return(-1);

    return(KDTtraverse(Root, NULL, TreePtr, SearchNames));
}

/*
 * Cleanup an KDT node name.
 */
extern char *KDTcleanName(Name, FullClean)
    char		       *Name;
    int				FullClean;
{
    register char	       *cp;
    register char	       *bp;
    register char	       *end;
    register int		len;
    char		       *Buffer;
    char		       *shortman;

    if (!Name || !Name[0])
	return((char *) NULL);

    if (FullClean) {
	/* XXX - Assume the new string won't be longer than the original. */
	Buffer = (char *) xmalloc(strlen(Name)+1);

	/*
	 * Skip over initial wording up to ','.  e.g. "SUNW,4/25" -> "4/25".
	 */
	if (cp = strchr(Name, ','))
	    Name = ++cp;

	for (cp = Name, bp = Buffer; cp && *cp; ++cp) {
	    /*
	     * Change '-' to ' '
	     * Change '_' to '/' (for SunOS 5.x)
	     * Skip ','
	     */
	    if (*cp == '-')
		*bp++ = ' ';
	    else if (*cp == '_')
		*bp++ = '/';
	    else if (*cp != ',')
		*bp++ = *cp;

	    *bp = CNULL;
	}
    } else {
	Buffer = strdup(Name);
	/* Make bp point at end of Buffer */
	bp = Buffer;
	bp += strlen(Buffer);
    }

    /*
     * Remove all trailing spaces.
     * bp points to end of Buffer at this point.
     */
    end = bp;
    for (cp = end - 1; cp > Buffer && *cp == ' '; --cp);
    if (cp != end - 1 && cp && *++cp == ' ')
	*cp = CNULL;

    /*
     * Remove manufacturer part of name.
     * If shortman is "Sun" and Name is "Sun 4/25",
     * change Name to "4/25".
     */
    cp = Buffer;
    if (shortman = GetManShort()) {
	len = strlen(shortman);
	if (EQN(cp, shortman, len) && (int)strlen(cp) > len+2 && 
	    cp[len] == ' ') {
	    cp += strlen(shortman);
	    while (cp && *cp && *cp == ' ')
		++cp;
	}
    }

    return(cp);
}

#if	defined(HAVE_DDI_PROP)
/*
 * Get device description info from the DDI property list.
 * This info is usually found on non-OBP Solaris 2.x machines.
 * i.e. x86 and PowerPC.
 */
static DevDesc_t *KDTgetDevDesc(PropList)
    KDTprop_t		       *PropList;
{
    static KDTprop_t		Prop;
    register KDTprop_t	       *PropPtr;
    static DevInfo_t	        DevInfo;
    char		       *Value;
    static char			ValBuff[8192];
    int				Amt;
    char		       *Key;
    static char			KeyBuff[100];

    DevInfo.DescList = NULL;

    /*
     * The PropList contains pointers to kernel memory.  Each 
     * element needed, needs to be read into user memory using the
     * pointer for the right location.
     */
    for (PropPtr = PropList; PropPtr; PropPtr = Prop.prop_next) {
	/* Read the main Prop structure */
	if (KVMget(kd, (u_long) PropPtr, (char *) &Prop,
		   sizeof(KDTprop_t), KDT_DATA) != 0)
	    continue;

#if	defined(PH_FROM_PROM)
	/*
	 * Info direct from PROM is usually better than through this interface
	 */
	if (FLAGS_ON(Prop.prop_flags, PH_FROM_PROM))
	    continue;
#endif	/* PH_FROM_PROM */

	/* Read the Prop Name */
	if (KVMget(kd, (u_long) Prop.prop_name, (char *) &KeyBuff,
		   sizeof(KeyBuff), KDT_STRING) != 0)
	    continue;
	if (!KeyBuff[0])
	    continue;
	Key = ExpandKey(KeyBuff);

	if (Prop.prop_len == 0) {
	    /* Boolean */
	    AddDevDesc(&DevInfo, Key, "Has", DA_APPEND);
	} else {
	    /* Read in the Value */
	    Amt = (Prop.prop_len < sizeof(ValBuff)) ? Prop.prop_len : 
		sizeof(ValBuff);
	    if (KVMget(kd, (u_long) Prop.prop_val, (char *) ValBuff,
		       Amt, KDT_DATA) != 0)
		continue;
	    ValBuff[Amt] = CNULL;
	    Value = DecodeVal(Key, ValBuff, Amt);
	    AddDevDesc(&DevInfo, Value, Key, DA_APPEND);
	}
    }

    return((DevInfo.DescList) ? DevInfo.DescList : (DevDesc_t *) NULL);
}
#endif	/* HAVE_DDI_PROP */

/*
 * Use what sources we can to set OS specific DevInfo information.
 */
static void SetOSDevInfo(DevData)
     DevData_t		       *DevData;
{
    static ProbeData_t		ProbeData;
    DevInfo_t		       *DevInfo = NULL;
    static char			Buff[256];
    char		       *IdStr;
    char		       *cp;
    char		       *DevName;
    static PCIinfo_t		Info;

    if (!DevData)
	return;

    DevName = DevData->DevName;
#if	defined(HAVE_DEVICEDB)
    /*
     * Look in the DeviceDB data for this device.  If it's found
     * create a new copy of DevInfo to use.  We do this because
     * DDBgetDevInfo() returns a DevInfo_t that is actually a generic
     * DDB entry that's also returned on other calls.
     */
    if (DevInfo = DDBgetDevInfo(&DevData))
	DevInfo = NewDevInfo(DevInfo);
#endif	/* HAVE_DEVICEDB */
    if (!DevData->DevAliases && DevInfo)
	DevData->DevAliases = DevInfo->Aliases;

#if	OSMVER == 5
    PCInewInfo(&Info);
#endif	/* OSMVER == 5 */
    Info.DevInfo = DevInfo;

    /*
     * If the device name is "pciVVVV,DDDD" then it's a PCI device.  The
     * VVVV is the Vendor ID (in hex) and DDDD is the Device ID (in hex).
     * These are defined in config/pci.cf.
     */
    if (EQN(DevData->DevName, "pci", 3) && 
	(cp = strchr(DevData->DevName, ','))) {
	/* VendorID */
	IdStr = DevData->DevName + 3;
	(void) strncpy(Buff, IdStr, cp - IdStr);
	Info.VendorID = strtol(Buff, (char **)NULL, 16);

	/* DeviceID */
	(void) strcpy(Buff, cp + 1);
	Info.DeviceID = strtol(Buff, (char **)NULL, 16);
    } else if (DevData->NodeID) {
#if	defined(HAVE_OPENPROM)
	/*
	 * Lookup device-id and vendor-id in the OBP table
	 */
	OBPnode_t	       *Node;

	if (Node = OBPfindNodeByID(NULL, DevData->NodeID)) {
	    if (IdStr = OBPfindPropVal(OBP_VENDORID, Node->PropTable, 
				       NULL, NULL))
		Info.VendorID = strtol(IdStr, (char **)NULL, 0);

	    if (IdStr = OBPfindPropVal(OBP_DEVICEID, Node->PropTable, 
				       NULL, NULL))
		Info.DeviceID = strtol(IdStr, (char **)NULL, 0);
	} else 
	    SImsg(SIM_DBG, "%s: Cannot find OBP NodeID=0x%x",
		  DevName, DevData->NodeID);
#endif	/* HAVE_OPENPROM */
    }

#if	OSMVER == 5
    /*
     * If we were able to find a VendorID and DeviceID then this is a PCI
     * device and we call PCIsetDeviceInfo() in order to set all the PCI info
     * we can muster.
     */
    if (Info.VendorID && Info.DeviceID)
	PCIsetDeviceInfo(&Info);
#endif	/* OSMVER == 5 */

    if (!DevInfo)
        DevInfo = Info.DevInfo;

    if (DevInfo) {
	/*
	 * Run DevInfo through DeviceCreate() in order to fill in any
	 * missing pieces.
	 */
	(void) memset(&ProbeData, 0, sizeof(ProbeData));
	ProbeData.DevData = DevData;
	ProbeData.UseDevInfo = DevInfo;
	DevInfo = DeviceCreate(&ProbeData);
    }

    DevData->OSDevInfo = DevInfo;
}

/*
 * Check a KDT device.
 */
static int KDTcheckDevice(KDTDevInfo, Parent, TreePtr, SearchNames)
    KDTdevinfo_t		*KDTDevInfo;
    KDTdevinfo_t		*Parent;
    DevInfo_t 		       **TreePtr;
    char		       **SearchNames;
{
    static DevData_t 		 DevData;
    DevInfo_t 			*DevInfo;
    DevDesc_t			*DevDesc = NULL;
    register DevDesc_t	        *DescPtr;
    char		        *Model;
    DevDefine_t			*DevDef = NULL;

    /* Make sure devdata is clean */
    memset((void *)&DevData, 0, sizeof(DevData));
    DevData.DevUnit 		= -1;
    DevData.Slave 		= -1;
    DevData.CtlrUnit 		= -1;
    DevData.DevNum 		= -1;

    /* Set what we know */
    if (KDTDevInfo && DEVI_NODEID(KDTDevInfo) != -1)
	DevData.NodeID = DEVI_NODEID(KDTDevInfo);

    if (KDTDevInfo && KDTDevInfo->devi_name && KDTDevInfo->devi_name[0]) {
	DevData.DevName = KDTcleanName(KDTDevInfo->devi_name, FALSE);
	/*
	 * If we have a Parent, we're not the ROOT node.
	 * Otherwise we are the ROOT node.
	 */
	if (Parent) {
	    /*
	     * The NodeID takes precedence over Unit number.
	     */
	    if (DEVI_NODEID(KDTDevInfo) > 0)
		DevData.DevUnit = DEVI_NODEID(KDTDevInfo);
	    else
		DevData.DevUnit = DEVI_UNIT(KDTDevInfo);
	} else {
	    DEVI_UNIT(KDTDevInfo) = -1;		/* Avoid SunOS 4.x bug */
	    DevData.Flags |= DD_IS_ROOT;
	}
    }
    if (Parent && Parent->devi_name && Parent->devi_name[0]) {
	DevData.CtlrName = KDTcleanName(Parent->devi_name, FALSE);
	DevData.CtlrUnit = DEVI_UNIT(Parent);
    }
    /* 
     * Device nodes that have a driver ALWAYS exist.
     * Some nodes may exist, without a driver, however.
     */
    if (DEVI_EXISTS(KDTDevInfo))
	DevData.Flags |= DD_IS_ALIVE;

#if	OSMVER == 5
    /*
     * Get device number
     */
    if (KDTDevInfo->devi_minor) {
	static struct ddi_minor_data	MinorData;

	if (KVMget(kd, (u_long) KDTDevInfo->devi_minor, (char *) &MinorData,
		   sizeof(struct ddi_minor_data), KDT_DATA) == 0 &&
	    MinorData.type == DDM_MINOR)
	    DevData.DevNum = MinorData.ddm_dev;
    }
#endif	/* OSMVER == 5 */

    /*
     * If NodeID is still unknown and we have DevNum, use DevNum for NodeID
     */
    if ((DevData.NodeID == -1 || DevData.NodeID == -2 || 
	 DevData.NodeID == 0) && 
	DevData.DevNum && (int)DevData.DevNum != -1)
	DevData.NodeID = DevData.DevNum;

#if	defined(HAVE_DDI_PROP)
    if (DEVI_PROPS(KDTDevInfo)) 
	DevDesc = KDTgetDevDesc(DEVI_PROPS(KDTDevInfo));
#endif	/* HAVE_DDI_PROP */

#define V(n)	( ((int)n && (int)n != -1 && (int)n != -2) ? n : 0 )
    SImsg(SIM_DBG, 
	  "KDT: Found <%s(%d)> NodeID=0x%x/0x%x DevNum=0x%x on <%s(%d)>%s",
	  ARG(DevData.DevName), DevData.DevUnit,
	  V(KDTDevInfo->devi_nodeid), V(DevData.NodeID),
	  V((int)DevData.DevNum), 
	  ARG(DevData.CtlrName), DevData.CtlrUnit,
	  (DevData.Flags & DD_IS_ALIVE) ? " [ALIVE]" : "");
#undef V

#if	OSMVER == 4
    /*
     * XXX Avoid hanging SLC & ELC machines which don't have floppy disks.
     */
    if (DevData.DevName && EQ(DevData.DevName, "fd")) {
#if	defined(CPU_SUN4C_20)
	if (CpuType == CPU_SUN4C_20) return;
#endif	/* CPU_SUN4C_20 */
#if	defined(CPU_SUN4C_25)
	if (CpuType == CPU_SUN4C_25) return;
#endif	/* CPU_SUN4C_25 */
#if	defined(CPU_SUN4C_30)
	if (CpuType == CPU_SUN4C_30) return;
#endif	/* CPU_SUN4C_30 */
    }
#endif	/* OSMVER == 4 */

    SetOSDevInfo(&DevData);

#if	defined(HAVE_OPENPROM)
    /*
     * See if we can find a "Model" property from OBP for this device and
     * if so, look to see if we can find a DevDefine (config/*.cf) entry.
     */
    if (Model = OBPfindPropVal(OBP_MODEL, NULL, DevData.NodeID, NULL))
	DevDef = DevDefGet(Model, 0, 0);
#endif	/* HAVE_OPENPROM */

    /* Probe and add device */
    if (TreePtr && (DevInfo = (DevInfo_t *) 
		    ProbeDevice(&DevData, TreePtr, SearchNames, DevDef))) {
	if (DevDesc && DevInfo) {
	    if (DevInfo->DescList) {
		for (DescPtr = DevInfo->DescList; DescPtr && DescPtr->Next;
		     DescPtr = DescPtr->Next);
		DescPtr->Next = DevDesc;
	    } else
		DevInfo->DescList = DevDesc;
	}
	AddDevice(DevInfo, TreePtr, SearchNames);
    }
}

/*
 * Recursively traverse and descend the KDT
 */
static int KDTtraverse(KDTDevInfo, Parent, TreePtr, SearchNames)
    KDTdevinfo_t		*KDTDevInfo;
    KDTdevinfo_t		*Parent;
    DevInfo_t 		       **TreePtr;
    char		       **SearchNames;
{
    static char 		 Name[128];
    KDTdevinfo_t		*Ptr;

    /*
     * If node name is a valid pointer, read the name from kernel space
     * and call probe routine to handle checking the device.
     */
    if (KDTDevInfo->devi_name) {
	if (KVMget(kd, (u_long) KDTDevInfo->devi_name, (char *) Name, 
		   sizeof(Name), KDT_STRING)) {
	    SImsg(SIM_GERR, "Cannot read KDT device name.");
	    Name[0] = CNULL;
	} else {
	    KDTDevInfo->devi_name = (char *) strdup(Name);
	    KDTcheckDevice(KDTDevInfo, Parent, TreePtr, SearchNames);
	}
    }

    /*
     * If this node has children, read the child data from kernel space
     * and DESCEND.
     */
    if (DEVI_CHILD(KDTDevInfo)) {
	Ptr = (KDTdevinfo_t *) xcalloc(1, sizeof(KDTdevinfo_t));
	if (KVMget(kd, (u_long) DEVI_CHILD(KDTDevInfo), (char *) Ptr,
		   sizeof(KDTdevinfo_t), KDT_DATA)) {
	    SImsg(SIM_GERR, "Cannot read KDT slave data for %s.", Name);
	} else {
	    DEVI_CHILD(KDTDevInfo) = (KDTdevinfo_t *) Ptr;
	    KDTtraverse(DEVI_CHILD(KDTDevInfo), KDTDevInfo, 
			 TreePtr, SearchNames);
	}
    }

    /*
     * If this node has a sibling, read the sibling data from kernel space
     * and TRAVERSE.
     */
    if (DEVI_SIBLING(KDTDevInfo)) {
	Ptr = (KDTdevinfo_t *) xcalloc(1, sizeof(KDTdevinfo_t));
	if (KVMget(kd, (u_long) DEVI_SIBLING(KDTDevInfo), (char *) Ptr,
		   sizeof(KDTdevinfo_t), KDT_DATA)) {
	    SImsg(SIM_GERR, "Cannot read KDT next data for %s.", Name);
	} else {
	    DEVI_SIBLING(KDTDevInfo) = (KDTdevinfo_t *) Ptr;
	    KDTtraverse(DEVI_SIBLING(KDTDevInfo), Parent, 
			 TreePtr, SearchNames);
	}
    }

    return(0);
}

/*
 * Get the System Model by reading and returning the name of the
 * root (top) device node from the kernel.
 */
extern char *KDTgetSysModel()
{
    KDTdevinfo_t	       *Root;
    static char			Name[128];

    if (!(Root = KDTgetRoot()))
	return((char *) NULL);
    
    if (!Root->devi_name) {
	SImsg(SIM_DBG, "Kernel Device Tree root node does not have a name.");
	return((char *) NULL);
    }

    /*
     * Read the name from kernel space.
     */
    if (KVMget(kd, (u_long) Root->devi_name, (char *) Name, 
	       sizeof(Name), KDT_STRING)) {
	SImsg(SIM_GERR, "Cannot read Kernel Device Tree root device name.");
	return((char *) NULL);
    }

    if (Name && *Name) {
#if	defined(HAVE_OPENPROM)
	if (DefGet(DL_SUBSYSMODEL, Name, 0, 0))
	    /* We need to get the real name if possible */
	    return(OBPgetSysModel());
	else
#endif	/* HAVE_OPENPROM */
	    return(Name);
    } else
	return((char *) NULL);
}
