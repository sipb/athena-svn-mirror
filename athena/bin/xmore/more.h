/*	This is the file more.h for the Xmore, a file browsing utility
 *      built upon Xlib and the XToolkit.
 *	It Contains: Lots of good defintions.
 *	
 *	Created: 	October 22, 1987
 *	By:		Chris D. Peterson
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/xmore/more.h,v $
 *      $Author: vrt $
 *      $Header: /afs/dev.mit.edu/source/repository/athena/bin/xmore/more.h,v 1.4 1993-05-05 13:26:06 vrt Exp $
 *	
 *  	Copyright 1987, 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

/* Std system and C include files */

#include <stdio.h>
#include <X11/Xos.h>
#ifdef POSIX
#include <dirent.h>
#else
#include <sys/dir.h>
#endif
#include <sys/stat.h>

/* X include files */

#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/StringDefs.h>

/* X toolkit Include files */

#include <X11/Intrinsic.h>
#if XtSpecificationRelease < 4
#include <X11/AsciiText.h>
#include <X11/Command.h>
#include <X11/Form.h>
#include <X11/VPaned.h>
#else
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Paned.h>
#endif
#include <X11/Shell.h>

/* program specific include files */

#include "ScrollByLine.h"
#include "mit-copyright.h"
#include "defs.h"

typedef void (*fcall)();	/* function pointer typedef. */

typedef struct _Fonts {
  XFontStruct * normal,		/* The normal text for the man pages. */
    * italic;			/* The italic text for the man pages. */
} Fonts;

/* 
 * The structure with the information about the file
 */

typedef struct _MemoryStruct {
  char * top_of_page;		/* a pointer to the top of the page. */
  char ** top_line;		/* a pointer to the top of the list of
				   lines in the file. */
} MemoryStruct;
   
