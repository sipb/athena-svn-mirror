/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * HP-UX ioscan(1m) related functions
 */

#include "bjhashtab.h"
#include "defs.h"

#if	defined(HAVE_DEVICE_SUPPORT)
/*
 * Data from ioscan(1m)
 */
typedef struct _IOSdata {
    char		       *DevType;	/* PROCESSOR,MEMORY,... */
    char		       *DevClass;	/* disk,graphics,lan,... */
    char		       *DevData;	/* N N N N N ... */
    char		       *DevCat;		/* pa,core,scsi */
    char		       *BusType;	/* pa,wsio,core,eisa,... */
    char		       *Driver;		/* Name of Device driver */
    char		       *NexusName;	/* eisa,bus_adapter,... */
    char		       *HWpath;		/* HW Path N/N/N... */
    char		       *SWpath;		/* SW Path root.bus... */
    char		       *SWstate;	/* CLAIMED,UNCLAIMED,... */
    char		       *Desc;		/* Description of device */
    int				Unit;		/* Device Unit */
    int				BusUnit;	/* My bus's Unit number */
    char		      **DevFiles;	/* List of device files */
    struct _IOSdata	       *Next;
} IOSdata_t;

#include <setjmp.h>
jmp_buf					Env;

#ifdef notnow
/*
 * Check to see if IOSmaster is the master for IOSchild
 */
static int IOScheckMaster(IOSchild, IOSmaster)
     IOSdata_t 		       *IOSchild;
     IOSdata_t 		       *IOSmaster;
{
    char		       *Last;
    int				SWLen = 0;
    int				HWLen = 0;

    if (!IOSchild || !IOSmaster)
	return FALSE;

#ifdef notnow
    SImsg(SIM_DBG, 
	  "IOScheckMaster: Child Type=<%s> Class=<%s> Unit=%d HW=<%s> SW=<%s> Desc=<%s>",
	  PRTS(IOSchild->DevType), PRTS(IOSchild->DevClass),
	  IOSchild->Unit, PRTS(IOSchild->HWpath), PRTS(IOSchild->SWpath),
	  PRTS(IOSchild->Desc));
#endif
    SImsg(SIM_DBG, 
	  "               Master Type=<%s> Class=<%s> Unit=%d HW=<%s> SW=<%s> Desc=<%s>",
	  PRTS(IOSmaster->DevType), PRTS(IOSmaster->DevClass),
	  IOSmaster->Unit, PRTS(IOSmaster->HWpath), PRTS(IOSmaster->SWpath),
	  PRTS(IOSmaster->Desc));

    if (Last = strrchr(IOSchild->SWpath, '.'))
	SWLen = Last - IOSchild->SWpath;
    else
	SWLen = strlen(IOSchild->SWpath);

#ifdef notdef
    /*
     * Check if the possible Master's (IOSchild) BusUnit == the
     * child's (IOSmaster) Unit AND the SWpath aligns
     */
    if ((IOSmaster->Unit == IOSchild->BusUnit) &&
	(IOSmaster->BusUnit == IOSchild->BusUnit) &&
	EQN(IOSmaster->SWpath, IOSchild->SWpath, SWLen))
	return(TRUE);
#endif

    /*
     * Does the software path (e.g. "root.bus_adaptor") and the
     * hardware path (e.g "4/0/1.9") match?
     */
    if (IOSmaster->HWpath)
	HWLen = strlen(IOSmaster->HWpath);

    if ((IOSchild->HWpath[HWLen] == '.' || IOSchild->HWpath[HWLen] == '/') &&
	(IOSchild->SWpath[SWLen] == '.' || IOSchild->SWpath[SWLen] == '/') &&
	EQN(IOSmaster->HWpath, IOSchild->HWpath, HWLen) &&
	EQN(IOSmaster->SWpath, IOSchild->SWpath, SWLen)) {

	SImsg(SIM_DBG, "IOScheckMaster: FOUND Master=<%s> of Child=<%s>",
	      PRTS(IOSmaster->HWpath), PRTS(IOSchild->HWpath));
	return(TRUE);
    }

    return(FALSE);
}

/*
 * Print the DevInfo tree recursively
 * --RECURSE--
 */
static int PrintTree(DevInfo, Level)
     DevInfo_t		       *DevInfo;
     int			Level;
{
    DevInfo_t		       *DevPtr;

    if (!DevInfo)
	return -1;

    if (DevInfo) {
	SImsg(SIM_DBG, "PrintTree(%d): %*sDevice=<%s> Next=<%s> Slaves=<%s>",
	      Level,
	      Level * 4, "", 
	      PRTS(DevInfo->Name),
	      (DevInfo->Next) ? PRTS(DevInfo->Next->Name) : "",
	      (DevInfo->Slaves) ? PRTS(DevInfo->Slaves->Name) : "");
    }

    /*    for (DevPtr = DevInfo->Slaves; DevPtr; DevPtr = DevPtr->Next) */
    if (DevInfo->Slaves)
	PrintTree(DevInfo->Slaves, Level+1);

    if (DevInfo->Next)
	PrintTree(DevInfo->Next, Level);

    return 0;
}
#endif	/* notnow */

/*
 * Find and return the master of DevInfo
 */
static DevInfo_t *IOSfindMaster(DevInfo, HashByHWpath)
     DevInfo_t		       *DevInfo;
     htab_t		       *HashByHWpath;
{
    char		       *Path;
    char		       *cp;
    DevInfo_t		       *Master;
    IOSdata_t		       *IOSdata;
    size_t			s;

    if (!DevInfo || !HashByHWpath)
	return (DevInfo_t *) NULL;

    if (!(IOSdata = (IOSdata_t *) DevInfo->OSdata))
	return (DevInfo_t *) NULL;

    SImsg(SIM_DBG, "IOSfindMaster: Check <%s> HW=<%s> DevInfo=0x%x",
	  PRTS(DevInfo->Name), PRTS(IOSdata->HWpath), DevInfo);
    Path = IOSdata->HWpath;

    for (cp = &Path[strlen(Path)-1]; cp && cp > Path && !DevInfo->Master; 
	 --cp) {
	if (*cp == '.' || *cp == '/') {
	    s = cp - Path;
	    SImsg(SIM_DBG, "IOSfindMaster: Lookup <%.*s> for <%s>",
		  s, Path, PRTS(DevInfo->Name));

	    if (hfind(HashByHWpath, Path, s)) {
		/*
		 * Found the Master.
		 */
		Master = (DevInfo_t *) hstuff(HashByHWpath);
		if (Master) {
		    SImsg(SIM_DBG, 
			  "IOSfindMaster: Master of <%s>[%s] is <%s>[%s]",
			  PRTS(DevInfo->Name), PRTS(Path),
			  PRTS(Master->Name),
			  PRTS( ((IOSdata_t *)Master->OSdata)->HWpath ));
		    return Master;
		}
	    }
	}
    }

    SImsg(SIM_DBG, "IOSfindMaster: NO MASTER found for <%s> <%s>",
	  PRTS(DevInfo->Name), PRTS(Path));

    return (DevInfo_t *) NULL;
}

/*
 * Called when the alarm goes off
 */
static void SignalHandler(SigNum)
     int			SigNum;
{
    SImsg(SIM_DBG, "SignalHandler: Received %s", SignalName(SigNum));
    longjmp(Env, SigNum);
}

/*
 * Take all the data we found from ioscan(1m) found in IOSdataList
 * and build a Device_t tree into TreePtr.
 */
static int IOSdataToDevice(TreePtr, SearchNames, IOSdataList)
     DevInfo_t 		      **TreePtr;
     char 		      **SearchNames;
     IOSdata_t 		       *IOSdataList;
{
    static ProbeData_t		ProbeData;
    IOSdata_t		       *IOSdata;
    DevInfo_t		       *DevInfo = NULL;
    DevInfo_t		       *Master;
    DevDefine_t		       *DevDefine;
    static htab_t	       *HashByHWpath;
    char		       *cp;
    int 			LongJumpVal;
    char		       *What;

    SImsg(SIM_DBG, "IOSdataToDevice START");

    for (IOSdata = IOSdataList; IOSdata; IOSdata = IOSdata->Next) {
	SImsg(SIM_DBG, 
	"Driver=<%s> NexusN=<%s> DevType=<%s> DevClass=<%s> BusType=<%s>",
	      PRTS(IOSdata->Driver), PRTS(IOSdata->NexusName),
	      PRTS(IOSdata->DevType), PRTS(IOSdata->DevClass), 
	      PRTS(IOSdata->BusType));
	SImsg(SIM_DBG, 
	      "\tHWpath=<%s> SWpath=<%s> Unit=%d BusUnit=%d",
	      PRTS(IOSdata->HWpath), PRTS(IOSdata->SWpath), IOSdata->Unit,
	      IOSdata->BusUnit);
	SImsg(SIM_DBG, "\tDesc=<%s>", IOSdata->Desc);

	if (!IOSdata->DevClass)
	    continue;

	DevDefine = DevDefGet(IOSdata->DevClass, 0, 0);
	if (!DevDefine) {
	    SImsg(SIM_DBG, "No such device type as `%s' - IGNORED.",
		  IOSdata->DevClass);
	    continue;
	}

	DevInfo = NewDevInfo(NULL);

	DevInfo->Name = MkDevName(IOSdata->Driver, IOSdata->Unit,
				  DevDefine->Type, DevDefine->Flags);
	/* Set alias/alt names if different from primary */
	if (cp = MkDevName(IOSdata->DevClass, IOSdata->Unit,
			   DevDefine->Type, DevDefine->Flags))
	    if (!EQ(DevInfo->Name, cp)) {
		DevInfo->Aliases = (char **) xcalloc(2, sizeof(char *));
		DevInfo->Aliases[0] = cp;
		DevInfo->AltName = cp;
	    }

	DevInfo->Type = DevDefine->Type;
	DevInfo->Unit = IOSdata->Unit;
	DevInfo->Model = IOSdata->Desc;
	DevInfo->ModelDesc = DevDefine->Desc;
	DevInfo->Files = IOSdata->DevFiles;
	AddDevDesc(DevInfo, IOSdata->SWstate, "Software State", DA_APPEND);
	AddDevDesc(DevInfo, IOSdata->SWpath, "Software Path", DA_APPEND);
	AddDevDesc(DevInfo, IOSdata->HWpath, "Hardware Path", DA_APPEND);
	AddDevDesc(DevInfo, IOSdata->DevClass, "OS Device Class", DA_APPEND);
	AddDevDesc(DevInfo, IOSdata->DevType, "OS Device Type", DA_APPEND);
	AddDevDesc(DevInfo, IOSdata->DevCat, "OS Device Category", DA_APPEND);
	AddDevDesc(DevInfo, IOSdata->BusType, "OS Bus Type", DA_APPEND);
	AddDevDesc(DevInfo, IOSdata->NexusName, "Nexus Name", DA_APPEND);
	AddDevDesc(DevInfo, IOSdata->DevData, "Device Data", DA_APPEND);
	if (IOSdata->BusUnit >= 0)
	    AddDevDesc(DevInfo, itoa(IOSdata->BusUnit), "Bus Unit", DA_APPEND);

	DevInfo->OSdata = (void *) IOSdata;

	/*
	 * Find the master of DevInfo.
	 * This assumes the master has already be created by this
	 * function and registered in HashByHWpath.  This is the
	 * expected behavior of ioscan(1m) and helps prevents ugly
	 * loops.
	 */
	if (Master = IOSfindMaster(DevInfo, HashByHWpath)) {
	    DevInfo->Master = Master;
	}

	/*
	 * Prep data needed for probe functions.
	 */
	(void) memset(&ProbeData, CNULL, sizeof(ProbeData));
	ProbeData.DevName = DevInfo->Name;
	ProbeData.AliasNames = DevInfo->Aliases;
	ProbeData.DevDefine = DevDefine;
	ProbeData.UseDevInfo = DevInfo;

	/*
	 * Set a timeout since some system calls can hang
	 */
	if ((LongJumpVal = setjmp(Env)) != 0) {
	    What = SignalName(LongJumpVal);
	    SImsg(SIM_DBG, "IOSdataToDevice: Probe terminated due to %s", 
		  What);
	    continue;
	}
	TimeOutStart(15, SignalHandler);	/* Timeout of 15 seconds */

	/*
	 * Special check for HP specific HIL
	 */
	if (EQ("hil", DevDefine->Name)) {
	    ProbeHIL(&ProbeData);
	}
	/*
	 * Call device specific probe routine, if any
	 */
	else if ((*DevDefine->Probe) != NULL) {
	    (*DevDefine->Probe)(&ProbeData);
	}

	TimeOutEnd();

	AddDevice(DevInfo, TreePtr, SearchNames);

	/* Add to hash last for future lookups */
	if (!HashByHWpath)
	    HashByHWpath = hcreate(32);
	hadd(HashByHWpath, IOSdata->HWpath, strlen(IOSdata->HWpath), DevInfo);
    }

    SImsg(SIM_DBG, "IOSdataToDevice END");

    return(0);
}

/*
 * Parse an input line from ioscan(1m) containg a list of files and add
 * to IOSdata->DevFiles list.
 */
static int IOSdataParseFiles(LineStr, LineNo, IOSdata)
     char 		       *LineStr;
     int 			LineNo;
     IOSdata_t 		       *IOSdata;
{
    char		      **Argv = NULL;
    int				Argc;
    register int		Count = 0;
    register int		OldCount = 0;
    register int		i;
    char		      **Ptr = NULL;

    if (!IOSdata)
	return(-1);

    /*
     * File names are seperated by white space
     */
    while (isspace(*LineStr))
	++LineStr;
    Argc = StrToArgv(LineStr, " ", &Argv, NULL, 0);
    for (i = 0; i < Argc; ++i) {
	if (!Argv[i] || Argv[i][0] != '/')
	    continue;
	++Count;
    }

    /*
     * XXX realloc() appears to be broken so we have to do things the 
     * hard way.
     */
#ifndef MAXDEVFILES
#define MAXDEVFILES 48
#endif

    if (!IOSdata->DevFiles) {
	Ptr = IOSdata->DevFiles = (char **) xcalloc(MAXDEVFILES + 1, 
						    sizeof(char *));
    } else {
	for (Ptr = IOSdata->DevFiles; Ptr && *Ptr; ++Ptr, ++OldCount);
    }

    if (OldCount + Count > MAXDEVFILES) {
	SImsg(SIM_WARN, "WARNING: MAXDEVFILES (%d) exceeded.", MAXDEVFILES);
	return(-1);
    }

    for (i = 0; i < Argc; ++i) {
	if (!Argv[i] || Argv[i][0] != '/')
	    continue;
	*Ptr = Argv[i];
	*(++Ptr) = NULL;
    }

    return(0);
}

/*
 * Clean a string of unprintable characters and excess white-space.
 */
static char *CleanStr(String, StrSize)
     char 		       *String;
     int 			StrSize;
{
    register int		i, n;
    char		       *NewString;

    NewString = (char *) xcalloc(1, StrSize + 1);

    for (i = 0, n = 0; i < StrSize; ++i) {
	if (i == 0)
	    /* Skip initial white space */
	    while (isspace(String[i]))
		++i;
	/* Skip extra white space */
	if (i > 0 && isspace(String[i-1]) && isspace(String[i]))
	    continue;
	if (isprint(String[i]))
	    NewString[n++] = String[i];
    }

    /* Remove trailing white space */
    while (isspace(NewString[n]))
	NewString[n--] = CNULL;

    return(NewString);
}

/*
 * Parse an input line from ioscan(1m) and create an IOSdata_t
 */
static IOSdata_t *IOSdataParse(LineStr, LineNo)
     char 		       *LineStr;
     int 			LineNo;
{
    char		      **Argv = NULL;
    int				Argc;
    IOSdata_t		       *IOSdata = NULL;

    Argc = StrToArgv(LineStr, ":", &Argv, NULL, 0);
    if (Argc != 19) {
	SImsg(SIM_WARN, 
	 "Input from %s: Line %d: Wrong number of fields (expect %d got %d).",
	 _PATH_IOSCAN, LineNo, 19+1, Argc+1);
	return((IOSdata_t *) NULL);
    }

    IOSdata = (IOSdata_t *) xcalloc(1, sizeof(IOSdata_t));
    IOSdata->Unit = IOSdata->BusUnit = -1;

    if (Argv[12]) IOSdata->Unit 	= atoi(Argv[12]);
    if (Argv[18]) IOSdata->BusUnit 	= atoi(Argv[18]);
    IOSdata->DevType		 	= Argv[16];
    IOSdata->DevClass 			= Argv[8];
    IOSdata->DevCat 			= Argv[0];
    IOSdata->DevData 			= Argv[11];
    IOSdata->BusType 			= Argv[1];
    IOSdata->Driver 			= Argv[9];
    IOSdata->NexusName 			= Argv[14];
    IOSdata->HWpath 			= Argv[10];
    IOSdata->SWpath 			= Argv[13];
    IOSdata->SWstate 			= Argv[15];
    if (Argv[17] && Argv[17][0])
	IOSdata->Desc 			= CleanStr(Argv[17], strlen(Argv[17]));

    return(IOSdata);
}

/*
 * Build the device tree using ioscan(1m)
 */
extern int BuildIOScan(TreePtr, SearchNames)
     DevInfo_t 		      **TreePtr;
     char 		      **SearchNames;
{
    IOSdata_t		       *IOSdataList = NULL;
    IOSdata_t		       *LastIOS = NULL;
    IOSdata_t		       *IOSdataPtr = NULL;
    FILE		       *IOScmd;
    int				LineNo = 0;
    char		        CmdBuff[sizeof(_PATH_IOSCAN) + 
				       sizeof(IOSCAN_ARGS) + 4];
    static char			LineBuff[8192];
    char		       *cp;

    /*
     * Enable the following for debugging of ioscan input.
     *
     *	#define DEBUG_IOSCAN
     */
#if 	defined(DEBUG_IOSCAN)
#undef  _PATH_IOSCAN
#define _PATH_IOSCAN "/bin/cat"
#undef  IOSCAN_ARGS
#define IOSCAN_ARGS "./ioscan.sysinfo.input"
#endif	/* DEBUG_IOSCAN */

    if (!FileExists(_PATH_IOSCAN)) {
	SImsg(SIM_CERR, 
	      "The ioscan(1m) command is required for device support, but is not installed on this system as <%s>.", 
	      _PATH_IOSCAN);
	return(-1);
    }

    (void) snprintf(CmdBuff, sizeof(CmdBuff), "%s %s", 
		    _PATH_IOSCAN, IOSCAN_ARGS);
    SImsg(SIM_DBG, "Running <%s>", CmdBuff);
    if (!(IOScmd = popen(CmdBuff, "r"))) {
	SImsg(SIM_GERR, "popen of `%s' failed: %s", CmdBuff, SYSERR);
	return(-1);
    }

    SImsg(SIM_DBG, "IOSCANINPUT START");
    while (fgets(LineBuff, sizeof(LineBuff), IOScmd)) {
	++LineNo;
	if (cp = strchr(LineBuff, '\n'))
	    *cp = CNULL;
	SImsg(SIM_DBG, "%s", LineBuff);
	if (strchr(LineBuff, ':')) {
	    /* Normal data line */
	    IOSdataPtr = IOSdataParse(LineBuff, LineNo);
	    if (!IOSdataPtr)
		continue;
	    if (LastIOS) {
		LastIOS->Next = IOSdataPtr;
		LastIOS = IOSdataPtr;
	    } else
		LastIOS = IOSdataList = IOSdataPtr;
	} else
	    /* Must be a list of files */
	    IOSdataParseFiles(LineBuff, LineNo, IOSdataPtr);
    }
    SImsg(SIM_DBG, "IOSCANINPUT END");

    pclose(IOScmd);

    if (Debug) {
	SImsg(SIM_DBG, "BuildIOScan DUMP START");
	for (IOSdataPtr = IOSdataList; IOSdataPtr; 
	     IOSdataPtr = IOSdataPtr->Next) {
	    SImsg(SIM_DBG, 
	  "Type=<%s> Class=<%s> Driver=<%s> Unit=%d HWpath=<%s> SWpath=<%s>",
		  PRTS(IOSdataPtr->DevType), PRTS(IOSdataPtr->DevClass),
		  PRTS(IOSdataPtr->Driver), IOSdataPtr->Unit,
		  PRTS(IOSdataPtr->HWpath), PRTS(IOSdataPtr->SWpath));
	}
	SImsg(SIM_DBG, "BuildIOScan DUMP END");
    }

#if	!defined(SYSINFO_DEV)
    if (Debug) {
	/*
	 * Provide more useful debugging info.
	 */
	(void) snprintf(CmdBuff, sizeof(CmdBuff),  "%s -k", _PATH_IOSCAN);
	SImsg(SIM_DBG, "Running <%s>", CmdBuff);
	if (!(IOScmd = popen(CmdBuff, "r"))) {
	    SImsg(SIM_GERR, "popen of `%s' failed: %s", CmdBuff, SYSERR);
	    return(-1);
	}
	while (fgets(LineBuff, sizeof(LineBuff), IOScmd))
	    SImsg(SIM_DBG|SIM_NONL, "IOSCANTREE: %s", LineBuff);
	pclose(IOScmd);
    }
#endif	/* SYSINFO_DEV */
	
    return(IOSdataToDevice(TreePtr, SearchNames, IOSdataList));
}
#endif	/* HAVE_DEVICE_SUPPORT */
