/*
 * Copyright (c) 1992-1998 by Michael A. Cooper.
 * All rights reserved.
 *
 * This software is subject to the terms found at 
 *	http://www.MagniComp.com/sysinfo/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

#include <stdio.h>
#include "mcregex.h"

/*
 * MagniComp Regular Expression functions
 *
 * REmatch() Should return 1 on match, 0 if no match, -1 on error.
 *
 * RE_TYPE should be defined by some .h file included in "mconfig.h"
 */
static char			Buff[BUFSIZ];

/*
 * Set error Message to MsgLoc
 */
static void SetError(Message, MsgLoc)
     char		       *Message;
     char		      **MsgLoc;
{
    if (!MsgLoc || !Message)
	return;

    *MsgLoc = Message;
}

#if	RE_TYPE == RE_REGCOMP

#include <regex.h>

#if	!defined(REG_OK)
#define REG_OK		0
#endif	/* REG_OK */

/*
 * POSIX regex
 */
int REmatch(String, Pattern, ErrMsgPtr)
    char		       *String;
    char		       *Pattern;
    char		      **ErrMsgPtr;
{
    int				Val;
    regex_t			RegEx;

    Val = regcomp(&RegEx, Pattern, REG_EXTENDED|REG_NOSUB);
    if (Val != 0) {
	(void) regerror(Val, &RegEx, Buff, sizeof(Buff));
	(void) snprintf(Buff, sizeof(Buff),  
			"Regular Expression Error: Pattern `%s' - %s",
			Pattern, Buff);
	SetError(Buff, ErrMsgPtr);
	return(-1);
    }

    Val = regexec(&RegEx, String, (size_t) 0, NULL, 0);
    regfree(&RegEx);
    if (Val == REG_OK)
      	/* Match! */
	return(1);

    return(0);
}
#endif	/* RE_REGCOMP */

#if	RE_TYPE == RE_COMP
/*
 * BSD regex
 */
int REmatch(String, Pattern, ErrMsgPtr)
    char		       *String;
    char		       *Pattern;
    char		      **ErrMsgPtr;
{
    char		       *ErrStr;
    int				Val;

    ErrStr = (char *) re_comp(Pattern);
    if (ErrStr) {
	(void) snprintf(Buff, sizeof(Buff),  
			"Regular Expression Error: Pattern `%s' - %s",
			Pattern, ErrStr);
	SetError(Buff, ErrMsgPtr);
	return(-1);
    }

    Val = re_exec(String);

    return(Val);
}
#endif	/* RE_COMP */

#if	RE_TYPE == RE_REGCMP
/*
 * System V regex
 */
int REmatch(String, Pattern, ErrMsgPtr)
    char		       *String;
    char		       *Pattern;
    char		      **ErrMsgPtr;
{
    char		       *CompExp = NULL;
    int				Val;

    CompExp = (char *) regcmp(Pattern, (char *)0);
    if (!CompExp) {
	(void) snprintf(Buff, sizeof(Buff),  
			"Regular Expression Error: Pattern `%s' - %s",
			Pattern, SYSERR);
	SetError(Buff, ErrMsgPtr);
	return(-1);
    }

    if (regex(CompExp, String))
	Val = 1;
    else
	Val = 0;

    (void) free(CompExp);

    return(Val);
}
#endif	/* RE_COMP */
