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
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/globals.h,v 1.4 1986-01-29 14:48:51 treese Exp $
 */

/* System Variables. */

extern int errno;				/* System error number. */

/* State Variables. */

extern char Current_Dir[];		/* Current CREF directory. */
extern char Root_Dir[];			/* CREF root directory. */
extern int Current_Index;		/* Current CREF entry. */
extern int Previous_Index;		/* Upper level CREF entry. */
extern ENTRY Entry_Table[];		/* Table of CREF entries. */
extern int Entry_Count;			/* Number of entries. */
extern int Index_Start;			/* Current top of index. */
extern int Is_Consultant;		/* Is user a consultant? */
extern COMMAND Command_Table[];		/* CREF command table. */
extern int Command_Count;		/* Number of CREF commands. */
extern char Save_File[];		/* Default save file. */
extern char Abbrev_File[];		/* Abbreviation file. */
extern ABBREV Abbrev_Table[];		/* Abbreviation table. */
extern int Abbrev_Count;		/* Number of abbreviations. */
