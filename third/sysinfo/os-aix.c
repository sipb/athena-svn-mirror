/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: os-aix.c,v 1.1.1.1 1996-10-07 20:16:54 ghudson Exp $";
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
    { GetAppArch },
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
extern char		       *GetOSVerFiles();
PSI_t GetOSVerPSI[] = {			/* Get OS Version */
    { GetOSVerFiles },
    { GetOSVerSysinfo },
    { GetOSVerUname },
    { NULL },
};
char 			       *GetModelODM();
PSI_t GetModelPSI[] = {			/* Get system Model */
    { GetModelFile },
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
static char *odmerror()
{
    static char odmerrstr[BUFSIZ];

    if (odm_err_msg(odmerrno, &odmerrstr) != 0)
	(void) sprintf(odmerrstr, "unknown ODM error %d", odmerrno);

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
    static char			Buff[BUFSIZ];
    char		       *errstr = NULL;
    int				ret;

    if (odm_initialize() == -1) {
	Error("ODM initialize failed: %s", odmerror());
	return(-1);
    }

    (void) sprintf(Buff, "attribute = '%s' and name = '%s'", Attr, Name);

    ret = (int) odm_get_obj(CuAt_CLASS, Buff, &CuAt, ODM_FIRST);
    if (ret == -1)
	errstr = odmerror();
    else if (ret == 0)
	errstr = "No entry found";

    if (errstr) {
	if (Debug) Error("ODM get \"%s\" from \"%s\" failed: %s",
			 Buff, CuAt_CLASS[0].classname, errstr);
	return((char *) NULL);
    }

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
	Error("ODM initialize failed: %s", odmerror());
	return((struct PdDv *) NULL);
    }

    ret = (int) odm_get_obj(PdDv_CLASS, Criteria, &PdDv, ODM_FIRST);
    if (ret == -1)
	errstr = odmerror();
    else if (ret == 0)
	errstr = "No entry found";

    if (errstr) {
	if (Debug) Error("ODM get \"%s\" from \"%s\" failed: %s",
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
static vpdinfo_t *GetVPDinfo(String)
    char		       *String;
{
    Define_t		       *VPDdef;
    static char			Buff[BUFSIZ];
    static char			Code[3];
    static vpdinfo_t		VPDinfo;

    (void) strncpy(Code, String, 2);	/* XXX Assume codes are only 2 bytes */
    Code[2] = CNULL;

    VPDdef = DefGet(DL_VPD, Code, -1, 0);
    if (!VPDdef) {
	if (Debug)
	    printf("Unknown VPD info \"%s\".\n", String);
	return((vpdinfo_t *) NULL);
    }

    VPDinfo.code = VPDdef->KeyStr;
    VPDinfo.title = VPDdef->ValStr1;
    /*
     * The "value" portion of "String" starts after the "code" portion.
     * CleanVPD() is harmful, so we need to pass it a private copy.
     */
    (void) strcpy(Buff, String + strlen(VPDinfo.code));
    VPDinfo.value = CleanVPD(Buff);

    return(&VPDinfo);
}

/*
 * Given a VPD string "vpdstr", decode it into a list of strings.
 */
static int DecodeVPD(DevInfo, VPDstr)
    DevInfo_t		       *DevInfo;
    char		       *VPDstr;
{
    static char			Buff[BUFSIZ];
    static char			Man[BUFSIZ];
    static char			Model[BUFSIZ];
    char		       *myVPDstr;
    register char	       *cp;
    vpdinfo_t		       *VPDinfo;

    if (!VPDstr)
	return(-1);

    Man[0] = Model[0] = CNULL;
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

	if (EQ(VPDinfo->code, "MF") && strlen(VPDinfo->value) < sizeof(Man))
	    (void) strcpy(Man, VPDinfo->value);
	if (EQ(VPDinfo->code, "TM") && strlen(VPDinfo->value) < sizeof(Model))
	    (void) strcpy(Model, VPDinfo->value);

	AddDevDesc(DevInfo, VPDinfo->value, VPDinfo->title, DA_APPEND);
    }

    Buff[0] = CNULL;
    if (Man[0])
	(void) strcpy(Buff, Man);
    if (Model[0]) {
	if (Buff[0])
	    (void) strcat(Buff, " ");
	(void) strcat(Buff, Model);
    }
    if (Buff[0])
	DevInfo->Model = strdup(Buff);

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
    static char			Buff[BUFSIZ];
    int 			ret;

    if (odm_initialize() == -1) {
	Error("ODM initialize failed: %s", odmerror());
	return(-1);
    }

    (void) sprintf(Buff, "name=%s", DevInfo->Name);
    ret = (int) odm_get_obj(CuVPD_CLASS, Buff, &cuvpd, ODM_FIRST);
    if (ret == -1) {
	if (Debug)
	    Error("ODM get VPD object for \"%s\" failed: %s", 
		  DevInfo->Name, odmerror());
	return(-1);
    } else if (ret == 0) {
	if (Debug) 
	    Error("No VPD information for \"%s\".", DevInfo->Name);
	return(-1);
    }

    if (Debug)
	printf("VPD: name = '%s' type = %d VPD = '%s'\n", 
	       cuvpd.name, cuvpd.vpd_type, cuvpd.vpd);

    return(DecodeVPD(DevInfo, cuvpd.vpd));
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
	if (Debug) Error("Cannot get \"%s\" for \"%s\" from ODM.",
			 AT_MODELCODE, NN_SYS0);
	return((char *) NULL);
    }

    SysType = (int) strtol(val, NULL, 0);

    if (Debug)
	printf("System type = 0x%x (%s)\n", SysType, val);

    Model = DefGet(DL_SYSMODEL, NULL, (long) SysType, 0);
    if (Model && Model->ValStr1)
	return(Model->ValStr1);

    if (Debug)
	Error("system model/type 0x%x is unknown.", SysType);

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
#ifdef notdef /* XXX */
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
	if (Debug) Error("Cannot read `%s' from kernel.", SysCfgSYM);
	return((char *) NULL);
    }
    KVMclose(kd);

    Define = DefGet(DL_CPU, NULL, _system_configuration.implementation, 0);
    if (Define && Define->ValStr1)
	return(Define->ValStr1);
#endif	/* notdef */
    return((char *) NULL);
}

/*
 * Get amount of physical memory from the ODM CuAt data.
 */
extern char *GetMemoryODM()
{
    register char	       *val;
    register int		amtval;
    static char			Buff[100];

    if ((val = GetAttrVal(NN_SYS0, AT_REALMEM)) == NULL) {
	if (Debug) Error("Cannot get \"%s\" for \"%s\" from ODM.",
			 AT_REALMEM, NN_SYS0);
	return((char *) NULL);
    }

    amtval = (int) strtol(val, NULL, 0);
    (void) sprintf(Buff, "%d MB", DivRndUp(amtval, KBYTES));

    return(Buff);
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
	    if (Debug) Error("Open %s failed: %s.", *PtrPtr, SYSERR);
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

    return(UnSupported);
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
extern DevInfo_t *ProbeMemory(Name, DevData, DevDefine, CuDvPtr, PdDvPtr)
    /*ARGSUSED*/
    char 		       *Name;
    DevData_t 		       *DevData;
    DevDefine_t		       *DevDefine;
    struct CuDv		       *CuDvPtr;
    struct PdDv		       *PdDvPtr;
{
    DevInfo_t 		       *DevInfo;
    char		       *cp;
    char			Buff[BUFSIZ];

    if ((cp = GetAttrVal(Name, AT_SIZE)) == NULL)
	return((DevInfo_t *) NULL);

    DevInfo = NewDevInfo((DevInfo_t *) NULL);

    (void) sprintf(Buff, "%s MB Memory Card", cp);
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
		if (Debug) Error("Cannot set environment $NLSPATH.");

	    /*
	     * If our LANG is "C", then set to our default in order to
	     * avoid a bug in AIX that fails to find any catalogs in 
	     * this case.
	     */
	    cp = getenv("LANG");
	    if (!cp || (cp && EQ(cp, "C"))) {
		(void) sprintf(Buff, "LANG=%s", DEFAULT_LANG);
		if (putenv(strdup(Buff)) != 0)
		    if (Debug) Error("Cannot set environment %s.", Buff);
		Buff[0] = CNULL;
	    }
	    (void) setlocale(LC_ALL, "");
	}

	catd = catopen(PdDvPtr->catalog, 0);
	if ((int) catd <= 0)
	    if (Debug) Error("Catalog open of \"%s\" failed: %s.",
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
    char		       *DevName;
    char		       *desc;
    char		       *cp;
    static char			Buff[BUFSIZ];
    struct PdDv		       *PdDvPtr;

    DevName = MkDevName(DevData->DevName, DevData->DevUnit, 0, 0);
    if (FindDeviceByName(DevName, *TreePtr)) {
	if (Debug) printf("Device %s already exists.\n", DevName);
	return((DevInfo_t *) NULL);
    }

    if (CuDvPtr->PdDvLn_Lvalue[0]) {
	(void) sprintf(Buff, "uniquetype='%s'", CuDvPtr->PdDvLn_Lvalue);
	PdDvPtr = GetPdDv(Buff);
    } else {
	if (Debug) Error("No PdDv link value for '%s'.", DevName);
	return((char *) NULL);
    }

    /*
     * If we can't get the description for this device, then
     * use the device specific probe routine if one is defined.
     */
    if ((desc = GetDescript(CuDvPtr, PdDvPtr)) == (char *) NULL) {
	if ((DevDefine = DevDefGet(DL_VPD, DevData->DevName, 0)) && 
	    DevDefine->Probe)
	    return((*DevDefine->Probe)(DevName, DevData, DevDefine, 
				       CuDvPtr, PdDvPtr));
	else {
	    if (Debug) 
		Error("No description found for '%s'.", DevName);
	    return((DevInfo_t *) NULL);
	}
    }

    DevInfo = NewDevInfo((DevInfo_t *) NULL);

    DevInfo->Name = DevName;
    DevInfo->Unit = DevData->DevUnit;
    DevInfo->Master = MkMasterFromDevData(DevData);
    DevInfo->Type = DT_GENERIC;
    DevInfo->ModelDesc = strdup(desc);
    SetDescript(DevInfo, CuDvPtr);

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
	Error("ODM initialize failed: %s", odmerror());
	return(-1);
    }

    for (op = ODM_FIRST; ; op = ODM_NEXT) {
	/*
	 * Retrieve the object from ODM.
	 */
	ret = (int) odm_get_obj(CuDv_CLASS, (char *)NULL, &CuDv, op);
	if (ret == -1) {
	    if (Debug) Error("ODM get object \"%s\" failed: %s", 
			     CuDv_CLASS[0].classname, odmerror());
	    return(-1);
	} else if (ret == 0)
	    /*
	     * We're done
	     */
	    break;

	if (CuDv.status != AVAILABLE) {
	    if (Debug) printf("Device \"%s\" is not available.\n", CuDv.name);
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

	if (Debug)
	    printf("ODM: Found '%s' parent '%s' location '%s' uniq = '%s'\n",
		   CuDv.name, CuDv.parent, CuDv.location, CuDv.PdDvLn_Lvalue);

	/* Probe and add device */
	if (DevInfo = ProbeODM(&DevData, TreePtr, &CuDv))
	    AddDevice(DevInfo, TreePtr, Names);
    }

    if (odm_terminate() != 0)
	if (Debug) Error("ODM Terminate did not succeed.");

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
    static char			DevName[BUFSIZ];
    static struct pginfo	pginfo;
    off_t			Amount = 0;
    static char			Buff[100];

    odm_initialize();
    odm_set_path(_PATH_ODM);

    CuAtPtr = get_CuAt_list(CuAt_CLASS, QueryStr, &ObjListInfo, 20, 1);
    if ((int)CuAtPtr == -1) {
	if (Debug) Error("get_CuAt_list paging info failed: %s.", SYSERR);
	odm_terminate();
	return((char *)NULL);
    }

    for (i = 0; i < ObjListInfo.num; ++i, ++CuAtPtr) {
	(void) sprintf(DevName, "/dev/%s", CuAtPtr->name);

	if (swapqry(DevName, &pginfo) == -1) {
	    if (Debug) Error("swapqry %s failed: %s.", DevName, SYSERR);
	    continue;
	}

	Amount += (off_t) ( (pginfo.size * getpagesize() ) / KBYTES );
    }

    odm_terminate();
    (void) sprintf(Buff, "%d MB", DivRndUp(Amount, KBYTES));

    return(Buff);
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
