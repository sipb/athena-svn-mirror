/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for the ZRetrieveSubscriptions and
 * ZRetrieveDefaultSubscriptions functions.
 *
 *	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZRetSubs.c,v $
 *	$Author: jtkohl $
 *
 *	Copyright (c) 1987,1988 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
/* $Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZRetSubs.c,v 1.17 1988-07-08 10:47:09 jtkohl Exp $ */

#ifndef lint
static char rcsid_ZRetrieveSubscriptions_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZRetSubs.c,v 1.17 1988-07-08 10:47:09 jtkohl Exp $";
#endif lint

#include <zephyr/mit-copyright.h>

#include <zephyr/zephyr_internal.h>

Code_t ZRetrieveSubscriptions(port,nsubs)
	u_short port;
	int *nsubs;
{
	int retval;
	ZNotice_t notice;
	char asciiport[50];
	
	if ((retval = ZMakeAscii(asciiport,sizeof(asciiport),
				 (unsigned char *)&port,
				 sizeof(u_short))) != ZERR_NONE)
		return (retval);

	(void) bzero((char *)&notice, sizeof(notice));
	notice.z_message = asciiport;
	notice.z_message_len = strlen(asciiport)+1;
	notice.z_opcode = CLIENT_GIMMESUBS;

	return(Z_RetSubs(&notice, nsubs, ZAUTH));
}

Code_t ZRetrieveDefaultSubscriptions(nsubs)
	int *nsubs;
{
	ZNotice_t notice;

	(void) bzero((char *)&notice, sizeof(notice));
	notice.z_message = (char *) 0;
	notice.z_message_len = 0;
	notice.z_opcode = CLIENT_GIMMEDEFS;

	return(Z_RetSubs(&notice, nsubs, ZNOAUTH));

}

static Code_t Z_RetSubs(notice, nsubs, auth_routine)
	register ZNotice_t *notice;
	int *nsubs;
	int (*auth_routine)();
{
	int retval,nrecv,gimmeack;
	register int i;
	ZNotice_t retnotice;
	char *ptr,*end,*ptr2;
	fd_set read, setup;
	int nfds;
	struct timeval tv;
	int gotone;

	retval = ZFlushSubscriptions();

	if (retval != ZERR_NONE && retval != ZERR_NOSUBSCRIPTIONS)
		return (retval);
	
	if (ZGetFD() < 0)
		if ((retval = ZOpenPort((u_short *)0)) != ZERR_NONE)
			return (retval);

	notice->z_kind = ACKED;
	notice->z_port = __Zephyr_port;
	notice->z_class = ZEPHYR_CTL_CLASS;
	notice->z_class_inst = ZEPHYR_CTL_CLIENT;
	notice->z_sender = 0;
	notice->z_recipient = "";
	notice->z_default_format = "";

	if ((retval = ZSendNotice(notice,auth_routine)) != ZERR_NONE)
		return (retval);

	nrecv = 0;
	gimmeack = 0;
	__subscriptions_list = (ZSubscription_t *) 0;

	FD_ZERO(&setup);
	FD_SET(ZGetFD(), &setup);
	nfds = ZGetFD() + 1;

	while (!nrecv || !gimmeack) {
		tv.tv_sec = 0;
		tv.tv_usec = 500000;
		for (i=0;i<HM_TIMEOUT*2;i++) { /* 30 secs in 1/2 sec
						  intervals */
			gotone = 0;
			read = setup;
			if (select(nfds, &read, (fd_set *) 0,
				   (fd_set *) 0, &tv) < 0)
				return (errno);
			if (FD_ISSET(ZGetFD(), &read))
			    i--;	/* make sure we time out the
					   full 30 secs */
			retval = ZCheckIfNotice(&retnotice,
						(struct sockaddr_in *)0,
						ZCompareMultiUIDPred,
						(char *)&notice->z_multiuid);
			if (retval == ZERR_NONE) {
				gotone = 1;
				break;
			}
			if (retval != ZERR_NONOTICE)
				return(retval);
		}
		
		if (!gotone)
			return(ETIMEDOUT);

		if (retnotice.z_kind == SERVNAK) {
			ZFreeNotice(&retnotice);
			return (ZERR_SERVNAK);
		}	
		/* non-matching protocol version numbers means the
		   server is probably an older version--must punt */
		if (strcmp(notice->z_version,retnotice.z_version)) {
			ZFreeNotice(&retnotice);
			return(ZERR_VERS);
		}
		if (retnotice.z_kind == SERVACK &&
		    !strcmp(retnotice.z_opcode,notice->z_opcode)) {
			ZFreeNotice(&retnotice);
			gimmeack = 1;
			continue;
		} 

		if (retnotice.z_kind != ACKED) {
			ZFreeNotice(&retnotice);
			return (ZERR_INTERNAL);
		}

		nrecv++;

		end = retnotice.z_message+retnotice.z_message_len;

		__subscriptions_num = 0;
		for (ptr=retnotice.z_message;ptr<end;ptr++)
			if (!*ptr)
				__subscriptions_num++;

		__subscriptions_num /= 3;

		__subscriptions_list = (ZSubscription_t *)
			malloc((unsigned)(__subscriptions_num*
					  sizeof(ZSubscription_t)));
		if (!__subscriptions_list) {
			ZFreeNotice(&retnotice);
			return (ENOMEM);
		}
	
		for (ptr=retnotice.z_message,i = 0; i< __subscriptions_num; i++) {
			__subscriptions_list[i].class = (char *)
				malloc((unsigned)strlen(ptr)+1);
			if (!__subscriptions_list[i].class) {
				ZFreeNotice(&retnotice);
				return (ENOMEM);
			}
			(void) strcpy(__subscriptions_list[i].class,ptr);
			ptr += strlen(ptr)+1;
			__subscriptions_list[i].classinst = (char *)
				malloc((unsigned)strlen(ptr)+1);
			if (!__subscriptions_list[i].classinst) {
				ZFreeNotice(&retnotice);
				return (ENOMEM);
			}
			(void) strcpy(__subscriptions_list[i].classinst,ptr);
			ptr += strlen(ptr)+1;
			ptr2 = ptr;
			if (!*ptr2)
				ptr2 = "*";
			__subscriptions_list[i].recipient = (char *)
				malloc((unsigned)strlen(ptr2)+1);
			if (!__subscriptions_list[i].recipient) {
				ZFreeNotice(&retnotice);
				return (ENOMEM);
			}
			(void) strcpy(__subscriptions_list[i].recipient,ptr2);
			ptr += strlen(ptr)+1;
		}
		ZFreeNotice(&retnotice);
	}

	__subscriptions_next = 0;
	*nsubs = __subscriptions_num;

	return (ZERR_NONE);
}
