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
 * Class Info
 */
#include "defs.h"

/*
 * Class information
 */
ClassInfo_t ClassInfo[] = {
    { CN_GENERAL,	GeneralShow,	GeneralList,
      "General Information", "G E N E R A L   I N F O R M A T I O N" },
    { CN_KERNEL,	KernelShow,	KernelList,
      "Kernel Information", "K E R N E L   I N F O R M A T I O N" },
    { CN_SYSCONF,	SysConfShow,	SysConfList,
      "SysConf Information", "S Y S C O N F   I N F O R M A T I O N" },
    { CN_DEVICE,	DeviceShow,	DeviceList,
      "Device Information", "D E V I C E   I N F O R M A T I O N" },
    { 0 },
};

/*
 * List class names.
 */
extern void ClassList()
{
    register ClassInfo_t       *ClassPtr;

    SImsg(SIM_INFO, 
"The following values may be specified with the `-class' option:\n");
    SImsg(SIM_INFO, "%-20s %s\n", "VALUE", "DESCRIPTION");

    for (ClassPtr = &ClassInfo[0]; ClassPtr->Name; ++ClassPtr)
	SImsg(SIM_INFO, "%-20s %s\n", ClassPtr->Name, ClassPtr->Label);
}

/*
 * Lookup a class named Name.
 */
static ClassInfo_t *ClassGetName(Name)
    char		       *Name;
{
    register ClassInfo_t       *ClassPtr;

    for (ClassPtr = &ClassInfo[0]; ClassPtr->Name; ++ClassPtr)
	if (EQ(Name, ClassPtr->Name))
	    return(ClassPtr);

    return((ClassInfo_t *) NULL);
}

/*
 * Set class information
 */
extern void ClassSetInfo(Names)
    char		       *Names;
{
    register ClassInfo_t       *ClassPtr;
    register char	       *cp;
    char		       *String;

    if (Names) {
	/*
	 * Enable a specific list of classes.
	 * The Names variable is a `,' seperated list
	 * of class names.
	 */
	String = Names;		/* XXX Names param is altered */
	for (cp = strtok(Names, ","); cp; cp = strtok((char *)NULL, ",")) {
	    ClassPtr = ClassGetName(cp);
	    if (!ClassPtr) {
		SImsg(SIM_GERR, "The class name `%s' is invalid.", cp);
		ClassList();
		exit(1);
	    }
	    ClassPtr->Enabled = TRUE;
	}
    } else 
	/*
	 * No specific class names were specified so enabled all of them.
	 */
	for (ClassPtr = ClassInfo; ClassPtr->Name; ++ClassPtr)
	    ClassPtr->Enabled = TRUE;
}

/*
 * Show a class Banner
 */
extern void ClassShowBanner(MyClass)
    ClassInfo_t		       *MyClass;
{
    if ((!VL_TERSE && !VL_BRIEF) && FormatType == FT_PRETTY)
	SImsg(SIM_INFO, "\n\n\t%s\n\n", MyClass->Banner);
}

/*
 * Show a Class Value
 */
extern void ClassShowValue(Lbl, Key, Value, MaxLen)
    char 		       *Lbl;
    char 		       *Key;
    char 		       *Value;
    int				MaxLen;
{
    char			Buff[256];
    int				Len;

    if (!Value || !*Value)
	return;

    if (VL_TERSE) {
	SImsg(SIM_INFO, "%s", Value);
    } else if (VL_BRIEF) {
	SImsg(SIM_INFO, "%s is %s", Key, Value);
    } else {
	if (Lbl) {
	    (void) strcpy(Buff, Lbl);
	    if (VL_ALL) {
		Len = strlen(Lbl);
		(void) snprintf(Buff + Len, sizeof(Buff)-Len, " (%s)", Key);
	    }
	} else
	    (void) strcpy(Buff, Key);
	(void) strcat(Buff, " is ");
	SImsg(SIM_INFO, "%-*s %s", MaxLen + 5, Buff, Value);
    }

    if (Value[strlen(Value) - 1] != '\n')
	SImsg(SIM_INFO, "\n");
}

/*
 * Call each enabled show function
 */
extern int ClassCall(NameStr)
    char		       *NameStr;
{
    ClassInfo_t		       *ClassPtr;
    char		      **Argv;
    int				Argc;

    Argc = StrToArgv(NameStr, ",", &Argv, NULL, 0);

    for (ClassPtr = &ClassInfo[0]; ClassPtr->Name; ++ClassPtr)
	if (ClassPtr->Enabled && ClassPtr->Show)
	    (*ClassPtr->Show)(ClassPtr,(Argc > 0) ? Argv : (char **) NULL);

    return(0);
}

/*
 * Call each Class List function
 */
extern void ClassCallList()
{
    register ClassInfo_t       *ClassPtr;

    for (ClassPtr = &ClassInfo[0]; ClassPtr->Name; ++ClassPtr)
	if (ClassPtr->List)
	    (*ClassPtr->List)();
}
