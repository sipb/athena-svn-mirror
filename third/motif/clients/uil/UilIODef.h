/* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993, 1994 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.4
*/ 
/*   $RCSfile: UilIODef.h,v $ $Revision: 1.1.1.1 $ $Date: 1999-01-30 03:16:37 $ */

/*
*  (c) Copyright 1989, 1990, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */

/*
**++
**  FACILITY:
**
**      User Interface Language Compiler (UIL)
**
**  ABSTRACT:
**
**      This include file defines the interface to the operating system
**	io facilities.  
**
**--
**/

#ifndef UilIODef_h
#define UilIODef_h


/*
**  Define a UIL File Control Block or FCB.
*/

#include <stdio.h>
#ifndef NULL
#define NULL 0L
#endif

typedef struct  
{
    FILE	*az_file_ptr;
    char	*c_buffer;
    boolean	v_position_before_get;
    z_key	last_key;
    char	expanded_name[ 256 ];
} uil_fcb_type;

#endif /* UilIODef_h */
/* DON'T ADD STUFF AFTER THIS #endif */
