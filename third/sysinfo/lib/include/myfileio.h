/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

/*
 * $Revision: 1.1.1.1 $
 */

/*
 * File I/O header
 */

#ifndef __myfileio_h__
#define __myfileio_h__ 

/*
 * Type used to read file I/O
 */
typedef struct {
    /* Public */
    int				FD;		/* File Descriptor */
    u_char			Delim;		/* Entry deliminator */
    /* Private */
    char		       *Buffer;		/* Storage buffer */
    char		       *Ptr;		/* Current ptr to Buffer */
    char		       *Start;		/* Start of current entry */
    ssize_t			BufSize;	/* Size of Buffer avail */
    ssize_t			BufUsage;	/* Usage of Buffer */
    ssize_t			Unread;		/* Amt left unread in Buffer */
    int				IntAlloc;	/* Created by FIOcreate() */
} FileIO_t;

/*
 * Declares
 */
extern FileIO_t		       *FIOcreate();	/* Create FIO */
extern int			FIOdestroy();	/* Destroy FIO */
extern char		       *FIOread();	/* Read entry */
   
#endif	/* __myfileio_h__ */
