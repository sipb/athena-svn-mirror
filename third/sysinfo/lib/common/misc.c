/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Miscellaneous functions
 */

#include "defs.h"
#if	defined(HAVE_STRING_H)
#include <string.h>
#endif
#if	defined(HAVE_STRINGS_H)
#include <strings.h>
#endif
#if	defined(HAVE_STDLIB_H)
#include <stdlib.h>
#endif
#include <time.h>
#include <sys/stat.h>

/*
 * Add a description to a Desc_t list
 */
extern int AddDesc(DescList, Label, Desc, Action)
    Desc_t		      **DescList;
    char		       *Label;
    char		       *Desc;
    int				Action;
{
    register Desc_t	       *Ptr;
    Desc_t		       *New;

    if (!DescList || !Desc)
	return -1;

    /*
     * Check for duplicates
     */
    for (Ptr = *DescList; Ptr; Ptr = Ptr->Next)
	if (EQ(Ptr->Desc, Desc) && EQ(Ptr->Label, Label))
	    return -1;

    /*
     * Create new dev description
     */
    New = (Desc_t *) xcalloc(1, sizeof(Desc_t));
    New->Desc = strdup(Desc);
    if (Label)
	New->Label = strdup(Label);
    if (Action & DA_PRIME)
	New->Flags |= DA_PRIME;

    /*
     * Add to list
     */
    if (*DescList) {
	if (Action & DA_INSERT) {
	    New->Next = *DescList;
	    *DescList = New;
	} else {
	    for (Ptr = *DescList; Ptr && Ptr->Next; 
		 Ptr = Ptr->Next);
	    Ptr->Next = New;
	}
    } else
	*DescList = New;

    return 0;
}

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
 * Convert Large_t to ASCII
 */
char *Ltoa(Num)
    Large_t 			Num;
{
    static char 		Buf[128];

#if	defined(HAVE_PRINTF_LL)
    (void) snprintf(Buf, sizeof(Buf),  "%lld", Num);
#else
    (void) snprintf(Buf, sizeof(Buf),  "%ld", Num);
#endif	/* HAVE_PRINTF_LL */

    return(Buf);
}

/*
 * Convert ASCII to Large_t
 */
Large_t atoL(String)
     char		       *String;
{
#if	defined(HAVE_STRTOLL)
    /* Only call strtoll() if Large_t is truely equal to long long */
    if (sizeof(Large_t) == sizeof(long long))
	return (Large_t) strtoll(String, NULL, 0);
    else
	return (Large_t) strtol(String, NULL, 0);
#else	/* !HAVE_STRTOLL */
    return (Large_t) strtol(String, NULL, 0);
#endif	/* HAVE_STRTOLL */
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

    SImsg(SIM_DBG, "File <%s> %s", FileName, 
	  (Exists) ? "Exists" : (char *) SYSERR);

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
 *
 * Flags:
 * 	CSF_BSWAP	Change "BADC" to "ABCD".
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
    char			c;
    int				Swap = TRUE;

    if (!String || !StrLen)
	return((char *) NULL);

    /* Skip over any leading white space */
    for (cp = String; cp && *cp && isspace(*cp); ++cp);
    Len = StrLen - (cp - String);

    /* Copy to working Buffer */
    Buffer = (char *) xcalloc(1, Len + 1);
    for (bp = Buffer, i = 0; i < Len; ++i, ++cp) {
	if (FLAGS_ON(Flags, CSF_BSWAP)) {
	    /*
	     * Byte Swap every other pass through.
	     */
	    if (Swap) {
		c = *cp;
		*cp = *(cp+1);
		*(cp+1) = c;
		Swap = FALSE;
	    } else 
		Swap = TRUE;
	}

	if (FLAGS_ON(Flags, CSF_ENDNONPR)) {
	    /*
	     * If this is a NonPrintable char, end
	     */
	    if (!isprint(*cp)) {
		break;
	    }
	} else if (FLAGS_ON(Flags, CSF_ALPHANUM)) {
	    /*
	     * If this is a not an Alpha Numeric, end
	     */
	    if (!isalnum(*cp)) {
		break;
	    }
	} else {
	    /*
	     * Skip if this is SPACE and last char was SPACE or this char
	     * is not printable
	     */
	    if ((LastChar && isspace(*cp) && isspace(LastChar)) || 
		!isprint(*cp)) {
		LastChar = *cp;
		continue;
	    }
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

    if (strlen(Buffer) == 0)
	return (char *) NULL;

    return(Buffer);
}

/*
 * Front end to llseek() since not all platforms have one.
 */
extern Offset_t myllseek(fd, Offset, Flags)
     int			fd;
     Offset_t			Offset;
     int			Flags;
{
#if	defined(HAVE_LSEEK64)
    return lseek64(fd, Offset, Flags);
#elif	defined(HAVE_LLSEEK)
    return llseek(fd, Offset, Flags);
#elif	defined(OS_LLSEEK)
    return OS_LLSEEK(fd, Offset, Flags);
#else
    return -1;
#endif
}

/*
 * Return the directory name portion of Path
 */
extern char *DirName(Path)
     char		       *Path;
{
    static char			Buff[MAXPATHLEN];
    char		       *cp;

    if (!Path)
	return (char *) NULL;

    (void) strncpy(Buff, Path, sizeof(Buff));
    if (cp = strrchr(Buff, '/'))
	*cp = CNULL;

    return Buff;
}

/*
 * Return the base (file) name portion of Path
 */
extern char *BaseName(Path)
     char		       *Path;
{
    char		       *cp;

    if (!Path)
	return (char *) NULL;

    if (cp = strrchr(Path, '/'))
	return cp + 1;

    return Path;
}

/*
 * Get current working directory
 */
extern char *GetCwd(Buff, BuffSize)
     char		       *Buff;
     size_t			BuffSize;
{
    return getcwd(Buff, BuffSize);
}

/*
 * Convert TimeVal into a time string.
 * Fmt is a format string suitable for use by strftime(3).
 */
extern char *TimeStr(TimeVal, Fmt)
     time_t		        TimeVal;
     char		       *Fmt;
{
    struct tm		       *Tm;
    static char			Buff[128];

    if (!Fmt)
	return (char *) NULL;

    if (Tm = localtime(&TimeVal))
	if (strftime(Buff, sizeof(Buff), Fmt, Tm))
	    return Buff;

    return (char *) NULL;
}
