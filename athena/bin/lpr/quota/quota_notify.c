/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/quota_notify.c,v $
 *	$Author: epeisach $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/quota_notify.c,v 1.2 1990-04-25 11:52:29 epeisach Exp $
 */

/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#if (!defined(lint) && !defined(SABER))
static char quota_notify_rcsid[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/quota_notify.c,v 1.2 1990-04-25 11:52:29 epeisach Exp $";
#endif (!defined(lint) && !defined(SABER))

#include "quota.h"
#include <krb.h>
#include "quota_limits.h"
#include "quota_ncs.h"
#include "quota_err.h"
#include "quota_db.h"
#ifdef ZEPHYR
#include <zephyr/zephyr.h>
#endif
#include "mit-copyright.h"

extern char qcurrency[];

/* These routines handle the notification to users that they are
   either over quota or approaching it real soon. Zephyr notification is 
   used if possible or mail
*/

QuotaReport_notify(qid, qreport, quotaRec)
quota_identifier   	 *qid;
quota_report	  	 *qreport;
quota_rec 		*quotaRec;
{
    char message[1024], message1[1024] ;
    char qprincipal[ANAME_SZ], qinstance[INST_SZ], qrealm[REALM_SZ];
    float ratio;

    parse_username(qid->username, qprincipal, qinstance, qrealm);

    if(quotaRec->quotaAmount > quotaRec->quotaLimit) {
	sprintf(message,
		"Your last print job was %d pages.\nYou were charged %d %s.",
		qreport->pages, 
		(qreport->pages * qreport->pcost), 
		qcurrency);
	if(strcmp(qcurrency, "cents")) 
	    sprintf(message1, 
		    "%s\nYour total print charges this semester are %d %s.",
		    message, (quotaRec->quotaAmount - quotaRec->quotaLimit),
		    qcurrency);
	else
	    sprintf(message1, 
		    "%s\nYour total print charges this semester are $%.2f.",
		    message, (float) (quotaRec->quotaAmount - quotaRec->quotaLimit)/100.0);
	goto notify;
    }

    message1[0] = '\0';


    if((quotaRec->quotaLimit != 0) &&
       (100 * (quotaRec->quotaLimit - quotaRec->quotaAmount))
       /quotaRec->quotaLimit < 10) {
	if(strcmp(qcurrency, "cents"))
	    sprintf(message1, 
		    "There are %d %s left in your %s print quota.\nLast job printed at %d %s/page\n",
		    (quotaRec->quotaLimit - quotaRec->quotaAmount),
		    qcurrency,
		    quotaRec->service,
		    qreport->pcost, qcurrency);
	else
	    sprintf(message1, 
		    "There are $%.2f left in your %s print quota.\nLast job printed at %d %s/page\n",
		    (float) (quotaRec->quotaLimit - quotaRec->quotaAmount)/100.0,
		    quotaRec->service,
		    qreport->pcost, qcurrency);

    }

    ratio = ((float) (quotaRec->quotaLimit - quotaRec->quotaAmount)) / 
	((qreport->pages == 0) ? 1 : 
	 (qreport->pages * qreport->pcost));

    if(ratio >=1 && ratio < 2) {
	if(strcmp(qcurrency, "cents"))
	    sprintf(message,"%s\nThere are %d %s left in your %s print quota.\n",
		    message1,
		    (quotaRec->quotaLimit - quotaRec->quotaAmount), 
		    qcurrency,
		    quotaRec->service);
	else
	    sprintf(message,"%s\nThere are $%.2f left in your %s print quota.\n",
		    message1,
		    (float) (quotaRec->quotaLimit - quotaRec->quotaAmount)/100.0, 
		    quotaRec->service);
	sprintf(message1, "%sYour last print job was %d pages.\n",
		message, qreport->pages);
	sprintf(message, "%sYou can print only one more print job of\n",
		message1);
	sprintf(message1, "%sthat size without going over your quota.\n",
		message);
	goto notify;
    }

    if(ratio < 1) {
	if(strcmp(qcurrency, "cents"))
	    sprintf(message,"%s\nThere are %d %s left in your %s print quota.\n",
		    message1,
		    quotaRec->quotaLimit - quotaRec->quotaAmount,
		    qcurrency,
		    quotaRec->service);
	else
	    sprintf(message,"%s\nThere is $%.2f left in your %s print quota.\n",
		    message1,
		    (float) (quotaRec->quotaLimit - quotaRec->quotaAmount)/100.0,
		    quotaRec->service);
	sprintf(message1, "%sYour last print job was %d pages.\n",
		message, qreport->pages);
	sprintf(message, "%sYou cannot print another print job of\n",
		message1);
	sprintf(message1, "%sthat size without going over your quota.\n",
		message);
	goto notify;
    }
    /* Don't notify */
    return 0;

/* We use notify user with the prober user and message as needed. */
/* At Athena, although the kerberos instance is valid as a part of the username
   we cannot send mail to someone that way. Use of principal only then. */

 notify:
    if(message1[0] == '\0') return 0;
#ifdef ZEPHYR
    return(NotifyUser(qprincipal, message1));
#else
    return 0;
#endif
}


#ifdef ZEPHYR
NotifyUser(user, message)
     char *user, *message;
{
    ZNotice_t notice;

    /* Return if NULL message */
    if (message[0] == '\0')
	return;

    bzero((char *)&notice, sizeof(notice));
    
    if (ZInitialize() != ZERR_NONE) 
	return;
    /* Something wrong, oh well... */

    notice.z_kind = UNACKED;
    notice.z_port = 0;
    notice.z_class = "MESSAGE";
    notice.z_class_inst = "PERSONAL";
    notice.z_opcode = "QUOTA";
    notice.z_sender = "Print Quota Server";
    notice.z_recipient = user;
    notice.z_default_format = "";
    notice.z_message = message;
    notice.z_message_len = strlen(notice.z_message)+1;

    if (ZSendNotice(&notice, ZNOAUTH) != ZERR_NONE) {
	return;
    }
    return;
}
#endif ZEPHYR

