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
 * ClassType Info
 */
#include "defs.h"

/*
 * ClassType definetions
 */
static ClassType_t ClassTypes[] = {
    { { DT_DISKDRIVE, DT_TAPEDRIVE, DT_CDROM, DT_DISKCTLR, DT_CONTROLLER, 0 },
      CT_SCSI,		CT_SCSI_S,	NULL },
    { { DT_DISKDRIVE, DT_TAPEDRIVE, DT_CDROM, DT_DISKCTLR, DT_CONTROLLER, 0 },
      CT_IPI,		CT_IPI_S,	NULL },
    { { DT_DISKDRIVE, DT_TAPEDRIVE, DT_CDROM, DT_DISKCTLR, DT_CONTROLLER, 0 },
      CT_SMD,		CT_SMD_S,	NULL },
    { { DT_DISKDRIVE, DT_TAPEDRIVE, DT_CDROM, DT_DISKCTLR, DT_CONTROLLER, 0 },
      CT_ATA,		CT_ATA_S,	"ATA/IDE/EIDE/UIDE" },
    { { DT_DISKDRIVE, DT_TAPEDRIVE, DT_CDROM, DT_DISKCTLR, DT_CONTROLLER, 0 },
      CT_ATA,		CT_IDE_S,	"ATA/IDE/EIDE/UIDE" },
    { { DT_BUS, 0 },    CT_PCI,		CT_PCI_S,	NULL },
    { { DT_BUS, 0 },    CT_ISA,		CT_ISA_S,	NULL },
    { { DT_BUS, 0 },    CT_PNPISA,	CT_PNPISA_S,	"Plug-N-Play ISA" },
    { { DT_BUS, 0 },    CT_EISA,	CT_EISA_S,	NULL },
    { { DT_BUS, 0 },    CT_MCA,		CT_MCA_S,	NULL },
    { { DT_BUS, 0 },    CT_SBUS,	CT_SBUS_S,	NULL },
    { { DT_NETIF, 0 },  CT_ETHER10M,	CT_ETHER10M_S,	"10-Mb Ethernet" },
    { { DT_NETIF, 0 },  CT_ETHER10M,	"ETHER",	"10-Mb Ethernet" },
    { { DT_NETIF, 0 },  CT_ETHER100M,	CT_ETHER100M_S,"100-Mb Fast Ethernet"},
    { { DT_NETIF, 0 },  CT_ETHER1G,	CT_ETHER1G_S, "1-GB Gigabit Ethernet"},
    { { DT_NETIF, 0 },  CT_FDDI,	CT_FDDI_S,	NULL },
    { { DT_NETIF, 0 },  CT_ATM,		CT_ATM_S,	NULL },
    { { DT_NETIF, 0 },  CT_TOKEN,	CT_TOKEN_S,	"Token Ring" },
    { { DT_NETIF, 0 },  CT_HIPPI,	CT_HIPPI_S,	NULL },
    { { DT_NETIF, 0 },  CT_ISDN,	CT_ISDN_S,	NULL },
    { { DT_MONITOR, 0 },CT_RGBCOLOR,	CT_RGBCOLOR_S,	"RGB Color" },
    { { DT_MONITOR, 0 },CT_NONRGBCOLOR,	CT_NONRGBCOLOR_S,"Non-RGB Color" },
    { { DT_MONITOR, 0 },CT_MONO,	CT_MONO_S,    "Monochrome/grayscale" },
    { 0 },
};

/*
 * Does DevType appear in ClassType->DevTypes list?
 */
static int IsDevType(DevType, ClassType)
     int			DevType;
     ClassType_t	       *ClassType;
{
    register int		i;

    for (i = 0; i < MAX_CLASS_DEVTYPES; ++i)
	if (ClassType->DevTypes[i] == DevType)
	    return(TRUE);

    return(FALSE);
}

/*
 * Lookup by DevType + ClassType.
 */
extern ClassType_t *ClassTypeGetByType(DevType, ClassType)
    int				DevType;
    int				ClassType;
{
    register ClassType_t       *Type;

    if (ClassType <= 0 || DevType <= 0)
	return((ClassType_t *) NULL);

    for (Type = ClassTypes; Type->Name; ++Type)
	if (IsDevType(DevType, Type) &&
	    Type->Type == ClassType)
	    return(Type);

    SImsg(SIM_DBG, "ClassTypeGetByType: Cannot find DevType=%d ClassType=%d",
	  DevType, ClassType);

    return((ClassType_t *) NULL);
}

/*
 * Lookup using DevType and Name of ClassType
 */
extern ClassType_t *ClassTypeGetByName(DevType, Name)
     int			DevType;
     char		       *Name;
{
    register ClassType_t       *TypePtr;

    if (!Name || DevType <= 0)
	return((ClassType_t *) NULL);

    for (TypePtr = ClassTypes; TypePtr->Name; ++TypePtr)
	if (IsDevType(DevType, TypePtr) && EQ(TypePtr->Name, Name))
	    return(TypePtr);

    SImsg(SIM_DBG, "ClassTypeGetByName: Cannot find DevType=%d ClassName=<%s>",
	  DevType, Name);

    return((ClassType_t *) NULL);
}
