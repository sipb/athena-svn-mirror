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
 * String utility functions
 */
#include "defs.h"
#include "strutil.h"

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
 * Is What a quote character?  If so, return the ending quote char
 * which is the next char in QuoteChars.  QuoteChars may contain
 * multiple pairs of quote characters. e.g ""<>''`'
 */
int IsQuote(What, QuoteChars)
     int			What;
     char		       *QuoteChars;
{
    int				QuotePair;
    static int			NumQuotes;
    static char		       *LastQuoteChars;

    if (!QuoteChars)
	return(0);

    if (LastQuoteChars != QuoteChars || !NumQuotes)
	NumQuotes = strlen(QuoteChars);
    LastQuoteChars = QuoteChars;

    for (QuotePair = 0; QuotePair < NumQuotes; QuotePair += 2) {
	if (What == QuoteChars[QuotePair])
	    return(QuoteChars[QuotePair+1]);
    }

    return(0);
}

/*
 * Parse a string into an argument vector.
 * List is the string to be parsed.
 * SepChars is the list of seperator characters which will be used to
 *		break apart List.
 * ArgvPtr is where the argument results will be stored.
 * QuoteChars is the list of characters which indicate single arguments.
 * 		This is a list of pairs.  The first char indicated begining
 *		of quoted argument, the next char indicates end. e.g ""<>''`'
 */
extern int
StrToArgv(List, SepChars, ArgvPtr, QuoteChars, Flags)
    char		       *List;
    char		       *SepChars;
    char		     ***ArgvPtr;
    char		       *QuoteChars;
    int				Flags;
{
    char		      **Argv = NULL;
    char		       *String;
    char		       *Ptr;
    char		       *Start;
    char		       *Next;
    char		       *End;
    int				EndQuote;
    register char	       *cp;
    register int		Count;
    register int		i;

    if (!List)
	return(0);

    String = List;
    for (Count = 0, Ptr = String; Ptr && *Ptr; ++Ptr) {
	if (EndQuote = IsQuote(*Ptr, QuoteChars))
	    while (*(++Ptr) != EndQuote);
	if (IsSep(*Ptr, SepChars))
	    ++Count;
	if (FLAGS_ON(Flags, STRTO_SKIPRP)) {
	    /* Skip over rest of SepChars for now */
	    while (Ptr && *Ptr && IsSep(*Ptr, SepChars))
		++Ptr;
	}
    }

    /*
     * Watch for ending seperator
     */
    if (IsSep(*(Ptr-1), SepChars))
	--Count;

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
     * We don't use strtok() because it doesn't like back-to-back
     * seperators.  e.g. "foo||bar" will skip the NULL argument.
     * strtok() also doesn't do QuoteChars like we do.
     */
    for (i = 0, Start = NULL, Ptr = String; i < Count; ++i) {
	Start = Ptr;
	if (!isspace(*SepChars))
	    /* Skip leading white space */
	    for ( ; Start && *Start && isspace(*Start); ++Start);
	if (EndQuote = IsQuote(*Start, QuoteChars))
	    ++Start;
	/* Find end of word */
	for (End = Start; End && *End; ++End) {
	    if (EndQuote) {
		if (*End == EndQuote)
		    break;
	    } else if (IsSep(*End, SepChars))
		break;
	}
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
	    Argv[i] = (char *) xmalloc( (End - Start) + 1 );
	    (void) strncpy(Argv[i], Start, End - Start);
	    Argv[i][End - Start] = CNULL;
	}
	Ptr = Next;
	if (!Next)
	    break;
	if (FLAGS_ON(Flags, STRTO_SKIPRP)) {
	    /* Skip over rest of SepChars for now */
	    while (Ptr && *Ptr && IsSep(*Ptr, SepChars))
		++Ptr;
	}
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
