/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * File I/O functions
 */

#include "defs.h"
#include "myfileio.h"

/*
 * Create a new IO
 */
extern FileIO_t *FIOcreate()
{
    FileIO_t		       *IO;

    IO = (FileIO_t *) xcalloc(1, sizeof(FileIO_t));

    IO->IntAlloc = 1;	/* We created this entry */

    return IO;
}

/*
 * Destroy this IO entry
 */
extern int FIOdestroy(IO)
     FileIO_t		       *IO;
{
    if (IO->Buffer) {
	(void) free(IO->Buffer);
	IO->Buffer = NULL;
    }

    /* If we created this, then we should destroy it */
    if (IO->IntAlloc)
	(void) free(IO);

    return 0;
}

/*
 * Read input from IO->FD looking for a deliminator (IO->Delim)
 * Return the string found up until IO->Delim.  On next call we
 * return the next string and so on.
 */
extern char *FIOread(IO)
     FileIO_t		       *IO;
{
    ssize_t			Amt;
    ssize_t			PtrOffSet;
    ssize_t			StartOffSet;
    int				Skip;
    char		       *cp;

    if (!IO)
	return (char *) NULL;

#define BUF_SIZE_START		64 * KBYTES
#define BUF_SIZE_CHUNK		(BUF_SIZE_START / 2)
#define READ_AMT		8 * KBYTES

    if (!IO->Buffer) {
	/* First time init */
	IO->BufSize = BUF_SIZE_START;
	IO->BufUsage = 0;
	IO->Ptr = IO->Buffer = (char *) xcalloc(1, IO->BufSize);
	if (!IO->Delim)
	    IO->Delim = '\n';
    }
    IO->Start = IO->Ptr;

    while (1) {
	if (IO->Unread <= 0) {
	    /* Fill up IO->Buffer */

	    if (IO->BufUsage + READ_AMT >= IO->BufSize - 1) {
	        /* Save current OffSets */
		PtrOffSet = IO->Ptr - IO->Buffer;
		StartOffSet = IO->Start - IO->Buffer;
		/* Increase IO->Buffer */
		IO->BufSize += BUF_SIZE_CHUNK;
		IO->Buffer = (char *) xrealloc(IO->Buffer, IO->BufSize);
		/* 
		 * Need to reset IO->Ptr and IO->Start because 
		 * IO->Buffer may be at a new location now
		 */
		IO->Ptr = IO->Buffer + PtrOffSet;
		IO->Start = IO->Buffer + StartOffSet;
	    }

	    Amt = read(IO->FD, IO->Ptr, READ_AMT);
	    if (Amt == 0)	/* EOF */
		break;
	    else if (Amt < 0) {
		SImsg(SIM_GERR, "FIOread: read failed: %s", SYSERR);
		break;
	    }
	    IO->BufUsage += Amt;
	    IO->Unread += Amt;
	}
	/*
	 * Look for the End Of Entry Deliminator char
	 */
	if (cp = (char *) memchr((void *) IO->Ptr, IO->Delim, IO->Unread)) {
	    /* Skip over newline if it's next and it's NOT the EOE */
	    if (IO->Delim != '\n' && *(cp+1) == '\n')
		Skip = 2;		/* Skip EOE + newline */
	    else
		Skip = 1;		/* Skip EOE */
	    *cp = CNULL;		/* Terminate IO->Start string */
	    IO->Unread -= cp - IO->Ptr + Skip;	/* Decrement EOE [+ newline] */
	    IO->Ptr = cp + Skip;		
	    return IO->Start;
	}
	IO->Ptr += IO->Unread;
	IO->Unread = 0;
    }

    SImsg(SIM_DBG, "FIOread: BufSize = %d", IO->BufSize);

    return (char *) NULL;
}
