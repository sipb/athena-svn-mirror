/* This file is part of the CREF finder.  It contains global variable
 * definitions.
 *
 *	Win Treese
 *	MIT Project Athena
 *
 *	Copyright (c) 1985 by the Massachusetts Institute of Technology
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/globals.h,v $
 *	$Author: treese $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/globals.h,v 1.1 1986-01-18 18:29:37 treese Exp $
 */

extern int errno;				/* System error number. */

/* State Variables. */

extern char Current_Dir[];		/* Current CREF directory. */
extern int Current_Entry;		/* Current CREF entry. */
extern ENTRY Entry_Table[MAX_ENTRIES];	/* Table of CREF entries. */
extern int Entry_Count;			/* Number of entries. */
extern int Index_Start;			/* Current top of index. */
extern int Is_Consultant;		/* Is user a consultant? */
