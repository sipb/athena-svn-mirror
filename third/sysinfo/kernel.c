/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: kernel.c,v 1.1.1.1 1996-10-07 20:16:48 ghudson Exp $";
#endif

/*
 * Kernel Variables
 */
#include "defs.h"

static char			ValBuff[100];

typedef struct {
    char		       *Name;		/* Data type name */
    char		     *(*Read)();	/* Function to read it */
} DataEntry_t;

static char		       *DTint();
static char		       *DTuint();
static char		       *DTshort();
static char		       *DTushort();
static char		       *DTlong();
static char		       *DTulong();
static char		       *DTstring();

#define DT_BOOL			"bool"		/* Boolean */

DataEntry_t DataTable[] = {
    { "int",		DTint },
    { "uint",		DTuint },
    { "short",		DTshort },
    { "ushort",		DTushort },
    { "long",		DTlong },
    { "ulong",		DTulong },
    { "string",		DTstring },
    { DT_BOOL		},
    { 0 },
};

/*
 * Read the data type `int'
 */
static char *DTint(kd, Addr)
    kvm_t		       *kd;
    OFF_T_TYPE			Addr;
{
    int				Val;

    if (KVMget(kd, Addr, (char *) &Val, sizeof(Val), KDT_DATA)) {
	if (Debug) Error("Read int value from 0x%x failed.", Addr);
	return((char *) NULL);
    }
    (void) sprintf(ValBuff, "%d", Val);

    return(ValBuff);
}

/*
 * Read the data type `uint'
 */
static char *DTuint(kd, Addr)
    kvm_t		       *kd;
    OFF_T_TYPE			Addr;
{
    u_int			Val;

    if (KVMget(kd, Addr, (char *) &Val, sizeof(Val), KDT_DATA)) {
	if (Debug) Error("Read uint value from 0x%x failed.", Addr);
	return((char *) NULL);
    }
    (void) sprintf(ValBuff, "%d", Val);

    return(ValBuff);
}

/*
 * Read the data type `long'
 */
static char *DTlong(kd, Addr)
    kvm_t		       *kd;
    OFF_T_TYPE			Addr;
{
    long			Val;

    if (KVMget(kd, Addr, (char *) &Val, sizeof(Val), KDT_DATA)) {
	if (Debug) Error("Read long value from 0x%x failed.", Addr);
	return((char *) NULL);
    }
    (void) sprintf(ValBuff, "%d", Val);

    return(ValBuff);
}

/*
 * Read the data type `ulong'
 */
static char *DTulong(kd, Addr)
    kvm_t		       *kd;
    OFF_T_TYPE			Addr;
{
    u_long			Val;

    if (KVMget(kd, Addr, (char *) &Val, sizeof(Val), KDT_DATA)) {
	if (Debug) Error("Read ulong value from 0x%x failed.", Addr);
	return((char *) NULL);
    }
    (void) sprintf(ValBuff, "%d", Val);

    return(ValBuff);
}

/*
 * Read the data type `short'
 */
static char *DTshort(kd, Addr)
    kvm_t		       *kd;
    OFF_T_TYPE			Addr;
{
    short			Val;

    if (KVMget(kd, Addr, (char *) &Val, sizeof(Val), KDT_DATA)) {
	if (Debug) Error("Read short value from 0x%x failed.", Addr);
	return((char *) NULL);
    }
    (void) sprintf(ValBuff, "%d", Val);

    return(ValBuff);
}

/*
 * Read the data type `ushort'
 */
static char *DTushort(kd, Addr)
    kvm_t		       *kd;
    OFF_T_TYPE			Addr;
{
    ushort			Val;

    if (KVMget(kd, Addr, (char *) &Val, sizeof(Val), KDT_DATA)) {
	if (Debug) Error("Read ushort value from 0x%x failed.", Addr);
	return((char *) NULL);
    }
    (void) sprintf(ValBuff, "%d", Val);

    return(ValBuff);
}

/*
 * Read the data type `string'
 */
static char *DTstring(kd, Addr)
    kvm_t		       *kd;
    OFF_T_TYPE			Addr;
{
    static char			Val[BUFSIZ];

    if (KVMget(kd, Addr, (char *) Val, sizeof(Val), KDT_STRING)) {
	if (Debug) Error("Read string value from 0x%x failed.", Addr);
	return((char *) NULL);
    }

    return(Val);
}

/*
 * Read a variable of type DataName from address Addr.
 */
static char *DataGetStr(DataName, kd, Addr)
    char		       *DataName;
    kvm_t		       *kd;
    OFF_T_TYPE			Addr;
{
    DataEntry_t		       *DataPtr;
    register int		i;

    for (DataPtr = DataTable; DataPtr && DataPtr->Name; ++DataPtr)
	if (EQ(DataName, DataPtr->Name))
	    break;

    if (!DataPtr->Name) {
	/*
	 * XXX Data type checking should really be done when parsing the
	 * config file, but that would mean using something other than
	 * the "Define" interface.
	 */
	if (Debug) {
	    Error("Invalid data type `%s'.  Valid types are:", DataName);
	    for (i = 0; DataTable[i].Name; ++i)
		Error("\t%s\n", DataTable[i]);
	}
	return((char *) NULL);
    }

    return (*DataPtr->Read)(kd, Addr);
}

/*
 * List valid arguments for the Kernel class.
 * XXX Maybe we should nlist the variables first and only list
 * those variables we find?
 */
extern void KernelList()
{
    register Define_t	       *KernDef;
    register char	       *SymName;

    KernDef = DefGetList(DL_KERNEL); 
    if (!KernDef) {
	if (Debug) Error("No kernel variables are defined.");
	return;
    }

    printf("\n\nThe following are valid arguments for `-class Kernel -show Name1,Name2,...':\n\n");
    printf("%-25s %s\n", "NAME", "DESCRIPTION");

    for ( ; KernDef; KernDef = KernDef->Next) {
	SymName = KernDef->KeyStr;
	if (*SymName == '_')
	    ++SymName;
	printf("%-25s %s\n", SymName, KernDef->ValStr2);
    }
}

/*
 * Check to see if a symbol named String appears
 * in Argv.
 * Return 1 if found.
 * Return 0 if not found.
 */
static int HasName(String, Argv)
    char		       *String;
    char		      **Argv;
{
    register char	      **cpp;
    register char	       *cp;

    cp = String;
    if (*cp == '_')
	++cp;

    for (cpp = Argv; cpp && *cpp; ++cpp)
	if (EQ(cp, *cpp))
	    return(1);

    return(0);
}

/*
 * Find a namelist with name Name in NameList.
 */
static nlist_t *NLget(Name, NameList)
    char		       *Name;
    nlist_t		       *NameList;
{
    register nlist_t	       *NLPtr;
    register char	       *cp = NULL;

    if (*Name == '_')
	cp = Name + 1;

    for (NLPtr = NameList; NLPtr && GetNlNamePtr(NLPtr); ++NLPtr) {
	if (EQ(Name, GetNlNamePtr(NLPtr)))
	    return(NLPtr);
	if (cp && EQ(cp, GetNlNamePtr(NLPtr)))
	    return(NLPtr);
    }

    return((nlist_t *) NULL);
}

/*
 * Show kernel variables
 */
extern void KernelShow(MyInfo, Names)
    ClassInfo_t		       *MyInfo;
    char		      **Names;
{
    Define_t		       *KernDef;
    Define_t		       *DefPtr;
    int				NumDef;
    int				NumNameList;
    nlist_t		       *NameList;
    nlist_t		       *NLPtr;
    static char		       *RptData[3];
    char		       *SymName;
    kvm_t		       *kd;
    char		       *ValStr;
    int				Len;
    int				MaxLen = 0;

    KernDef = DefGetList(DL_KERNEL); 
    if (!KernDef) {
	if (Debug) Error("No kernel variables are defined.");
	return;
    }

    ClassShowLabel(MyInfo);

    /*
     * Get number of variables
     */
    for (DefPtr = KernDef, NumDef = 0; DefPtr; DefPtr = DefPtr->Next) {
	if (Names) {
	    if (HasName(DefPtr->KeyStr, Names))
		++NumDef;
	} else
	    ++NumDef;
    }

    NameList = (nlist_t *) xcalloc(NumDef + 2, sizeof(nlist_t));

    /*
     * Prepare namelist
     */
    for (DefPtr = KernDef, NLPtr = NameList, NumNameList = 0; 
	 DefPtr; DefPtr = DefPtr->Next) {
	if (Names && !HasName(DefPtr->KeyStr, Names))
	    continue;
	GetNlNamePtr(NLPtr) = DefPtr->KeyStr;
	++NLPtr;
	++NumNameList;
    }

    kd = KVMopen();
    if (!kd) {
	if (Debug) Error("Cannot open kernel image.");
	return;
    }

    /*
     * Get all addresses
     */
    NLPtr = KVMnlist(kd, NULL, NameList, NumNameList);
    if (!NLPtr) {
	if (Debug) Error("Kernel variable nlist failed.");
	(void) free(NameList);
	return;
    }

    if (!VL_BRIEF || !VL_TERSE)
	/*
	 * Find the longest descriptive string for later use.
	 */
	for (DefPtr = KernDef; DefPtr; DefPtr = DefPtr->Next) {
	    NLPtr = NLget(DefPtr->KeyStr, NameList);
	    if (CheckNlist(NLPtr))
		continue;
	    Len = 0;
	    if (DefPtr->ValStr2)
		Len = (int) strlen(DefPtr->ValStr2);
	    if (VL_ALL) {
		Len += (int) strlen(DefPtr->KeyStr);
		Len += 2;	/* () */
	    }
	    Len += 4;		/* _is_ */
	    if (Len > MaxLen)
		MaxLen = Len;
	}

    /*
     * For each variable in the definetion list that we have an
     * address for, read the value from the kernel using a data type
     * specific function.
     */
    for (DefPtr = KernDef; DefPtr; DefPtr = DefPtr->Next) {
	NLPtr = NLget(DefPtr->KeyStr, NameList);
	if (!NLPtr) {
	    if (Debug) Error("Cannot lookup `%s' namelist entry.", 
			     DefPtr->KeyStr);
	    continue;
	}
	if (CheckNlist(NLPtr)) {
	    if (Debug) Error("Symbol `%s' was not found in kernel.", 
			     GetNlNamePtr(NLPtr));
	    continue;
	}

	if (EQ(DefPtr->ValStr1, DT_BOOL))
	    ValStr = "TRUE";
	else
	    ValStr = DataGetStr(DefPtr->ValStr1, kd, NLPtr->n_value);
	if (!ValStr) 
	    continue;

	/*
	 * Print out what we got
	 */
	SymName = DefPtr->KeyStr;
	if (*SymName == '_')
	    ++SymName;

	if (FormatType == FT_PRETTY) {
	    ClassShowValue(DefPtr->ValStr2, SymName, ValStr, MaxLen);
	} else if (FormatType == FT_REPORT) {
	    RptData[0] = DefPtr->ValStr2;
	    RptData[1] = ValStr;
	    Report(CN_KERNEL, NULL, SymName, RptData, 2);
	}
    }

    KVMclose(kd);
    (void) free(NameList);
}
