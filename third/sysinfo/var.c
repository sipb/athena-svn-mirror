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
    register char	       *Value;
    register char	       *cp;
    register char	       *cp2;
    extern char		       *CurrentConfDir;

    if (EQ(Variable, "OSname")) {
	if (Value = GetOSName()) {
	    /*
	     * Copy Value into OSnameBuf while skipping '/' and
	     * lower casing
	     */
	    for (cp = Value, cp2 = OSnameBuf; 
		 cp && *cp && cp2 < &OSnameBuf[sizeof(OSnameBuf)]; ++cp)
		if (*cp != '/')
		    *cp2++ = tolower(*cp);
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
    } else if (EQ(Variable, "KArch")) {
	if (Value = GetKernArch()) {
	    (void) strcpy(KArchBuf, Value);
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
