/*
 *	Win Treese, Jeff Jimenez
 *	Student Consulting Staff
 *	MIT Project Athena
 *
 *	Copyright (c) 1985 by the Massachusetts Institute of Technology
 *
 *	Permission to use, copy, modify, and distribute this program
 *	for any purpose and without fee is hereby granted, provided
 *	that this copyright and permission notice appear on all copies
 *	and supporting documentation, the name of M.I.T. not be used
 *	in advertising or publicity pertaining to distribution of the
 *	program without specific prior permission, and notice be given
 *	in supporting documentation that copying and distribution is
 *	by permission of M.I.T.	 M.I.T. makes no representations about
 *	the suitability of this software for any purpose.  It is pro-
 *	vided "as is" without express or implied warranty.
 */

/* This file is part of the CREF finder.  It contains global variable
 * definitions for the curses (non-X) browser.
 *
 *	$Id: cur_globals.h,v 1.2 1999-01-22 23:13:34 ghudson Exp $
 */

#include <mit-copyright.h>

/* System Variables. */

#include <errno.h>

/* State Variables. */

extern char Current_Dir[];		/* Current CREF directory. */
extern char Root_Dir[];			/* CREF root directory. */
extern int Current_Ind;		/* Current CREF entry. */
extern int Previous_Ind;		/* Upper level CREF entry. */
extern ENTRY Entry_Table[];		/* Table of CREF entries. */
extern int Entry_Count;			/* Number of entries. */
extern int Ind_Start;			/* Current top of index. */
extern COMMAND Command_Table[];		/* CREF command table. */
extern int Command_Count;		/* Number of CREF commands. */
extern char Save_File[];		/* Default save file. */
extern char Abbrev_File[];		/* Abbreviation file. */
extern ABBREV Abbrev_Table[];		/* Abbreviation table. */
extern int Abbrev_Count;		/* Number of abbreviations. */
extern char Log_File[];			/* Administrative log file. */
extern char *Prog_Name;			/* Name this program was invoked as */

extern char *Header;			/* Display header for the browser */
extern char *Prompt;			/* Prompt string for the browser */
extern char *Subdir_Arg;		/* Argument to the -f flag (if any)*/
