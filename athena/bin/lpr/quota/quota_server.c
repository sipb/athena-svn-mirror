/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/quota_server.c,v $
 *	$Author: epeisach $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/quota_server.c,v 1.11 1991-09-25 11:38:20 epeisach Exp $
 */

/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#if (!defined(lint) && !defined(SABER))
static char quota_server_rcsid[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/quota_server.c,v 1.11 1991-09-25 11:38:20 epeisach Exp $";
#endif (!defined(lint) && !defined(SABER))

#include "mit-copyright.h"
#include "quota.h"
#include <krb.h>
#include "quota_limits.h"
#include "quota_ncs.h"
#include "quota_err.h"
#include "quota_db.h"
#include "gquota_db.h"
#include "uuid.h"
#include <sys/file.h>

#define MAX_RETRY 3	/* Max retry to get db lock */

extern char qcurrency[];             /* The quota currency */
char *set_service();
long uid_add_string();
void Quota_report_log(), Quota_report_group_log(), 
    Quota_modify_log(), Quota_modify_group_log();

quota_error_code QuotaReport(h, auth, qid, qreport, cks)
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
	gquota_rec gquotarec;
	int ret, more, retval;
	int retry = 0;

	/* For now - no backauth */
	*cks = 0;

	CHECK_PROTECT();

	if (QD || !access(SHUTDOWNFILE, F_OK)) return(QDBDOWN);

	service = set_service((char *) qid->service);
	if(ret=check_krb_auth(h, auth, &ad)) 
	    return QBADTKTS;
	make_kname(ad.pname, ad.pinst, ad.prealm, name);

	if(!is_sacct(name, service))
	    return QNOAUTH;

	/* This machine is authorized to connect */

	parse_username(qid->username, qprincipal, qinstance, qrealm);
	(void) strncpy(quotarec.name, qprincipal, ANAME_SZ);
	(void) strncpy(quotarec.instance, qinstance, INST_SZ);
	(void) strncpy(quotarec.realm, qrealm, REALM_SZ);    
    redo:
	ret = quota_db_get_principal(qprincipal, qinstance,
					service,
					qrealm, &quotarec,
					(unsigned int)1, &more);
    
	if (!(ret)) {
	    syslog(LOG_ERR, "Cannot charge: User does not exist in quota dbase: %s from %s", qid->username, name);
	    /* we return 0 so that the entry get's cleared on the client side */
	    /*  return QNOEXISTS;*/
	    return 0;
	}
	if (ret < 0 && retry < MAX_RETRY)  {
	    sleep(1);
	    retry++;
	    goto redo;
	}
	if (ret < 0)  {
	    syslog(LOG_ERR, "Database error:User does not exist in quota dbase: %s from %s %d", qid->username, name, ret);
	    return QDBASEERROR;
	}
	if (quotarec.deleted) {
	    syslog(LOG_ERR, "Deleted user printed: %s from %s", qid->username, name);
	    /* We continue and post the bill anyway... Sigh... */
	}

	if (qid->account == (long) 0) {
	    if(qreport->pages * qreport->pcost < 0) 
		return QNONEG;

	    /* report users changes */
	    quotarec.quotaAmount += qreport->pages * qreport->pcost;
	    
	    if (quota_db_put_principal(&quotarec, (unsigned int) 1) < 0) {
		syslog(LOG_ERR, "Database error: In user quota dbase: %s from %s", qid->username, name);
		return QDBASEERROR;
	    }

	    (void) QuotaReport_notify(qid, qreport, &quotarec);
	    Quota_report_log(qid, qreport);

	} else {
	    /* This is a group charge ... handle it */
	    retry = 0;
	redo1:
	    retval = gquota_db_get_group(qid->account, service,
					 &gquotarec, (unsigned int)1,
					 &more);
	    if (!(retval)) {
		syslog(LOG_ERR, "Cannot charge: Group does not exist in group dbase: %d from %s", qid->account, name);	
		/* we return 0 so that the entry get's cleared on the client side */
		return 0;
	    } else
	    if (retval < 0 && retry < MAX_RETRY)  {
		sleep(1);
		retry++;
		goto redo1;
	    } else if (retval < 0) {
		syslog(LOG_ERR, "Database error: In Group quota dbase: %d from %s %d", qid->account, name, retval);
		return QGDBASEERROR;
	    }
	    if (gquotarec.deleted) {
		syslog(LOG_ERR, "Printed to deleted group account: %d from %s", qid->account, name);
		/* Continue anyway .. sigh .. */
	    }
	    if (!is_group_user(quotarec.uid, &gquotarec)) {
		syslog(LOG_ERR, "Unknown user %s printed to group account %d", name, qid->account);
		/* oh well post it anyway */
	    }

	    /* Give a discount - AD guaranteed to be != 0 */
	    /* We must change the pcost so the reporting mechanism will DTRT */
	    qreport->pcost = (qreport->pcost * AN) / AD;

	    if(qreport->pages * qreport->pcost < 0) 
		return QNONEG;

	    /* report users changes */
	    gquotarec.quotaAmount += qreport->pages * qreport->pcost;
	    
	    if (gquota_db_put_group(&gquotarec, (unsigned int) 1) < 0) {
		syslog(LOG_ERR, "Database error: In writing group quota dbase: %d from %s", qid->account, name);
		return QGDBASEERROR;
	    }
	    
	    
	    (void) QuotaReport_group_notify(qid, qreport, &gquotarec, 
				     is_group_admin(quotarec.uid, &gquotarec));

	    Quota_report_group_log(qid, qreport);
	}
	return 0;
    }


quota_error_code QuotaQuery (h, auth, qid, qret)
handle_t h;
krb_ktext *auth;
quota_identifier *qid;
quota_return *qret;
    {
	AUTH_DAT ad;
	unsigned char name[MAX_K_NAME_SZ];
	char *service;
	char qprincipal[ANAME_SZ], qinstance[INST_SZ], qrealm[REALM_SZ];
	char kprincipal[ANAME_SZ], kinstance[INST_SZ], krealm[REALM_SZ];
	quota_rec quotarec;
	int more, retval;
	int authuser = 0;	/* If user is in the access list for special */
	int retry = 0;

	service = set_service((char *) qid->service);

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

	make_kname(ad.pname, ad.pinst, ad.prealm, (char *) name);

	if(is_suser((char *) name)) authuser = 1;
	if (QD && !authuser) return(QDBDOWN);
	parse_username(qid->username, qprincipal, qinstance, qrealm);
	(void) strncpy(quotarec.name, qprincipal, ANAME_SZ);
	(void) strncpy(quotarec.instance, qinstance, INST_SZ);
	(void) strncpy(quotarec.realm, qrealm, REALM_SZ);    

	parse_username(name, kprincipal, kinstance, krealm);
	if(((strcmp(quotarec.name, kprincipal) != 0) || 
	    (strcmp(quotarec.instance, kinstance) != 0) ||
	    (strcmp(quotarec.realm, krealm) != 0)) && (authuser == 0)) 
	    return QNOAUTH;
redo:
	retval = quota_db_get_principal(qprincipal, qinstance,
					service,
					qrealm, &quotarec,
					(unsigned int)1, &more);
    
	if (!(retval)) 
	    return QNOEXISTS;
	if (retval < 0 && retry < MAX_RETRY)  {
	    sleep(1);
	    retry++;
	    goto redo;
	}
	if (retval < 0) 
	    return QDBASEERROR;

	if (quotarec.deleted) 
	    return QUSERDELETED;

    /* Set the return variables */
	qret->uid = quotarec.uid;
	qret->usage = quotarec.quotaAmount;
	qret->limit = quotarec.quotaLimit;
	(void) strcpy((char *) qret->currency, qcurrency);
	qret->last_bill = quotarec.lastBilling;
	qret->last_charge = quotarec.lastCharge;
	qret->pending_charge = quotarec.pendingCharge;
	qret->last_amount = quotarec.lastQuotaAmount;
	qret->ytd_billed = quotarec.yearToDateCharge;


	if(quotarec.allowedToPrint == FALSE) 
	    (void) strcpy((char *) qret->message, "Printing is disabled");
	else
	    (void) strcpy((char *) qret->message, "Printing is allowed");
	return 0;

    }

quota_error_code QuotaModifyUser (h, auth, qid, qtype, qamount)
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
	int retry = 0;

	CHECK_PROTECT();

	if ((QD && (qtype != U_NEW)) || !access(SHUTDOWNFILE, F_OK) ) 
	    return(QDBDOWN);

#if 0
	syslog(LOG_DEBUG, "Quotamodify %s, %s, %d:amt %d, type %d\n", 
	       qid->username, qid->service, qid->account,qamount,qtype);
#endif
	service=set_service((char *) qid->service);

	if(check_krb_auth(h, auth, &ad))
	    return QBADTKTS;

	make_kname(ad.pname, ad.pinst, ad.prealm, name);

	if(!is_suser(name))
	    return QNOAUTH;

	/* Ok - now the user making the request is "God" */

	if(qamount < 0) 
	    return QNONEG;
	
	parse_username(qid->username, qprincipal, qinstance, qrealm);

	/* Handle U_NEW special..., all others require the user to be
	   present in the database, so get entry... */
	if(qtype == U_NEW) {
	    (void) strncpy(quotarec.name, qprincipal, ANAME_SZ);
	    (void) strncpy(quotarec.instance, qinstance, INST_SZ);
	    (void) strncpy(quotarec.realm, qrealm, REALM_SZ);
	    (void) strncpy(quotarec.service, service, SERV_SZ);

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
	    make_kname(qprincipal, qinstance, qrealm, uname);
	    if ((quotarec.uid = uid_add_string(uname)) < (long)0)
		return(QUIDERROR);

#ifdef DEBUG
	    printf("\tPrincipal - %s\n\tInstance - %s\n\tRealm - %s\n\tService - %s\n",
		   qprincipal, qinstance, qrealm, service);
	    printf("\tLimit - %d\n\tAmount - %d\n", quotarec.quotaLimit, 
		   quotarec.quotaAmount);
#endif DEBUG
	redo:
	    retval = quota_db_get_principal(qprincipal, qinstance,
					    service,
					    qrealm, &quotarec,
					    (unsigned int)1, &more);

	    if (!(retval)) {
		if (quota_db_put_principal(&quotarec, (unsigned int) 1) < 0) 
		    return QDBASEERROR;
	    } else if (retval > 0) {  /* Already exists */
		return QEXISTS;
	    } else if (retval < 0 && retry < MAX_RETRY)  {
		sleep(1);
		retry++;
		goto redo;
	    } else {
		return QDBASEERROR;
	    }
	    return 0;
	}

	/* This is not a U_NEW entry */
	retry = 0;
    redo1:
	retval = quota_db_get_principal(qprincipal, qinstance,
					service,
					qrealm, &quotarec,
					(unsigned int)1, &more);
	if(!(retval)) {
	    /* User does not exist - can't modify nothing */
	    return QNOEXISTS;
	}  else if (retval < 0 && retry < MAX_RETRY)  {
		sleep(1);
		retry++;
		goto redo1;
	    }
	else if (retval < 0) {
	    return QDBASEERROR;
	}

	if (quotarec.deleted) 
	    return QUSERDELETED;
    
	/* Let's modify this sucker !!! */
	switch ((int) qtype) {
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

quota_error_code QuotaModifyAccount(h, auth, qaccount, qtype, qid, qamount)
handle_t h;
krb_ktext *auth;
quota_account qaccount;
quota_identifier *qid;
modify_user_type qtype;
quota_value qamount;
    {
	AUTH_DAT ad;
        char name[MAX_K_NAME_SZ];
        char qprincipal[ANAME_SZ], qinstance[INST_SZ], qrealm[REALM_SZ];
        quota_rec quotarec;
	gquota_rec gquotarec, gquotarec1;
        int retval,more,tmp;
        extern int qdefault;
        char *service;
	int authuser = 0;
	int read_group_record = 0;
	int retry = 0;

	CHECK_PROTECT();

	if ((QD && (qtype != A_NEW)) || !access(SHUTDOWNFILE, F_OK) ) 
	    return(QDBDOWN);

	service=set_service((char *) qid->service);

        if(check_krb_auth(h, auth, &ad))
            return QBADTKTS;

	if(qaccount <= (long) 0)
	    return(QBADACCOUNT);

	make_kname(ad.pname, ad.pinst, ad.prealm, name);
        parse_username(qid->username, qprincipal, qinstance, qrealm);

	/* If we are superuser we can do anything */
        if(is_suser(name)) authuser = 1;

	/* Superuser can only do the following operations */
	/* If anyone else tries, shoot 'em */
	if (!authuser && (qtype == A_NEW || qtype == A_SET || 
	    qtype == A_SUBTRACT || qtype == A_DELETE || 
	    qtype == A_ADJUST))
	    return QNOAUTH;

	/* If we got this far and am not superuser then it is
	 * for all other operations. Check to see if the user
	 * is a group admin */
	if (!authuser) {
	redo:
	    retval = quota_db_get_principal(ad.pname, ad.pinst,
					    service,
					    ad.prealm, &quotarec,
					    (unsigned int)1, &more);
	    if (!(retval))
		return QNOAUTH;    /* user not in database */
	    if (retval < 0 && retry < MAX_RETRY)  {
		sleep(1);
		retry++;
		goto redo;
	    }
	    if (retval < 0)
		return QDBASEERROR;
	    if (quotarec.deleted)
		return QUSERDELETED;

	    retry = 0;
	redo1:
	    retval = gquota_db_get_group(qaccount, service,
					 &gquotarec, (unsigned int)1,
					 &more);
	    if (!(retval))
		return QNOGROUPEXISTS;    /* group not in database */
	    else if (retval < 0 && retry < MAX_RETRY)  {
		sleep(1);
		retry++;
		goto redo1;
	    } else if (retval < 0)
		return QGDBASEERROR;
	    if (gquotarec.deleted)
		return QGROUPDELETED;

	    if (!is_group_admin(quotarec.uid, &gquotarec))
		return QNOAUTH;

	    read_group_record++;    /* So we need to read it again */
	}

	/* Now process the request */
	if (qtype == A_NEW) {

	    /* Put in the key */
	    gquotarec.account = qaccount;
	    (void) strncpy(gquotarec.service, service, SERV_SZ);

	    gquotarec.admin[0] = (long) 0;   /* No admins exists yet for group */
	    gquotarec.user[0] = (long) 0;    /* No users exists yet for group */

            gquotarec.quotaAmount = 0;
            if (qamount != 0)
                gquotarec.quotaLimit = qamount;
            else
                gquotarec.quotaLimit = qdefault;

            if (gquotarec.quotaLimit > gquotarec.quotaAmount)
                gquotarec.allowedToPrint = 1;
            else
                gquotarec.allowedToPrint = 0;

            gquotarec.lastBilling      = (gquota_time)    0;
            gquotarec.lastCharge       = (gquota_amount)  0;
            gquotarec.pendingCharge    = (gquota_amount)  0;
            gquotarec.lastQuotaAmount  = (gquota_amount)  0;
            gquotarec.yearToDateCharge = (gquota_amount)  0;
            gquotarec.deleted          = (gquota_deleted) 0;

	    if (read_group_record)
		return QGROUPEXISTS;

	    /* We havent read it yet ... do it now */
	    retry = 0;
	redo2:
	    retval = gquota_db_get_group(qaccount, service,
					 &gquotarec1, (unsigned int)1,
					 &more);
	    if (!(retval)) {
		if (gquota_db_put_group(&gquotarec, (unsigned int)1) < 0)
		    return QGDBASEERROR;
	    } else if (retval > 0) {
		return QGROUPEXISTS;
	    } else if (retval < 0 && retry < MAX_RETRY)  {
		sleep(1);
		retry++;
		goto redo2;
	    } else {
                return QGDBASEERROR;
            }
	    return 0;
	}

	/* This is not an A_NEW entry */
	if (!read_group_record) {
	    retry = 0;
	redo3:
	    retval = gquota_db_get_group(qaccount, service,
					 &gquotarec, (unsigned int)1,
					 &more);
	    if (!(retval))
		return QNOGROUPEXISTS;    /* group not in database */
	    else if (retval < 0 && retry < MAX_RETRY)  {
		sleep(1);
		retry++;
		goto redo3;
	    } else if (retval < 0)
		return QGDBASEERROR;
	    if (gquotarec.deleted)
		return QGROUPDELETED;
	}

	switch ((int) qtype) {
	case A_SET:
	    gquotarec.quotaLimit = qamount;
            break;
        case A_ADD:
            gquotarec.quotaLimit += qamount;
            break;
        case A_SUBTRACT:
            if(gquotarec.quotaLimit < qamount)
                return QNEGATIVELIMIT;
            gquotarec.quotaLimit -= qamount;
            break;
        case A_DELETE:
            gquotarec.deleted = (gquota_deleted) 1;
            break;
        case A_ALLOW_PRINT:
            gquotarec.allowedToPrint = (gquota_allowed) 1;
            break;
        case A_DISALLOW_PRINT:
            gquotarec.allowedToPrint = (gquota_allowed) 0;
            break;
        case A_ADJUST:
            if(gquotarec.quotaAmount < qamount)
                return QNEGATIVEUSAGE;
            gquotarec.quotaAmount -= qamount;
            break;
	case A_ADD_ADMIN:
	case A_DELETE_ADMIN:
	case A_ADD_USER:
	case A_DELETE_USER:
	    /* Read the users record to get the uid */
	    retry = 0;
	redo4:
	    retval = quota_db_get_principal(qprincipal, qinstance,
					    service,
					    qrealm, &quotarec,
					    (unsigned int)1, &more);
	    if(!(retval))
		return QNOEXISTS;
	    if (retval < 0 && retry < MAX_RETRY)  {
		sleep(1);
		retry++;
		goto redo4;
	    }
	    if (retval < 0)
		return QDBASEERROR;
	    if (quotarec.deleted)
		return QUSERDELETED;

	    if (qtype == A_ADD_ADMIN || qtype == A_DELETE_ADMIN) {
		tmp = gquota_db_find_admin(quotarec.uid, &gquotarec);
		if (qtype == A_ADD_ADMIN) {
		    if (tmp >= 0) return QGADMINEXISTS;
		    if (gquota_db_insert_admin(quotarec.uid, &gquotarec) < 0)
			return QGMAXADMINEXCEEDED;
		} else if (qtype == A_DELETE_ADMIN) {
		    if (tmp < 0) return QGNOADMINEXISTS;
		    gquota_db_remove_admin(quotarec.uid,&gquotarec, tmp);
		}
	    } else {
		tmp = gquota_db_find_user(quotarec.uid, &gquotarec);
		if (qtype == A_ADD_USER) {
		    if (tmp >= 0) return QGUSEREXISTS;
		    if (gquota_db_insert_user(quotarec.uid, &gquotarec) < 0)
			return QGMAXUSEREXCEEDED;
		} else if (qtype == A_DELETE_USER) {
		    if (tmp < 0) return QGNOUSEREXISTS;
		    gquota_db_remove_user(quotarec.uid, &gquotarec, tmp);
		}
	    }
	    break;

        default:
            return QNOTVALIDTYPE;
        }

        if (gquota_db_put_group(&gquotarec, (unsigned int) 1) < 0)
            return QGDBASEERROR;
	
	(void) Quota_modify_group_log(qaccount, qid, &ad, qtype, qamount);
	
        return 0;
    }


quota_error_code QuotaQueryAccount(h, auth, qaccount, qid, startadmin, 
				   maxadmin, startuser, maxuser, qret, 
				   numadmin, admin, numuser, user, flag)
handle_t h;
krb_ktext *auth;
quota_account qaccount;
quota_identifier *qid;
qstartingpoint startadmin;
qmaxtotransfer maxadmin;
qstartingpoint startuser;
qmaxtotransfer maxuser;
quota_return *qret;
ndr_$long_int *numadmin;
Principal admin[G_ADMINMAXRETURN];
ndr_$long_int *numuser;
Principal user[G_USERMAXRETURN];
ndr_$long_int *flag;

    {
        AUTH_DAT ad;
        char name[MAX_K_NAME_SZ];
        char *service, *tmp;
#if 0
        char qprincipal[ANAME_SZ], qinstance[INST_SZ], qrealm[REALM_SZ];
        char kprincipal[ANAME_SZ], kinstance[INST_SZ], krealm[REALM_SZ];
#endif
        quota_rec quotarec;
        gquota_rec gquotarec;
        int more, retval, i, j;
        int authuser = 0;       /* If user is in the access list for special */
	int read_group_record = 0;
	int retry = 0;

	service = set_service((char *) qid->service);
	CHECK_PROTECT();

        /* Initialize so in case of error return, we are ok */
        qret->currency[0] = NULL;
        qret->message[0] = NULL;

	*numadmin = *numuser = 0;
	*flag = GQUOTA_NONE;
	for (i = 0; i < G_ADMINMAXRETURN; i++)
	    admin[i][0] = '\0';
	for (i = 0; i < G_USERMAXRETURN; i++)
	    user[i][0] = '\0';

	if (QD) return(QDBDOWN);

        if(check_krb_auth(h, auth, &ad))
            return QBADTKTS;

        make_kname(ad.pname, ad.pinst, ad.prealm, name);

	/* Is he superuser, if so, he can do as he feels */
        if(is_suser(name)) authuser = 1;

	/* If not superuser check if the person is a group admin */
        if (!authuser) {
	    retry = 0;
	redo5:
            retval = quota_db_get_principal(ad.pname, ad.pinst,
                                            service,
                                            ad.prealm, &quotarec,
                                            (unsigned int)1, &more);
	    if (retval < 0 && retry < MAX_RETRY)  {
		sleep(1);
		retry++;
		goto redo5;
	    }
	    if (retval < 0)	
                return QDBASEERROR;
            else if (!(retval))
                return QNOAUTH;    /* user not in database */
            else
            if (quotarec.deleted)
                return QUSERDELETED;

	    retry = 0;
	redo6:
            retval = gquota_db_get_group(qaccount, service,
                                         &gquotarec, (unsigned int)1,
                                         &more);
	    if (retval < 0 && retry < MAX_RETRY)  {
		sleep(1);
		retry++;
		goto redo6;
	    }
	    if (retval < 0)
                return QGDBASEERROR;
            else if (!(retval))
                return QNOGROUPEXISTS;    /* group not in database */
            if (gquotarec.deleted)
                return QGROUPDELETED;

            if (!is_group_admin(quotarec.uid, &gquotarec))
                return QNOAUTH;
	    read_group_record++;
	}

	/* Ok, we have permission to look ... let take a peek */
	if (!read_group_record) {
	    retry = 0;
	redo7:
            retval = gquota_db_get_group(qaccount, service,
                                         &gquotarec, (unsigned int)1,
                                         &more);
            if (!(retval))
                return QNOGROUPEXISTS;    /* group not in database */
	    else if (retval < 0 && retry < MAX_RETRY)  {
		sleep(1);
		retry++;
		goto redo7;
	    }
            else if (retval < 0)
                return QGDBASEERROR;
            if (gquotarec.deleted)
                return QGROUPDELETED;
        }
	qret->uid = 0;
        qret->usage = gquotarec.quotaAmount;
        qret->limit = gquotarec.quotaLimit;
        (void) strcpy((char *) qret->currency, qcurrency);
        qret->last_bill = gquotarec.lastBilling;
        qret->last_charge = gquotarec.lastCharge;
        qret->pending_charge = gquotarec.pendingCharge;
        qret->last_amount = gquotarec.lastQuotaAmount;
        qret->ytd_billed = gquotarec.yearToDateCharge;

        if(gquotarec.allowedToPrint == FALSE)
            (void) strcpy((char *) qret->message, "Printing is disabled");
        else
            (void) strcpy((char *) qret->message, "Printing is allowed");

	/* Now fill in list of admin */
	if (((int) gquotarec.admin[0] != 0) && (startadmin != -1)) {
	    maxadmin = (maxadmin > G_ADMINMAXRETURN) ? 
		G_ADMINMAXRETURN : maxadmin;
	    maxadmin = (maxadmin > (int)gquotarec.admin[0]) ? 
		(int)gquotarec.admin[0] : maxadmin;

	    /* check for a valid startadmin */
	    if (startadmin == 0) startadmin = 1;
	    if ((startadmin < -1) || (startadmin > (int) gquotarec.admin[0]))
		return BAD_PARAMETER;

	    for (i = startadmin, j = 0; i <= maxadmin; i++, j++) {
		tmp = (char *)uid_num_to_string(gquotarec.admin[i]);
		if (tmp == (char *) NULL) {
		    /* Cant get the name for uid. Must be an error */
		    /* in the uid_database ... nothing fatal but needs */
		    /* the database to be recreated. */
		    syslog(LOG_ERR, 
			   "uid %d in admin_list for account %d == NULL", 
			   (int)gquotarec.admin[i], (int) qaccount);
		} else {
		    (void) strncpy((char *) admin[j], tmp, MAX_K_NAME_SZ);
		    admin[j][MAX_K_NAME_SZ - 1] = '\0';
		    (*numadmin)++;
		}
	    }
	    /* If we still have more admins, set the flag */
	    if (i <= (int)gquotarec.admin[0])
		*flag = GQUOTA_MORE_ADMIN;
	}
	/* Now fill in the list of user*/
	if (((int) gquotarec.user[0] != 0) && (startuser != -1)) {
	    maxuser = (maxuser > G_USERMAXRETURN) ? 
		G_USERMAXRETURN : maxuser;
	    maxuser = (maxuser > (int)gquotarec.user[0]) ? 
		(int)gquotarec.user[0] : maxuser;

	    /* check for a valid startuser */
	    if (startuser == 0) startuser = 1;
	    if ((startuser < -1) || (startuser > (int) gquotarec.user[0]))
		return BAD_PARAMETER;

	    for (i = startuser, j = 0; i <= maxuser; i++, j++) {
		tmp = (char *)uid_num_to_string(gquotarec.user[i]);
		if (tmp == (char *) NULL) {
		    /* Cant get the name for uid. Must be an error */
		    /* in the uid_database ... nothing fatal but needs */
		    /* the database to be recreated. */
		    syslog(LOG_ERR, 
			   "uid %d in user_list for account %d == NULL", 
			   (int)gquotarec.user[i], (int) qaccount);
		} else {
		    (void) strncpy((char *) user[j], tmp, MAX_K_NAME_SZ);
		    user[j][MAX_K_NAME_SZ - 1] = '\0';
		    (*numuser)++;
		}
	    }
	    /* If we have more users add to the flag */
	    if (i <= (int)gquotarec.user[0])
		*flag += GQUOTA_MORE_USER;
	}
	return 0;
    }

/*ARGSUSED*/
quota_error_code QuotaServerStatus(h, auth, message)
handle_t h;
krb_ktext *auth;
quota_message message;
{
    if(QD) (void) strncpy((char *) message, "Quota server currently shutdown",
			  MESSAGE_SZ);
    else if(!access(SHUTDOWNFILE, F_OK))
	(void) strncpy((char *) message, "Quota server shutdown for backup",
	       MESSAGE_SZ); 
    else (void) strncpy((char *) message, "Quota server up and running",
			MESSAGE_SZ);
    return 0;
}

/* Warning these must reflect the idl file */
static print_quota_v2$epv_t print_quota_v2$mgr_epv = {
    QuotaReport,
    QuotaQuery,
    QuotaModifyUser,
    QuotaModifyAccount,
    QuotaQueryAccount,
    QuotaServerStatus
    };

register_quota_manager_v2()
{
        status_$t st;
	extern uuid_$t uuid_$nil;
        rpc_$register_mgr(&uuid_$nil, &print_quota_v2$if_spec, 
			  print_quota_v2$server_epv,
			  (rpc_$mgr_epv_t) &print_quota_v2$mgr_epv, &st);

    if (st.all != 0) {
	syslog(LOG_ERR, "Can't register - %s\n", error_text(st));
	exit(1);
    }
}
