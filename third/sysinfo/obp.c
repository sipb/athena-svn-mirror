/*
 * Copyright (c) 1992-1996 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: obp.c,v 1.1.1.1 1996-10-07 20:16:53 ghudson Exp $";
#endif

/*
 * Open Boot PROM (OBP) routines
 */

#include "defs.h"
#include "kdt-sunos.h"

#if	defined(HAVE_OPENPROM)

#define OBP_IS_BOOL(tp)		(tp->Key[strlen(tp->Key)-1] == '?' \
				 || !tp->Value || tp->Value[0] == CNULL)

/*
 * OBPio_t is used to do I/O with openprom.
 */
typedef union {
    char			opio_buff[OPROMMAXPARAM];
    struct openpromio		opio_oprom;
} OBPio_t;

/*
 * OBPprop_t is used to store properties for OBP nodes.
 */
typedef struct _OBPprop {
    char		       *Key;
    char		       *Value;
    struct _OBPprop	       *Next;
} OBPprop_t;

/*
 * OBP Node entry used to represent tree of all OBP nodes.
 */
typedef struct _obpnode {
    char		       *Name;
    OBPnodeid_t		        NodeID;
    OBPnodeid_t		        ParentID;
    char		       *ParentName;
    OBPprop_t		       *PropTable;
    struct _obpnode	       *Children;
    struct _obpnode	       *Next;
} OBPnode_t;

/*
 * Sub System Variable parameters.
 */
typedef struct _SubSysVar {
    OBPnode_t		       *NodeTree;
    int				CPUspeed;
    int				NumCPU;
} OBPsubSysVar_t;

/*
 * Pointer to Tree of all OBP nodes. 
 */
static OBPnode_t	       *OBPnodeTree = NULL;

/*
 * Various declarations
 */
static long			CpuClockFreq = 0;

/*
 * Print debugging info of Prop Info
 */
static void OBPprintPropInfo(Prop)
    OBPprop_t		       *Prop;
{
    register OBPprop_t	       *Ptr;

    for (Ptr = Prop; Ptr; Ptr = Ptr->Next)
	printf("OBPpropInfo:\tKey = <%s> Value = <%s>\n",
	       (Ptr->Key) ? Ptr->Key : "", (Ptr->Value) ? Ptr->Value : "");
}

/*
 * Print debugging info of OBP Node
 */
static void OBPprintInfo(Node)
    OBPnode_t		       *Node;
{
    printf("OBPnode: Name=<%s> ID=<%d> ParentID=<%d> ParentName=<%s>\n",
	   (Node->Name) ? Node->Name : "", Node->NodeID, Node->ParentID, 
	   (Node->ParentName) ? Node->ParentName : "");

    if (Node->PropTable)
	OBPprintPropInfo(Node->PropTable);

    if (Node->Children)
	OBPprintInfo(Node->Children);

    if (Node->Next)
	OBPprintInfo(Node->Next);
}

/*
 * Get next OBP tree node id using "what" as what to get.
 */
static OBPnodeid_t OBPnodeNext(NodeID, obp_fd, What)
    OBPnodeid_t		        NodeID;
    int				obp_fd;
    int				What;
{
    OBPio_t			opio;
    struct openpromio	       *op;
    int			       *idptr;

    op = &(opio.opio_oprom);
    idptr = (OBPnodeid_t *) (op->oprom_array);
    memset((void *)opio.opio_buff, 0, sizeof(opio.opio_buff));
    *idptr = NodeID;
    op->oprom_size = sizeof(opio.opio_buff);

    if (ioctl(obp_fd, What, op) < 0) {
	if (Debug) Error("OBP ioctl %s failed: %s.", 
			 (What == OPROMNEXT) ? "OPROMNEXT" : "OPROMCHILD", 
			 SYSERR);
	return(0);
    }

    return(*((OBPnodeid_t *)op->oprom_array));
}

/*
 * Get the OBP table value for the key "Key".
 */
static char *OBPfindPropVal(Key, Table)
    char		       *Key;
    OBPprop_t		       *Table;
{
    register OBPprop_t	       *Ptr;

    /*
     * See if this node has it's own clock frequency
     */
    for (Ptr = Table; Ptr; Ptr = Ptr->Next)
	if (Ptr->Key[0] && EQ(Ptr->Key, Key))
	    return(Ptr->Value);

    return((char *) NULL);
}

/*
 * Get the property value of propname from the OBP.
 */
static char *OBPgetPropVal(PropName, obp_fd)
    char		       *PropName;
    int				obp_fd;
{
    static OBPio_t		opio;
    struct openpromio	       *op;

    op = &(opio.opio_oprom);
    op->oprom_size = sizeof(opio.opio_buff);
    (void) strcpy(op->oprom_array, PropName);

    if (ioctl(obp_fd, OPROMGETPROP, op) < 0) {
	if (Debug) Error("OBP ioctl OPROMGETPROP failed for '%s': %s.", 
			 PropName, SYSERR);
	return((char *)NULL);
    }

    if (op->oprom_size == -1) {
	if (Debug) Error("OBP no data available for '%s'.", PropName);
	return((char *)NULL);
    }

    /*
     * op->oprom_array points at opio.opio_buff where the results
     * really live.
     */
    return(DecodeVal(op->oprom_array, op->oprom_size));
}

/*
 * Create and copy a OBPprop_t
 */
static OBPprop_t *OBPcopyProp(old)
    OBPprop_t	       *old;
{
    OBPprop_t	       *new;

    new = (OBPprop_t *) xmalloc(sizeof(OBPprop_t));
    memcpy((void *)new, (void *)old, sizeof(OBPprop_t));

    return(new);
}

/*
 * Get all OBP properties for the current OBP node and return
 * a table of the results.
 */
static OBPprop_t *OBPgetProps(obp_fd, OBPpropPtr)
    int				obp_fd;
    OBPprop_t		      **OBPpropPtr;
{
    static OBPio_t		opio;
    struct openpromio	       *op;
    static OBPprop_t		NewProp;
    OBPprop_t		       *PropTable;
    register OBPprop_t         *PropPtr;
    register char	       *cp;

    if (OBPpropPtr)
	PropTable = *OBPpropPtr;
    else
	return((OBPprop_t *) NULL);

    op = &(opio.opio_oprom);
    memset((void *)opio.opio_buff, 0, sizeof(opio.opio_buff));

    for ( ; ; ) {
	/*
	 * Get next property
	 */
	op->oprom_size = sizeof(opio.opio_buff);
	if (ioctl(obp_fd, OPROMNXTPROP, op) < 0) {
	    if (Debug) Error("OBP ioctl OPROMNXTPROP failed: %s.", SYSERR);
	    return((OBPprop_t *) NULL);
	}

	/*
	 * We're done
	 */
	if (!op->oprom_size)
	    break;

	NewProp.Key = strdup(op->oprom_array);

	/*
	 * Get the property value
	 */
	if (cp = OBPgetPropVal(op->oprom_array, obp_fd))
	    NewProp.Value = strdup(cp);
	else
	    NewProp.Value = NULL;

	/*
	 * Add this entry to the linked list.
	 */
	NewProp.Next = NULL;
	if (PropTable) {
	    for (PropPtr = PropTable; PropPtr && PropPtr->Next;
		 PropPtr = PropPtr->Next);
	    PropPtr->Next = OBPcopyProp(&NewProp);
	} else
	    PropTable = OBPcopyProp(&NewProp);
    }

    *OBPpropPtr = PropTable;

    return(PropTable);
}

/*
 * Get the CPU model.
 *
 * The model is based upon the type of CPU and the clock frequency
 * of the CPU.
 */
static char *OBPgetCPUmodel(cputype, OBPprop, clockfreqptr)
    char		       *cputype;
    OBPprop_t		       *OBPprop;
    long		       *clockfreqptr;
{
    long 			clockfreq = 0;
    register int		i;
    static char			Buff[BUFSIZ];
    Define_t		       *Def;
    char		       *Str;

    Str = OBPfindPropVal(OBP_CLOCKFREQ, OBPprop);
    if (Str)
	clockfreq = strtol(Str, (char **)NULL, 0);

    /*
     * No individual clock frequency, so we use the "global"
     * clock freqency that (hopefully) was set earlier.
     */
    if (!clockfreq)
	clockfreq = CpuClockFreq;

    /*
     * Set the frequency we're using so our caller can use it.
     */
    if (clockfreq && clockfreqptr)
	*clockfreqptr = clockfreq;

    /*
     * Scan the CPU types table looking for an entry that has
     * the same type name and the same clock frequency.
     */
    for (Def = DefGetList(DL_CPU); Def; Def = Def->Next) {
	if (EQ(Def->KeyStr, cputype)) {
	    if (Def->KeyNum == 0) {
		(void) sprintf(Buff, "%s %s", 
			       FreqStr(clockfreq), Def->ValStr1);
		return(strdup(Buff));
	    } else if ((Def->KeyNum == -1) ||
		       (clockfreq / MHERTZ) == Def->KeyNum)
		return(Def->ValStr1);
	}
    }

    return((char *)NULL);
}

/*
 * Get a nice informative string describing OBPprop information and
 * set the info into DevInfo.
 */
static void OBPsetOBPinfo(OBPprop, DevInfo)
    OBPprop_t		       *OBPprop;
    DevInfo_t		       *DevInfo;
{
    char		       *ExpName = NULL;
    char		       *Val = NULL;
    Define_t		       *Def;

    /*
     * Try to see if this is one of the types of OBP info
     * that we want to use a more descriptive text than
     * the key itself.
     */
    Def = DefGet(DL_OBP, OBPprop->Key, -1, 0);
    if (Def && Def->ValStr1)
	ExpName = Def->ValStr1;

    if (!ExpName)
	ExpName = ExpandKey(OBPprop->Key);

    if (!OBP_IS_BOOL(OBPprop) && OBPprop->Value) {
	if (EQ(OBPprop->Key, OBP_CLOCKFREQ))
	    Val = strdup(FreqStr(strtol(OBPprop->Value, 
					(char **)NULL, 0)));
	else if (EQ(OBPprop->Key, OBP_ECACHESIZE) ||
		 EQ(OBPprop->Key, OBP_DCACHESIZE))
	    Val = GetSizeStr((u_long) strtol(OBPprop->Value, 
					     (char **)NULL, 0), BYTES);
	else
	    Val = OBPprop->Value;
    }

    if (OBP_IS_BOOL(OBPprop))
	AddDevDesc(DevInfo, ExpName, "Has", DA_APPEND);
    else
	AddDevDesc(DevInfo, Val, ExpName, DA_APPEND);
}

/*
 * Fix an OBP node name to be compatable with the
 * conventions used in SunOS 5.x.
 *
 * i.e. All '/' characters are mapped to '_'.
 */
static char *OBPfixNodeName(string)
    char		       *string;
{
    register char	       *cp;
    static char			buff[BUFSIZ];

    (void) strcpy(buff, string);
    cp = buff;
    while (cp = strchr(cp, '/'))
	*cp++ = '_';

    return(buff);
}

/*
 * Get (make) name of a CPU
 */
extern char *GetCpuName(ncpus, cpuid)
    int			       *ncpus;
    int				cpuid;
{
    static char			buff[BUFSIZ];

    (void) sprintf(buff, "cpu%d", (cpuid >= 0) ? cpuid : (*ncpus)++);

    return(buff);
}

/*
 * Probe a CPU using OBP info.
 */
extern DevInfo_t *OBPprobeCPU(NodeName, DevData, DevDefine,
			      Node, TreePtr, SearchNames)
    char		       *NodeName;
    DevData_t		       *DevData;
    DevDefine_t		       *DevDefine;
    OBPnode_t		       *Node;
    DevInfo_t		      **TreePtr;
    char		      **SearchNames;
    /*ARGSUSED*/
{
    register OBPprop_t	       *PropPtr;
    DevInfo_t		       *DevInfo;
    DevInfo_t		       *fdev;
    register char	       *cp;
    register int		i;
    static char			buff[BUFSIZ];
    static int			ncpus = 0;
    int				cpuid = -1;
    char		       *cpumodel = NULL;
    long			clockfreq = 0;

    /*
     * This function may be called accidentally if a PROM/kernel node
     * called "cpu" is found.  In such a case, we return immediately.
     * This function should only be called explicitly.
     */
    if (NodeName) {
	return((DevInfo_t *) NULL);
    }

    if (!(DevInfo = NewDevInfo(NULL)))
	return((DevInfo_t *)NULL);

    if (Debug) {
	(void) sprintf(buff, "Node ID is %d", Node->NodeID);
	AddDevDesc(DevInfo, buff, NULL, DA_APPEND);
    }

    for (PropPtr = Node->PropTable; PropPtr; PropPtr = PropPtr->Next) {
	/*
	 * See if this is the CPU type
	 */
	if (EQ(OBP_NAME, PropPtr->Key)) {
	    cpumodel = PropPtr->Value;
	    DevInfo->Model = OBPgetCPUmodel(cpumodel, Node->PropTable, 
					    &clockfreq);
	    continue;
	} else if (EQ(PropPtr->Key, OBP_CPUID) && PropPtr->Value) {
	    cpuid = (int) strtol(PropPtr->Value, (char **)NULL, 0);
	} else if (EQ(PropPtr->Key, OBP_DEVTYPE) ||
		   EQ(PropPtr->Key, OBP_CLOCKFREQ)) {
	    continue;
	}

	OBPsetOBPinfo(PropPtr, DevInfo);
    }

    if (cp = GetCpuName(&ncpus, cpuid))
	DevInfo->Name = strdup(cp);

    if (SearchNames && !SearchCheck(DevInfo->Name, SearchNames))
	return((DevInfo_t *) NULL);

    if (cpumodel) {
	if (clockfreq)
	    (void) sprintf(buff, "%s %s", FreqStr(clockfreq), cpumodel);
	else
	    (void) strcpy(buff, cpumodel);
	AddDevDesc(DevInfo, buff, NULL, DA_INSERT|DA_PRIME);
	/*
	 * This must be an unknown type of CPU
	 */
	if (!DevInfo->Model)
	    DevInfo->Model = PrimeDesc(DevInfo);
    }

    DevInfo->Type = DT_CPU;
    DevInfo->NodeID = Node->NodeID;

    /*
     * If no device by our name exists try to find a device
     * with a name which matches our CPU Model.  This should
     * be the root device node.  If so, make our device name
     * match the root device node name, so AddDevice() will 
     * add our info to the root device node later.
     */
    if (TreePtr && *TreePtr &&
	!FindDeviceByName(DevInfo->Name, *TreePtr)) {

	fdev = FindDeviceByName(cpumodel, *TreePtr);
	if (!fdev) {
	    if (Debug) printf("Cannot find CPU device <%s>\n", cpumodel);
	    cp = OBPfixNodeName(cpumodel);
	    if (!EQ(cp, cpumodel))
		fdev = FindDeviceByName(cp, *TreePtr);
	} else
	    if (Debug) printf("Found CPU device <%s>\n", cpumodel);
	if (fdev && !fdev->Master)
	    DevInfo->Name = fdev->Name;
    }

    return(DevInfo);
}

/*
 * Get part description information using a model (part) name.
 */
static Define_t *OBPGetPart(PartName)
    char		       *PartName;
{
    Define_t		       *PartList;
    register Define_t	       *Ptr;
    register int		i;
    register char	       *cp;

    PartList = DefGetList(DL_PART);
    if (!PartList) {
	if (Debug) Error("No Part List found.");
	return((Define_t *) NULL);
    }

    for (Ptr = PartList; Ptr; Ptr = Ptr->Next) {
	if (EQN(Ptr->KeyStr, PartName, strlen(Ptr->KeyStr)))
	    return(Ptr);
	/* Try it without the manufacturer part of part name */
	if ((cp = strchr(Ptr->KeyStr, ',')) && 
	    EQN(++cp, PartName, strlen(cp)))
	    return(Ptr);
    }

    return((Define_t *) NULL);
}

/*
 * This is for sun4d and sun4u platforms.
 *
 * If this device has a board number, then we make a "fake"
 * device called OBP_SYSBOARD that we insert in between the
 * Device and it's logical/current Master.
 */
static void OBPsetBoard(DevInfo, OBPprop, BoardNum, TreePtr, SearchNames)
    DevInfo_t		       *DevInfo;
    OBPprop_t		       *OBPprop;
    int				BoardNum;
    DevInfo_t		      **TreePtr;
    char		      **SearchNames;
{
    static char			BoardName[BUFSIZ];
    static char			buff[BUFSIZ];
    char		       *What;
    char		       *BoardType;
    register DevInfo_t	       *DevPtr;
    register DevInfo_t	       *SaveDev;
    register DevInfo_t	       *LastDev;
    register DevInfo_t	       *OldMaster = NULL;
    register DevInfo_t	       *Master = NULL;
    DevDefine_t		       *DevDefine = NULL;

    if (BoardNum < 0 || !DevInfo)
	return;

    (void) sprintf(BoardName, "%s%d", OBP_SYSBOARD, BoardNum);

    /*
     * Find the Old Master
     */
    if (DevInfo->Master) {
	OldMaster = FindDeviceByNodeID(DevInfo->Master->NodeID, 
				       *TreePtr);
	if (!OldMaster)
	    OldMaster = FindDeviceByName(DevInfo->Master->Name, 
					 *TreePtr);
    }

    /*
     * If the Master (sysboard) device doesn't exit, create one.
     */
    if (!(Master = FindDeviceByName(BoardName, *TreePtr))) {
	Master = NewDevInfo(NULL);
	Master->Name = strdup(BoardName);
	Master->Unit = BoardNum;
	if (DevDefine = DevDefGet(OBP_SYSBOARD, 0, 0)) {
	    Master->Model = DevDefine->Model;
	    Master->ModelDesc = DevDefine->Desc;
	    Master->Type = DevDefine->Type;
	}
	if (OldMaster) {
	    Master->MasterName = OldMaster->Name;
	    Master->Master = OldMaster;
	}
	AddDevice(Master, TreePtr, SearchNames);
    }

    /*
     * Insert sysboard between Device and it's logical Master.
     */
    if (OldMaster) {
	Master->Master = OldMaster;

	/*
	 * Find Device in OldMaster and remove it.
	 */
	SaveDev = NULL;
	for (DevPtr = OldMaster->Slaves, LastDev = DevPtr; DevPtr; 
	     LastDev = DevPtr, DevPtr = DevPtr->Next) {

	    if (!EQ(DevPtr->Name, DevInfo->Name))
		continue;

	    SaveDev = DevPtr;
	    LastDev->Next = DevPtr->Next;
	    break;
	}

	/*
	 * Add SaveDev back to new Master
	 */
	SaveDev->Next = NULL;
	if (Master->Slaves) {
	    for (DevPtr = Master->Slaves; DevPtr && DevPtr->Next;
		 DevPtr = DevPtr->Next);
	    DevPtr->Next = SaveDev;
	} else
	    Master->Slaves = SaveDev;
    }

    /*
     * Update Device
     */
    if (Master) {
	DevInfo->MasterName = Master->Name;
	DevInfo->Master = Master;
    }

    /*
     * If this is an FHC device (sun4u), then try to figure out more
     * info about the type of board this is.
     */
    if (EQN(DevInfo->Name, OBP_FHC, strlen(OBP_FHC))) {
	BoardType = OBPfindPropVal(OBP_BOARDTYPE, OBPprop);
	if (BoardType) {
	    /* 
	     * There are different board types:
	     *	VALUE		DESCRIPTION
	     *	mem		CPU & Memory
	     *	cpu		CPU & Memory
	     *	dual-sbus	I/O
	     *	?		Graphics
	     */
	    if (EQ(BoardType, "mem") || 
		EQ(BoardType, "cpu"))
		What = "CPU & Memory";
	    else if (EQ(BoardType, "dual-sbus"))
		What = "Sbus I/O";
	    else
		What = NULL;
	    if (What) {
		(void) sprintf(buff, "%s Board", What);
		Master->Model = strdup(buff);
	    }
	}    
    }
}

/*
 * Decode memory string into number of MBYTES.
 */
static int OBPGetMemSize(MemStr)
    char		       *MemStr;
{
    register char	       *cp;
    char		       *MemBuff;
    u_long			Amt = 0;

    if (strchr(MemStr, '.')) {
	MemBuff = strdup(MemStr);
	for (cp = strtok(MemBuff, "."); cp; cp = strtok((char *)NULL, "."))
	    Amt += strtol(cp, (char **) NULL, 0);
	(void) free(MemBuff);
    } else
	Amt = strtol(MemStr, (char **) NULL, 0);

    return(Amt / MBYTES);
}

/*
 * Decode memory string into memory groups and add as a dev description.
 */
static void OBPSetMemGrps(DevInfo, MemStr)
    DevInfo_t		       *DevInfo;
    char		       *MemStr;
{
    static char			Buff[BUFSIZ];
    char			Str[100];
    register char	       *cp;
    char		       *MemBuff;
    int				GrpNum = 0;

    if (!strchr(MemStr, '.'))
	return;

    Buff[0] = CNULL;
    MemBuff = strdup(MemStr);
    for (cp = strtok(MemBuff, "."); cp; cp = strtok((char *)NULL, ".")) {
	(void) sprintf(Str, "#%d=%s", GrpNum++, 
		       GetSizeStr((u_long) strtol(cp, (char **) NULL, 0), 
				  BYTES));
	if (Buff[0] == CNULL)
	    strcpy(Buff, Str);
	else {
	    strcat(Buff, ", ");
	    strcat(Buff, Str);
	}
    }
    (void) free(MemBuff);

    AddDevDesc(DevInfo, Buff, "Memory Groups", DA_APPEND);
}

/*
 * Probe an OBP node.
 */
extern DevInfo_t *OBPprobe(Node, TreePtr, SearchNames)
    OBPnode_t		       *Node;
    DevInfo_t		      **TreePtr;
    char		      **SearchNames;
{
    register OBPprop_t	       *PropPtr;
    DevInfo_t		       *DevInfo = NULL;
    DevInfo_t		       *ParentDevInfo;
    register char	      **cpp;
    static char			buff[BUFSIZ];
    int				DevID = -1;
    int				BoardNum = -1;
    char		       *Part = NULL;
    Define_t		       *PartInfo = NULL;
    int				MemSize = 0;

    /*
     * First see if we can find this device by it's OBP node ID
     */
    if (Node->NodeID && TreePtr)
	DevInfo = FindDeviceByNodeID(Node->NodeID, *TreePtr);

    /*
     * Try to find the device by name.   This usually only
     * works for the root "system" node and a few others.
     */
    if (!DevInfo && Node->Name && TreePtr)
	DevInfo = FindDeviceByName(Node->Name, *TreePtr);

    /*
     * Didn't find it.
     */
    if (!DevInfo)
	return((DevInfo_t *)NULL);

    /*
     * Get descriptive and other info
     */
    for (PropPtr = Node->PropTable; PropPtr; PropPtr = PropPtr->Next) {
	/*
	 * Skip these
	 */
	if (EQ(PropPtr->Key, OBP_NAME))
	    continue;
	else if (EQ(PropPtr->Key, OBP_MODEL))
	    Part = PropPtr->Value;
	else if (EQ(PropPtr->Key, OBP_BOARD) && PropPtr->Value)
	    BoardNum = strtol(PropPtr->Value, (char **) NULL, 0);
	else if (EQ(PropPtr->Key, OBP_DEVICEID) && PropPtr->Value)
	    DevID = strtol(PropPtr->Value, (char **) NULL, 0);
	else if (EQ(PropPtr->Key, OBP_SIZE) && 
		 EQ(Node->Name, OBP_MEMUNIT)) {
	    MemSize = OBPGetMemSize(PropPtr->Value);
	    OBPSetMemGrps(DevInfo, PropPtr->Value);
	} else if (EQ(PropPtr->Key, OBP_KEYBOARD)) {
	    DevInfo->Slaves = ProbeKbd((DevInfo_t **)NULL);
	}

	OBPsetOBPinfo(PropPtr, DevInfo);
    }

    /*
     * Handle special cases
     */
    if (MemSize && DevInfo->Model) {
	(void) sprintf(buff, "%d MB %s", MemSize, DevInfo->Model);
	DevInfo->Model = strdup(buff);
    }
    if (BoardNum >= 0)
	OBPsetBoard(DevInfo, Node->PropTable, BoardNum, TreePtr, SearchNames);

    /*
     * Add information to existing Device.
     */

    if (Part) {
	PartInfo = OBPGetPart(Part);
	if (PartInfo) {
	    DevInfo->Model = PartInfo->ValStr1;
	    AddDevDesc(DevInfo, PartInfo->ValStr2, NULL, DA_INSERT|DA_PRIME);
	} else if (DevInfo->Model) {
	    (void) sprintf(buff, "%s (%s)", DevInfo->Model, Part);
	    DevInfo->Model = strdup(buff);
	} else
	    DevInfo->Model = strdup(Part);
    }

    return((DevInfo_t *)NULL);
}

/*
 * Check over an OBP device.
 */
static void OBPdevCheck(Node, TreePtr, SearchNames)
    OBPnode_t		       *Node;
    DevInfo_t		      **TreePtr;
    char		      **SearchNames;
{
    register OBPprop_t	       *PropPtr;
    static int			first = TRUE;
    static char			buff[BUFSIZ];
    DevDefine_t		       *DevDefine = NULL;
    DevInfo_t		       *DevInfo = NULL;
    DevInfo_t		       *master = NULL;

    /*
     * Iterate over all the properties looking for interesting properties
     * or until we find the device type.  If the device type is one which
     * we found in our probe list, go ahead and probe it.
     */
    for (PropPtr = Node->PropTable; PropPtr; PropPtr = PropPtr->Next) {
	if (Debug) printf("OBPdevCheck:     NodeID %d <%s> = <%s> (0x%x)\n",
			  Node->NodeID,
			  PropPtr->Key, 
			  (PropPtr->Value) ? PropPtr->Value : "",
			  (PropPtr->Value) ? atol(PropPtr->Value) : 0);

	/*
	 * If this is the first node and this property is the Clock Frequency
	 * then save it for later use by the individual CPUs.
	 */
	if (first && !CpuClockFreq && EQ(PropPtr->Key, OBP_CLOCKFREQ) &&
	    PropPtr->Value)
	    CpuClockFreq = strtol(PropPtr->Value, (char **)NULL, 0);

	if (EQ(PropPtr->Key, OBP_DEVTYPE))
	    DevDefine = DevDefGet(PropPtr->Value, 0, 0);
    }

    if (Debug) 
	printf("OBPdevCheck: NodeID %d Name = <%s> DevDefine.Name = <%s>\n",
	       Node->NodeID,
	       (Node->Name[0]) ? Node->Name : "", 
	       (DevDefine && DevDefine->Name) ? DevDefine->Name : "");

    /*
     * We use DevDefine->Probe for our own internal (obp.c) use so we
     * need to make sure we only call the right Probe functions
     */
    if (DevDefine && DevDefine->Probe == OBPprobeCPU)
	DevInfo = OBPprobeCPU((char *) NULL, (DevData_t *)NULL, 
			      (DevDefine_t *) NULL,
			      Node, TreePtr, SearchNames);

    if (!DevInfo)
	DevInfo = OBPprobe(Node, TreePtr, SearchNames);

    if (DevInfo) {
	/*
	 * Find or create of Master
	 */
	if (TreePtr && !DevInfo->Master) {
	    master = FindDeviceByNodeID(Node->ParentID, *TreePtr);
	    if (!master && Node->ParentName) {
		master = NewDevInfo(NULL);
		master->Name = strdup(Node->ParentName);
		master->NodeID = Node->ParentID;
	    }
	    DevInfo->Master = master;
	}
	DevInfo->NodeID = Node->NodeID;
	AddDevice(DevInfo, TreePtr, SearchNames);
    }

    first = FALSE;
}

/*
 * Traverse OBP device tree and build device info
 */
static void OBPdevTraverse(Node, TreePtr, SearchNames)
    OBPnode_t		       *Node;
    DevInfo_t		      **TreePtr;
    char		      **SearchNames;
{
    OBPdevCheck(Node, TreePtr, SearchNames);

    if (Node->Next)
	OBPdevTraverse(Node->Next, TreePtr, SearchNames);

    if (Node->Children)
	OBPdevTraverse(Node->Children, TreePtr, SearchNames);
}

/*
 * Find an OBP node in Node with an ID of NodeID.
 */
static OBPnode_t *OBPfindNodeByID(Node, NodeID)
    OBPnode_t		       *Node;
    OBPnodeid_t		        NodeID;
{
    OBPnode_t		       *Found = NULL;

    if (!Node)
	return((OBPnode_t *) NULL);

    if (Node->NodeID == NodeID)
	return(Node);

    if (Node->Children)
	if (Found = OBPfindNodeByID(Node->Children, NodeID))
	    return(Found);

    if (Node->Next)
	if (Found = OBPfindNodeByID(Node->Next, NodeID))
	    return(Found);

    return((OBPnode_t *) NULL);
}

/*
 * Find an OBP node in Node with a device type of MatchType
 */
static OBPnode_t *OBPfindNodeByType(Node, MatchType)
    OBPnode_t		       *Node;
    char		       *MatchType;
{
    OBPnode_t		       *Found = NULL;
    char		       *NodeType;

    if (!Node)
	return((OBPnode_t *) NULL);

    NodeType = OBPfindPropVal(OBP_DEVTYPE, Node->PropTable);
    if (NodeType && EQ(NodeType, MatchType))
	return(Node);

    if (Node->Children)
	if (Found = OBPfindNodeByType(Node->Children, MatchType))
	    return(Found);

    if (Node->Next)
	if (Found = OBPfindNodeByType(Node->Next, MatchType))
	    return(Found);

    return((OBPnode_t *) NULL);
}

/*
 * Find an OBP node in Node with a name of Name.
 */
static OBPnode_t *OBPfindNodeByName(Node, Name)
    OBPnode_t		       *Node;
    char		       *Name;
{
    OBPnode_t		       *Found = NULL;
    char		       *cp1;
    char		       *cp2;

    if (!Node)
	return((OBPnode_t *) NULL);

    if (EQ(Name, Node->Name))
	return(Node);

    /*
     * Try a match by stripping out any commas found in the names.
     */
    if (cp1 = strchr(Name, ','))
	++cp1;
    else
	cp1 = Name;
    if (cp2 = strchr(Node->Name, ','))
	++cp2;
    else
	cp2 = Node->Name;
    if (EQ(cp1, cp2))
	return(Node);

    if (Node->Children)
	if (Found = OBPfindNodeByName(Node->Children, Name))
	    return(Found);

    if (Node->Next)
	if (Found = OBPfindNodeByName(Node->Next, Name))
	    return(Found);

    return((OBPnode_t *) NULL);
}

/*
 * Traverse tree of OBP I/O nodes
 */
static int OBPnodeTraverse(NodeTree, ParentID, ParentName, NodeID, obp_fd)
    OBPnode_t		      **NodeTree;
    OBPnodeid_t		        ParentID;
    char		       *ParentName;
    OBPnodeid_t		        NodeID;
    int				obp_fd;
{
    OBPnodeid_t		        NewID;
    OBPnode_t		       *NewNode;
    register OBPnode_t	       *NodePtr;
    register OBPnode_t	       *Master;

    NewNode = (OBPnode_t *) xcalloc(1, sizeof(OBPnode_t));
    NewNode->NodeID = NodeID;
    NewNode->ParentID = ParentID;
    NewNode->ParentName = ParentName;
    if (OBPgetProps(obp_fd, &(NewNode->PropTable)))
	NewNode->Name = OBPfindPropVal(OBP_NAME, NewNode->PropTable);

    /*
     * Add new node to NodeTree
     */
    Master = OBPfindNodeByID(*NodeTree, ParentID);
    if (Master) {
	if (Master->Children) {
	    for (NodePtr = Master->Children; NodePtr && NodePtr->Next; 
		 NodePtr = NodePtr->Next);
	    NodePtr->Next = NewNode;
	} else
	    Master->Children = NewNode;
    } else
	*NodeTree = NewNode;

    /*
     * Traverse Children and Peers (Next)
     */
    if (NewID = OBPnodeNext(NodeID, obp_fd, OPROMCHILD))
	OBPnodeTraverse(NodeTree, NewNode->NodeID, NewNode->Name,
			NewID, obp_fd);
    if (NewID = OBPnodeNext(NodeID, obp_fd, OPROMNEXT))
	OBPnodeTraverse(NodeTree, ParentID, ParentName, NewID, obp_fd);

    return(0);
}

/*
 * Build tree of OBP nodes based on what we find using I/O calls
 * to the OBP device.
 */
extern int OBPbuildNodeTree(NodeTree)
    OBPnode_t		      **NodeTree;
{
    int				obp_fd;
    int				status;

    if (!NodeTree) {
	if (Debug) Error("OBPbuildeNodeTree: NULL NodeTree Ptr.");
	return(-1);
    }

    /*
     * Note: Only one process at a time can open _PATH_OPENPROM.
     */
    if ((obp_fd = open(_PATH_OPENPROM, O_RDONLY)) < 0) {
	if (Debug) Error("Cannot open \"%s\": %s.", _PATH_OPENPROM, SYSERR);
	return(-1);
    }

    status = OBPnodeTraverse(NodeTree, (OBPnodeid_t) 0, (char *) NULL,
			     OBPnodeNext((OBPnodeid_t) 0, obp_fd, OPROMNEXT),
			     obp_fd);

    (void) close(obp_fd);

    if (Debug && NodeTree && *NodeTree)
	OBPprintInfo(*NodeTree);

    return(status);
}

/*
 * Build device tree from OBP device.
 */
extern int OBPbuild(TreePtr, SearchNames)
    DevInfo_t		      **TreePtr;
    char		      **SearchNames;
{
    int				status = 0;

    /*
     * Build the OBP Node Tree if it hasn't been done already.
     */
    if (!OBPnodeTree) {
	status = OBPbuildNodeTree(&OBPnodeTree);
	if (status != 0)
	    return(status);
    }

    /*
     * Build the device tree by walking the OBP Node Tree
     */
    OBPdevTraverse(*OBPnodeTree, TreePtr, SearchNames);

    return(status);
}

/*
 * Get the "ROM Version" by querying the OBP
 */
extern char *OBPgetRomVersion()
{
    static char			Version[OPROMMAXPARAM];
#if	defined(OPROMGETVERSION)
    int				obp_fd;
    struct openpromio	       *op;
    static OBPio_t		opio;

    /*
     * Note: Only one process at a time can open _PATH_OPENPROM.
     */
    if ((obp_fd = open(_PATH_OPENPROM, O_RDONLY)) < 0) {
	if (Debug) Error("Cannot open \"%s\": %s.", _PATH_OPENPROM, SYSERR);
	return((char *) NULL);
    }

    op = &(opio.opio_oprom);
    op->oprom_size = sizeof(opio.opio_buff);

    if (ioctl(obp_fd, OPROMGETVERSION, op) < 0) {
	if (Debug) Error("OBP ioctl OPROMGETVERSION failed: %s", SYSERR);
	return((char *) NULL);
    }
    (void) close(obp_fd);

    strcpy(Version, op->oprom_array);
#endif	/* OPROMGETVERSION */

    return((Version[0]) ? Version : (char *) NULL);
}

/*
 * Get sub system model variables
 */
extern char *OBPsubSysGetVar(Variable, Params)
    char		       *Variable;
    Opaque_t		        Params;
{
    char		       *Banner;
    register char	       *cp;
    register char	       *End;
    OBPsubSysVar_t	       *SubSysVar;

    SubSysVar = (OBPsubSysVar_t *) Params;

    if (EQ(Variable, "BannerName")) {
	Banner = OBPfindPropVal(OBP_BANNERNAME, 
				SubSysVar->NodeTree->PropTable);
	if (Banner) {
	    End = &Banner[strlen(Banner)];
	    if (cp = strchr(Banner, '-')) {
		if (strncmp(cp, "-slot ", 6) == 0 && cp < End)
		    Banner = cp + 6;
		else if (strncmp(cp, "-way ", 5) == 0 && cp < End)
		    Banner = cp + 5;
	    }
	    return(Banner);
	}
    } else if (EQ(Variable, "NumCPU")) {
	if (SubSysVar->NumCPU)
	    return(itoa(SubSysVar->NumCPU));
    } else if (EQ(Variable, "CPUspeed")) {
	if (SubSysVar->CPUspeed)
	    return(itoa(SubSysVar->CPUspeed));
    } else {
	if (Debug) Error("OBPsubSysGetVar: Unknown variable `%s'", Variable);
    }

    return((char *) NULL);
}

/*
 * Get the clock frequency from the OBP property table.
 */
static int OBPgetCPUclockfreq(PropTable)
    OBPprop_t		       *PropTable;
{
    char		       *ClockStr;
    long			ClockFreq;

    ClockStr = OBPfindPropVal(OBP_CLOCKFREQ, PropTable);
    if (!ClockStr) {
	if (Debug) Error("OBPgetCPUclockfreq: Could not find CPU clockfreq.");
	return(0);
    }
    ClockFreq = strtol(ClockStr, (char **)NULL, 0);

    if (ClockFreq > MHERTZ)
	return((int) (ClockFreq / MHERTZ));
    else
	return((int) ClockFreq);
}

/*
 * Get the speed of any CPU (they should all be the same).
 */
static int OBPgetCPUspeed(NodeTree)
    OBPnode_t		       *NodeTree;
{
    OBPnode_t		       *Node;
    int				Speed;

    /*
     * First try to find a CPU node.
     */
    Node = OBPfindNodeByType(NodeTree, OBP_CPU);
    if (Node) {
	Speed = OBPgetCPUclockfreq(Node->PropTable);
	if (Speed)
	    return(Speed);
    } else {
	if (Debug) Error("OBPgetCPUspeed: Could not find any CPU nodes.");
    }

    /*
     * Some machines have the clock frequency in the root node.
     */
    Speed = OBPgetCPUclockfreq(NodeTree->PropTable);
    if (Speed)
	return(Speed);
    else
	if (Debug) Error("OBPgetCPUspeed: No clockfreq in root Node.");

    return(0);
}

/*
 * Get the sub system model type.
 * i.e. "Ultra-1 140"
 */
extern char *OBPgetSubSysModel(SubSysDef)
    Define_t		       *SubSysDef;
{
    static char			ErrBuff[BUFSIZ];
    static OBPsubSysVar_t	SubSysVar;
    int				CPUspeed = 0;
    int				HasCPUspeed = 0;
    int				NumCPU = 0;
    int				HasNumCPU = 0;
    int				DevsMatch = 0;
    char		      **HasDevArgv = NULL;
    int				HasDevArgc = 0;
    int				TypesMatch = 0;
    char		      **HasTypeArgv = NULL;
    int				HasTypeArgc = 0;
    char		       *ExpandStr = NULL;
    char		       *SubSysName = NULL;
    char		       *cp;
    register int		i;

    if (SubSysDef->ValStr1)
	ExpandStr = SubSysDef->ValStr1;
    else
	return((char *) NULL);

    if (SubSysDef->ValStr2 && *SubSysDef->ValStr2)
	HasCPUspeed = (int) strtol(SubSysDef->ValStr2, NULL, 0);
    if (SubSysDef->ValStr3 && *SubSysDef->ValStr3)
	HasNumCPU = (int) strtol(SubSysDef->ValStr3, NULL, 0);
    if (SubSysDef->ValStr4 && *SubSysDef->ValStr4)
	HasDevArgc = StrToArgv(SubSysDef->ValStr4, " ", &HasDevArgv);
    if (SubSysDef->ValStr5 && *SubSysDef->ValStr5)
	HasTypeArgc = StrToArgv(SubSysDef->ValStr5, " ", &HasTypeArgv);

    /*
     * Gather info needed for comparision and use in model name expansion
     */
    if (!OBPnodeTree)
	OBPbuildNodeTree(&OBPnodeTree);

    /*
     * Look for matching device names
     */
    for (i = 0; i < HasDevArgc; ++i)
	if (HasDevArgv[i] && HasDevArgv[i][0] && 
	    OBPfindNodeByName(OBPnodeTree, HasDevArgv[i]))
	    ++DevsMatch;

    /*
     * Look for matching device types
     */
    for (i = 0; i < HasTypeArgc; ++i)
	if (HasTypeArgv[i] && HasTypeArgv[i][0] && 
	    OBPfindNodeByType(OBPnodeTree, HasTypeArgv[i]))
	    ++TypesMatch;

    cp = GetNumCpu();
    if (cp)
	NumCPU = (int) strtol(cp, (char **)NULL, 0);

    CPUspeed = OBPgetCPUspeed(OBPnodeTree);

    /*
     * Expand name if needed
     */
    if (((HasCPUspeed && HasCPUspeed == CPUspeed) || !HasCPUspeed) &&
	((HasNumCPU && HasNumCPU == NumCPU) || !HasNumCPU) && 
	(((HasTypeArgc - 1) == TypesMatch) || !HasTypeArgc) &&
	(((HasDevArgc - 1) == DevsMatch) || !HasDevArgc)) {

	SubSysVar.NodeTree = OBPnodeTree;
	SubSysVar.CPUspeed = CPUspeed;
	SubSysVar.NumCPU = NumCPU;
	SubSysName = VarSub(ExpandStr, ErrBuff, OBPsubSysGetVar, 
			    (Opaque_t) &SubSysVar);
	if (SubSysName)
	    return(SubSysName);
	else if (Debug)
	    Error("Variable error in `%s': %s", ExpandStr, ErrBuff);
    }

    return((char *) NULL);
}

/*
 * Get system model type from OBP directly.
 */
extern char *OBPgetSysModel()
{
    int				obp_fd;
    char		       *Name;
    char		       *SubName;
    static OBPprop_t	       *OBPprop = NULL;
    register Define_t	       *DefPtr;
    register Define_t	       *SubSysDefs;

    /*
     * Note: Only one process at a time can open _PATH_OPENPROM.
     */
    if ((obp_fd = open(_PATH_OPENPROM, O_RDONLY)) < 0) {
	if (Debug) Error("Cannot open \"%s\": %s.", _PATH_OPENPROM, SYSERR);
	return((char *) NULL);
    }

    /*
     * Position ourselves at root OBP node.
     */
    (void) OBPnodeNext((OBPnodeid_t) 0, obp_fd, OPROMNEXT);

    /*
     * Get all the properties for the current node.
     */
    if (!OBPgetProps(obp_fd, &OBPprop)) {
	if (Debug) Error("Get OBP Property table failed.");
	return((char *)NULL);
    }

    (void) close(obp_fd);

    /*
     * Find the root node's name
     */
    Name = OBPfindPropVal(OBP_NAME, OBPprop);
    if (!Name) {
	if (Debug) Error("Could not find system model in OBP.");
	return((char *) NULL);
    }

    /*
     * See if there is a sub system model name for this.
     */
    SubSysDefs = DefGet(DL_SUBSYSMODEL, Name, 0, 0);
    if (!SubSysDefs) {
	/* No Sub System Model defined, so we just return the base name */
	if (Debug) Error("No sub system model defined for `%s'", Name);
	return(Name);
    }

    for (DefPtr = SubSysDefs; DefPtr; DefPtr = DefPtr->Next)
	if (EQ(Name, DefPtr->KeyStr) && (SubName = OBPgetSubSysModel(DefPtr)))
	    return(SubName);

    return(Name);
}

#endif	/* HAVE_OPENPROM */
