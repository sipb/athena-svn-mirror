/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-2000, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: getprinter.h,v 1.1.1.4 2000-03-31 15:48:09 mwhitson Exp $
 ***************************************************************************/




#ifndef _GETPRINTER_H_
#define _GETPRINTER_H_ 1

/* PROTOTYPES */
char *Get_printer(void);
void Fix_Rm_Rp_info(void);
void Get_all_printcap_entries(void);

#endif
