/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Variable functions
 */

#include "defs.h"

/*
 * Get Variable Values
 */
extern char *VarGetVal(Variable, Params)
    char		       *Variable;
    Opaque_t			Params;
    /*ARGSUSED*/
{
    static char			OSnameBuf[100];
    static char			OSverBuf[100];
    static char			OSmajverBuf[100];
    static char			KArchBuf[100];
    register char	       *cp;
    register char	       *cp2;
    extern char		       *CurrentConfDir;
    static MCSIquery_t		Query;

    (void) memset(&Query, CNULL, sizeof(&Query));
    Query.Flags |= MCSIF_STRING;

    if (EQ(Variable, "OSname")) {
	Query.Cmd = MCSI_OSNAME;
	if (mcSysInfo(&Query) == 0) {
	    /*
	     * Copy Value into OSnameBuf while skipping '/' and
	     * lower casing
	     */
	    for (cp = Query.Out, cp2 = OSnameBuf; 
		 cp && *cp && cp2 < &OSnameBuf[sizeof(OSnameBuf)]; ++cp)
		if (*cp != '/')
		    *cp2++ = tolower(*cp);
	    return(OSnameBuf);
	}
    } else if (EQ(Variable, "OSver")) {
	Query.Cmd = MCSI_OSVER;
	if (mcSysInfo(&Query) == 0) {
	    (void) snprintf(OSverBuf, sizeof(OSverBuf), "%s", Query.Out);
	    strtolower(OSverBuf);
	    return(OSverBuf);
	}
    } else if (EQ(Variable, "OSmajver")) {
	Query.Cmd = MCSI_OSVER;
	if (mcSysInfo(&Query) == 0) {
	    (void) snprintf(OSmajverBuf, sizeof(OSmajverBuf), "%s", Query.Out);
	    if (cp = strchr(OSmajverBuf, '.'))
		*cp = CNULL;
	    strtolower(OSmajverBuf);
	    return(OSmajverBuf);
	}
    } else if (EQ(Variable, "KArch")) {
	Query.Cmd = MCSI_KERNARCH;
	if (mcSysInfo(&Query) == 0) {
	    (void) snprintf(KArchBuf, sizeof(KArchBuf), "%s", Query.Out);
	    if (cp = strchr(KArchBuf, '.'))
		*cp = CNULL;
	    strtolower(KArchBuf);
	    return(KArchBuf);
	}
    } else if (EQ(Variable, "ConfDir")) {
	return(CurrentConfDir);
    } else if (EQ(Variable, "DefConfFile")) {
	return(DEFAULT_CONFIG_FILE);
    }

    return((char *) NULL);
}

/*
 * Substitute variables in String and return result.
 */
extern char *VarSub(String, ErrBuff, ErrBuffLen, GetVarFunc, VarParams)
    char		       *String;
    char		       *ErrBuff;
    size_t			ErrBuffLen;
    char		     *(*GetVarFunc)();
    Opaque_t			VarParams;
{
    static char			Buf[BUFSIZ];
    static char			Var[BUFSIZ];
    register char	       *BufPtr;
    register char	       *StrPtr;
    register char	       *Value;
    int				ValLen;
    int				BufLen;
    char		       *Start;
    char			EndChar;

    Buf[0] = CNULL;
    for (BufPtr = Buf, StrPtr = String; StrPtr && *StrPtr; ) {
	if (*StrPtr == '$') {
	    ++StrPtr;

	    /* Look at start and end */
	    EndChar = CNULL;
	    if (*StrPtr == '{')
		EndChar = '}';
	    if (*StrPtr == '(')
		EndChar = ')';
	    if (!EndChar) {
		(void) snprintf(ErrBuff, ErrBuffLen,
			       "Variables must start with `(' or `{': `%s'", 
			       String);
		return((char *) NULL);
	    }
	    ++StrPtr;
	    if (!StrPtr || !*StrPtr) {
		(void) snprintf(ErrBuff, ErrBuffLen, 
				"Empty variable: `%s'", String);
		return((char *) NULL);
	    }

	    /* Set the Start and find the end of the variable */
	    Start = StrPtr;
	    for ( ; StrPtr && *StrPtr && *StrPtr != EndChar; ++StrPtr);
	    if (!StrPtr || !*StrPtr) {
		(void) snprintf(ErrBuff, ErrBuffLen,
			       "Variable `%s' has no ending `%c' character", 
			       Start, EndChar);
		return((char *) NULL);
	    }

	    /* Extract Variable name */
	    if (StrPtr - Start >= sizeof(Var)) {
		(void) snprintf(ErrBuff, sizeof(ErrBuff),  
		       "Variable name is too long `%s' (maximum is %d)",
			       Start, sizeof(Var));
		return((char *) NULL);
	    }
	    (void) strncpy(Var, Start, StrPtr - Start);
	    Var[StrPtr - Start] = CNULL;

	    /* Get the Variable's Value */
	    Value = (*GetVarFunc)(Var, VarParams);
	    if (!Value) {
		(void) snprintf(ErrBuff, ErrBuffLen, 
				"Invalid variable name: `%s'", Var);
		return((char *) NULL);
	    }

	    /* Append Value to Buf */
	    ValLen = strlen(Value);
	    BufLen = strlen(Buf);
	    if (ValLen + BufLen + 1 >= sizeof(Buf)) {
		(void) snprintf(ErrBuff, ErrBuffLen,
				"Variable buffer size too small.");
		return((char *) NULL);
	    }
	    (void) strncat(Buf, Value, ValLen);

	    /* Fix up pointers */
	    BufLen += ValLen;
	    BufPtr = &Buf[BufLen];
	    ++StrPtr;
	} else
	    *BufPtr++ = *StrPtr++;
	*BufPtr = CNULL;
    }

    if (!Buf[0])
	(void) snprintf(ErrBuff, ErrBuffLen, "Empty buffer");

    return( (Buf[0]) ? Buf : (char *) NULL );
}
