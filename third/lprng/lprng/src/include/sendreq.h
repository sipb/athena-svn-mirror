/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-2000, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: sendreq.h,v 1.1.1.3 2000-03-31 15:48:06 mwhitson Exp $
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
