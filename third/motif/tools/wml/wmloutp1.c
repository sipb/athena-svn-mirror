/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: wmloutp1.c,v $ $Revision: 1.1.1.1 $ $Date: 1999-01-30 03:17:15 $"
#endif
#endif
/*
*  (c) Copyright 1989, 1990, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */

/*
 * This is the standard output module for creating the UIL compiler
 * .h files.
 */


#include "wml.h"


void wmlOutput ()

{

/*
 * Output the .h files
 */
wmlOutputHFiles ();
if ( wml_err_count > 0 ) return;

/*
 * Output the keyword (token) tables
 */
wmlOutputKeyWordFiles ();
if ( wml_err_count > 0 ) return;

/*
 * Output the .mm files
 */
wmlOutputMmFiles ();
if ( wml_err_count > 0 ) return;

return;

}

