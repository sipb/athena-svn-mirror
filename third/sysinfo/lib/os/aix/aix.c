/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * AIX specific functions
 */

#include "defs.h"
#include <nl_types.h>
#include <locale.h>

/*
 * Platform Specific Interfaces
 */
extern char			       *GetKernArchCfg();
PSI_t GetKernArchPSI[] = {		/* Get Kernel Architecture */
    { GetKernArchCfg },
    { GetKernArchSysinfo },
    { GetKernArchDef },
    { GetAppArchSysinfo },
    { GetAppArchDef },
    { NULL },
};
PSI_t GetAppArchPSI[] = {		/* Get Application Architecture */
    { GetAppArchSysinfo },
    { GetAppArchDef },
    { NULL },
};
PSI_t GetCpuTypePSI[] = {		/* Get CPU Type */
    { GetCpuTypeSysinfo },
    { GetCpuTypeDef },
    { NULL },
};
PSI_t GetNumCpuPSI[] = {		/* Get Number of CPU's */
    { GetNumCpuSysconf },
    { NULL },
};
PSI_t GetKernVerPSI[] = {		/* Get Kernel Version */
    { NULL },
};
PSI_t GetOSNamePSI[] = {		/* Get OS Name */
    { GetOSNameSysinfo },
    { GetOSNameUname },
    { NULL },
};
PSI_t GetOSDistPSI[] = {		/* Get OS Release */
    { NULL },
};
extern char		       *GetOSVerODM();
extern char		       *GetOSVerFiles();
PSI_t GetOSVerPSI[] = {			/* Get OS Version */
    { GetOSVerODM },
    { GetOSVerFiles },
    { GetOSVerSysinfo },
    { GetOSVerUname },
    { NULL },
};
char 			       *GetModelUname();
char 			       *GetModelODM();
PSI_t GetModelPSI[] = {			/* Get system Model */
    { GetModelFile },
    { GetModelUname },
    { GetModelODM },
    { NULL },
};
PSI_t GetSerialPSI[] = {		/* Get Serial Number */
    { NULL },
};
PSI_t GetRomVerPSI[] = {		/* Get ROM Version */
    { NULL },
};
PSI_t GetManShortPSI[] = {		/* Get Short Man Name */
    { GetManShortSysinfo },
    { GetManShortDef },
    { NULL },
};
PSI_t GetManLongPSI[] = {		/* Get Long Man Name */
    { GetManLongSysinfo },
    { GetManLongDef },
    { NULL },
};
extern char		       *GetMemoryODM();
PSI_t GetMemoryPSI[] = {		/* Get amount of memory */
    { GetMemoryODM },
    { NULL },
};
extern char		       *GetVirtMemODM();
PSI_t GetVirtMemPSI[] = {		/* Get amount of virtual memory */
    { GetVirtMemODM },
    { NULL },
};
PSI_t GetBootTimePSI[] = {		/* Get System Boot Time */
    { GetBootTimeUtmp },
    { NULL },
};

/*
 * Get ODM Error string.
 */
extern char *odmerror()
{
    static char odmerrstr[256];

    if (odm_err_msg(odmerrno, (char **) &odmerrstr))
	(void) snprintf(odmerrstr, sizeof(odmerrstr),
			"unknown ODM error %d", odmerrno);

    return(odmerrstr);
}

/*
 * Get the value of attribute 'Attr' with name 'Name' 
 * from the Custom Attribute ODM.
 */
static char *GetAttrVal(Name, Attr)
    char		       *Name;
    char		       *Attr;
{
    static struct CuAt		CuAt;
    static char			Query[128];
    char		       *errstr = NULL;
    int				ret;

    if (odm_initialize() == -1) {
	SImsg(SIM_GERR, "ODM initialize failed: %s", odmerror());
	return((char *) NULL);
    }

    (void) snprintf(Query, sizeof(Query), "attribute = '%s' and name = '%s'", 
		    Attr, Name);

    ret = (int) odm_get_obj(CuAt_CLASS, Query, &CuAt, ODM_FIRST);
    if (ret == -1)
	errstr = odmerror();
    else if (ret == 0)
	errstr = "No entry found";

    if (errstr) {
	SImsg(SIM_GERR, "ODM get \"%s\" from \"%s\" failed: %s",
			 Query, CuAt_CLASS[0].classname, errstr);
	return((char *) NULL);
    }

    SImsg(SIM_DBG, "GetAttrVal: Query=<%s> Found=<%s>",
	  Query, (CuAt.value[0]) ? CuAt.value : "");

    return((CuAt.value[0]) ? CuAt.value : (char *) NULL);
}

/*
 * Get the PvDv ODM entry with the criteria 'Criteria'.
 */
static struct PdDv *GetPdDv(Criteria)
    char		       *Criteria;
{
    static struct PdDv		PdDv;
    char		       *errstr = NULL;
    int				ret;

    if (odm_initialize() == -1) {
	SImsg(SIM_GERR, "ODM initialize failed: %s", odmerror());
	return((struct PdDv *) NULL);
    }

    ret = (int) odm_get_obj(PdDv_CLASS, Criteria, &PdDv, ODM_FIRST);
    if (ret == -1)
	errstr = odmerror();
    else if (ret == 0)
	errstr = "No entry found";

    if (errstr) {
	SImsg(SIM_GERR, "ODM get \"%s\" from \"%s\" failed: %s",
			 Criteria, PdDv_CLASS[0].classname, errstr);
	return((struct PdDv *) NULL);
    }

    return(&PdDv);
}

/* 
 * Clean up a VPD string.  Remove initial non alpha-numeric characters
 * as well as any trailing white space.
 */
static char *CleanVPD(string)
    char		       *string;
{
    register char	       *cp, *end;

    while (string && *string && !isalnum(*string))
	++string;

    cp = end = &string[strlen(string) - 1];

    while (cp && *cp && isspace(*cp))
	--cp;

    if (cp != end)
	*(cp+1) = CNULL;

    return(string);
}

/*
 * Find the VPD info entry for "String".  Return a vpdinfo_t
 * entry for the matching entry and set it's "value" correctly.
 */
static VPDinfo_t *GetVPDinfo(String)
    char		       *String;
{
    static char			Buff[BUFSIZ];
    static char			Code[3];
    static VPDinfo_t		VPDinfo;

    (void) strncpy(Code, String, 2);	/* XXX Assume codes are only 2 bytes */
    Code[2] = CNULL;

    VPDinfo.Code = Code;
    /*
     * The "value" portion of "String" starts after the "code" portion.
     * CleanVPD() is harmful, so we need to pass it a private copy.
     */
    (void) snprintf(Buff, sizeof(Buff), "%s", String + strlen(VPDinfo.Code));
    VPDinfo.Value = CleanVPD(Buff);

    return(&VPDinfo);
}

/*
 * Given a VPD string "vpdstr", decode it into a list of strings.
 */
static int DecodeVPD(DevInfo, VPDstr)
    DevInfo_t		       *DevInfo;
    char		       *VPDstr;
{
    char		       *myVPDstr;
    register char	       *cp;
    Define_t		       *Def;
    VPDinfo_t		       *VPDinfo;

    if (!VPDstr)
	return(-1);

    /* strtok() is destructive */
    myVPDstr = strdup(VPDstr);

    /*
     * Each field in "VPDstr" is seperated by a "*", followed by
     * the code for the field and it's value.  Looks something like:
     *
     *		*TM 19445*MF IBM
     */
    for (cp = strtok(myVPDstr, "*"); cp; cp = strtok((char *)NULL, "*")) {
	if (!(VPDinfo = GetVPDinfo(cp)))
	    continue;

	if (EQ(VPDinfo->Code, "MF"))
	    DevInfo->Vendor = strdup(VPDinfo->Value);
	else if (EQ(VPDinfo->Code, "TM"))
	    DevInfo->Model = strdup(VPDinfo->Value);
	else if (EQ(VPDinfo->Code, "DS"))
	    DevInfo->ModelDesc = strdup(VPDinfo->Value);
	else if (EQ(VPDinfo->Code, "SN"))
	    DevInfo->Serial = strdup(VPDinfo->Value);
	else {
	    if (Def = DefGet(DL_VPD, VPDinfo->Code, -1, 0))
		AddDevDesc(DevInfo, VPDinfo->Value, Def->ValStr1, DA_APPEND);
	    else
		SImsg(SIM_UNKN, "Unknown VPD code=`%s' value=`%s'.", 
		      VPDinfo->Code, VPDinfo->Value);
	}
    }

    if (myVPDstr)
	(void) free(myVPDstr);

    return(0);
}

/*
 * Get and decode the Vital Product Data informatin for device "Name".
 */
static int GetVPD(DevInfo)
    DevInfo_t		       *DevInfo;
{
    static struct CuVPD		cuvpd;
    static char			Query[128];
    int 			ret;

    if (odm_initialize() == -1) {
	SImsg(SIM_GERR, "ODM initialize failed: %s", odmerror());
	return(-1);
    }

    (void) snprintf(Query, sizeof(Query),  "name=%s", DevInfo->Name);
    ret = (int) odm_get_obj(CuVPD_CLASS, Query, &cuvpd, ODM_FIRST);
    if (ret == -1) {
	SImsg(SIM_GERR, "ODM get VPD object for <%s> failed: %s", 
		  DevInfo->Name, odmerror());
	return(-1);
    } else if (ret == 0) {
	return(-1);
    }

    SImsg(SIM_DBG, " VPD: name=<%s> type=<%d> VPD=<%s>", 
	       cuvpd.name, cuvpd.vpd_type, cuvpd.vpd);

    return(DecodeVPD(DevInfo, cuvpd.vpd));
}

/*
 * Get the system model name using uname()
 */
extern char *GetModelUname()
{
    char		        Uname[] = "/usr/bin/uname";
    char			Cmd[128];
    static char			Buff[128];
    FILE		       *PFH;
    char		       *cp;

    if (access(Uname, X_OK) != 0) {
	SImsg(SIM_DBG, "%s: Doesn't exist/not executable.", Uname);
	return (char *) NULL;
    }

    (void) snprintf(Cmd, sizeof(Cmd), "%s -M 2>/dev/null", Uname);

    if (!(PFH = popen(Cmd, "r"))) {
	SImsg(SIM_DBG, "%s: popen for read failed: %s", Cmd, SYSERR);
	return (char *) NULL;
    }
	
    if (!fgets(Buff, sizeof(Buff), PFH)) {
	SImsg(SIM_DBG, "%s: Read failed.", Cmd);
	return (char *) NULL;
    }

    pclose(PFH);

    if (Buff[0] == ' ' || Buff[0] == '\n') {
	SImsg(SIM_DBG, "%s: No input received.", Cmd);
	return (char *) NULL;
    }

    if (cp = strchr(Buff, '\n'))
	*cp = CNULL;

    if (cp = strchr(Buff, ','))
	++cp;
    else
	cp = Buff;

    SImsg(SIM_DBG, "GetModelUname: model=<%s>", cp);

    return cp;
}

/*
 * Get the system model name.
 */
extern char *GetModelODM()
{
    Define_t		       *Model;
    register char	       *val, *cp;
    register int		i, SysType;

    if ((val = GetAttrVal(NN_SYS0, AT_MODELCODE)) == NULL) {
	SImsg(SIM_GERR, "Cannot get \"%s\" for \"%s\" from ODM.",
			 AT_MODELCODE, NN_SYS0);
	return((char *) NULL);
    }

    SysType = (int) strtol(val, NULL, 0);

    SImsg(SIM_DBG, "System type = 0x%x (%s)", SysType, val);

    Model = DefGet(DL_SYSMODEL, NULL, (long) SysType, 0);
    if (Model && Model->ValStr1)
	return(Model->ValStr1);

    SImsg(SIM_UNKN, "system model/type 0x%x is unknown.", SysType);

    return((char *) NULL);
}

/*
 * Use the kernels _system_configuration structure to determine
 * our kernel architecture.
 * 
 * XXX This isn't working right now
 */
#include <sys/systemcfg.h>
static char			SysCfgSYM[] = "_system_configuration";
extern char *GetKernArchCfg()
{
#ifdef	BROKEN
    struct nlist	       *NLPtr;
    kvm_t		       *kd;
    Define_t		       *Define;

    /*
     * Get _system_configuration
     */
    if (!(kd = KVMopen()))
	return((char *) NULL);
    if ((NLPtr = KVMnlist(kd, SysCfgSYM, (struct nlist *)NULL, 0)) == NULL)
	return((char *) NULL);
    if (CheckNlist(NLPtr))
	return((char *) NULL);
    if (KVMget(kd, NLPtr->n_value, (char *) &_system_configuration, 
	       sizeof(_system_configuration), KDT_DATA)) {
	SImsg(SIM_GERR, "Cannot read `%s' from kernel.", SysCfgSYM);
	return((char *) NULL);
    }
    KVMclose(kd);

    Define = DefGet(DL_CPU, NULL, _system_configuration.implementation, 0);
    if (Define && Define->ValStr1)
	return(Define->ValStr1);
#endif	/* BROKEN */
    return((char *) NULL);
}

/*
 * Get amount of physical memory from the ODM CuAt data.
 */
extern char *GetMemoryODM()
{
    char		       *Val;
    Large_t			Amt;

    if ((Val = GetAttrVal(NN_SYS0, AT_REALMEM)) == NULL) {
	SImsg(SIM_GERR, "Cannot get \"%s\" for \"%s\" from ODM.",
	      AT_REALMEM, NN_SYS0);
	return((char *) NULL);
    }

    Amt = atoL(Val);

    return GetSizeStr(Amt, (Large_t)KBYTES);
}

/*
 * Get version of OS
 *
 * The following is based on what the /bin/oslevel script does.
 * There are several files that contain the full OS version/revision
 * level.  The first line of these files contains the version number.
 * The line either looks like '3.2.5' or '3250'.
 */
static char *OSVfiles[] = {
    OSV_OPP, OSV_MAINT, (char *)0
};

extern char *GetOSVerFiles()
{
    register char	       *Ptr;
    register char	       *Ptr2;
    register char	      **PtrPtr;
    FILE		       *FilePtr;
    static char			Buff[100];
    static char			Version[100];
    int				Len;

    Version[0] = CNULL;
    for (PtrPtr = OSVfiles; PtrPtr && *PtrPtr; ++PtrPtr) {
	FilePtr = fopen(*PtrPtr, "r");
	if (!FilePtr) {
	    SImsg(SIM_GERR, "Open %s failed: %s.", *PtrPtr, SYSERR);
	    continue;
	}
	if (fgets(Buff, sizeof(Buff), FilePtr) && isdigit(Buff[0])) {
	    Len = strlen(Buff);
	    if (Buff[Len-1] == '\n')
		Buff[Len-1] = CNULL;
	    if (strchr(Buff, '.'))
		(void) strcpy(Version, Buff);
	    else
		for (Ptr = Buff, Ptr2 = Version; Ptr && *Ptr; ++Ptr, ++Ptr2) {
		    /* If the line ends in 0, don't add it */
		    if (!(*Ptr == '0' && *(Ptr+1) == CNULL))
			*Ptr2 = *Ptr;
		    if (*(Ptr + 1))
			*++Ptr2 = '.';
		}
	    /* Zap trailing '.' if present */
	    Len = strlen(Version);
	    if (Version[Len-1] == '.')
		Version[Len-1] = CNULL;
	}
	(void) fclose(FilePtr);
	if (Version[0])
	    return(Version);
    }

    return((char *) NULL);
}

/*
 * Take a device name, remove and then return the unit number.
 * e.g. Take "hdisk0", remove "0", and then return 0.
 */
static int GetUnit(Name)
    char		       *Name;
{
    int				unit;
    register char	       *cp;

    for (cp = Name; cp && *cp; ++cp)
	if (isdigit(*cp)) {
	    unit = (int) atoi(cp);
	    *cp = CNULL;
	    return(unit);
	}

    return(-1);
}

/*
 * Get the location information from CuDvPtr.
 */
static char *GetLocation(CuDvPtr)
    struct CuDv		       *CuDvPtr;
{
    if (CuDvPtr->location[0])
	return(CuDvPtr->location);

    return((char *) NULL);
}

/*
 * Get Class information.
 */
static char *GetClassInfo(CuDvPtr)
    struct CuDv		       *CuDvPtr;
{
    if (CuDvPtr->PdDvLn_Lvalue[0])
	return(CuDvPtr->PdDvLn_Lvalue);

    return((char *) NULL);
}

/*
 * Set the description informatin for DevInfo.
 */
static void SetDescript(DevInfo, CuDvPtr)
    DevInfo_t		       *DevInfo;
    struct CuDv		       *CuDvPtr;
{
    AddDevDesc(DevInfo, GetLocation(CuDvPtr), "Location", DA_APPEND);
    AddDevDesc(DevInfo, GetClassInfo(CuDvPtr), "Class/SubClass/Type", 
	       DA_APPEND);
    GetVPD(DevInfo);
}

/*
 * Special routine to get memory information.
 */
extern DevInfo_t *ProbeMemory(ProbeData, CuDvPtr, PdDvPtr)
     ProbeData_t	       *ProbeData;
     struct CuDv	       *CuDvPtr;
     struct PdDv	       *PdDvPtr;
{
    DevInfo_t 		       *DevInfo;
    char		       *cp;
    char			Buff[64];
    char 		       *Name;
    DevData_t 		       *DevData;

    if (!ProbeData)
	return((DevInfo_t *) NULL);
    Name = ProbeData->DevName;
    DevData = ProbeData->DevData;
    
    if ((cp = GetAttrVal(Name, AT_SIZE)) == NULL)
	return((DevInfo_t *) NULL);

    DevInfo = NewDevInfo((DevInfo_t *) NULL);

    (void) snprintf(Buff, sizeof(Buff),  "%s MB Memory Card", cp);
    DevInfo->Model = strdup(Buff);

    DevInfo->Name = Name;
    DevInfo->Unit = DevData->DevUnit;
    DevInfo->Master = MkMasterFromDevData(DevData);
    DevInfo->Type = DT_MEMORY;
    SetDescript(DevInfo, CuDvPtr);

    return(DevInfo);
}

/*
 * Get description information
 */
static char *GetDescript(CuDvPtr, PdDvPtr)
    struct CuDv		       *CuDvPtr;
    struct PdDv		       *PdDvPtr;
{
    static char			Buff[BUFSIZ];
    static char			lastcat[BUFSIZ];
    static nl_catd		catd;
    char		       *msg = NULL;
    char		       *cp;

    if (!PdDvPtr->catalog[0])
	return((char *) NULL);

    /* 
     * Reuse open catalog if it's the same as the last time 
     */
    if ((int) catd <= 0 || !lastcat[0] || strcmp(PdDvPtr->catalog, lastcat)) {
	if ((int) catd > 0)
	    (void) catclose(catd);

	if ((int) catd == 0) {
	    /*
	     * First time stuff.
	     */

	    if (putenv(ENV_NLSPATH) != 0)
		SImsg(SIM_GERR, "Cannot set environment $NLSPATH.");

	    /*
	     * If our LANG is "C", then set to our default in order to
	     * avoid a bug in AIX that fails to find any catalogs in 
	     * this case.
	     */
	    cp = (char *) getenv("LANG");
	    if (!cp || (cp && EQ(cp, "C"))) {
		(void) snprintf(Buff, sizeof(Buff),  "LANG=%s", DEFAULT_LANG);
		if (putenv(strdup(Buff)) != 0)
		    SImsg(SIM_GERR, "Cannot set environment %s.", Buff);
		Buff[0] = CNULL;
	    }
	    (void) setlocale(LC_ALL, "");
	}

	catd = catopen(PdDvPtr->catalog, 0);
	if ((int) catd <= 0)
	    SImsg(SIM_GERR, "Catalog open of \"%s\" failed: %s.",
			     PdDvPtr->catalog, SYSERR);
    }

    /*
     * Retrieve the message from the catalog
     */
    if ((int) catd > 0) {
	msg = catgets(catd, PdDvPtr->setno, PdDvPtr->msgno, Buff);
	/* Save catalog name */
	(void) strcpy(lastcat, PdDvPtr->catalog);
    }

    return((msg && *msg) ? msg : (char *) NULL);
}

/*
 * Set the DevInfo type and classtype using the ODM class info.
 */
static int SetDevType(DevInfo, CuDvPtr)
     DevInfo_t		       *DevInfo;
    struct CuDv		       *CuDvPtr;
{
    static char			Buff[128];
    char		       *Class = NULL;
    char		       *SubClass = NULL;
    char		       *SubClassType = NULL;
    char		       *cp;
    DevType_t		       *DevType;
    ClassType_t		       *ClassType;

    if (!DevInfo || !CuDvPtr)
	return -1;

    if (cp = GetClassInfo(CuDvPtr)) {
	snprintf(Buff, sizeof(Buff), "%s", cp);
	Class = Buff;
	if (cp = strchr(Class, '/'))
	    *cp++ = CNULL;
	if (cp && *cp) {
	    SubClass = cp;
	    if (cp = strchr(cp, '/')) {
		*cp++ = CNULL;
		if (cp && *cp) {
		    SubClassType = cp;
		    if (cp = strchr(cp, '/'))
			*cp = CNULL;
		}
	    }
	}
    } else 
	return -1;

    if (Class) {
	if (DevType = TypeGetByName(Class))
	    DevInfo->Type = DevType->Type;
    }

    if (SubClass) {
	if (DevInfo->Type) {
	    if (ClassType = ClassTypeGetByName(DevInfo->Type, SubClass))
		DevInfo->ClassType = ClassType->Type;
	} else {
	    /* 
	     * The Class wasn't found so try looking up the SubClass
	     * as the primary device type.
	     */
	    if (DevType = TypeGetByName(SubClass))
		DevInfo->Type = DevType->Type;
	}
    }

    /*
     * As a last resort, try the sub class type as the class
     * type or primary device type.
     */
    if (SubClassType)
	if (DevInfo->Type && !DevInfo->ClassType) {
	    if (ClassType = ClassTypeGetByName(DevInfo->Type, SubClassType))
		DevInfo->ClassType = ClassType->Type;
	} else if (!DevInfo->Type) {
	    if (DevType = TypeGetByName(SubClassType))
		DevInfo->Type = DevType->Type;
	}

    return 0;
}

/*
 * General routine to get device information from ODM.
 */
extern DevInfo_t *ProbeODM(DevData, TreePtr, CuDvPtr)
    /*ARGSUSED*/
    DevData_t 		       *DevData;
    DevInfo_t 		      **TreePtr;
    struct CuDv		       *CuDvPtr;
{
    DevDefine_t		       *DevDefine;
    DevInfo_t 		       *DevInfo;
    DevInfo_t 		       *Master;
    char		       *MasterName;
    char		       *DevName;
    char		       *Desc;
    static char			Query[128];
    struct PdDv		       *PdDvPtr;
    static ProbeData_t		ProbeData;
    static DevFind_t		Find;

    if (!DevData || !TreePtr || !CuDvPtr)
	return((DevInfo_t *) NULL);

    DevName = MkDevName(DevData->DevName, DevData->DevUnit, 0, 0);

    /*
     * If there's no location then it's not a real device.
     */
    if (!CuDvPtr->location || !CuDvPtr->location[0]) {
	SImsg(SIM_DBG, "ODM:       <%s> No Location.  Skipping.", DevName);
	return((DevInfo_t *) NULL);
    }

    /*
     * Lookup the type name and if it's NONE, skip it.
     */
    if (DevDefine = DevDefGet(DevData->DevName, 0, 0))
	if (DevDefine->Type == DT_NONE) {
	    SImsg(SIM_DBG, "ODM:       <%s> Type is <none>.  Skipping.",
		  DevName);
	    return((DevInfo_t *) NULL);
	}
	    
    (void) memset(&Find, 0, sizeof(Find));
    Find.Tree = *TreePtr;
    Find.NodeName = DevName;
    DevInfo = DevFind(&Find);

    if (CuDvPtr->PdDvLn_Lvalue[0]) {
	(void) snprintf(Query, sizeof(Query), "uniquetype='%s'", 
			CuDvPtr->PdDvLn_Lvalue);
	PdDvPtr = GetPdDv(Query);
    } else {
	SImsg(SIM_GERR, "No PdDv link value for '%s'.", DevName);
	return((DevInfo_t *) NULL);
    }

    if (!DevInfo)
	DevInfo = NewDevInfo((DevInfo_t *) NULL);

    /*
     * If we can't get the description for this device, then
     * use the device specific probe routine if one is defined.
     */
    if ((Desc = GetDescript(CuDvPtr, PdDvPtr)) == (char *) NULL) {
	if (DevDefine && DevDefine->Probe) {
	    (void) memset(&ProbeData, CNULL, sizeof(ProbeData));
	    ProbeData.DevName = DevName;
	    ProbeData.DevData = DevData;
	    ProbeData.DevDefine = DevDefine;
	    return((*DevDefine->Probe)(&ProbeData, CuDvPtr, PdDvPtr));
	} else {
	    SImsg(SIM_DBG, 
		  "ODM:       <%s> No AIX catalog entry in <%s> found.", 
		  DevName,
		  (PdDvPtr->catalog) ? PdDvPtr->catalog : "(no catalog)");
	}
    }

    DevInfo->Name = DevName;
    DevInfo->Unit = DevData->DevUnit;
    SetDevType(DevInfo, CuDvPtr);
    if (!DevInfo->Type)
	DevInfo->Type = DT_GENERIC;
    if (Desc)
	DevInfo->ModelDesc = strdup(Desc);
    SetDescript(DevInfo, CuDvPtr);
    if (!DevInfo->MasterName)
	DevInfo->MasterName = MkMasterName(DevData);
    if (!DevInfo->Master) {
	(void) memset(&Find, 0, sizeof(Find));
	Find.Tree = *TreePtr;
	Find.NodeName = DevInfo->MasterName;
	if (Master = DevFind(&Find))
	    DevInfo->Master = Master;
	else {
	    DevInfo->Master = MkMasterFromDevData(DevData);
	    if (DevInfo->Master)
		DevInfo->MasterName = DevInfo->Master->Name;
	}
    }

    return(DevInfo);
}

/*
 * Build device tree by looking at the Object Database Manager (ODM)
 */
static int BuildODM(TreePtr, Names)
    DevInfo_t 		      **TreePtr;
    char		      **Names;
{
    static struct CuDv		CuDv;
    extern struct Class		CuDv_CLASS[];
    static DevData_t 		DevData;
    DevInfo_t 		       *DevInfo;
    int 			op, ret;

    if (odm_initialize() == -1) {
	SImsg(SIM_GERR, "ODM initialize failed: %s", odmerror());
	return(-1);
    }

    for (op = ODM_FIRST; ; op = ODM_NEXT) {
	/*
	 * Retrieve the object from ODM.
	 */
	ret = (int) odm_get_obj(CuDv_CLASS, (char *)NULL, &CuDv, op);
	if (ret == -1) {
	    SImsg(SIM_GERR, "ODM get object \"%s\" failed: %s", 
			     CuDv_CLASS[0].classname, odmerror());
	    return(-1);
	} else if (ret == 0)
	    /*
	     * We're done
	     */
	    break;

	if (CuDv.status != AVAILABLE) {
	    SImsg(SIM_GERR, "ODM lookup info for \"%s\" is not available.", 
		  CuDv.name);
	    continue;
	}

	/*
	 * This should never happen
	 */
	if (CuDv.name[0] == CNULL)
	    continue;

	/* Make sure devdata is clean */
	bzero(&DevData, sizeof(DevData_t));

	/* Set what we know */
	DevData.DevName = strdup(CuDv.name);
	DevData.DevUnit = GetUnit(DevData.DevName);
	if (CuDv.parent[0]) {
	    DevData.CtlrName = strdup(CuDv.parent);
	    DevData.CtlrUnit = GetUnit(DevData.CtlrName);
	}
	DevData.Flags |= DD_IS_ALIVE;

	SImsg(SIM_DBG, "ODM: Found=<%s> parent=<%s> location=<%s> uniq=<%s>",
	      CuDv.name, CuDv.parent, CuDv.location, CuDv.PdDvLn_Lvalue);

	/* Probe and add device */
	if (DevInfo = ProbeODM(&DevData, TreePtr, &CuDv))
	    AddDevice(DevInfo, TreePtr, Names);
    }

    if (odm_terminate() != 0)
	SImsg(SIM_WARN, "ODM Terminate did not succeed.");

    return(0);
}

/*
 * Build device tree using TreePtr.
 * Calls bus and method specific functions to
 * search for devices.
 */
extern int BuildDevices(TreePtr, Names)
    DevInfo_t 		       **TreePtr;
    char		       **Names;
{
    int 			 Found = 1;

    if (BuildODM(TreePtr, Names) == 0)
	Found = 0;

    return(Found);
}

#include <sys/vminfo.h>

static char			QueryStr[] = 
    "value = 'paging' and attribute = 'type'";

extern char *GetVirtMemODM()
{
    struct CuAt		       *CuAtPtr;
    struct objlistinfo		ObjListInfo;
    register int		i;
    static char			DevName[MAXPATHLEN];
    static struct pginfo	pginfo;
    Large_t			Amount = 0;

    odm_initialize();
    odm_set_path(_PATH_ODM);

    CuAtPtr = get_CuAt_list(CuAt_CLASS, QueryStr, &ObjListInfo, 20, 1);
    if ((int)CuAtPtr == -1) {
	SImsg(SIM_GERR, "get_CuAt_list paging info failed: %s.", SYSERR);
	odm_terminate();
	return((char *)NULL);
    }

    for (i = 0; i < ObjListInfo.num; ++i, ++CuAtPtr) {
	(void) snprintf(DevName, sizeof(DevName),  "/dev/%s", CuAtPtr->name);

	if (swapqry(DevName, &pginfo) == -1) {
	    SImsg(SIM_GERR, "swapqry %s failed: %s.", DevName, SYSERR);
	    continue;
	}

	Amount += (Large_t) ( (pginfo.size * getpagesize() ) / KBYTES );
    }

    odm_terminate();

    return GetSizeStr(Amount, (Large_t) KBYTES);
}

/*
 * Initialize the OS specific parts of the Device Types table
 */
void DevTypesInit()
{
    register int		i;

    for (i = 0; DevTypes[i].Name; ++i)
	switch (DevTypes[i].Type) {
	case DT_MEMORY:		DevTypes[i].Probe = ProbeMemory;	break;
	}
}

#if	defined(HAVE_PARTINFO_SUPPORT)
/*
 * Build Partition information
 */
#include <fstab.h>
#include <sys/statvfs.h>
extern int BuildPartInfo(PartInfoTree, SearchExp)
     PartInfo_t		      **PartInfoTree;
     char		      **SearchExp;
{
    FILE		       *FilePtr;
    struct fstab	       *Fstab;
    struct statvfs		Statvfs;
    PartInfo_t		       *PartInfo;
    char		      **Argv;

    setfsent();

    while (Fstab = getfsent()) {
	if (EQ(Fstab->fs_type, FSTAB_XX) || !EQN(Fstab->fs_spec, "/dev/", 5))
	    continue;

	PartInfo = PartInfoCreate(NULL);
	PartInfo->DevPath = strdup(Fstab->fs_spec);
	PartInfo->MntName = strdup(Fstab->fs_file);
	PartInfo->Type = strdup(Fstab->fs_type);

	if (EQ(Fstab->fs_type, FSTAB_SW))
	    PartInfo->Usage = PIU_SWAP;
	else 
	    PartInfo->Usage = PIU_FILESYS;

	if (statvfs(PartInfo->MntName, &Statvfs) == 0) {
	    PartInfo->Size = (Large_t) (Statvfs.f_blocks * Statvfs.f_frsize);
	    PartInfo->AmtUsed = (Large_t) 
		((Statvfs.f_blocks - Statvfs.f_bfree) * Statvfs.f_frsize);
	} else {
	    SImsg(SIM_GERR, "%s: Statvfs failed: %s", 
		  PartInfo->MntName, SYSERR);
	}

	(void) PartInfoAdd(PartInfoTree, PartInfo);
    }

    endfsent();

    return 0;
}
#endif	/* HAVE_PARTINFO_SUPPORT */
