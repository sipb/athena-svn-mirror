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
 * Open Boot PROM (OBP) routines
 */

#include "defs.h"
#include "sunos-kdt.h"
#include "sunos-obp.h"

#if	defined(HAVE_OPENPROM)

#define OBP_IS_BOOL(tp)		(tp->Key[strlen(tp->Key)-1] == '?' \
				 || !tp->Value || tp->Value[0] == CNULL)

/*
 * Pointer to Tree of all OBP nodes. 
 */
static OBPnode_t	       *OBPnodeTree = NULL;

/*
 * Various declarations
 */
static long			CpuClockFreq = 0;

/*
 * Function declarations
 */
static int			OBPgetCPUspeed();

/*
 * Print debugging info of Prop Info
 */
static void OBPprintPropInfo(NodeName, Prop)
    char		       *NodeName;
    OBPprop_t		       *Prop;
{
    register OBPprop_t	       *Ptr;

    for (Ptr = Prop; Ptr; Ptr = Ptr->Next)
	SImsg(SIM_DBG, "OBPpropInfo:\t<%s> <%s>=<%s>",
	      PRTS(NodeName),
	      (Ptr->Key) ? Ptr->Key : "", (Ptr->Value) ? Ptr->Value : "");
}

/*
 * Print debugging info of OBP Node
 */
static void OBPprintInfo(Node)
    OBPnode_t		       *Node;
{
    SImsg(SIM_DBG, 
	  "OBPnode: Name=<%s> ID=<0x%x> ParentID=<0x%x> ParentName=<%s>",
	  (Node->Name) ? Node->Name : "", Node->NodeID, Node->ParentID, 
	  (Node->ParentName) ? Node->ParentName : "");

    if (Node->PropTable)
	OBPprintPropInfo(Node->Name, Node->PropTable);

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
	SImsg(SIM_GERR, "OBP ioctl %s failed: %s.", 
	      (What == OPROMNEXT) ? "OPROMNEXT" : "OPROMCHILD", 
	      SYSERR);
	return(0);
    }

    return(*((OBPnodeid_t *)op->oprom_array));
}

/*
 * Set the property value of propname from the OBP.
 */
static int OBPsetPropVal(Prop, obp_fd)
     OBPprop_t		       *Prop;
     int			obp_fd;
{
    static OBPio_t		opio;
    struct openpromio	       *op;
    register char	       *cp;

    op = &(opio.opio_oprom);
    op->oprom_size = sizeof(opio.opio_buff);
    (void) strcpy(op->oprom_array, Prop->Key);

    if (ioctl(obp_fd, OPROMGETPROP, op) < 0) {
	SImsg(SIM_GERR, "OBP ioctl OPROMGETPROP failed for '%s': %s.", 
	      Prop->Key, SYSERR);
	return(-1);
    }

    if (op->oprom_size == -1) {
	SImsg(SIM_DBG, "OBP no data available for '%s'.", Prop->Key);
	return(-1);
    }

    /*
     * Copy Raw info
     */
    Prop->RawLen = op->oprom_size;
    Prop->Raw = (void *) xcalloc(1, Prop->RawLen);
    (void) memcpy(Prop->Raw, op->oprom_array, Prop->RawLen);

    /*
     * Now create a nice ASCII String for Value.
     * op->oprom_array points at opio.opio_buff where the results
     * really live.
     */
    if (cp = DecodeVal(Prop->Key, op->oprom_array, op->oprom_size))
	Prop->Value = strdup(cp);

    return(0);
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
	    SImsg(SIM_GERR, "OBP ioctl OPROMNXTPROP failed: %s.", SYSERR);
	    return((OBPprop_t *) NULL);
	}

	/*
	 * We're done
	 */
	if (!op->oprom_size)
	    break;

	(void) memset(&NewProp, 0, sizeof(NewProp));
	NewProp.Key = strdup(op->oprom_array);

	/*
	 * Set Value, Raw, RawLen
	 */
	(void) OBPsetPropVal(&NewProp, obp_fd);

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
 * Get info about a CPU.
 *
 * CpuType is the type/model to lookup.
 * OBPprop is the OBP Property table for CpuType.
 * Results are stored in ModelPtr, VendorPtr, FreqPtr
 */
static int OBPgetCPUinfo(CpuType, OBPprop, ModelPtr, VendorPtr, FreqPtr)
    char		       *CpuType;
    OBPprop_t		       *OBPprop;
    char		      **ModelPtr;
    char		      **VendorPtr;
    long		       *FreqPtr;
{
    long 			ClockFreq = 0;
    Define_t		       *Def;
    char		       *Str;
    char		       *Model = NULL;
    char		       *cp;

    Str = OBPfindPropVal(OBP_CLOCKFREQ, OBPprop, NULL, NULL);
    if (Str)
	ClockFreq = strtol(Str, (char **)NULL, 0);

    /*
     * No individual clock frequency, so we use the "global"
     * clock freqency that (hopefully) was set earlier.
     */
    if (!ClockFreq)
	ClockFreq = CpuClockFreq;

    /*
     * Set the frequency we're using so our caller can use it.
     */
    if (ClockFreq && FreqPtr)
	*FreqPtr = ClockFreq;

    /*
     * If CpuType is of form 'Foo,Bar' then use 'Foo' as the Vendor
     * and 'Bar' as the actual Model.
     */
    if (cp = strchr(CpuType, ',')) {
        Model = strdup(cp + 1);
	Str = (char *) xmalloc(cp - CpuType);
	(void) strncpy(Str, CpuType, cp - CpuType);
	Str[cp - CpuType] = CNULL;
	*VendorPtr = Str;
    }

    /*
     * Scan the CPU types table looking for an entry that has
     * the same type name and the same clock frequency.
     */
    for (Def = DefGetList(DL_CPU); Def; Def = Def->Next) {
	if (EQ(Def->KeyStr, CpuType)) {
	    if ((Def->KeyNum == 0) || (Def->KeyNum == -1) ||
		((ClockFreq / MHERTZ) == Def->KeyNum))
		*ModelPtr = Def->ValStr1;
	}
    }

    if (!*ModelPtr) {
        /* No special CPU Definetion was found, so use the OBP value */
        if (Model)
  	    *ModelPtr = strdup(Model);		/* Without Vendor Str */
        else
	    *ModelPtr = strdup(CpuType);
    }

    SImsg(SIM_DBG, "OBPgetCPUinfo(%s) Vendor=<%s> Model=<%s> Freq=%d",
	  PRTS(CpuType), PRTS(*VendorPtr), PRTS(*ModelPtr), *FreqPtr);

    return(0);
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
	    Val = GetSizeStr((Large_t) strtol(OBPprop->Value, 
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
    static char			Buff[256];

    (void) snprintf(Buff, sizeof(Buff), "%s", string);
    cp = Buff;
    while (cp = strchr(cp, '/'))
	*cp++ = '_';

    return(Buff);
}

/*
 * Get (make) name of a CPU
 */
extern char *GetCpuName(ncpus, cpuid)
    int			       *ncpus;
    int				cpuid;
{
    static char			Buff[32];

    (void) snprintf(Buff, sizeof(Buff), "cpu%d", 
		    (cpuid >= 0) ? cpuid : (*ncpus)++);

    return(Buff);
}

/*
 * Probe a CPU using OBP info.
 */
extern DevInfo_t *OBPprobeCPU(ProbeData, Node, TreePtr, SearchNames)
     ProbeData_t	       *ProbeData;
     OBPnode_t		       *Node;
     DevInfo_t		      **TreePtr;
     char		      **SearchNames;
{
    register OBPprop_t	       *PropPtr;
    DevInfo_t		       *DevInfo;
    DevInfo_t		       *fdev;
    static DevFind_t	        Find;
    register char	       *cp;
    register int		i;
    static char			Buff[256];
    static int			ncpus = 0;
    int				cpuid = -1;
    char		       *CpuModel = NULL;
    char		       *CpuModelPart = NULL;
    char		       *CpuVendor = NULL;
    long			ClockFreq = 0;
    char		       *ClockFreqStr = NULL;

    /*
     * This function may be called accidentally if a PROM/kernel node
     * called "cpu" is found.  In such a case, we return immediately.
     * This function should only be called explicitly.
     */
    if (ProbeData)
	return((DevInfo_t *) NULL);

    if (!(DevInfo = NewDevInfo(NULL)))
	return((DevInfo_t *)NULL);

    if (Debug) {
	(void) snprintf(Buff, sizeof(Buff),  "Node ID is 0x%x", Node->NodeID);
	AddDevDesc(DevInfo, Buff, NULL, DA_APPEND);
    }

    for (PropPtr = Node->PropTable; PropPtr; PropPtr = PropPtr->Next) {
	/*
	 * See if this is the CPU type
	 */
	if (EQ(OBP_NAME, PropPtr->Key)) {
	    CpuModelPart = strdup(PropPtr->Value);
	    OBPgetCPUinfo(PropPtr->Value, Node->PropTable,
			  &CpuModel, &CpuVendor, &ClockFreq);
	    continue;
	} else if (EQ(PropPtr->Key, OBP_CPUID) && PropPtr->Value) {
	    cpuid = (int) strtol(PropPtr->Value, (char **)NULL, 0);
	} else if (EQ(PropPtr->Key, OBP_DEVTYPE) ||
		   EQ(PropPtr->Key, OBP_DEVTYPE2) ||
		   EQ(PropPtr->Key, OBP_CLOCKFREQ)) {
	    continue;
	}

	OBPsetOBPinfo(PropPtr, DevInfo);
    }

    if (cp = GetCpuName(&ncpus, cpuid))
	DevInfo->Name = strdup(cp);

    if (SearchNames && !SearchCheck(DevInfo->Name, SearchNames))
	return((DevInfo_t *) NULL);

    if (CpuVendor)
	if (!(DevInfo->Vendor = GetVendorName(CpuVendor)))
	    DevInfo->Vendor = CpuVendor;
    if (ClockFreq) {
	ClockFreqStr = FreqStr(ClockFreq);
	DevInfo->ModelDesc = strdup(ClockFreqStr);
	AddDevDesc(DevInfo, ClockFreqStr, "Speed", DA_INSERT);
    }
    if (CpuModel)
	DevInfo->Model = CpuModel;
    /*
     * This must be an unknown type of CPU
     */
    if (!DevInfo->Model)
	DevInfo->Model = PrimeDesc(DevInfo);

    DevInfo->Type = DT_CPU;
    DevInfo->NodeID = Node->NodeID;

    /*
     * The rest of this function depends on CpuModel being available
     * so if it's not, return what we have so far.
     */
    if (!CpuModel)
        return(DevInfo);

    /*
     * If no device by our name exists try to find a device
     * with a name which matches our CPU Model.  This should
     * be the root device node.  If so, make our device name
     * match the root device node name, so AddDevice() will 
     * add our info to the root device node later.
     */
    if (TreePtr && *TreePtr) {
	(void) memset(&Find, 0, sizeof(Find));
	Find.Tree = *TreePtr;
	Find.NodeName = DevInfo->Name;
	if (!DevFind(&Find)) {
	    Find.NodeName = CpuModelPart;
	    fdev = DevFind(&Find);
	    if (!fdev) {
		SImsg(SIM_DBG, "Cannot find CPU device <%s>", 
		      PRTS(CpuModelPart));
		cp = OBPfixNodeName(CpuModelPart);
		if (!EQ(cp, CpuModelPart)) {
		    Find.NodeName = cp;
		    fdev = DevFind(&Find);
		}
	    } else
		SImsg(SIM_DBG, "Found CPU device <%s>", PRTS(CpuModelPart));
	    if (fdev && !fdev->Master)
		DevInfo->Name = fdev->Name;
	}
    }

    if (CpuModelPart)
	(void) free(CpuModelPart);

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
	SImsg(SIM_DBG, "No Part List found.");
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
    static char			BoardName[64];
    static char			Buff[128];
    static DevFind_t		Find;
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

    (void) snprintf(BoardName, sizeof(BoardName), "%s%d", 
		    OBP_SYSBOARD, BoardNum);

    /*
     * Find the Old Master
     */
    if (DevInfo->Master) {
	(void) memset(&Find, 0, sizeof(Find));
	Find.Tree = *TreePtr;
	Find.NodeID = DevInfo->Master->NodeID;
	Find.NodeName = DevInfo->Master->Name;
	OldMaster = DevFind(&Find);
    }

    /*
     * If the Master (sysboard) device doesn't exit, create one.
     */
    (void) memset(&Find, 0, sizeof(Find));
    Find.Tree = *TreePtr;
    Find.NodeName = BoardName;
    if (!(Master = DevFind(&Find))) {
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
	BoardType = OBPfindPropVal(OBP_BOARDTYPE, OBPprop, NULL, NULL);
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
		(void) snprintf(Buff, sizeof(Buff),  "%s Board", What);
		Master->Model = strdup(Buff);
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
    static char			Buff[512];
    register char	       *bPtr;
    register char	       *End;
    char			Str[100];
    register char	       *cp;
    char		       *MemBuff;
    int				GrpNum = 0;

    if (!strchr(MemStr, '.'))
	return;

    Buff[0] = CNULL;
    bPtr = Buff;
    End = &Buff[sizeof(Buff)-1];
    MemBuff = strdup(MemStr);
    for (cp = strtok(MemBuff, "."); cp && bPtr < End; 
	 cp = strtok((char *)NULL, ".")) {
	(void) snprintf(bPtr, sizeof(Buff)-(End-bPtr), "#%d=%s", 
			GrpNum++, 
			GetSizeStr((Large_t) 
				   strtol(cp, (char **) NULL, 0), BYTES));
	if (bPtr != Buff)
	    strcat(bPtr, ", ");
	bPtr += strlen(bPtr);
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
    static DevFind_t		Find;
    register char	      **cpp;
    register char	       *cp;
    static char			Buff[128];
    int				BoardNum = -1;
    char		       *ObpModel = NULL;
    char		       *ObpModelBase = NULL;
    char		       *Vendor = NULL;
    Define_t		       *PartInfo = NULL;
    int				MemSize = 0;

    /*
     * See if we can find the node by it's NodeID or NodeName.
     * NodeName works for the root "system" node and a few others.
     */
    (void) memset(&Find, 0, sizeof(Find));
    Find.Tree = *TreePtr;
    Find.NodeID = Node->NodeID;
    Find.NodeName = Node->Name;
    DevInfo = DevFind(&Find);
    if (!DevInfo)
	return((DevInfo_t *)NULL);

    /*
     * Get all the info we need
     */
    for (PropPtr = Node->PropTable; PropPtr; PropPtr = PropPtr->Next) {
	/*
	 * Skip these
	 */
	if (EQ(PropPtr->Key, OBP_NAME))
	    continue;
	else if (EQ(PropPtr->Key, OBP_MODEL))
	    ObpModel = PropPtr->Value;
	else if (EQ(PropPtr->Key, OBP_BOARD) && PropPtr->Value)
	    BoardNum = strtol(PropPtr->Value, (char **) NULL, 0);
	else if (EQ(PropPtr->Key, OBP_SIZE) && 
		 EQ(Node->Name, OBP_MEMUNIT)) {
	    MemSize = OBPGetMemSize(PropPtr->Value);
	    OBPSetMemGrps(DevInfo, PropPtr->Value);
	    /*
	     * If this node has a key="keyboard" and is _NOT_ the
	     * node called "aliases", then this is the kbd parent
	     */
	} else if (EQ(PropPtr->Key, OBP_KEYBOARD) && 
		   !EQ(Node->Name, OBP_ALIASES)) {
	    DevInfo->Slaves = ProbeKbd((DevInfo_t **)NULL);
	}

	OBPsetOBPinfo(PropPtr, DevInfo);
    }

    /*
     * Handle special cases
     */
    if (MemSize && DevInfo->Model) {
	(void) snprintf(Buff, sizeof(Buff), "%d MB %s", 
			MemSize, DevInfo->Model);
	DevInfo->Model = strdup(Buff);
    }
    if (BoardNum >= 0)
	OBPsetBoard(DevInfo, Node->PropTable, BoardNum, TreePtr, SearchNames);

    /*
     * Add information to existing Device.
     */
    if (ObpModel) {
	/*
	 * If ObpModel is of form 'Foo,Bar' then use 'Foo' as the Vendor
	 * and 'Bar' as the actual Model.
	 */
	if (cp = strchr(ObpModel, ',')) {
	    ObpModelBase = cp + 1;
	    Vendor = (char *) xmalloc(cp - ObpModel);
	    (void) strncpy(Vendor, ObpModel, cp - ObpModel);
	    Vendor[cp - ObpModel] = CNULL;
	    if (cp = GetVendorName(Vendor)) {
		DevInfo->Vendor = cp;
		(void) free(Vendor);
	    } else
		DevInfo->Vendor = Vendor;
	}

	/*
	 * Now lookup the Part (including the Vendor part if any)
	 */
	PartInfo = OBPGetPart(ObpModel);
	if (PartInfo) {
	    DevInfo->Model = PartInfo->ValStr1;
	    AddDevDesc(DevInfo, PartInfo->ValStr2, NULL, DA_INSERT|DA_PRIME);
	} else if (!DevInfo->Model)
	    DevInfo->Model = (ObpModelBase) ? ObpModelBase : ObpModel;
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
    DevDefine_t		       *DevDefine = NULL;
    DevType_t		       *DevType = NULL;
    ClassType_t		       *ClassType = NULL;
    DevInfo_t		       *DevInfo = NULL;
    DevInfo_t		       *master = NULL;
    static DevFind_t		Find;

    /*
     * Iterate over all the properties looking for interesting properties
     * or until we find the device type.  If the device type is one which
     * we found in our probe list, go ahead and probe it.
     */
    for (PropPtr = Node->PropTable; PropPtr; PropPtr = PropPtr->Next) {
	SImsg(SIM_DBG, "OBPdevCheck:     NodeID 0x%x <%s> = <%s> (0x%x)",
	      Node->NodeID, PropPtr->Key, 
	      (PropPtr->Value) ? PropPtr->Value : "",
	      (PropPtr->Value) ? atol(PropPtr->Value) : 0);

	/*
	 * If this is the first node and this property is the Clock Frequency
	 * then save it for later use by the individual CPUs.
	 */
	if (first && !CpuClockFreq && EQ(PropPtr->Key, OBP_CLOCKFREQ) &&
	    PropPtr->Value)
	    CpuClockFreq = strtol(PropPtr->Value, (char **)NULL, 0);

	if (EQ(PropPtr->Key, OBP_DEVTYPE) || EQ(PropPtr->Key, OBP_DEVTYPE2)) {
	    /*
	     * OBP isn't consistant in what OBP_DEVTYPE really is so
	     * we do multiple lookups
	     */
	    DevDefine = DevDefGet(PropPtr->Value, 0, 0);
	    DevType = TypeGetByName(PropPtr->Value);
	    ClassType = ClassTypeGetByName(PropPtr->Value);
	}
    }

    SImsg(SIM_DBG, 
	  "OBPdevCheck: NodeID 0x%x Name = <%s> DevDefine.Name = <%s>",
	  Node->NodeID, (Node->Name[0]) ? Node->Name : "", 
	  (DevDefine && DevDefine->Name) ? DevDefine->Name : "");

    /*
     * We use DevDefine->Probe for our own internal (obp.c) use so we
     * need to make sure we only call the right Probe functions
     */
    if (DevDefine && DevDefine->Probe == OBPprobeCPU)
	DevInfo = OBPprobeCPU((ProbeData_t *) NULL,
			      Node, TreePtr, SearchNames);

    if (!DevInfo)
	DevInfo = OBPprobe(Node, TreePtr, SearchNames);

    if (DevInfo) {
	/*
	 * Find or create of Master
	 */
	if (TreePtr && !DevInfo->Master) {
	    (void) memset(&Find, 0, sizeof(Find));
	    Find.NodeID = Node->ParentID;
	    Find.Tree = *TreePtr;
	    master = DevFind(&Find);
	    if (!master && Node->ParentName) {
		master = NewDevInfo(NULL);
		master->Name = strdup(Node->ParentName);
		master->NodeID = Node->ParentID;
	    }
	    DevInfo->Master = master;
	}
	DevInfo->NodeID = Node->NodeID;
	if (DevType && !DevInfo->Type)
	    DevInfo->Type = DevType->Type;
	if (ClassType && !DevInfo->ClassType)
	    DevInfo->ClassType = ClassType->Type;
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
extern OBPnode_t *OBPfindNodeByID(Node, NodeID)
    OBPnode_t		       *Node;
    OBPnodeid_t		        NodeID;
{
    OBPnode_t		       *Found = NULL;

    if (!Node)
	Node = OBPnodeTree;

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

    if ((NodeType = OBPfindPropVal(OBP_DEVTYPE, Node->PropTable, NULL, NULL))
	||
	(NodeType = OBPfindPropVal(OBP_DEVTYPE2, Node->PropTable, NULL, NULL)))
	if (EQ(NodeType, MatchType))
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
 * Get the OBP property entry which matches "Key"
 * Look for Key in Table, by NodeID, or by NodeName (whichever is
 * not NULL).
 */
extern OBPprop_t *OBPfindProp(Key, Table, NodeID, NodeName)
    char		       *Key;
    OBPprop_t		       *Table;
    OBPnodeid_t		        NodeID;
    char		       *NodeName;
{
    register OBPprop_t	       *Ptr;
    OBPprop_t		       *PropTable = NULL;
    OBPnode_t		       *Node;

    if (Table) 
	PropTable = Table;
    else if (NodeID) {
	if (!OBPnodeTree)
	    OBPbuildNodeTree(&OBPnodeTree);
	Node = OBPfindNodeByID(OBPnodeTree, NodeID);
	if (Node)
	    PropTable = Node->PropTable;
    } else if (NodeName) {
	if (!OBPnodeTree)
	    OBPbuildNodeTree(&OBPnodeTree);
	Node = OBPfindNodeByName(OBPnodeTree, NodeName);
	if (Node)
	    PropTable = Node->PropTable;
    } else
	return((OBPprop_t *) NULL);

    /*
     * See if this node has it's own clock frequency
     */
    for (Ptr = PropTable; Ptr; Ptr = Ptr->Next)
	if (Ptr->Key[0] && EQ(Ptr->Key, Key))
	    return(Ptr);

    return((OBPprop_t *) NULL);
}

/*
 * Front-end to OBPfindProp() 
 */
extern char *OBPfindPropVal(Key, Table, NodeID, NodeName)
    char		       *Key;
    OBPprop_t		       *Table;
    OBPnodeid_t		        NodeID;
    char		       *NodeName;
{
    OBPprop_t		       *Prop;

    if (Prop = OBPfindProp(Key, Table, NodeID, NodeName))
	return(Prop->Value);
    else
	return((char *) NULL);
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
	NewNode->Name = OBPfindPropVal(OBP_NAME, NewNode->PropTable, 
				       NULL, NULL);

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
	SImsg(SIM_DBG, "OBPbuildeNodeTree: NULL NodeTree Ptr.");
	return(-1);
    }

    /*
     * Note: Only one process at a time can open _PATH_OPENPROM.
     */
    if ((obp_fd = open(_PATH_OPENPROM, O_RDONLY)) < 0) {
	SImsg(SIM_GERR, "Cannot open \"%s\": %s.", _PATH_OPENPROM, SYSERR);
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
	SImsg(SIM_GERR, "Cannot open \"%s\": %s.", _PATH_OPENPROM, SYSERR);
	return((char *) NULL);
    }

    op = &(opio.opio_oprom);
    op->oprom_size = sizeof(opio.opio_buff);

    if (ioctl(obp_fd, OPROMGETVERSION, op) < 0) {
	SImsg(SIM_GERR, "OBP ioctl OPROMGETVERSION failed: %s", SYSERR);
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
    static char		       *Banner;
    static int			CPUspeed = 0;
    static int			ClockFreq = 0;
    register char	       *cp;
    register char	       *End;
    SubSysVar_t		       *SubSysVar;
    OBPnode_t		       *OBPtree;

    SubSysVar = (SubSysVar_t *) Params;
    SubSysVar->IntVal = 0;
    SubSysVar->StrVal = NULL;
    OBPtree = SubSysVar->OBPtree;

    if (EQ(Variable, "BannerName")) {
	Banner = OBPfindPropVal(OBP_BANNERNAME, 
				OBPtree->PropTable, NULL, NULL);
	if (Banner) {
	    End = &Banner[strlen(Banner)];
	    if (cp = strchr(Banner, '-')) {
		if (strncmp(cp, "-slot ", 6) == 0 && cp < End)
		    Banner = cp + 6;
		else if (strncmp(cp, "-way ", 5) == 0 && cp < End)
		    Banner = cp + 5;
	    }
	    SubSysVar->StrVal = Banner;
	    return(Banner);
	}
    } else if (EQ(Variable, "ClockFreq")) {
	if (!ClockFreq) {
	    cp = OBPfindPropVal(OBP_CLOCKFREQ, OBPtree->PropTable, NULL, NULL);
	    if (cp)
		ClockFreq = (int) ((long)atoi(cp) / (long)MHERTZ);
	    else
		return((char *) NULL);	
	}
	SubSysVar->IntVal = ClockFreq;
	SubSysVar->StrVal = itoa(ClockFreq);
	return(SubSysVar->StrVal);
    } else if (EQ(Variable, "NumCPU")) {
	if (cp = GetNumCpu()) {
	    SubSysVar->IntVal = atoi(cp);
	    SubSysVar->StrVal = cp;
	}
	return(cp);
    } else if (EQ(Variable, "CPUspeed")) {
	if (!CPUspeed)
	    CPUspeed = OBPgetCPUspeed(OBPnodeTree);
	SubSysVar->IntVal = CPUspeed;
	SubSysVar->StrVal = itoa(CPUspeed);
	return(SubSysVar->StrVal);
    } else {
	SImsg(SIM_UNKN, "OBPsubSysGetVar: Unknown variable `%s'", Variable);
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

    ClockStr = OBPfindPropVal(OBP_CLOCKFREQ, PropTable, NULL, NULL);
    if (!ClockStr) {
	SImsg(SIM_DBG, "OBPgetCPUclockfreq: Could not find CPU clockfreq.");
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
	SImsg(SIM_DBG, "OBPgetCPUspeed: Could not find any CPU nodes.");
    }

    /*
     * Some machines have the clock frequency in the root node.
     */
    Speed = OBPgetCPUclockfreq(NodeTree->PropTable);
    if (Speed)
	return(Speed);
    else
	SImsg(SIM_DBG, "OBPgetCPUspeed: No clockfreq in root Node.");

    return(0);
}

/*
 * Set Conditionals.
 * Conditions are set in SubSysDef->Conditions.
 * If String is of form 'key=val', break apart and set Conditions.
 * If String is of form 'string' (no '=') and DefaultKey is set, do
 * backwards compatibility.
 */
Condition_t *SetCondition(SubSysDef, String, DefaultKey)
     Define_t		       *SubSysDef;
     char		       *String;
     char		       *DefaultKey;
{
    register char	       *cp;
    register char	       *StrKey = NULL;
    register char	       *StrVal = NULL;
    Condition_t		       *Cond = NULL;
    static Condition_t	       *LastCon;
    static Define_t	       *LastSubSysDef;

    /*
     * See if this is 'key=val'
     */
    cp = strchr(String, '=');
    if (!cp) {
	if (DefaultKey) {
	    /*
	     * Do backwards compatibility
	     */
	    StrKey = DefaultKey;
	    StrVal = String;
	} else
	    return((Condition_t *) NULL);
    } else
	*cp = CNULL;

    /*
     * If we've changed to a new SubSysDef, invalidate LastCon
     */
    if (LastSubSysDef && LastSubSysDef != SubSysDef)
	LastCon = NULL;
    LastSubSysDef = SubSysDef;

    Cond = (Condition_t *) xcalloc(1, sizeof(Condition_t));
    if (StrKey)
	Cond->Key = StrKey;
    else
	Cond->Key = String;
    if (StrVal) {
	Cond->StrVal = StrVal;
	Cond->IntVal = atoi(StrVal);
    } else if (++cp) {
	Cond->StrVal = cp;
	Cond->IntVal = atoi(cp);
    }

    /*
     * Add to end of linked list.
     */
    if (!SubSysDef->Conditions)
	LastCon = SubSysDef->Conditions = Cond;
    else {
	if (LastCon) {
	    LastCon->Next = Cond;
	    LastCon = Cond;
	} else
	    LastCon = SubSysDef->Conditions = Cond;
    }

    return(Cond);
}

/*
 * Get the sub system model type.
 * i.e. "Ultra-1 140"
 */
extern char *OBPgetSubSysModel(SubSysDef)
    Define_t		       *SubSysDef;
{
    static char			ErrBuff[128];
    static SubSysVar_t		SubSysVar;
    Condition_t		       *Conditions;
    register Condition_t       *Cond;
    int				AllMatch;
    register int		Argc;
    char		      **Argv = NULL;
    char		       *ExpandStr = NULL;
    char		       *SubSysName = NULL;
    int				CPUspeedDelta = CPU_SPEED_DELTA;
    char		       *cp;
    register int		i;

    if (SubSysDef->ValStr1)
	ExpandStr = SubSysDef->ValStr1;
    else
	return((char *) NULL);

    /*
     * Gather info needed for comparision and use in model name expansion
     */
    if (!OBPnodeTree)
	OBPbuildNodeTree(&OBPnodeTree);
    SubSysVar.OBPtree = OBPnodeTree;

    if (SubSysDef->Conditions) {
	/* Reset */
	for (Cond = SubSysDef->Conditions; Cond; Cond = Cond->Next)
	    Cond->Matches = FALSE;
    } else {
	/* Initialize */
	/*
	 * The first field (ValStr2) is what should be used normally.
	 * We default to CPUspeed if there is no 'key=val' for backwards
	 * compat.  The remaining ValStr[3-5] parse are also for backwards
	 * compat.
	 */
	if (SubSysDef->ValStr2 && *SubSysDef->ValStr2)
	    if (Argc = StrToArgv(SubSysDef->ValStr2, " ", &Argv, NULL, 0))
		for (i = 0; i < Argc; ++i)
		    (void) SetCondition(SubSysDef, Argv[i], "CPUspeed");
	if (SubSysDef->ValStr3 && *SubSysDef->ValStr3)
	    if (Argc = StrToArgv(SubSysDef->ValStr3, " ", &Argv, NULL, 0))
		for (i = 0; i < Argc; ++i)
		    (void) SetCondition(SubSysDef, Argv[i], "NumCPU");
	if (SubSysDef->ValStr4 && *SubSysDef->ValStr4)
	    if (Argc = StrToArgv(SubSysDef->ValStr4, " ", &Argv, NULL, 0))
		for (i = 0; i < Argc; ++i)
		    (void) SetCondition(SubSysDef, Argv[i], "HasDev");
	if (SubSysDef->ValStr5 && *SubSysDef->ValStr5)
	    if (Argc = StrToArgv(SubSysDef->ValStr5, " ", &Argv, NULL, 0))
		for (i = 0; i < Argc; ++i)
		    (void) SetCondition(SubSysDef, Argv[i], "HasType");
    }

    /*
     * Iterate through each condition and set Matches==TRUE if condition
     * is met.
     */
    for (Cond = SubSysDef->Conditions; Cond; Cond = Cond->Next) {
 	if (EQ("HasDev", Cond->Key) && Cond->StrVal) {
	    if (OBPfindNodeByName(OBPnodeTree, Cond->StrVal))
		Cond->Matches = TRUE;
	} else if (EQ("HasType", Cond->Key) && Cond->StrVal) {
	    if (OBPfindNodeByType(OBPnodeTree, Cond->StrVal))
		Cond->Matches = TRUE;
	} else if (EQ("CPUspeedDelta", Cond->Key) && Cond->IntVal) {
	    /* 
	     * Set the CPUspeedDelta 
	     * This variable specifies how much leeway is given when
	     * comparing CPUspeed (in MHz) below.
	     */
	    CPUspeedDelta = Cond->IntVal;
	    /* Don't use this entry for comparisons */
	    Cond->Flags |= CONFL_SETONLY;
	} else if (cp = OBPsubSysGetVar(Cond->Key, (Opaque_t) &SubSysVar)) {
	    if (EQ("CPUspeed", Cond->Key)) {
		/*
		 * SubSysVar.IntVal is system's actual CPUspeed.
		 * Cond->IntVal is CPUspeed from the .cf parameter.
		 * CPUspeed may vary by +/- CPUspeedDelta.
		 */
		if (SubSysVar.IntVal >= (Cond->IntVal - CPUspeedDelta) &&
		    SubSysVar.IntVal <= (Cond->IntVal + CPUspeedDelta))
		    Cond->Matches = TRUE;
	    } else if (EQ("NumCPU", Cond->Key)) {
	        if (SubSysVar.IntVal == Cond->IntVal)
		    Cond->Matches = TRUE;
	    } else if (EQ("ClockFreq", Cond->Key)) {
	        if (Cond->IntVal == atoi(cp))
		    Cond->Matches = TRUE;
	    }
	} else {
	    SImsg(SIM_DBG, "%s: Bad keyword in SubSysModel rule.",
		  Cond->Key);
	}
    }

    /*
     * If all conditions Match (are TRUE), then everything is GO.  If any ONE
     * item is not a Match (FALSE), then it's no GO.
     */
    for (AllMatch = TRUE, Cond = SubSysDef->Conditions; Cond; 
	 Cond = Cond->Next) {
	if (FLAGS_ON(Cond->Flags, CONFL_SETONLY))
	    continue;
	if (!Cond->Matches) {
	    AllMatch = FALSE;
	    break;
	}
    }

    /*
     * Expand name if needed.
     */
    if (AllMatch) {
	SubSysName = VarSub(ExpandStr, ErrBuff, sizeof(ErrBuff),
			    OBPsubSysGetVar, (Opaque_t) &SubSysVar);
	if (SubSysName)
	    return(SubSysName);
	else
	    SImsg(SIM_GERR, "Variable error in `%s': %s", ExpandStr, ErrBuff);
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
	SImsg(SIM_GERR, "Cannot open \"%s\": %s.", _PATH_OPENPROM, SYSERR);
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
	SImsg(SIM_GERR, "Get OBP Property table failed.");
	return((char *)NULL);
    }

    (void) close(obp_fd);

    /*
     * Find the root node's name
     */
    Name = OBPfindPropVal(OBP_NAME, OBPprop, NULL, NULL);
    if (!Name) {
	SImsg(SIM_GERR, "Could not find system model in OBP.");
	return((char *) NULL);
    }

    /*
     * See if there is a sub system model name for this.
     */
    SubSysDefs = DefGet(DL_SUBSYSMODEL, Name, 0, 0);
    if (!SubSysDefs) {
	/* No Sub System Model defined, so we just return the base name */
	SImsg(SIM_DBG, "No sub system model defined for `%s'", Name);
	return(Name);
    }

    for (DefPtr = SubSysDefs; DefPtr; DefPtr = DefPtr->Next)
	if (EQ(Name, DefPtr->KeyStr) && (SubName = OBPgetSubSysModel(DefPtr)))
	    return(SubName);

    return(Name);
}

#endif	/* HAVE_OPENPROM */
