/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1999, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: readstatus.h,v 1.1.1.2 1999-05-04 18:07:07 danw Exp $
 ***************************************************************************/



#ifndef _READSTATUS_H_
#define _READSTATUS_H_ 1



/* PROTOTYPES */

int Read_status_info( char *host, int sock,
    int output, int timeout, int displayformat,
    int longformat, int status_line_count, int lp_mode );


int Pr_status_check( char *name );
void Pr_status_clear( void );

#endif
