/*
 * Copyright (c) 1992-1998 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not
 * sold for profit or used in part or in whole for commercial gain
 * without prior written agreement, and the author is credited
 * appropriately.
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.3 $";
#endif

/*
 * Type Info
 */
#include "defs.h"

extern void PrintDiskdrive();
extern void PrintFrameBuffer();
extern void PrintMonitor();
extern void PrintNetIf();
extern void PrintDevice();
extern void PrintGeneric();

/*
 * Device Type information
 */
DevType_t DevTypes[] = {
    { DT_GENERIC,	DTN_GENERIC,	NULL,			PrintGeneric },
    { DT_PSEUDO,	DTN_PSEUDO,	"pseudo device",	PrintGeneric },
    /* DT_DRIVER is like DT_PSUEDO, but DT_DRIVER devices have unit#'s */
    { DT_DRIVER,	DTN_DRIVER,	"device driver",	PrintGeneric },
    { DT_DEVICE,	DTN_DEVICE,	"device",		PrintGeneric },
    { DT_BUS,		DTN_BUS,	"system bus",		PrintGeneric },
    { DT_BRIDGE,	DTN_BRIDGE,	"bus bridge",		PrintGeneric },
    { DT_CONTROLLER,	DTN_CONTROLLER,	"controller",		PrintGeneric },
    { DT_CARD,		DTN_CARD,	"system card",		PrintGeneric },
    { DT_CPU,		DTN_CPU,	"CPU",			PrintGeneric },
    { DT_DISKCTLR,	DTN_DISKCTLR,	"disk controller",	PrintGeneric },
    { DT_DISKCTLR,	DTN_MSD,	"disk controller",	PrintGeneric },
    { DT_DISKDRIVE,	DTN_DISKDRIVE,	"disk drive",	  PrintDiskdrive },
    { DT_FRAMEBUFFER,	DTN_FRAMEBUFFER,"frame buffer",	  PrintFrameBuffer },
    { DT_FRAMEBUFFER,	DTN_FB,		"frame buffer",	  PrintFrameBuffer },
    { DT_FRAMEBUFFER,	DTN_VID,	"frame buffer",	  PrintFrameBuffer },
    { DT_GFXACCEL,	DTN_GFXACCEL,	"graphics accelerator",	PrintGeneric },
    { DT_KEYBOARD,	DTN_KEYBOARD,	"Keyboard",	  	PrintGeneric },
    { DT_KEYBOARD,	DTN_KEY,	"Keyboard",	  	PrintGeneric },
    { DT_POINTER,	DTN_POINTER,	"pointer",	  	PrintGeneric },
    { DT_POINTER,	DTN_PTR,	"pointer",	  	PrintGeneric },
    { DT_POINTER,	DTN_MOUSE,	"mouse",	  	PrintGeneric },
    { DT_NETIF,		DTN_NETIF,	"network interface",  	PrintNetIf },
    { DT_NETIF,		DTN_NET,	"network interface",  	PrintNetIf },
    { DT_TAPECTLR,	DTN_TAPECTLR,	"tape controller",	PrintGeneric },
    { DT_TAPEDRIVE,	DTN_TAPEDRIVE,	"tape drive",		PrintGeneric },
    { DT_CDROM,		DTN_CDROM,	"CD-ROM drive",		PrintGeneric },
    { DT_FLOPPY,	DTN_FLOPPY,	"floppy disk drive", PrintDiskdrive },
    { DT_FLOPPYCTLR,	DTN_FLOPPYCTLR,	"floppy disk controller",PrintGeneric},
    { DT_MEMORY,	DTN_MEMORY,	"memory",		PrintGeneric },
    { DT_MEMORY,	DTN_MEM,	"memory",		PrintGeneric },
    { DT_SERIAL,	DTN_SERIAL,	"serial device",	PrintGeneric },
    { DT_SERIAL,	DTN_COM,	"serial device",	PrintGeneric },
    { DT_PARALLEL,	DTN_PARALLEL,	"parallel device",	PrintGeneric },
    { DT_PARALLEL,	DTN_PRT,	"parallel device",	PrintGeneric },
    { DT_AUDIO,		DTN_AUDIO,	"audio device",		PrintGeneric },
    { DT_MFC,		DTN_MFC,	"Multi Function Card",	PrintGeneric },
    { DT_MONITOR,	DTN_MONITOR,	"video monitor",	PrintMonitor },
    { DT_PRINTER,	DTN_PRINTER,	"printer",		PrintGeneric },
    { DT_WORM,		DTN_WORM,	"WORM",			PrintGeneric },
    { DT_SCANNER,	DTN_SCANNER,	"scanner",		PrintGeneric },
    { DT_CONSOLE,	DTN_CONSOLE,	"console",		PrintGeneric },
    { 0 },
};

/*
 * List type names.
 */
extern void TypeList()
{
    register DevType_t	       *TypePtr;

    SImsg(SIM_INFO, 
"The following values may be specified with the `-type' option:\n");
    SImsg(SIM_INFO, "%-20s %s\n", "VALUE", "DESCRIPTION");

    for (TypePtr = DevTypes; TypePtr->Name; ++TypePtr)
	SImsg(SIM_INFO, "%-20s %s\n", PRTS(TypePtr->Name), PRTS(TypePtr->Desc));
}

/*
 * Get a device type.
 */
extern DevType_t *TypeGetByType(DevType)
    int				DevType;
{
    register int 		i;

    if (DevType <= 0)
	return((DevType_t *) NULL);

    for (i = 0; DevTypes[i].Name; ++i)
	if (DevTypes[i].Type == DevType)
	    return(&DevTypes[i]);

    return((DevType_t *) NULL);
}

/*
 * Get a device type by name
 */
extern DevType_t *TypeGetByName(Name)
    char		       *Name;
{
    register DevType_t	       *TypePtr;

    if (!Name)
	return((DevType_t *) NULL);

    for (TypePtr = DevTypes; TypePtr->Name; ++TypePtr)
	if (EQ(TypePtr->Name, Name))
	    return(TypePtr);

    return((DevType_t *) NULL);
}

/*
 * Set type information
 */
extern void TypeSetInfo(Names)
    char		       *Names;
{
    register DevType_t	       *TypePtr;
    register char	       *cp;
    char		       *String;

    if (Names) {
	/*
	 * Enable a specific list of typees.
	 * The Names variable is a `,' seperated list
	 * of type names.
	 */
	String = Names;		/* XXX Names param is altered */
	for (cp = strtok(Names, ","); cp; cp = strtok((char *)NULL, ",")) {
	    TypePtr = TypeGetByName(cp);
	    if (!TypePtr) {
		SImsg(SIM_GERR, "The type name `%s' is invalid.", cp);
		TypeList();
		exit(1);
	    }
	    TypePtr->Enabled = TRUE;
	}
    } else 
	/*
	 * No specific type names were specified so enabled all of them.
	 */
	for (TypePtr = DevTypes; TypePtr->Name; ++TypePtr)
	    TypePtr->Enabled = TRUE;
}
