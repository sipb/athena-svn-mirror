/*
 * Copyright (c) 1992-1998 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not
 * sold for profit or used in part or in whole for commercial gain
 * without prior written agreement, and the author is credited
 * appropriately.
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * SunOS/Solaris DeviceDB routines.
 *
 * These routines will parse the "devicedb" file that comes with 
 * i86pc Solaris 2.x.  A copy of devicedb is distributed with
 * this program for use on SPARC Solaris 2.x which sometimes contains
 * some of these devices. 
 */

#include "defs.h"

#if	defined(HAVE_DEVICEDB)
/*
 * List of devicedb files to search for
 */
static char *DeviceDBList[] = {
    /* So far (Solaris 2.6) only i86pc has /platform/.../master */
    "/platform/${KArch}/boot/solaris/devicedb/master",
    "${ConfDir}/${OSname}_${OSver}.ddb", 
    "${ConfDir}/${OSname}_${OSmajver}.ddb",
    "${ConfDir}/${OSname}.ddb",
    "${ConfDir}/local.ddb",
    NULL
};

/*
 * Parse a line from a DeviceDB file.
 */
static DevInfo_t *DDBparse(String, FileName, LineNo)
    char		       *String;
    char		       *FileName;
    int				LineNo;
{
    char		      **Argv;
    int				Argc;
    char		      **NameArgv;
    int				NameArgc;
    char		       *Model = NULL;
    DevInfo_t	       	       *DevInfo = NULL;
    DevType_t		       *DevType;
    ClassType_t		       *Class;

    Argc = StrToArgv(String, " \t", &Argv, "\"\"", 0);
    if (Argc < 1) {
	SImsg(SIM_GERR, "%s: Line %d: No fields found.", FileName, LineNo);
	return((DevInfo_t *) NULL);
    }
    if (Argc < 6) {
	SImsg(SIM_GERR, "%s: Line %d: Wrong number of fields (%d) found.", 
	      FileName, LineNo, Argc);
	return((DevInfo_t *) NULL);
    }

    DevInfo = NewDevInfo(NULL);

    if (Argv[0]) {			/* Device IDs */
	NameArgc = StrToArgv(Argv[0], "|", &NameArgv, NULL, 0);
	if (NameArgc < 1) {
	    SImsg(SIM_GERR, "%s: Line %d: No names found.", FileName, LineNo);
	    return((DevInfo_t *) NULL);
	}
	DevInfo->Aliases = NameArgv;
    }

    /*
     * If the driver name is not 'none' use it as the device name.
     * This enables us to later find the real device names in the
     * device trees.
     */
    if (Argv[4]) {			/* Name of driver */
	if (!EQ(Argv[4], "none")) {
	    char	       *cp;

	    DevInfo->Name = strdup(Argv[4]);
	    /* Zap the extension part of driver name, if any */
	    if (cp = strchr(DevInfo->Name, '.'))
		*cp = CNULL;
	}
    }

    if (Argv[1] && !DevInfo->Name) {	/* Device Name */
	if (!EQ(Argv[1], "none"))
	    DevInfo->Name = strdup(Argv[1]);
	else
	    DevInfo->Name = strdup(NameArgv[0]);
    }

    if (Argv[2]) {			/* Device Type */
	if (DevType = TypeGetByName(Argv[2]))
	    DevInfo->Type = DevType->Type;
	else if (Debug && !EQ(Argv[2], "oth"))
	    SImsg(SIM_DBG, "\tDeviceDB: Type <%s> is not defined.", Argv[2]);
    }

    if (Argv[3] && !EQ(Argv[3], "all"))	/* Bus Type */
	if (Class = ClassTypeGetByName(DT_BUS, Argv[3]))
	    DevInfo->ClassType = Class->Type;

    if (Argv[5])			/* Description/Model */
	DevInfo->Model = strdup(Argv[5]);

    return(DevInfo);
}

/*
 * Read a DeviceDB file and parse each line.
 */
static DevInfo_t *DDBreadFile(FileName)
     char		       *FileName;
{
    DevInfo_t		       *DevInfoTree = NULL;
    DevInfo_t		       *LastDevInfo;
    DevInfo_t		       *DevInfo;
    static char			Buff[BUFSIZ];
    register char	       *cp;
    FILE		       *FilePtr;
    register int		LineNo = 0;
    char		      **Argv;
    int				Argc;

    FilePtr = fopen(FileName, "r");
    if (!FilePtr) {
	SImsg(SIM_GERR, "Cannot open DeviceDB file %s: %s.", FileName, SYSERR);
	return((DevInfo_t *) NULL);
    }

    SImsg(SIM_DBG, "Reading DeviceDB file `%s' . . .", FileName);

    while (fgets(Buff, sizeof(Buff), FilePtr)) {
	++LineNo;
	if (Buff[0] == '#' || Buff[0] == '\n')
	    continue;
	if (cp = strchr(Buff, '\n'))	*cp = CNULL;
	if (cp = strchr(Buff, '#'))	*cp = CNULL;
	if (!Buff[0])
	    continue;

	if (LineNo == 1) {
	    if (EQN("version", Buff, 7))
		SImsg(SIM_DBG, "\tDeviceDB %s", Buff);
	} else {
	    if (DevInfo = DDBparse(Buff, FileName, LineNo)) {
		if (!DevInfoTree)
		    DevInfoTree = LastDevInfo = DevInfo;
		else {
		    LastDevInfo->Next = DevInfo;
		    LastDevInfo = DevInfo;
		}
	    }
	}
    }

    (void) fclose(FilePtr);

    return(DevInfoTree);
}

/*
 * Find and parse each devicedb (ddb) file we find.
 */
extern DevInfo_t *DDBreadFiles()
{
    static char			ErrBuff[BUFSIZ];
    register char	      **cpp;
    char		       *File;
    DevInfo_t		       *DevInfo;
    DevInfo_t		       *DevInfoTree = NULL;
    DevInfo_t		       *LastDevInfo = NULL;

    for (cpp = DeviceDBList; cpp && *cpp; ++cpp) {
	File = VarSub(*cpp, ErrBuff, sizeof(ErrBuff),
		      VarGetVal, (Opaque_t)NULL);
	if (!File) {
	    SImsg(SIM_GERR, "Internal error: %s", ErrBuff);
	    continue;
	}
	if (DevInfo = DDBreadFile(File)) {
	    if (!DevInfoTree)
		DevInfoTree = LastDevInfo = DevInfo;
	    else {
		LastDevInfo->Next = DevInfo;
		LastDevInfo = DevInfo;
	    }
	}
    }

    return(DevInfoTree);
}

/*
 * Get the Device Info tree from the Solaris DeviceDB file(s).
 */
extern DevInfo_t *DDBgetDevInfo(DevData)
     DevData_t		       *DevData;
{
    static DevInfo_t	       *DevInfoDeviceDB;
    register DevInfo_t	       *Ptr;
    register char	      **cpp;

    if (!DevData->DevName)
	return((DevInfo_t *) NULL);

    if (!DevInfoDeviceDB)
	DevInfoDeviceDB = DDBreadFiles();

    for (Ptr = DevInfoDeviceDB; Ptr; Ptr = Ptr->Next) {
	if (Ptr->Name && EQ(DevData->DevName, Ptr->Name))
	    return(Ptr);
	for (cpp = Ptr->Aliases; cpp && *cpp; ++cpp)
	    if (EQ(DevData->DevName, *cpp))
		return(Ptr);
    }

    return((DevInfo_t *) NULL);
}
#endif	/* HAVE_DEVICEDB */
