#ifndef lint
  static char rcsid_module_c[] = "$Id: globals.c,v 1.2 1999-01-22 23:15:42 ghudson Exp $";
#endif lint

/*	This is the file globals.c for the Xmore, a file browsing utility
 *      built upon Xlib and the XToolkit.
 *	It Contains: The global function definitions.
 *	
 *	Created: 	October 22, 1987
 *	By:		Chris D. Peterson
 *
 *      $Id: globals.c,v 1.2 1999-01-22 23:15:42 ghudson Exp $
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
