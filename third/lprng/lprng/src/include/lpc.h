/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-2000, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: lpc.h,v 1.1.1.3 2000-03-31 15:48:08 mwhitson Exp $
 ***************************************************************************/



#ifndef _LPC_H_
#define _LPC_H_ 1

extern char LPC_optstr[]; /* number of status lines */
EXTERN char *Server;
EXTERN int Mail_fd;

/* PROTOTYPES */

void Get_parms( int argc, char **argv );

#endif
