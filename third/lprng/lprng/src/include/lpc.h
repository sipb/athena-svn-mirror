/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1999, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: lpc.h,v 1.1.1.1 1999-05-04 18:07:08 danw Exp $
 ***************************************************************************/



#ifndef _LPC_H_
#define _LPC_H_ 1

extern char LPC_optstr[]; /* number of status lines */
EXTERN char *Server;

/* PROTOTYPES */

void Get_parms( int argc, char **argv );

#endif
