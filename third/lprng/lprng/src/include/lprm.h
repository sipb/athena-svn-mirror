/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1999, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: lprm.h,v 1.1.1.1.2.1 1999-07-30 22:08:49 ghudson Exp $
 ***************************************************************************/



#ifndef _LPRM_1_
#define _LPRM_1_


EXTERN char *Auth_JOB; /* Auth type to use, overriding printcap */
EXTERN int All_printers;    /* show all printers */
EXTERN int LP_mode;    /* show all printers */

/* PROTOTYPES */

void Get_parms( int argc, char **argv );

#endif
