#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * FreeBSD ISA functions
 * As of FreeBSD 4.0, all the ISA headers where moved out of /usr/include
 * and into /usr/src/sys only.  So it looks as if the ISA code is being
 * phased out.  Therefor, we no longer support ISA on FreeBSD 4.0 and later.
 */

#include "defs.h"

#if	OSMVER <= 3

/*
 * ISA table type
 */
typedef struct {
    char		       *SymName;	/* Symbol name */
    int				DevType;	/* Type of Devices */
} ISAtable_t;

/*
 * List of ISA device tables to look for in kernel
 */
ISAtable_t ISAtables[] = {
    { "_isa_devtab_tty",	DT_SERIAL },
    { "_isa_devtab_bio",	0 },
    { "_isa_devtab_net",	DT_NETIF },
    { "_isa_devtab_null",	0 },
    { 0 },
};

static kvm_t		       *kd;

/*
 * Get the driver name for IsaDev.
 */
static char *ISAgetDriverName(IsaDev)
     struct isa_device	       *IsaDev;
{
    static struct isa_driver	Driver;
    static char			Name[64];

    if (!IsaDev)
	return((char *) NULL);

    if (!IsaDev->id_driver) {
	SImsg(SIM_DBG, "No id_driver found for IsaDev 0x%x", IsaDev);
	return((char *) NULL);
    }

    /*
     * Read in the id_driver
     */
    if (KVMget(kd, (KVMaddr_t) IsaDev->id_driver, (void *) &Driver,
		   sizeof(Driver), KDT_DATA) != 0) {
	SImsg(SIM_GERR, "Cannot read isa_driver entry from 0x%x.", 
	      IsaDev->id_driver);
	return((char *) NULL);
    }

    /*
     * Now read in the actual driver name.
     */
    Name[0] = CNULL;
    if (KVMget(kd, (KVMaddr_t) Driver.name, (void *) Name,
		   sizeof(Name), KDT_STRING) != 0) {
	SImsg(SIM_GERR, "Cannot read id_driver.name entry from 0x%x.", 
	      Driver.name);
	return((char *) NULL);
    }

    if (!Name[0])
	return((char *) NULL);
    else
	return(Name);
}

/*
 * Probe an ISA device.
 *
 * Return -1 on error.
 * Return 0 on success.
 */
static int ISAprobeDevice(IsaDev, DrName, DevType, IsaMaster, 
			  TreePtr, SearchNames)
     struct isa_device	       *IsaDev;
     char		       *DrName;
     int			DevType;
     DevInfo_t 		       *IsaMaster;
     DevInfo_t 		      **TreePtr;
     char		      **SearchNames;
{
    DevInfo_t		       *DevInfo;
    DevDefine_t		       *DevDef = NULL;
    static DevData_t		DevData;

    if (!IsaDev || !DrName)
	return(-1);

    SImsg(SIM_DBG, "ISA: Probing <%s(%d)> DevType=%d", 
	  DrName, IsaDev->id_unit, DevType);

    (void) memset(&DevData, 0, sizeof(DevData));
    DevData.NodeID = IsaDev->id_id;
    DevData.DevName = DrName;
    DevData.DevUnit = IsaDev->id_unit;
    DevData.DevType = DevType;
    DevData.Flags = DD_IS_ALIVE;
    DevData.CtlrDevInfo = IsaMaster;
    DevDef = DevDefGet(DrName, 0, 0);

    if (DevInfo = ProbeDevice(&DevData, TreePtr, SearchNames, DevDef)) {
	/* 
	 * Add ISA specific info to DevInfo
	 */
	if (IsaDev->id_irq > 0)
	    AddDevDesc(DevInfo, itoax(IsaDev->id_irq), "IRQ", DA_APPEND);
	if (IsaDev->id_iobase > 0)
	    AddDevDesc(DevInfo, itoax(IsaDev->id_iobase), "IObase", DA_APPEND);
	if (IsaDev->id_drq > 0)
	    AddDevDesc(DevInfo, itoax(IsaDev->id_drq), "DRQ", DA_APPEND);
	if (IsaDev->id_msize > 0)
	    AddDevDesc(DevInfo, itoa(IsaDev->id_msize), "Size of I/O Memory",
		       DA_APPEND);
	if (IsaDev->id_maddr > 0)
	    AddDevDesc(DevInfo,  itoax(IsaDev->id_maddr), 
		       "Physical I/O Memory Address", DA_APPEND);
	/* 
	 * Now add the device to the Device Tree
	 */
	AddDevice(DevInfo, TreePtr, SearchNames);
	return(0);
    }
	
    return(-1);
}

/*
 * Iterate over a isa_devtab_* table and call the probe functin for
 * each device we find.
 */
static int ISAprobeDevTab(DevTable, DevType, TreePtr, SearchNames)
     struct isa_device	       *DevTable;
     int			DevType;
     DevInfo_t 		      **TreePtr;
     char		      **SearchNames;
{
    static DevInfo_t	       *IsaMaster;
    static struct isa_device	Device;
    struct isa_device	       *Ptr;
    char		       *DrName;
    int				Found = 0;

    if (!DevTable)
	return(-1);

    if (!IsaMaster) {
	/* Create a nice top master node */
	IsaMaster = NewDevInfo(NULL);
	IsaMaster->Driver = "isa";
	IsaMaster->Name = "isa0";	/* Fake */
	IsaMaster->Unit = 0;		/* Fake */
	IsaMaster->Type = DT_BUS;
	IsaMaster->ClassType = CT_ISA;
	AddDevice(IsaMaster, TreePtr, SearchNames);
    }

    for (Ptr = DevTable; Ptr; ++Ptr) {
	if (KVMget(kd, (KVMaddr_t) Ptr, (void *) &Device,
		   sizeof(Device), KDT_DATA) != 0) {
	    SImsg(SIM_GERR, "Cannot read isa_device entry from 0x%x.", Ptr);
	    /* Return now since this may be serious */
	    return(-1);
	}

	if (!Device.id_driver)
	    break;

	if (!(DrName = ISAgetDriverName(&Device)))
	    continue;

	SImsg(SIM_DBG, 
"ISA: <%s(%d)> id=0x%x (%d) iobase=0x%x irq=0x%x alive=%d enabled=%d flags=0x%x",
	      DrName, Device.id_unit, Device.id_id, Device.id_id,
	      Device.id_iobase, Device.id_irq,
	      Device.id_alive, Device.id_enabled,
	      Device.id_flags);

	/*
	 * Only probe device if it's enabled
	 */
	if (Device.id_enabled)
	    if (ISAprobeDevice(&Device, DrName, DevType, 
			       IsaMaster, TreePtr, SearchNames) == 0)
		++Found;
    }

    return(Found);
}

/*
 * Get the pointer to the first entry of a isa_devtab_* table from the kernel
 */
static struct isa_device *ISAgetDevTabPtr(Symbol)
     char		       *Symbol;
{
    struct nlist	       *nlPtr;
    struct isa_device	       *DevTabPtr;

    if (!Symbol)
	return((struct isa_device *) NULL);

    if ((nlPtr = KVMnlist(kd, Symbol, (struct nlist *)NULL, 0)) == NULL)
	return((struct isa_device *) NULL);

    if (CheckNlist(nlPtr))
	return((struct isa_device *) NULL);

    DevTabPtr = (struct isa_device *) nlPtr->n_value;

    return(DevTabPtr);
}
#endif	/* OSMVER <= 3 */

/*
 * Probe for ISA devices
 */
extern int ISAprobe(TreePtr, SearchNames)
    DevInfo_t 		       **TreePtr;
    char		       **SearchNames;
{
#if	OSMVER <= 3
    ISAtable_t		       *Table;
    struct isa_device	       *DevTabPtr;
    int				Found = 0;

    SImsg(SIM_DBG, "ISA: Probing...");

    if (!kd)
	if (!(kd = KVMopen()))
	    return(-1);

    for (Table = ISAtables; Table && Table->SymName; ++Table) {
	if (DevTabPtr = ISAgetDevTabPtr(Table->SymName))
	    Found += ISAprobeDevTab(DevTabPtr, Table->DevType, 
				    TreePtr, SearchNames);
    }

    (void) KVMclose(kd);
    kd = NULL;

    if (Found)
	return(0);
    else {
	SImsg(SIM_DBG, "ISA: No devices where found.");
	return(-1);
    }
#else
    return -1;
#endif	/* OSMVER <= 3 */
}
