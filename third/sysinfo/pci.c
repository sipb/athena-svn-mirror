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
 * General PCI functions
 */

#include "defs.h"
#include "probe.h"

/*
 * Set what PCI Device info we can by looking things up in pci.cf
 */
extern void PCIsetDeviceInfo(Info)
     PCIinfo_t		       *Info;
{
    DevInfo_t		       *DevInfo = NULL;
    DevType_t		       *DevType = NULL;
    ClassType_t		       *ClassType = NULL;
    Define_t		       *Define;
    static char			Buff[256];
    static char			Query[64];
    int				Len;
    PCIid_t			VendorID;
    PCIid_t			DeviceID;

    /*
     * Ok for empty DeviceID
     */
    if (!Info || !Info->VendorID)
	return;

    VendorID = Info->VendorID;
    DeviceID = Info->DeviceID;
    if (Info->DevInfo)
	DevInfo = Info->DevInfo;
    else
	DevInfo = Info->DevInfo = NewDevInfo(NULL);

    /* See if we can find the Vendor */
    Define = DefGet("PCIvendor", NULL, VendorID);
    if (!Define) {
	/*
	 * The VendorID is unknown so use the ID numbers.
	 */
	SImsg(SIM_UNKN, "PCI Vendor=0x%04x Device=0x%04x", VendorID, DeviceID);
    } else {
	/*
	 * We found the vendor
	 */
	DevInfo->Vendor = Define->ValStr1;

	/* Now see if we can find a device from the vendor */
	(void) snprintf(Query, sizeof(Query), "PCIdevice0x%04x", VendorID);
	if (Define = DefGet(Query, NULL, DeviceID)) {
	    /*
	     * We found the Device!
	     */
	    DevInfo->Model = Define->ValStr1;

	    if (Define->ValStr2)
		/*
		 * See if we can determine the DevType
		 */
		if (DevType = TypeGetByName(Define->ValStr2))
		    DevInfo->Type = DevType->Type;
	    if (Define->ValStr3) {
		/*
		 * Now see if we can find the ClassType for this DevType 
		 */
		if (DevType) {
		    if (ClassType = ClassTypeGetByName(DevType->Type, 
						       Define->ValStr3))
			DevInfo->ClassType = ClassType->Type;
		}
		if (!ClassType)
		    /* Couldn't find the class type so use the name we have */
		    DevInfo->ModelDesc = Define->ValStr3;
	    }
	} else {
	    SImsg(SIM_UNKN, "PCI device 0x%04x from %s (0x%04x)", 
		  DeviceID, DevInfo->Vendor, VendorID);
	}
    }

    if (!DevType && !DevInfo->Model && !DevInfo->ModelDesc && 
	!DevInfo->ClassType)
	DevInfo->ModelDesc = "PCI";

    if (Info->Bus >= 0 && Info->Unit >= 0 && Info->SubUnit >= 0) {
	(void) snprintf(Buff, sizeof(Buff), "pci%d:%d:%d", 
			Info->Bus, Info->Unit, Info->SubUnit);
	AddDevDesc(DevInfo, Buff, "PCI Name", DA_APPEND);
    }
    if (VendorID) {
	(void) snprintf(Buff, sizeof(Buff), "0x%04x", VendorID);
	AddDevDesc(DevInfo, Buff, "PCI Vendor ID", DA_APPEND);
    }
    if (DeviceID) {
	(void) snprintf(Buff, sizeof(Buff), "0x%04x", DeviceID);
	AddDevDesc(DevInfo, Buff, "PCI Device ID", DA_APPEND);
    }
#define VI(i)	( (PCIid_t)(i) != 0 && (PCIid_t)(i) != (PCIid_t)-1 )
    if (VI(Info->Class)) {
	(void) snprintf(Buff, sizeof(Buff), "0x%06x", Info->Class);
	AddDevDesc(DevInfo, Buff, "PCI Device Class", DA_APPEND);
    }
    if (VI(Info->SubDeviceID)) {
	(void) snprintf(Buff, sizeof(Buff), "0x%08lx", Info->SubDeviceID);
	AddDevDesc(DevInfo, Buff, "PCI Device Sub Vendor ID", DA_APPEND);
    }
    if (VI(Info->Revision)) {
	(void) snprintf(Buff, sizeof(Buff), "0x%02x", Info->Revision);
	AddDevDesc(DevInfo, Buff, "PCI Device Revision", DA_APPEND);
    }
    if (VI(Info->Header)) {
	(void) snprintf(Buff, sizeof(Buff), "0x%02x", Info->Header);
	AddDevDesc(DevInfo, Buff, "PCI Device Header", DA_APPEND);
    }
#undef VI
}

/*
 * Create and return a new PCInfo_t
 */
extern PCIinfo_t *PCInewInfo(Info)
     PCIinfo_t		       *Info;
{
    PCIinfo_t		       *New;

    if (Info) {
	(void) memset(Info, 0, sizeof(PCIinfo_t));
	New = Info;
    } else
	New = (PCIinfo_t *) xcalloc(1, sizeof(PCIinfo_t));

    New->Bus = New->Unit = New->SubUnit = -1;
    New->Header = New->Revision = New->Class = New->SubDeviceID = -1;

    return(New);
}
