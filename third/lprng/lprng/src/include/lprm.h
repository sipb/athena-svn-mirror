/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1999, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: lprm.h,v 1.4 2001-03-07 01:20:17 ghudson Exp $
 ***************************************************************************/



#ifndef _LPRM_1_
#define _LPRM_1_


EXTERN char *Auth_JOB; /* Auth type to use, overriding printcap */
EXTERN int All_printers;    /* show all printers */
EXTERN int LP_mode;    /* show all printers */

/* PROTOTYPES */

void Get_parms( int argc, char **argv );

#endif
