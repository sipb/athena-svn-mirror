/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for the ZPeekIfNotice function.
 *
 *	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZPeekIfNot.c,v $
 *	$Author: rfrench $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
/* $Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZPeekIfNot.c,v 1.1 1987-06-12 16:59:12 rfrench Exp $ */

#include <zephyr/mit-copyright.h>

#include <zephyr/zephyr_internal.h>

Code_t ZPeekIfNotice(buffer,buffer_len,notice,auth,predicate,args)
	ZPacket_t	buffer;
	int		buffer_len;
	ZNotice_t	*notice;
	int		*auth;
	int		(*predicate)();
	char		*args;
{
	ZNotice_t tmpnotice;
	int qcount,retval,tmpauth;
	struct _Z_InputQ *qptr;

	if (__Q_Length)
		retval = Z_ReadEnqueue();
	else
		retval = Z_ReadWait();
	
	if (retval != ZERR_NONE)
		return (retval);
	
	qptr = __Q_Head;
	qcount = __Q_Length;

	for (;;qcount--) {
		if ((retval = ZParseNotice(qptr->packet,qptr->packet_len,
					   &tmpnotice,&tmpauth)) != ZERR_NONE)
			return (retval);
		if ((predicate)(&tmpnotice,args)) {
			if (qptr->packet_len > buffer_len)
				return (ZERR_PKTLEN);
			bcopy(qptr->packet,buffer,qptr->packet_len);
			if ((retval = ZParseNotice(buffer,qptr->packet_len,
						   notice,auth))
			    != ZERR_NONE)
				return (retval);
			return (ZERR_NONE);
		} 
		/* Grunch! */
		if (qcount == 1) {
			if ((retval = Z_ReadWait()) != ZERR_NONE)
				return (retval);
			qcount++;
			qptr = __Q_Tail;
		} 
		else
			qptr = qptr->next;
	}
}
