/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Kernel Variables
 */
#include "defs.h"

#if	defined(HAVE_NLIST)
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
#endif	/* HAVE_NLIST */

/*
 * Get Kernel Variables from kernel.
 * Returns ptr to Define_t in Query->Out:
 * 	KeyStr		Kernel Variable
 *	ValStr1		Type of variable (Boolean, etc)
 *	ValStr2		Description of Variable
 *	ValStr3		Value from kernel
 */
extern int KernelVarsMCSI(Query)
     MCSIquery_t	       *Query;
{
#if	defined(HAVE_NLIST)
    Define_t		       *KernDef;
    Define_t		       *DefPtr;
    int				NumDef;
    int				NumNameList;
    int				NumValid = 0;
    nlist_t		       *NameList;
    nlist_t		       *NLPtr;
    char		       *SymName;
    kvm_t		       *kd;
    char		       *ValStr;

    if (!Query) {
	errno = ENXIO;
	return -1;
    }

    if (Query->Op == MCSIOP_DESTROY)
	return DefDestroy((Define_t *) Query->Out);
    /* Else do MCSIOP_CREATE */

    KernDef = DefGetList(DL_KERNEL); 
    if (!KernDef) {
	SImsg(SIM_DBG, "No kernel variables are defined.");
	return -1;
    }

    /*
     * Get number of variables
     */
    for (DefPtr = KernDef, NumDef = 0; DefPtr; DefPtr = DefPtr->Next)
	++NumDef;

    NameList = (nlist_t *) xcalloc(NumDef + 2, sizeof(nlist_t));

    /*
     * Prepare namelist
     */
    for (DefPtr = KernDef, NLPtr = NameList, NumNameList = 0; 
	 DefPtr; DefPtr = DefPtr->Next) {
	GetNlNamePtr(NLPtr) = DefPtr->KeyStr;
	++NLPtr;
	++NumNameList;
    }

    kd = KVMopen();
    if (!kd) {
	SImsg(SIM_GERR, "Cannot open kernel image.");
	return -1;
    }

    /*
     * Get all addresses
     */
    NLPtr = KVMnlist(kd, NULL, NameList, NumNameList);
    if (!NLPtr) {
	SImsg(SIM_GERR, "Kernel variable nlist failed.");
	(void) free(NameList);
	return -1;
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

	if (*DefPtr->KeyStr == '_')
	    ++DefPtr->KeyStr;

	/* The value we found */
	DefPtr->ValStr3 = strdup(ValStr);
	++NumValid;
    }

    KVMclose(kd);
    (void) free(NameList);
    Query->Out = (Opaque_t) KernDef;
    Query->OutSize = NumValid;
#else
    SImsg(SIM_DBG, "Kernel Variables not available on this OS");
#endif	/* HAVE_NLIST */

    return 0;
}
