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
 * Configuration File functions
 */
#include "defs.h"

#define EMPTY(a,i,n)	((i >= n) || !a[i] || !a[i][0])

/*
 * List of config files to search for
 */
static char *CFosFileList[] = {
    "${ConfDir}/${OSname}_${OSver}.cf", 
    "${ConfDir}/${OSname}_${OSmajver}.cf",
    "${ConfDir}/${OSname}.cf",
    "${ConfDir}/${DefConfFile}",
    NULL
};

extern char		       *ConfDir;		/* Default */
extern char		       *ConfFile;		/* Default */
char			       *CurrentConfDir;		/* working Conf Dir */
int				DidCFread;		/* CFread() did */

/*
 * Configuration keys
 */
typedef struct {
    char		       *Key;		/* Keyword */
    int			      (*Parse)();	/* Parsing function */
} CFkeys_t;

static int			CFdevice(), CFdefine(), CFinclude();
static int			CFconfdir();

CFkeys_t CFkeys[] = {
    { "ConfDir",		CFconfdir },
    { "Device",			CFdevice },
    { "Define",			CFdefine },
    { "Include",		CFinclude },
    { NULL },
};

/*
 * Device Flag Key words
 */
KeyTab_t DevFlagKeys[] = {
    { "DefInfo",		DDT_DEFINFO },
    { "LenCmp",			DDT_LENCMP },
    { "NoUnit",			DDT_NOUNIT },
    { "AssUnit",		DDT_ASSUNIT },
    { "UnitNum",		DDT_UNITNUM },
    { "Zombie",			DDT_ZOMBIE },
    { NULL },
};

/*
 * Find Word in the CFkeys table.
 */
CFkeys_t *CFgetKey(String, FileName, LineNo)
    char		       *String;
    char		       *FileName;
    int				LineNo;
{
    static char			Key[128];
    register char	       *cp;
    register int		i;
    register int		Len;

    /* Skip leading white space */
    for (cp = String; cp && *cp && isspace(*cp); ++cp);
    /* Copy into Key buffer */
    Len = strlen(cp);
    (void) strncpy(Key, cp, Len);
    Key[Len] = CNULL;
    /* Find end of key and terminate */
    for (cp = Key; cp && *cp && isalpha(*cp); ++cp);
    if (cp != Key)
	*cp = CNULL;

    for (i = 0; CFkeys[i].Key; ++i)
	if (EQ(Key, CFkeys[i].Key))
	    return(&CFkeys[i]);

    SImsg(SIM_GERR, "%s: Line %d: Unknown keyword `%s'.", 
	  FileName, LineNo, Key);

    return((CFkeys_t *) NULL);
}

/*
 * Read and parse CF file FileName.
 * Return 0 on success or nothing to do.
 * Return -1 on open file error
 * Return > 0 on parsing error.
 */
int CFread(FileName, DispErrs)
    char		       *FileName;
    int				DispErrs;
{
    static char			Buff[BUFSIZ];
    register char	       *cp;
    FILE		       *FilePtr;
    register int		LineNo = 0;
    char		      **Argv;
    CFkeys_t		       *CFkey;
    int				Argc;

    FilePtr = fopen(FileName, "r");
    if (!FilePtr) {
	if (DispErrs)
	    SImsg(SIM_CERR, "%s: open failed: %s", FileName, SYSERR);
	return(-1);
    }

    SImsg(SIM_DBG, "Reading `%s' . . .", FileName);

    while (fgets(Buff, sizeof(Buff), FilePtr)) {
	++LineNo;
	if (Buff[0] == '#' || Buff[0] == '\n')
	    continue;
	if (cp = strchr(Buff, '\n'))	*cp = CNULL;
	if (cp = strchr(Buff, '#'))	*cp = CNULL;
	if (!Buff[0])
	    continue;

	CFkey = CFgetKey(Buff, FileName, LineNo);
	if (!CFkey) {
	    (void) fclose(FilePtr);
	    return(1);
	}

	if ( (*CFkey->Parse)(FileName, LineNo, Buff) != 0) {
	    (void) fclose(FilePtr);
	    return(1);
	}
    }

    DidCFread = TRUE;

    (void) fclose(FilePtr);
    return(0);
}

static char *CFgetPath(PathName, LineNo, String)
    char		       *PathName;
    int				LineNo;
    char		       *String;
{
    register char	       *cp;
    char		       *IncFile;
    static char			Path[MAXPATHLEN];
    static char			ErrBuff[BUFSIZ];

    /* Skip leading white space */
    for (cp = String; cp && *cp && isspace(*cp); ++cp);
    /* Skip include word */
    for ( ; cp && *cp && !isspace(*cp); ++cp);
    /* Skip white space */
    for ( ; cp && *cp && isspace(*cp); ++cp);
    if (!cp) {
	SImsg(SIM_GERR, "%s: Line %d: No pathname specified.", 
	      PathName, LineNo);
	return((char *) NULL);
    }
    IncFile = cp;
    /* Find end of filename */
    for ( ; cp && *cp && !isspace(*cp); ++cp);
    if (cp)
	*cp = CNULL;

    IncFile = VarSub(IncFile, ErrBuff, sizeof(ErrBuff),
		     VarGetVal, (Opaque_t)NULL);
    if (!IncFile) {
	SImsg(SIM_GERR, "%s: Line %d: %s", PathName, LineNo, ErrBuff);
	return((char *) NULL);
    }

    if (IncFile[0] == '/' || IncFile[0] == '.')
	(void) strcpy(Path, IncFile);
    else
	(void) snprintf(Path, sizeof(Path),  "%s/%s", CurrentConfDir, IncFile);

    return(Path);
}

/*
 * Get "ConfDir" CF line
 */
static int CFconfdir(PathName, LineNo, String)
    char		       *PathName;
    int				LineNo;
    char		       *String;
{
    char		       *Path;

    Path = CFgetPath(PathName, LineNo, String);

    if (FileExists(Path) && !IsDir(Path)) {
	SImsg(SIM_GERR, "%s: Line %d: %s is not a directory.",
	      PathName, LineNo, Path);
	return(-1);
    }

    CurrentConfDir = strdup(Path);
    SImsg(SIM_DBG, "ConfDir is now <%s>", CurrentConfDir);

    return(0);
}

/*
 * Set "include" CF line
 */
static int CFinclude(PathName, LineNo, String)
    char		       *PathName;
    int				LineNo;
    char		       *String;
{
    char		       *Path;

    Path = CFgetPath(PathName, LineNo, String);

    if (FileExists(Path) && !IsFile(Path)) {
	SImsg(SIM_GERR, "%s: Line %d: %s is not a file.",
	      PathName, LineNo, Path);
	return(-1);
    }

    if (CFread(Path, FALSE) > 0) {
	SImsg(SIM_GERR, "%s: Line %d: Error while parsing `%s'",
	      PathName, LineNo, Path);
	return(-1);
    }

    return(0);
}

/*
 * Parse a "define" CF line
 */
static int CFdefine(FileName, LineNo, String)
    char		       *FileName;
    int				LineNo;
    char		       *String;
{
    char		      **Argv;
    int				Argc;
    Define_t	       	       *DefPtr;

    Argc = StrToArgv(String, "|", &Argv, NULL, 0);
    if (Argc < 1) {
	SImsg(SIM_GERR, "%s: Line %d: No fields found.", FileName, LineNo);
	return(-1);
    }

    if (EMPTY(Argv, 1, Argc)) {
	SImsg(SIM_GERR, "%s: Line %d: No table name found in field 2.", 
	      FileName, LineNo);
	DestroyArgv(&Argv, Argc);
	return(-1);
    }
#ifdef STRICT_CF_CHECKING
    if (!DefValid(Argv[1], Argc)) {
	SImsg(SIM_GERR, 
	      "%s: Line %d: `%s' is an invalid definetion table (field 2).", 
	      FileName, LineNo, Argv[1]);
	DestroyArgv(&Argv, Argc);
	return(-1);
    }
#endif	/* STRICT_CF_CHECKING */
    if (EMPTY(Argv, 2, Argc) && EMPTY(Argv, 3, Argc)) {
	SImsg(SIM_GERR, "%s: Line %d: No key found in field 3 or 4.", 
	      FileName, LineNo);
	DestroyArgv(&Argv, Argc);
	return(-1);
    }
    if (EMPTY(Argv, 4, Argc) && EMPTY(Argv, 5, Argc)) {
	SImsg(SIM_GERR, "%s: Line %d: No values found in field 5 or 6.", 
	      FileName, LineNo);
	DestroyArgv(&Argv, Argc);
	return(-1);
    }

    DefPtr = (Define_t *) xcalloc(1, sizeof(Define_t));
    DefPtr->KeyNum = -1;
    if (!EMPTY(Argv, 2, Argc)) DefPtr->KeyStr = strdup(Argv[2]);
    if (!EMPTY(Argv, 3, Argc)) DefPtr->KeyNum = strtol(Argv[3], (char **)NULL,
						       0);
    if (!EMPTY(Argv, 4, Argc)) DefPtr->ValStr1 = strdup(Argv[4]);
    if (!EMPTY(Argv, 5, Argc)) DefPtr->ValStr2 = strdup(Argv[5]);
    if (!EMPTY(Argv, 6, Argc)) DefPtr->ValStr3 = strdup(Argv[6]);
    if (!EMPTY(Argv, 7, Argc)) DefPtr->ValStr4 = strdup(Argv[7]);
    if (!EMPTY(Argv, 8, Argc)) DefPtr->ValStr5 = strdup(Argv[8]);

    DefAdd(DefPtr, Argv[1]);

    DestroyArgv(&Argv, Argc);
    return(0);
}

/*
 * Parse a "device" CF line
 */
static int CFdevice(FileName, LineNo, String)
    char		       *FileName;
    int				LineNo;
    char		       *String;
{
    DevDefine_t		       *DevDefine;
    DevType_t		       *DevType;
    ClassType_t		       *Class;
    register char	       *cp;
    register int		i;
    register int		n;
    char		      **Argv = NULL;
    int				Argc = 0;
    char		      **FlagArgv = NULL;
    int				FlagArgc = 0;
    char		       *ProbeName = NULL;
    char		       *TypeName = NULL;
    char		       *NameField;
    char		      **NameArgv;
    int				NameArgc;
    static int			First = TRUE;

#if	defined(HAVE_DEVICE_SUPPORT)
    /*
     * Initialize DevTypes[] on our first call.
     */
    if (First) {
	First = FALSE;
	DevTypesInit();
    }
#endif	/* HAVE_DEVICE_SUPPORT */

    Argc = StrToArgv(String, "|", &Argv, NULL, 0);
    if (Argc < 1) {
	SImsg(SIM_GERR, "%s: Line %d: No fields found.", FileName, LineNo);
	return(-1);
    }

    if (EMPTY(Argv, 1, Argc) && EMPTY(Argv, 3, Argc)) {
	SImsg(SIM_GERR, 
	      "%s: Line %d: The name (2) and ident (4) fields are both empty.",
	      FileName, LineNo);
	DestroyArgv(&Argv, Argc);
	return(-1);
    }
    if (EMPTY(Argv, 2, Argc)) {
	SImsg(SIM_GERR, "%s: Line %d: The type (2) field cannot be empty.",
	      FileName, LineNo);
	DestroyArgv(&Argv, Argc);
	return(-1);
    }

    DevDefine = (DevDefine_t *) xcalloc(1, sizeof(DevDefine_t));
    if (!EMPTY(Argv, 1, Argc)) {
	NameField = Argv[1];
	NameArgc = StrToArgv(NameField, " ", &NameArgv, NULL, 0);
	if (NameArgc > 1) {
	    DevDefine->Name = strdup(NameArgv[0]);
	    DevDefine->Aliases = &NameArgv[1];
	} else
	    DevDefine->Name = strdup(Argv[1]);
    }

    /*
     * The type field (Argv[2]) can either be "probename" or
     * "probename/typename".  e.g. "generic" or "generic/diskctlr"
     */
    ProbeName = Argv[2];
    if (cp = strchr(ProbeName, '/')) {
	*cp++ = CNULL;
	TypeName = cp;
    } else
	TypeName = ProbeName;

    DevType = TypeGetByName(ProbeName);
    if (!DevType) {
	SImsg(SIM_GERR, "%s: Line %d: `%s' is an invalid device type name.",
	      FileName, LineNo, ProbeName);
	DestroyArgv(&Argv, Argc);
	return(-1);
    }
    DevDefine->Probe = DevType->Probe;

    DevType = TypeGetByName(TypeName);
    if (!DevType) {
	SImsg(SIM_GERR, "%s: Line %d: `%s' is an invalid device type name.",
	      FileName, LineNo, TypeName);
	DestroyArgv(&Argv, Argc);
	return(-1);
    }
    DevDefine->Type = DevType->Type;

    if (!EMPTY(Argv, 3, Argc)) DevDefine->Ident = strtol(Argv[3], NULL, 0);
    if (!EMPTY(Argv, 4, Argc)) DevDefine->Vendor = strdup(Argv[4]);
    if (!EMPTY(Argv, 5, Argc)) DevDefine->Model = strdup(Argv[5]);
    if (!EMPTY(Argv, 6, Argc)) DevDefine->Desc = strdup(Argv[6]);
    if (!EMPTY(Argv, 7, Argc))
	if (Class = ClassTypeGetByName(DevDefine->Type, Argv[7]))
	    DevDefine->ClassType = Class->Type;
    if (!EMPTY(Argv, 8, Argc)) {
	FlagArgc = StrToArgv(Argv[8], ",", &FlagArgv, NULL);
	for (n = 0; n < FlagArgc; ++n) {
	    for (i = 0; DevFlagKeys[i].Key; ++i)
		if (EQ(DevFlagKeys[i].Key, FlagArgv[n]))
		    DevDefine->Flags |= DevFlagKeys[i].Lvalue;
	    if (!DevDefine->Flags) {
		if (isalpha(FlagArgv[n][0])) {
		    SImsg(SIM_GERR, "%s: Line %d: Unknown flag name `%s'.",
			  FileName, LineNo, FlagArgv[n]);
		    DestroyArgv(&Argv, Argc);
		    return(-1);
		}
		/* Must be a device specific flag */
		DevDefine->DevFlags = strtol(FlagArgv[n], (char **)NULL, 0);
	    }
	}
	DestroyArgv(&FlagArgv, FlagArgc);
    }
    if (!EMPTY(Argv, 9, Argc)) DevDefine->File = strdup(Argv[9]);

    (void) DevDefAdd(DevDefine);

    DestroyArgv(&Argv, Argc);
    return(0);
}

/*
 * Find and parse each config file found in CFosFileList.
 */
extern int CFparseList()
{
    static char			ErrBuff[256];
    register char	      **cpp;
    char		       *File;
    int				RetStatus = 0;
    int				Status = 0;

    for (cpp = CFosFileList; cpp && *cpp; ++cpp) {
	File = VarSub(*cpp, ErrBuff, sizeof(ErrBuff),
		      VarGetVal, (Opaque_t)NULL);
	if (!File) {
	    SImsg(SIM_GERR, "Internal error: %s", ErrBuff);
	    continue;
	}
	Status = CFread(File, FALSE);
	if (Status == 0)
	    return(0);
	else if (Status > 0)
	    ++Status;
    }

    return(Status);
}

/*
 * Find and parse config files
 */
extern int CFparse(myConfFile, myConfDir)
    char		       *myConfFile;
    char		       *myConfDir;
{
    int				Status1 = 0;
    int				Status2 = 0;

    /*
     * Set the name of the configuration directory
     */
    if (myConfDir)
	CurrentConfDir = myConfDir;
    else
	CurrentConfDir = ConfDir;

    if (myConfFile) {
	/* User specified a .cf file */
	Status1 = CFread(myConfFile, TRUE);
	if (Status1 > 0)
	    /* Parsing error */
	    ++Status1;
	else if (Status1 < 0)
	    /* Problems opening a user specified .cf file are hard errors */
	    return(-1);
    } else {
	/* We only care about parsing errors for the master file */
	if (CFread(MASTER_CONFIG_FILE, FALSE) > 0)
	    ++Status1;
    }

    /*
     * Parse the standard set of files
     */
    Status2 = CFparseList();

    if (!DidCFread) {
	SImsg(SIM_CERR, "Could not locate any configuration files in `%s'.", 
	      myConfDir);
	return(-1);
    }

    return(Status1 + Status2);
}
