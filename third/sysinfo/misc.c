/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: misc.c,v 1.1.1.1 1996-10-07 20:16:53 ghudson Exp $";
#endif

/*
 * Miscellaneous functions
 */

#include "defs.h"

/*
 * Print error message
 */
#if	ARG_TYPE == ARG_STDARG
/*
 * StdArg version of Error()
 */
extern void Error(char *fmt, ...)
{
    va_list 			args;
    extern char 	       *ProgramName;

    (void) fflush(stdout);
    (void) fprintf(stderr, "%s: ", ProgramName);
    va_start(args, fmt);
    (void) vfprintf(stderr, fmt, args);
    va_end(args);
    (void) fprintf(stderr, "\n");
}
#endif	/* ARG_STDARG */
#if	ARG_TYPE == ARG_VARARGS
/*
 * Varargs version of Error()
 */
extern void Error(va_alist)
    va_dcl
{
    va_list 			args;
    char 		       *fmt;
    extern char 	       *ProgramName;

    (void) fflush(stdout);
    (void) fprintf(stderr, "%s: ", ProgramName);
    va_start(args);
    fmt = (char *) va_arg(args, char *);
    (void) vfprintf(stderr, fmt, args);
    va_end(args);
    (void) fprintf(stderr, "\n");
}
#endif	/* ARG_VARARGS */
#if	!defined(ARG_TYPE)
/*
 * Non-Varargs version of Error()
 */
extern void Error(fmt, a1, a2, a3, a4, a5, a6)
    char 		       *fmt;
{
    extern char 	       *ProgramName;

    (void) fflush(stdout);
    (void) fprintf(stderr, "%s: ", ProgramName);
    (void) fprintf(stderr, fmt, a1, a2, a3, a4, a5, a6);
    (void) fprintf(stderr, "\n");
}
#endif 	/* !ARG_TYPE */

/*
 * Front end to calloc()
 */
char *xcalloc(nele, esize)
    int 			nele;
    int 			esize;
{
    char 		       *p;

    if ((p = (char *) calloc(nele, esize)) == NULL) {
	Error("calloc(%d, %d) failed.", nele, esize);
	exit(1);
    }

    return(p);
}

char *xmalloc(size)
    int				size;
{
    char		       *newptr;

    if (!(newptr = (char *) malloc(size))) {
	Error("malloc size %d failed: %s", size, SYSERR);
	exit(1);
    }

    return(newptr);
}

char *xrealloc(ptr, size)
    char		       *ptr;
    int				size;
{
    char		       *newptr;

    if (!(newptr = (char *) realloc(ptr, size))) {
	Error("realloc 0x%x size %d failed: %s", ptr, size, SYSERR);
	exit(1);
    }

    return(newptr);
}

/*
 * Convert integer to ASCII
 */
char *itoa(Num)
    int 			Num;
{
    static char 		Buf[BUFSIZ];

    (void) sprintf(Buf, "%d", Num);

    return(Buf);
}

#if	RE_TYPE == RE_COMP
/*
 * Perform Regular Expression matching
 *
 * Should return 1 on match, 0 if no match, -1 on error.
 *
 */
int REMatch(String, Pattern)
    char		       *String;
    char		       *Pattern;
{
    char		       *ErrStr;
    int				Val;

    ErrStr = (char *) re_comp(Pattern);
    if (ErrStr) {
	if (Debug) Error("Regular Expression Error: Pattern `%s' - %s",
			 Pattern, ErrStr);
	return(-1);
    }

    Val = re_exec(String);

    return(Val);
}
#endif	/* RE_COMP */

#if	RE_TYPE == RE_REGCOMP
#include <regex.h>
/*
 * Perform Regular Expression matching
 *
 * Should return 1 on match, 0 if no match, -1 on error.
 *
 */
int REMatch(String, Pattern)
    char		       *String;
    char		       *Pattern;
{
    int				Val;
    regex_t			RegEx;
    static char			Buff[BUFSIZ];

    Val = regcomp(&RegEx, Pattern, REG_EXTENDED|REG_NOSUB);
    if (Val != 0) {
	(void) regerror(Val, &RegEx, Buff, sizeof(Buff));
	if (Debug) Error("Regular Expression Error: Pattern `%s' - %s",
			 Pattern, Buff);
	return(-1);
    }

    Val = regexec(&RegEx, String, (size_t) 0, NULL, 0);
    regfree(&RegEx);
    if (Val >= 1)
	return(1);

    return(0);
}
#endif	/* RE_REGCOMP */

#if	RE_TYPE == RE_REGCMP
/*
 * Perform Regular Expression matching
 *
 * Should return 1 on match, 0 if no match, -1 on error.
 *
 */
int REMatch(String, Pattern)
    char		       *String;
    char		       *Pattern;
{
    char		       *CompExp = NULL;
    int				Val;

    CompExp = (char *) regcmp(Pattern, (char *)0);
    if (!CompExp) {
	if (Debug) Error("Regular Expression Error: Pattern `%s' - %s",
			 Pattern, SYSERR);
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
