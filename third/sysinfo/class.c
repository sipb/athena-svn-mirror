/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: class.c,v 1.1.1.1 1996-10-07 20:16:49 ghudson Exp $";
#endif

/*
 * Class Info
 */
#include "defs.h"

/*
 * Class information
 */
ClassInfo_t ClassInfo[] = {
    { CN_GENERAL,	"General Information",	GeneralShow,	GeneralList },
    { CN_KERNEL,	"Kernel Information",	KernelShow,	KernelList },
    { CN_SYSCONF,	"SysConf Information",	SysConfShow,	SysConfList },
    { CN_DEVICE,	"Device Information",	DeviceShow,	DeviceList },
    { 0 },
};

/*
 * List class names.
 */
extern void ClassList()
{
    register ClassInfo_t       *ClassPtr;

    printf(
"The following values may be specified with the `-class' option:\n");
    printf("%-20s %s\n", "VALUE", "DESCRIPTION");

    for (ClassPtr = &ClassInfo[0]; ClassPtr->Name; ++ClassPtr)
	printf("%-20s %s\n", ClassPtr->Name, ClassPtr->Label);
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
		Error("The class name `%s' is invalid.", cp);
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
 * Show a class label
 */
extern void ClassShowLabel(MyClass)
    ClassInfo_t		       *MyClass;
{
    if ((!VL_TERSE && !VL_BRIEF) && FormatType == FT_PRETTY)
	printf("\n\n\t%s\n\n", MyClass->Label);
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
    char			Buff[BUFSIZ];

    if (!Value || !*Value)
	return;

    if (VL_TERSE) {
	printf("%s", Value);
    } else if (VL_BRIEF) {
	printf("%s is %s", Key, Value);
    } else {
	if (Lbl) {
	    (void) strcpy(Buff, Lbl);
	    if (VL_ALL)
		(void) sprintf(Buff + strlen(Lbl), " (%s)", Key);
	} else
	    (void) strcpy(Buff, Key);
	(void) strcat(Buff, " is ");
	printf("%-*s %s", MaxLen + 5, Buff, Value);
    }

    if (Value[strlen(Value) - 1] != '\n')
	printf("\n");
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

    Argc = StrToArgv(NameStr, ",", &Argv);

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
