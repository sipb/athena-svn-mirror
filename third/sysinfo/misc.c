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
 * Miscellaneous functions
 */

#include "defs.h"
#include <sys/stat.h>

/*
 * Front end to calloc()
 */
void *xcalloc(nele, esize)
    size_t 			nele;
    size_t 			esize;
{
    void 		       *p;

#if	defined(SYSINFO_ALLOC_DEBUG)
    SImsg(SIM_DBG, "calloc nele %d size %d", nele, esize);
#endif
    if ((p = (void *) calloc(nele, esize)) == NULL) {
	SImsg(SIM_GERR, "calloc(%d, %d) failed.", nele, esize);
	exit(1);
    }

    return(p);
}

void *xmalloc(size)
    size_t			size;
{
    void		       *newptr;

#if	defined(SYSINFO_ALLOC_DEBUG)
    SImsg(SIM_DBG, "malloc size %d", size);
#endif
    if (!(newptr = (void *) malloc(size))) {
	SImsg(SIM_GERR, "malloc size %d failed: %s", size, SYSERR);
	exit(1);
    }

    return(newptr);
}

void *xrealloc(ptr, size)
    void		       *ptr;
    size_t			size;
{
    void		       *newptr;

#if	defined(SYSINFO_ALLOC_DEBUG)
    SImsg(SIM_DBG, "realloc 0x%x size %d", ptr, size);
#endif
    if (!(newptr = (void *) realloc(ptr, size))) {
	SImsg(SIM_GERR, "realloc 0x%x size %d failed: %s", ptr, size, SYSERR);
	exit(1);
    }

    return(newptr);
}

/*
 * Convert integer to ASCII
 */
char *itoa(Num)
    u_long 			Num;
{
    static char 		Buf[64];

    (void) snprintf(Buf, sizeof(Buf),  "%d", Num);

    return(Buf);
}

/*
 * Convert integer to string version in hex
 */
char *itoax(Num)
    u_long 			Num;
{
    static char 		Buff[64];

    (void) snprintf(Buff, sizeof(Buff),  "0x%x", Num);

    return(Buff);
}

/*
 * Test to see if FileName exists.
 */
extern int FileExists(FileName)
     char		       *FileName;
{
    int				Exists = FALSE;
    struct stat			StatBuf;

    if (!FileName)
	return(0);

    if (stat(FileName, &StatBuf) == 0)
	Exists = TRUE;

    SImsg(SIM_DBG, "File <%s> %s", FileName, (Exists) ? "Exists" : SYSERR);

    return(Exists);
}

/*
 * Test to see if Path is a file
 */
extern int IsFile(PathName)
     char		       *PathName;
{
    int				RetVal = FALSE;
    struct stat			StatBuf;

    if (!PathName)
	return(0);

    if (stat(PathName, &StatBuf) == 0 && S_ISREG(StatBuf.st_mode))
	RetVal = TRUE;

    SImsg(SIM_DBG, "Path <%s> %s", PathName, (RetVal) ? "IsFile" : "NotFile");

    return(RetVal);
}

/*
 * Test to see if Path is a Directory
 */
extern int IsDir(PathName)
     char		       *PathName;
{
    int				RetVal = FALSE;
    struct stat			StatBuf;

    if (!PathName)
	return(0);

    if (stat(PathName, &StatBuf) == 0 && S_ISDIR(StatBuf.st_mode))
	RetVal = TRUE;

    SImsg(SIM_DBG, "Path <%s> %s", PathName, (RetVal) ? "IsDir" : "NotDir");

    return(RetVal);
}


/*
 * Clean "String" by removing leading and trailing whitespace.
 * Returns pointer to new (allocated) copy.
 */
extern char *CleanString(String, StrLen, Flags)
     char		       *String;
     int			StrLen;
     int			Flags;
{
    register char	       *cp;
    register char	       *bp;
    register char	       *end;
    register int		Len;
    register int		i;
    char		       *Buffer;
    char			LastChar = CNULL;

    if (!String || !StrLen)
	return((char *) NULL);

    /* Skip over any leading white space */
    for (cp = String; cp && *cp && isspace(*cp); ++cp);
    Len = StrLen - (cp - String);

    /* Copy to working Buffer */
    Buffer = (char *) xcalloc(1, Len + 1);
    for (bp = Buffer, i = 0; i < Len; ++i, ++cp) {
	/*
	 * Skip if this is SPACE and last char was SPACE or this char
	 * is not printable
	 */
	if ((LastChar && isspace(*cp) && isspace(LastChar)) || !isprint(*cp)) {
	    LastChar = *cp;
	    continue;
	}
	*bp++ = *cp;
	*bp = CNULL;
	LastChar = *cp;
    }
    
    /*
     * Remove all trailing spaces.
     * bp points to end of Buffer at this point.
     */
    end = bp;
    for (cp = end - 1; cp > Buffer && isspace(*cp); --cp);
    if (cp != end - 1 && cp && isspace(*++cp))
	*cp = CNULL;

    return(Buffer);
}
