/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: cf.c,v 1.1.1.1 1996-10-07 20:16:49 ghudson Exp $";
#endif

/*
 * Configuration File functions
 */
#include "defs.h"

#define EMPTY(a,i,n)	((i >= n) || !a[i] || !a[i][0])

/*
 * List of config files to search for
 */
static char *CFileList[] = {
    "${ConfDir}/${OSname}_${OSver}.cf", 
    "${ConfDir}/${OSname}_${OSmajver}.cf",
    "${ConfDir}/${OSname}.cf",
    "${ConfDir}/${DefConfFile}",
    NULL
};

/*
 * Configuration keys
 */
typedef struct {
    char		       *Key;		/* Keyword */
    int			      (*Parse)();	/* Parsing function */
} CFkeys_t;

static int			CFdevice(), CFdefine(), CFinclude();

CFkeys_t CFkeys[] = {
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
    static char			Key[BUFSIZ];
    register char	       *cp;
    register int		i;

    /* Skip leading white space */
    for (cp = String; cp && *cp && isspace(*cp); ++cp);
    (void) strcpy(Key, cp);
    /* Find end of key and terminate */
    for (cp = Key; cp && *cp && isalpha(*cp); ++cp);
    if (cp != Key)
	*cp = CNULL;

    for (i = 0; CFkeys[i].Key; ++i)
	if (EQ(Key, CFkeys[i].Key))
	    return(&CFkeys[i]);

    Error("%s: Line %d: Unknown keyword `%s'.", FileName, LineNo, Key);
    return((CFkeys_t *) NULL);
}

/*
 * Get Value of CF Variables
 */
extern char *CFgetVar(Variable, Params)
    char		       *Variable;
    Opaque_t			Params;
    /*ARGSUSED*/
{
    static char			OSnameBuf[100];
    static char			OSverBuf[100];
    static char			OSmajverBuf[100];
    register char	       *Value;
    register char	       *cp;
    extern char		       *ConfDir;

    if (EQ(Variable, "OSname")) {
	if (Value = GetOSName()) {
	    (void) strcpy(OSnameBuf, Value);
	    strtolower(OSnameBuf);
	    return(OSnameBuf);
	}
    } else if (EQ(Variable, "OSver")) {
	if (Value = GetOSVer()) {
	    (void) strcpy(OSverBuf, Value);
	    strtolower(OSverBuf);
	    return(OSverBuf);
	}
    } else if (EQ(Variable, "OSmajver")) {
	if (Value = GetOSVer()) {
	    (void) strcpy(OSmajverBuf, Value);
	    if (cp = strchr(OSmajverBuf, '.'))
		*cp = CNULL;
	    strtolower(OSmajverBuf);
	    return(OSmajverBuf);
	}
    } else if (EQ(Variable, "ConfDir")) {
	return(ConfDir);
    } else if (EQ(Variable, "DefConfFile")) {
	return(DEFAULT_CONFIG_FILE);
    }

    return((char *) NULL);
}

/*
 * Read and parse CF file FileName.
 * Return 0 on success.
 * Return -1 on open file error.
 * Return > 0 on parsing error.
 */
int CFread(FileName, SilentFail)
    char		       *FileName;
    int				SilentFail;
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
	if (SilentFail || Debug)
	    Error("Cannot open config file %s: %s.", FileName, SYSERR);
	return(-1);
    }

    if (Debug) printf("Reading `%s' . . .\n", FileName);

    while (fgets(Buff, sizeof(Buff), FilePtr)) {
	++LineNo;
	if (Buff[0] == '#' || Buff[0] == '\n')
	    continue;
	if (cp = strchr(Buff, '\n'))	*cp = CNULL;
	if (cp = strchr(Buff, '#'))	*cp = CNULL;
	if (!Buff[0])
	    continue;

	CFkey = CFgetKey(Buff, FileName, LineNo);
	if (!CFkey)
	    return(1);

	if ( (*CFkey->Parse)(FileName, LineNo, Buff) != 0)
	    return(1);
    }

    (void) fclose(FilePtr);
    return(0);
}

/*
 * Set "include" CF line
 */
static int CFinclude(FileName, LineNo, String)
    char		       *FileName;
    int				LineNo;
    char		       *String;
{
    register char	       *cp;
    char		       *IncFile;
    static char			Path[MAXPATHLEN];
    static char			ErrBuff[BUFSIZ];
    extern char		       *ConfDir;

    /* Skip leading white space */
    for (cp = String; cp && *cp && isspace(*cp); ++cp);
    /* Skip include word */
    for ( ; cp && *cp && !isspace(*cp); ++cp);
    /* Skip white space */
    for ( ; cp && *cp && isspace(*cp); ++cp);
    if (!cp) {
	Error("%s: Line %d: No filename specified.", FileName, LineNo);
	return(-1);
    }
    IncFile = cp;
    /* Find end of filename */
    for ( ; cp && *cp && !isspace(*cp); ++cp);
    if (cp)
	*cp = CNULL;

    IncFile = VarSub(IncFile, ErrBuff, CFgetVar, (Opaque_t)NULL);
    if (!IncFile) {
	Error("%s: Line %d: %s", FileName, LineNo, ErrBuff);
	return(-1);
    }

    if (IncFile[0] == '/')
	(void) strcpy(Path, IncFile);
    else
	(void) sprintf(Path, "%s/%s", ConfDir, IncFile);

    if (CFread(Path, FALSE) > 0) {
	Error("%s: Line %d: Error while parsing `%s'.",
	      FileName, LineNo, Path);
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

    Argc = StrToArgv(String, "|", &Argv);
    if (Argc < 1) {
	Error("%s: Line %d: No fields found.", FileName, LineNo);
	return(-1);
    }

    if (EMPTY(Argv, 1, Argc)) {
	Error("%s: Line %d: No table name found in field 2.", 
	      FileName, LineNo);
	DestroyArgv(&Argv, Argc);
	return(-1);
    }
#ifdef STRICT_CF_CHECKING
    if (!DefValid(Argv[1], Argc)) {
	Error("%s: Line %d: `%s' is an invalid definetion table (field 2).", 
	      FileName, LineNo, Argv[1]);
	DestroyArgv(&Argv, Argc);
	return(-1);
    }
#endif	/* STRICT_CF_CHECKING */
    if (EMPTY(Argv, 2, Argc) && EMPTY(Argv, 3, Argc)) {
	Error("%s: Line %d: No key found in field 3 or 4.", FileName, LineNo);
	DestroyArgv(&Argv, Argc);
	return(-1);
    }
    if (EMPTY(Argv, 4, Argc) && EMPTY(Argv, 5, Argc)) {
	Error("%s: Line %d: No values found in field 5 or 6.", 
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
    register char	       *cp;
    register int		i;
    char		      **Argv;
    int				Argc;
    char		       *ProbeName = NULL;
    char		       *TypeName = NULL;
    char		       *NameField;
    char		      **NameArgv;
    int				NameArgc;
    static int			First = TRUE;

    /*
     * Initialize DevTypes[] on our first call.
     */
    if (First) {
	First = FALSE;
	DevTypesInit();
    }

    Argc = StrToArgv(String, "|", &Argv);
    if (Argc < 1) {
	Error("%s: Line %d: No fields found.", FileName, LineNo);
	return(-1);
    }

    if (EMPTY(Argv, 1, Argc) && EMPTY(Argv, 3, Argc)) {
	Error("%s: Line %d: The name (2) and ident (4) fields are both empty.",
	      FileName, LineNo);
	DestroyArgv(&Argv, Argc);
	return(-1);
    }
    if (EMPTY(Argv, 2, Argc)) {
	Error("%s: Line %d: The type (2) field cannot be empty.",
	      FileName, LineNo);
	DestroyArgv(&Argv, Argc);
	return(-1);
    }

    DevDefine = (DevDefine_t *) xcalloc(1, sizeof(DevDefine_t));
    if (!EMPTY(Argv, 1, Argc)) {
	NameField = Argv[1];
	NameArgc = StrToArgv(NameField, " ", &NameArgv);
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
	Error("%s: Line %d: `%s' is an invalid device type name.",
	      FileName, LineNo, ProbeName);
	DestroyArgv(&Argv, Argc);
	return(-1);
    }
    DevDefine->Probe = DevType->Probe;

    DevType = TypeGetByName(TypeName);
    if (!DevType) {
	Error("%s: Line %d: `%s' is an invalid device type name.",
	      FileName, LineNo, TypeName);
	DestroyArgv(&Argv, Argc);
	return(-1);
    }
    DevDefine->Type = DevType->Type;

    if (!EMPTY(Argv, 3, Argc)) DevDefine->Ident = strtol(Argv[3], NULL, 0);
    if (!EMPTY(Argv, 4, Argc)) DevDefine->Model = strdup(Argv[4]);
    if (!EMPTY(Argv, 5, Argc)) DevDefine->Desc = strdup(Argv[5]);
    if (!EMPTY(Argv, 6, Argc)) {
	for (i = 0; DevFlagKeys[i].Key; ++i)
	    if (EQ(DevFlagKeys[i].Key, Argv[6]))
		DevDefine->Flags = DevFlagKeys[i].Lvalue;
	if (!DevDefine->Flags) {
	    if (isalpha(Argv[6][0])) {
		Error("%s: Line %d: Unknown flag name `%s'.",
		      FileName, LineNo, Argv[6]);
		DestroyArgv(&Argv, Argc);
		return(-1);
	    }
	    /* Must be a device specific flag */
	    DevDefine->DevFlags = strtol(Argv[6], (char **)NULL, 0);
	}
    }
    if (!EMPTY(Argv, 7, Argc)) DevDefine->File = strdup(Argv[7]);

    (void) DevDefAdd(DevDefine);

    DestroyArgv(&Argv, Argc);
    return(0);
}

/*
 * Find and parse a config file
 */
extern int CFparse(ConfFile, ConfDir)
    char		       *ConfFile;
    char		       *ConfDir;
{
    static char			ErrBuff[BUFSIZ];
    register char	      **cpp;
    char		       *File;

    if (ConfFile)
	return(CFread(ConfFile, FALSE));

    for (cpp = CFileList; cpp && *cpp; ++cpp) {
	File = VarSub(*cpp, ErrBuff, CFgetVar, (Opaque_t)NULL);
	if (!File) {
	    Error("Internal error: %s", ErrBuff);
	    continue;
	}
	if (CFread(File, FALSE) == 0)
	    return(0);
    }

    Error("Could not locate any configuration files in `%s'.", ConfDir);
    return(-1);
}
