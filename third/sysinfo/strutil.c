/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: strutil.c,v 1.1.1.1 1996-10-07 20:16:49 ghudson Exp $";
#endif

/*
 * String utility functions
 */
#include "defs.h"

/*
 * Is What in List?
 */
int IsSep(What, List)
    int				What;
    char		       *List;
{
    register char	       *cp;

    for (cp = List; cp < &List[strlen(List)]; ++cp)
	if (What == *cp)
	    return(1);

    return(0);
}

/*
 * Find the first seperator from List in String and
 * return a pointer.
 */
char *FindSep(String, List)
    char		       *String;
    char		       *List;
{
    register char	       *cp;

    for (cp = String; cp < &String[strlen(String)]; ++cp)
	if (IsSep(*cp, List))
	    return(cp);

    return((char *)NULL);
}

/*
 * Parse a string into an argument vector.
 */
extern int
StrToArgv(List, SepChars, ArgvPtr)
    char		       *List;
    char		       *SepChars;
    char		     ***ArgvPtr;
{
    char		      **Argv = NULL;
    char		       *String;
    char		       *Ptr;
    char		       *Start;
    char		       *Next;
    char		       *End;
    register char	       *cp;
    register int		Count;
    register int		i;

    String = List;
    for (Count = 0, Ptr = String; Ptr && *Ptr; ++Ptr)
	if (IsSep(*Ptr, SepChars))
	    ++Count;

    /*
     * There's only one argument with no seperator?
     */
    if (Count == 0 && String && *String)
	++Count;
    else if (!Count)
	return(0);
    else
	Count++;

    Argv = (char **) xcalloc(Count+1, sizeof(char *));

    /* 
     * XXX We don't use strtok() because it doesn't like back-to-back
     * seperators.  e.g. "foo||bar" will skip the NULL argument.
     */
    for (i = 0, Start = NULL, Ptr = String; i < Count; ++i) {
	Start = Ptr;
	if (!isspace(*SepChars))
	    /* Skip leading white space */
	    for ( ; Start && *Start && isspace(*Start); ++Start);
	/* Find end of word */
	for (End = Start; End && *End; ++End)
	    if (IsSep(*End, SepChars))
		break;
	/* Mark next field */
	Next = End;
	if (Next)
	    ++Next;
	/* Remove trailing white space */
	if (!isspace(*SepChars) && isspace(*(End-1))) {
	    while (--End > Start && *End && isspace(*End));
	    ++End;
	}
	if (End > Start) {
	    Argv[i] = xmalloc( (End - Start) + 1 );
	    (void) strncpy(Argv[i], Start, End - Start);
	    Argv[i][End - Start] = CNULL;
	}
	Ptr = Next;
    }

    *ArgvPtr = Argv;

    return(Count);
}

/*
 * Destroy an argument vector such as was created by StrToArgv().
 */
extern void
DestroyArgv(ArgvPtr, Argc)
    char		     ***ArgvPtr;
    int				Argc;
{
    register int		i;
    char		      **Argv;

    if (Argc <= 0 || !ArgvPtr)
	return;

    Argv = *ArgvPtr;
    for (i = 0; i < Argc; ++i)
	if (Argv[i])
	    (void) free(Argv[i]);

    (void) free(*ArgvPtr);
    *ArgvPtr = NULL;
}

/*
 * Make String all lower case.
 */
extern void strtolower(String)
    char		       *String;
{
    register char	       *cp;

    for (cp = String; cp && *cp; ++cp)
	if (isupper(*cp))
	    *cp = tolower(*cp);
}

/*
 * Return an all lower case version of String
 */
extern char *strlower(String)
    char		       *String;
{
    static char		       *Buffer = NULL;
    register char	       *cp;

    if (Buffer)
	free(Buffer);

    Buffer = strdup(String);

    for (cp = Buffer; cp && *cp; ++cp)
	if (isupper(*cp))
	    *cp = tolower(*cp);

    return(Buffer);
}

/*
 * Return an all upper case version of String
 */
extern char *strupper(String)
    char		       *String;
{
    static char		       *Buffer = NULL;
    register char	       *cp;

    if (Buffer)
	free(Buffer);

    Buffer = strdup(String);

    for (cp = Buffer; cp && *cp; ++cp)
	if (islower(*cp))
	    *cp = toupper(*cp);

    return(Buffer);
}
