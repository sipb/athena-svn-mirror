/*	This is the file globals.h for the Xmore, a file browsing utility
 *      built upon Xlib and the XToolkit.
 *	It Contains: The global function definitions.
 *	
 *	Created: 	October 22, 1987
 *	By:		Chris D. Peterson
 *
 *      $Id: globals.h,v 1.2 1999-01-22 23:15:43 ghudson Exp $
 *	
 *  	Copyright 1987, 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

#include "more.h"
#include "mit-copyright.h"

extern Fonts fonts;		/* The fonts used for the man pages. */

extern Cursor main_cursor;		/* The main cursor, for xman. */
extern Cursor help_cursor;		/* The help cursor, for xman. */

extern char * help_file_name;		/* The name of the helpfile. */

extern MemoryStruct * global_memory_struct; /* The tempory memory struct */

extern Widget help_widget;		/* The help widget. */
