/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1999, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: accounting.h,v 1.1.1.3.2.1 2001-03-07 01:42:23 ghudson Exp $
 ***************************************************************************/



#ifndef _ACCOUNTING_H_
#define _ACCOUNTING_H_ 1

/* PROTOTYPES */

int Setup_accounting( struct job *job );
int Do_accounting( int end, char *command, struct job *job,
	int timeout );

#endif
