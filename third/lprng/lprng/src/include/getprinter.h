/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1999, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: getprinter.h,v 1.1.1.2 1999-05-04 18:07:13 danw Exp $
 ***************************************************************************/




#ifndef _GETPRINTER_H_
#define _GETPRINTER_H_ 1

/* PROTOTYPES */

char *Get_printer(void);
char *Find_pc_entry( char *pr, struct line_list *alias,
	struct line_list *entry );
void Fix_Rm_Rp_info(void);
void Get_all_printcap_entries(void);

#endif
