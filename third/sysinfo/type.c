/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: type.c,v 1.1.1.1 1996-10-07 20:16:49 ghudson Exp $";
#endif

/*
 * Type Info
 */
#include "defs.h"

extern void PrintDiskdrive();
extern void PrintFrameBuffer();
extern void PrintNetIf();
extern void PrintDevice();
extern void PrintGeneric();

/*
 * Device Type information
 */
DevType_t DevTypes[] = {
    { DT_GENERIC,	DTN_GENERIC,	NULL,			PrintGeneric },
    { DT_DEVICE,	DTN_DEVICE,	"device",		PrintGeneric },
    { DT_BUS,		DTN_BUS,	"system bus",		PrintGeneric },
    { DT_CARD,		DTN_CARD,	"system card",		PrintGeneric },
    { DT_CPU,		DTN_CPU,	"CPU",			PrintGeneric },
    { DT_DISKCTLR,	DTN_DISKCTLR,	"disk controller",	PrintGeneric },
    { DT_DISKDRIVE,	DTN_DISKDRIVE,	"disk drive",	  PrintDiskdrive },
    { DT_FRAMEBUFFER,	DTN_FRAMEBUFFER,"frame buffer",	  PrintFrameBuffer },
    { DT_KEYBOARD,	DTN_KEYBOARD,	"Keyboard",	  	PrintGeneric },
    { DT_NETIF,		DTN_NETIF,	"network interface",  	PrintNetIf },
    { DT_PSEUDO,	DTN_PSEUDO,	"pseudo device",	PrintGeneric },
    { DT_TAPECTLR,	DTN_TAPECTLR,	"tape controller",	PrintGeneric },
    { DT_TAPEDRIVE,	DTN_TAPEDRIVE,	"tape drive",		PrintGeneric },
    { DT_CDROM,		DTN_CDROM,	"CD-ROM drive",		PrintGeneric },
    { DT_MEMORY,	DTN_MEMORY,	NULL,			PrintGeneric },
    { DT_SERIAL,	DTN_SERIAL,	"serial device",	PrintGeneric },
    { DT_PARALLEL,	DTN_PARALLEL,	"parallel device",	PrintGeneric },
    { DT_AUDIO,		DTN_AUDIO,	"audio device",		PrintGeneric },
    { 0 },
};

/*
 * List type names.
 */
extern void TypeList()
{
    register DevType_t	       *TypePtr;

    printf(
"The following values may be specified with the `-type' option:\n");
    printf("%-20s %s\n", "VALUE", "DESCRIPTION");

    for (TypePtr = DevTypes; TypePtr->Name; ++TypePtr)
	printf("%-20s %s\n", PS(TypePtr->Name), PS(TypePtr->Desc));
}

/*
 * Lookup a type named Name.
 */
static DevType_t *TypeGetName(Name)
    char		       *Name;
{
    register DevType_t	       *TypePtr;

    for (TypePtr = DevTypes; TypePtr->Name; ++TypePtr)
	if (EQ(Name, TypePtr->Name))
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
	    TypePtr = TypeGetName(cp);
	    if (!TypePtr) {
		Error("The type name `%s' is invalid.", cp);
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
