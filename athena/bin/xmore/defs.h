/*	This is the file defs.h for the Xmore, a file browsing utility
 *      built upon Xlib and the XToolkit.
 *	It Contains: Many usefile definitions.
 *	
 *	Created: 	October 23 1987
 *	By:		Chris D. Peterson
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/xmore/defs.h,v $
 *      $Author: probe $
 *      $Header: /afs/dev.mit.edu/source/repository/athena/bin/xmore/defs.h,v 1.1 1990-07-17 11:12:31 probe Exp $
 *	
 *  	Copyright 1987, 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

#include "mit-copyright.h"

/*
 * This is the helpfile and is site specific, make sure you change this.
 */

#ifndef HELPFILE
#define HELPFILE "/usr/lib/X11/xmore.help" /* The default helpfile */
#endif

/* text page fonts. */
#ifdef ATHENA
#define NORMALFONT "fixed"
#define ITALICFONT "helvetica-bold12"
#define BOLDFONT   "helvetica-boldoblique12"
#else
#define NORMALFONT "8x13"
#define ITALICFONT "8x13bold"
#define BOLDFONT   "8x13bold"
#endif ATHENA

#define MAIN_CURSOR "left_ptr"	/* The default topcursor. */
#define HELP_CURSOR "left_ptr"	/* The default Help cursor. */
#define BACKSPACE 010		/* I doubt you would want to change this. */
#define INDENT 15

#define TYP20STR "MMMMMMMMMMMMMMMMMMMM"

/* The default size of the window. */

#define TOO_SMALL 100
#define DEFAULT_WIDTH 550
#define DEFAULT_HEIGHT 800

/* 
 * function defintions 
 */

/* Standard library function definitions. */

/* fonts.c */

void OpenFonts();
void AddCursor();

/* help.c */

Boolean CreateHelp();
void PopdownHelp(),PopupHelp();

/* main.c */

void main(),Quit(),TextExit(),PrintWarning(),PrintError();
Widget CreateScroll(),CreatePane();

/* pages.c */

void InitPage();
void PrintPage();

