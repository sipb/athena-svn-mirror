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
 * Kernel Variables
 */
#include "defs.h"

#if	defined(HAVE_KVM) && defined(HAVE_NLIST)
static char			ValBuff[100];

typedef struct {
    char		       *Name;		/* Data type name */
    char		     *(*Read)();	/* Function to read it */
} DataEntry_t;

static char		       *DTstring();
static char		       *DTint();
static char		       *DTuint();
static char		       *DTshort();
static char		       *DTushort();
static char		       *DTlong();
static char		       *DTulong();
#if	defined(HAVE_INT64_T)
static char		       *DTint64();
#endif
#if	defined(HAVE_UINT64_T)
static char		       *DTuint64();
#endif

#define DT_BOOL			"bool"		/* Boolean */

DataEntry_t DataTable[] = {
    { "string",		DTstring },
    { "int",		DTint },
    { "uint",		DTuint },
    { "short",		DTshort },
    { "ushort",		DTushort },
    { "long",		DTlong },
    { "ulong",		DTulong },
#if	defined(HAVE_INT64_T)
    { "int64",		DTint64 },
#endif
#if	defined(HAVE_UINT64_T)
    { "uint64",		DTuint64 },
#endif
    { DT_BOOL		},
    { 0 },
};

/*
 * Read the data type `int'
 */
static char *DTint(kd, Addr)
    kvm_t		       *kd;
    KVMaddr_t			Addr;
{
    int				Val;

    if (KVMget(kd, Addr, (char *) &Val, sizeof(Val), KDT_DATA)) {
	SImsg(SIM_GERR, "Read int value from 0x%x failed.", Addr);
	return((char *) NULL);
    }
    if (!Val) return((char *) NULL);
    (void) snprintf(ValBuff, sizeof(ValBuff),  "%d", Val);

    return(ValBuff);
}

/*
 * Read the data type `uint'
 */
static char *DTuint(kd, Addr)
    kvm_t		       *kd;
    KVMaddr_t			Addr;
{
    u_int			Val;

    if (KVMget(kd, Addr, (char *) &Val, sizeof(Val), KDT_DATA)) {
	SImsg(SIM_GERR, "Read uint value from 0x%x failed.", Addr);
	return((char *) NULL);
    }
    if (!Val) return((char *) NULL);
    (void) snprintf(ValBuff, sizeof(ValBuff),  "%d", Val);

    return(ValBuff);
}

/*
 * Read the data type `long'
 */
static char *DTlong(kd, Addr)
    kvm_t		       *kd;
    KVMaddr_t			Addr;
{
    long			Val;

    if (KVMget(kd, Addr, (char *) &Val, sizeof(Val), KDT_DATA)) {
	SImsg(SIM_GERR, "Read long value from 0x%x failed.", Addr);
	return((char *) NULL);
    }
    if (!Val) return((char *) NULL);
    (void) snprintf(ValBuff, sizeof(ValBuff),  "%d", Val);

    return(ValBuff);
}

/*
 * Read the data type `ulong'
 */
static char *DTulong(kd, Addr)
    kvm_t		       *kd;
    KVMaddr_t			Addr;
{
    u_long			Val;

    if (KVMget(kd, Addr, (char *) &Val, sizeof(Val), KDT_DATA)) {
	SImsg(SIM_GERR, "Read ulong value from 0x%x failed.", Addr);
	return((char *) NULL);
    }
    if (!Val) return((char *) NULL);
    (void) snprintf(ValBuff, sizeof(ValBuff),  "%d", Val);

    return(ValBuff);
}

/*
 * Read the data type `short'
 */
static char *DTshort(kd, Addr)
    kvm_t		       *kd;
    KVMaddr_t			Addr;
{
    short			Val;

    if (KVMget(kd, Addr, (char *) &Val, sizeof(Val), KDT_DATA)) {
	SImsg(SIM_GERR, "Read short value from 0x%x failed.", Addr);
	return((char *) NULL);
    }
    if (!Val) return((char *) NULL);
    (void) snprintf(ValBuff, sizeof(ValBuff),  "%d", Val);

    return(ValBuff);
}

/*
 * Read the data type `ushort'
 */
static char *DTushort(kd, Addr)
    kvm_t		       *kd;
    KVMaddr_t			Addr;
{
    ushort			Val = 0;

    if (KVMget(kd, Addr, (char *) &Val, sizeof(Val), KDT_DATA)) {
	SImsg(SIM_GERR, "Read ushort value from 0x%x failed.", Addr);
	return((char *) NULL);
    }
    if (Val == 0) return((char *) NULL);
    (void) snprintf(ValBuff, sizeof(ValBuff),  "%d", Val);

    return(ValBuff);
}

#if	defined(HAVE_INT64_T)
/*
 * Read the data type `int64'
 */
static char *DTint64(kd, Addr)
    kvm_t		       *kd;
    KVMaddr_t			Addr;
{
    int64_t			Val = 0;

    if (KVMget(kd, Addr, (char *) &Val, sizeof(Val), KDT_DATA)) {
	SImsg(SIM_GERR, "Read int64_t value from 0x%x failed.", Addr);
	return((char *) NULL);
    }
    if (Val == 0) return((char *) NULL);
    (void) snprintf(ValBuff, sizeof(ValBuff),  "%lld", Val);

    return(ValBuff);
}
#endif	/* HAVE_INT64_T */

#if	defined(HAVE_UINT64_T)
/*
 * Read the data type `uint64'
 */
static char *DTuint64(kd, Addr)
    kvm_t		       *kd;
    KVMaddr_t			Addr;
{
    uint64_t			Val = 0;

    if (KVMget(kd, Addr, (char *) &Val, sizeof(Val), KDT_DATA)) {
	SImsg(SIM_GERR, "Read uint64_t value from 0x%x failed.", Addr);
	return((char *) NULL);
    }
    if (Val == 0) return((char *) NULL);
    (void) snprintf(ValBuff, sizeof(ValBuff),  "%lld", Val);

    return(ValBuff);
}
#endif	/* HAVE_UINT64_T */

/*
 * Read the data type `string'
 */
static char *DTstring(kd, Addr)
    kvm_t		       *kd;
    KVMaddr_t			Addr;
{
    static char			Val[BUFSIZ];

    if (KVMget(kd, Addr, (char *) Val, sizeof(Val), KDT_STRING)) {
	SImsg(SIM_GERR, "Read string value from 0x%x failed.", Addr);
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
    KVMaddr_t			Addr;
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
	    SImsg(SIM_GERR, "Invalid data type `%s'.  Valid types are:", 
		  DataName);
	    for (i = 0; DataTable[i].Name; ++i)
		SImsg(SIM_GERR, "\t%s", DataTable[i].Name);
	}
	return((char *) NULL);
    }

    return (*DataPtr->Read)(kd, Addr);
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
#endif	/* HAVE_KVM && HAVE_NLIST */

/*
 * Show kernel variables
 */
extern void KernelShow(MyInfo, Names)
    ClassInfo_t		       *MyInfo;
    char		      **Names;
{
#if	defined(HAVE_KVM) && defined(HAVE_NLIST)
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
	SImsg(SIM_WARN, "No kernel variables are defined.");
	return;
    }

    ClassShowBanner(MyInfo);

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
	SImsg(SIM_GERR, "Cannot open kernel image.");
	return;
    }

    /*
     * Get all addresses
     */
    NLPtr = KVMnlist(kd, NULL, NameList, NumNameList);
    if (!NLPtr) {
	SImsg(SIM_GERR, "Kernel variable nlist failed.");
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
	    SImsg(SIM_GERR, "Cannot lookup `%s' namelist entry.", 
			     DefPtr->KeyStr);
	    continue;
	}
	if (CheckNlist(NLPtr))
	    continue;

	if (EQ(DefPtr->ValStr1, DT_BOOL))
	    ValStr = "TRUE";
	else
	    ValStr = DataGetStr(DefPtr->ValStr1, kd, 
				(KVMaddr_t) NLPtr->n_value);
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
#else
    SImsg(SIM_DBG, "KernelShow() not available on this OS");
#endif	/* HAVE_KVM && HAVE_NLIST */
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
	SImsg(SIM_WARN, "No kernel variables are defined.");
	return;
    }

    SImsg(SIM_INFO, "\n\nThe following are valid arguments for `-class Kernel -show Name1,Name2,...':\n\n");
    SImsg(SIM_INFO, "%-25s %s\n", "NAME", "DESCRIPTION");

    for ( ; KernDef; KernDef = KernDef->Next) {
	SymName = KernDef->KeyStr;
	if (*SymName == '_')
	    ++SymName;
	SImsg(SIM_INFO, "%-25s %s\n", SymName, KernDef->ValStr2);
    }
}
