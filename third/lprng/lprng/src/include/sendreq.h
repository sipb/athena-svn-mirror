/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1999, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: sendreq.h,v 1.1.1.3.2.1 2001-03-07 01:42:55 ghudson Exp $
 ***************************************************************************/



#ifndef _SENDREQ_H_
#define _SENDREQ_H_ 1

/* PROTOTYPES */
int Send_request(
	int class,					/* 'Q'= LPQ, 'C'= LPC, M = lprm */
	int format,					/* X for option */
	char **options,				/* options to send */
	int connect_tmout,		/* timeout on connection */
	int transfer_timeout,		/* timeout on transfer */
	int output					/* output on this FD */
	);

#endif
