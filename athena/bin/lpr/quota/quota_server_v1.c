/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/quota_server_v1.c,v $
 *	$Author: ilham $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/quota_server_v1.c,v 1.3 1990-07-11 16:21:46 ilham Exp $
 */

/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#if (!defined(lint) && !defined(SABER))
static char quota_server_rcsid[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/quota_server_v1.c,v 1.3 1990-07-11 16:21:46 ilham Exp $";
#endif (!defined(lint) && !defined(SABER))

#include "mit-copyright.h"
#include "quota.h"
#include <krb.h>
#include "quota_limits.h"
#include "quota_ncs_v1.h"
#include "quota_err.h"
#include "quota_db.h"
#include "uuid.h"
extern char qcurrency[];             /* The quota currency */
char *set_service();


quota_error_code QuotaReport_v1(h, auth, qid, qreport, cks)
handle_t h;
krb_ktext *auth;
unsigned long *cks;
quota_identifier *qid;
quota_report *qreport;
    {
	char *service;
	AUTH_DAT ad;
	char name[MAX_K_NAME_SZ];
	char qprincipal[ANAME_SZ], qinstance[INST_SZ], qrealm[REALM_SZ];
	quota_rec quotarec;
	int ret, more;

	/* For now - no backauth */
	*cks = 0;

	CHECK_PROTECT();

	service = set_service(qid->service);
	if(ret=check_krb_auth(h, auth, &ad)) 
	    return QBADTKTS;
	make_kname(ad.pname, ad.pinst, ad.prealm, name);

	if(!is_sacct(name, service))
	    return QNOAUTH;

	if (QD) return(QDBASEERROR);

	/* This machine is authorized to connect */

	parse_username(qid->username, qprincipal, qinstance, qrealm);
	strncpy(quotarec.name, qprincipal, ANAME_SZ);
	strncpy(quotarec.instance, qinstance, INST_SZ);
	strncpy(quotarec.realm, qrealm, REALM_SZ);    
	ret = quota_db_get_principal(qprincipal, qinstance,
					service,
					qrealm, &quotarec,
					(unsigned int)1, &more);
    
	if (!(ret)) {
	    syslog(LOG_ERR, "Cannot charge:User does not exist in quota dbase %s from %s", qid->username, name);
	    /* we return 0 so that the entry get's cleared on the client side */
/*	    return QNOEXISTS;*/
	    return 0;
	}
	if (ret < 0)  {
	    syslog(LOG_ERR, "Database error:User does not exist in quota dbase %s from %s", qid->username, name);
	    return QDBASEERROR;
	}

	if (quotarec.deleted) {
	    syslog(LOG_ERR, "Deleted user printed: %s from %s", qid->username, name);
	    /* We continue and post the bill anyway... Sigh... */
	}

	if(qreport->pages * qreport->pcost < 0) 
	    return QNONEG;

	/* report users changes */
	quotarec.quotaAmount += qreport->pages * qreport->pcost;

	if (quota_db_put_principal(&quotarec, (unsigned int) 1) < 0) {
	    syslog(LOG_ERR, "Database error:User does not exist in quota dbase %s from %s", qid->username, name);
	    return QDBASEERROR;
	}

	QuotaReport_notify(qid, qreport, &quotarec);
	Quota_report_log(qid, qreport);
	return 0;
    }

quota_error_code QuotaQuery_v1(h, auth, qid, qret)
handle_t h;
krb_ktext *auth;
quota_identifier *qid;
quota_return_v1 *qret;
    {
	AUTH_DAT ad;
	char name[MAX_K_NAME_SZ];
	char *service;
	char qprincipal[ANAME_SZ], qinstance[INST_SZ], qrealm[REALM_SZ];
	char kprincipal[ANAME_SZ], kinstance[INST_SZ], krealm[REALM_SZ];
	quota_rec quotarec;
	int more, retval;
	int authuser = 0;	/* If user is in the access list for special */

	service = set_service(qid->service);

	CHECK_PROTECT();

#ifdef DEBUG
	syslog(LOG_DEBUG, "QuotaQuery %s, %s, %d, len %d\n", 
	       qid->username, service, qid->account,auth->length);
#endif DEBUG


	/* Initialize so in case of error return, we are ok */
	qret->currency[0] = NULL;
	qret->message[0] = NULL;

	if(check_krb_auth(h, auth, &ad))
	    return QBADTKTS;

	make_kname(ad.pname, ad.pinst, ad.prealm, name);

	if(is_suser(name)) authuser = 1;

	parse_username(qid->username, qprincipal, qinstance, qrealm);
	strncpy(quotarec.name, qprincipal, ANAME_SZ);
	strncpy(quotarec.instance, qinstance, INST_SZ);
	strncpy(quotarec.realm, qrealm, REALM_SZ);    

	parse_username(name, kprincipal, kinstance, krealm);
	if(((strcmp(quotarec.name, kprincipal) != 0) || 
	    (strcmp(quotarec.instance, kinstance) != 0) ||
	    (strcmp(quotarec.realm, krealm) != 0)) && (authuser == 0)) 
	    return QNOAUTH;

	retval = quota_db_get_principal(qprincipal, qinstance,
					service,
					qrealm, &quotarec,
					(unsigned int)1, &more);
    
	if (!(retval)) 
	    return QNOEXISTS;
	if (retval < 0) 
	    return QDBASEERROR;

	if (quotarec.deleted) 
	    return QUSERDELETED;

    /* Set the return variables */
	qret->usage = quotarec.quotaAmount;
	qret->limit = quotarec.quotaLimit;
	strcpy(qret->currency, qcurrency);
	qret->last_bill = quotarec.lastBilling;
	qret->last_charge = quotarec.lastCharge;
	qret->pending_charge = quotarec.pendingCharge;
	qret->last_amount = quotarec.lastQuotaAmount;
	qret->ytd_billed = quotarec.yearToDateCharge;


	if(quotarec.allowedToPrint == FALSE) 
	    strcpy(qret->message, "Printing is disabled");
	else
	    strcpy(qret->message, "Printing is allowed");
	return 0;

    }

quota_error_code QuotaModifyUser_v1(h, auth, qid, qtype, qamount)
handle_t h;
krb_ktext *auth;
quota_identifier *qid;
modify_user_type qtype;
quota_value qamount;
    {
	AUTH_DAT ad;
	char name[MAX_K_NAME_SZ], uname[MAX_K_NAME_SZ];
	char qprincipal[ANAME_SZ], qinstance[INST_SZ], qrealm[REALM_SZ];
	quota_rec quotarec;
	int retval,more;
	extern int qdefault;
	char *service;

	CHECK_PROTECT();

#if 0
	syslog(LOG_DEBUG, "Quotamodify %s, %s, %d:amt %d, type %d\n", 
	       qid->username, qid->service, qid->account,qamount,qtype);
#endif
	service=set_service(qid->service);

	if(check_krb_auth(h, auth, &ad))
	    return QBADTKTS;

	make_kname(ad.pname, ad.pinst, ad.prealm, name);

	if(!is_suser(name))
	    return QNOAUTH;

	if (QD) return(QDBASEERROR);

	/* Ok - now the user making the request is "God" */

	if(qamount < 0) 
	    return QNONEG;
	
	parse_username(qid->username, qprincipal, qinstance, qrealm);

	/* Handle U_NEW special..., all others require the user to be
	   present in the database, so get entry... */
	if(qtype == U_NEW) {
	    strncpy(quotarec.name, qprincipal, ANAME_SZ);
	    strncpy(quotarec.instance, qinstance, INST_SZ);
	    strncpy(quotarec.realm, qrealm, REALM_SZ);
	    strncpy(quotarec.service, service, SERV_SZ);

	    quotarec.quotaAmount = 0;
	    if (qamount != 0)
		quotarec.quotaLimit = qamount;
	    else
		quotarec.quotaLimit = qdefault;
    
	    if (quotarec.quotaLimit > quotarec.quotaAmount)
		quotarec.allowedToPrint = 1;
	    else
		quotarec.allowedToPrint = 0;

	    quotarec.lastBilling      = (quota_time)    0;
	    quotarec.lastCharge       = (quota_amount)  0;
	    quotarec.pendingCharge    = (quota_amount)  0;
	    quotarec.lastQuotaAmount  = (quota_amount)  0;
	    quotarec.yearToDateCharge = (quota_amount)  0;
	    quotarec.deleted          = (quota_deleted) 0;

            /* If we cannot generate new uid then error */
	    /* This is really the wrong error number, but 
	       the V1 client did not have the new error messages. */
	    /* This code is for forward compatibility */
            make_kname(qprincipal, qinstance, qrealm, uname);
            if ((quotarec.uid = (long) uid_add_string(uname)) < (long)0)
                return(QDBASEERROR);

#ifdef DEBUG
	    printf("\tPrincipal - %s\n\tInstance - %s\n\tRealm - %s\n\tService - %s\n",
		   qprincipal, qinstance, qrealm, service);
	    printf("\tLimit - %d\n\tAmount - %d\n", quotarec.quotaLimit, 
		   quotarec.quotaAmount);
#endif DEBUG

	    retval = quota_db_get_principal(qprincipal, qinstance,
					    service,
					    qrealm, &quotarec,
					    (unsigned int)1, &more);

	    if (!(retval)) {
		if (quota_db_put_principal(&quotarec, (unsigned int) 1) < 0) 
		    return QDBASEERROR;
	    } else if (retval > 0) {  /* Already exists */
		return QEXISTS;
	    } else {
		return QDBASEERROR;
	    }
	    return 0;
	}

	/* This is not a U_NEW entry */

	retval = quota_db_get_principal(qprincipal, qinstance,
					service,
					qrealm, &quotarec,
					(unsigned int)1, &more);
	if(!(retval)) {
	    /* User does not exist - can't modify nothing */
	    return QNOEXISTS;
	} else if (retval < 0) {
	    return QDBASEERROR;
	}

	if (quotarec.deleted) 
	    return QUSERDELETED;
    
	/* Let's modify this sucker !!! */
	switch (qtype) {
	case U_SET:
	    quotarec.quotaLimit = qamount;
	    break;
	case U_ADD:
	    quotarec.quotaLimit += qamount;
	    break;
	case U_SUBTRACT:
	    if(quotarec.quotaLimit < qamount) 
		return QNEGATIVELIMIT;
	    quotarec.quotaLimit -= qamount;
	    break;
	case U_DELETE:
	    quotarec.deleted = (quota_deleted) 1;
	    break;
	case U_ALLOW_PRINT:
	    quotarec.allowedToPrint = (quota_allowed) 1;
	    break;
	case U_DISALLOW_PRINT:
	    quotarec.allowedToPrint = (quota_allowed) 0;
	    break;
	case U_ADJUST:
	    if(quotarec.quotaAmount < qamount) 
		return QNEGATIVEUSAGE;
	    quotarec.quotaAmount -= qamount;
	    break;
	default:
	    return QNOTVALIDTYPE;
	}

	if (quota_db_put_principal(&quotarec, (unsigned int) 1) < 0) 
	    return QDBASEERROR;
    
	(void) Quota_modify_log(qid, &ad, qtype, qamount);

	return 0;
    }

quota_error_code QuotaModifyAccount_v1(h, auth, qtype, qid, qamount)
handle_t h;
krb_ktext *auth;
quota_identifier *qid;
modify_user_type qtype;
quota_value qamount;
    {
	CHECK_PROTECT();
	/* Not implemented yet... */
	return QNOTVALIDTYPE;
    }




/* Warning these must reflect the idl file */
static print_quota_v1$epv_t print_quota_v1$mgr_epv = {
    QuotaReport_v1,
    QuotaQuery_v1,
    QuotaModifyUser_v1,
    QuotaModifyAccount_v1
    };

register_quota_manager_v1()
{
        status_$t st;
	extern uuid_$t uuid_$nil;
        rpc_$register_mgr(&uuid_$nil, &print_quota_v1$if_spec, 
			  print_quota_v1$server_epv,
			  (rpc_$mgr_epv_t) &print_quota_v1$mgr_epv, &st);

    if (st.all != 0) {
	syslog(LOG_ERR, "Can't register - %s\n", error_text(st));
	exit(1);
    }
}
