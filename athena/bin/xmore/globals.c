#ifndef lint
  static char rcsid_module_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/xmore/globals.c,v 1.1 1993-10-12 05:34:41 probe Exp $";
#endif lint

/*	This is the file globals.c for the Xmore, a file browsing utility
 *      built upon Xlib and the XToolkit.
 *	It Contains: The global function definitions.
 *	
 *	Created: 	October 22, 1987
 *	By:		Chris D. Peterson
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/xmore/globals.c,v $
 *      $Author: probe $
 *      $Header: /afs/dev.mit.edu/source/repository/athena/bin/xmore/globals.c,v 1.1 1993-10-12 05:34:41 probe Exp $
 *	
 *  	Copyright 1987, 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

#include "more.h"
#include "mit-copyright.h"

Fonts fonts;			/* The fonts used for the man pages. */

Cursor main_cursor;		/* The main cursor, for xman. */
Cursor help_cursor;		/* The help cursor, for xman. */

char * help_file_name;		/* The name of the helpfile. */

MemoryStruct * global_memory_struct; /* The tempory memory struct */

Widget help_widget;		/* The help widget. */
