/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: var.c,v 1.1.1.1 1996-10-07 20:16:49 ghudson Exp $";
#endif

/*
 * Variable functions
 */

#include "defs.h"

/*
 * Substitute variables in String and return result.
 */
extern char *VarSub(String, ErrBuff, GetVarFunc, VarParams)
    char		       *String;
    char		       *ErrBuff;
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
		(void) sprintf(ErrBuff, 
			       "Variables must start with `(' or `{': `%s'", 
			       String);
		return((char *) NULL);
	    }
	    ++StrPtr;
	    if (!StrPtr || !*StrPtr) {
		(void) sprintf(ErrBuff, "Empty variable: `%s'", String);
		return((char *) NULL);
	    }

	    /* Set the Start and find the end of the variable */
	    Start = StrPtr;
	    for ( ; StrPtr && *StrPtr && *StrPtr != EndChar; ++StrPtr);
	    if (!StrPtr || !*StrPtr) {
		(void) sprintf(ErrBuff, 
			       "Variable `%s' has no ending `%c' character", 
			       Start, EndChar);
		return((char *) NULL);
	    }

	    /* Extract Variable name */
	    if (StrPtr - Start >= sizeof(Var)) {
		(void) sprintf(ErrBuff, 
		       "Variable name is too long `%s' (maximum is %d)",
			       Start, sizeof(Var));
		return((char *) NULL);
	    }
	    (void) strncpy(Var, Start, StrPtr - Start);
	    Var[StrPtr - Start] = CNULL;

	    /* Get the Variable's Value */
	    Value = (*GetVarFunc)(Var, VarParams);
	    if (!Value) {
		(void) sprintf(ErrBuff, "Invalid variable name: `%s'", Var);
		return((char *) NULL);
	    }

	    /* Append Value to Buf */
	    ValLen = strlen(Value);
	    BufLen = strlen(Buf);
	    if (ValLen + BufLen + 1 >= sizeof(Buf)) {
		(void) sprintf(ErrBuff, "Variable buffer size too small.");
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
	(void) sprintf(ErrBuff, "Empty buffer");

    return( (Buf[0]) ? Buf : (char *) NULL );
}
