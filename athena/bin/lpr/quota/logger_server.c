/*
 *	$Id: logger_server.c,v 1.9 1999-01-22 23:11:02 ghudson Exp $
 */

/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#if (!defined(lint) && !defined(SABER))
static char logger_server_rcsid[] = "$Id: logger_server.c,v 1.9 1999-01-22 23:11:02 ghudson Exp $";
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
#include "logger.h"
#include "logger_ncs.h"
char *set_service();


#ifdef __STDC__
quota_error_code LoggerJournal(handle_t h, krb_ktext *auth, 
				  quota_identifier *qid, 
				  startingpoint start, maxtotransfer maxnum, 
				  loggerflags flags, ndr_$long_int *numtrans,
				  LogEnt LogEnts[LOGMAXRETURN], 
				  quota_currency currency)
#else
quota_error_code LoggerJournal(h,auth,qid,start,maxnum,flags,numtrans,LogEnts,currency)
handle_t h;
krb_ktext *auth;
quota_identifier *qid;
startingpoint start;
maxtotransfer maxnum;
loggerflags flags;
ndr_$long_int *numtrans;
LogEnt LogEnts[LOGMAXRETURN];
quota_currency currency;
#endif
{

/* This is the meat of this whole user interface - we handle the world... */

    /* User making request */
    char uname[ANAME_SZ], uinstance[INST_SZ], urealm[REALM_SZ]; 
    /* Information request for */
    char rname[ANAME_SZ], rinstance[INST_SZ], rrealm[REALM_SZ];
    /* Group Information request for user */
    char gname[ANAME_SZ], ginstance[INST_SZ], grealm[REALM_SZ];
    int authuser = 0;	/* If user is in the access list for special */
    String_num name, instance, realm;
    String_num name1=0, instance1=0, realm1=0;
    User_db *userlook;
    User_str userin;
    log_header jhead;
    log_entity *ent;
    LogEnt *lent;
    int i, more, is_group = 0, read_group_record = 0;
    int retval, limited_group = 0;
    AUTH_DAT ad;
    char name_1[MAX_K_NAME_SZ];
    extern char qcurrency[];             /* The quota currency */
    quota_rec quotarec;
    gquota_rec gquotarec;
    char *service;
    char grp_str[128];

    /* Init for error returns */
    *numtrans = 0;

    (void) strcpy((char *)currency, qcurrency);

    CHECK_PROTECT();

    service=set_service((char *)qid->service);
    if (qid->account != 0) is_group++;

    /* First verify that the user is authenticated - you must be 
       authenticated for any info */
    if(check_krb_auth(h, auth, &ad))
	return QBADTKTS;
    
    make_kname(ad.pname, ad.pinst, ad.prealm, name_1);

    if(is_suser(name_1)) authuser = 1;
    
    if (!authuser && is_group) {
	retval = quota_db_get_principal(ad.pname, ad.pinst,
					service,
					ad.prealm, &quotarec,
					(unsigned int)1, &more);
	if (!(retval))
	    return QNOAUTH;    /* user not in database */
	else if (retval < 0)
	    return QDBASEERROR;
	if (quotarec.deleted)
	    return QUSERDELETED;
	
	retval = gquota_db_get_group(qid->account, service,
				     &gquotarec, (unsigned int)1,
				     &more);
	if (!(retval))
	    return QNOGROUPEXISTS;    /* group not in database */
	else if (retval < 0)
	    return QGDBASEERROR;
	if (gquotarec.deleted)
	    return QGROUPDELETED;
	
	if (!is_group_admin(quotarec.uid, &gquotarec))
	    return QNOAUTH;
	read_group_record++;
    }

    (void) strncpy(uname, ad.pname, ANAME_SZ);
    (void) strncpy(uinstance, ad.pinst, INST_SZ);
    (void) strncpy(urealm, ad.prealm, REALM_SZ);

    if (is_group) {
	if (!read_group_record) {
	    retval = gquota_db_get_group(qid->account, service,
					 &gquotarec, (unsigned int)1,
					 &more);
	    if (!(retval))
		return QNOGROUPEXISTS;    /* group not in database */
	    else if (retval < 0)
		return QGDBASEERROR;
	    if (gquotarec.deleted)
		return QGROUPDELETED;
	}
	(void) sprintf(grp_str,":%d", qid->account);

	(void) strncpy(rname, grp_str, ANAME_SZ);
	(void) strncpy(rinstance, grp_str, INST_SZ);
	(void) strncpy(rrealm, grp_str, REALM_SZ);
     
	if (qid->username[0] != '\0') {
	    parse_username(qid->username, gname, ginstance, grealm);
	    limited_group = 1;
	}
    }
    else {
	/* Find out who the info is about!!! */
	parse_username(qid->username, rname, rinstance, rrealm);
    }

    if (!is_group) 
	if(((strcmp(rname, uname) != 0) || (strcmp(rinstance, uinstance) != 0) ||
	    (strcmp(rrealm, urealm) != 0)) && (authuser == 0)) {
	    return QNOAUTH;
	}

    /* Ok - so now the user has full access to requested info -
       within limitations. */

    /* Start by allocating memory for the header of the structure. */
    /* Allocate memory for return */
    /* We are happy to comply with all requests which are reasonable in length
       but we do impose a limit on the return, so that we don't die of
       excessiveness...
       */
    maxnum = (maxnum > LOGMAXRETURN) ? LOGMAXRETURN : maxnum;

    *numtrans = 0;

    /* To do this, we must convert the requested info into Stringnums. If 
       any do not exist, then there is no info. This makes our life easy!
       */
    if((((name = logger_string_to_num(rname)) == 0) && 
	rname[0] != '\0') ||
       (((instance = logger_string_to_num(rinstance)) == 0) && 
	rinstance[0] != '\0')||
       (((realm = logger_string_to_num(rrealm)) == 0) && 
	rrealm[0] != '\0'))
	{
	    /* There are no entries. We have already nulled out the info */
	    return 0;
	}
    
    if (limited_group) {
	    if((((name1 = logger_string_to_num(gname)) == 0) && 
		gname[0] != '\0') ||
	       (((instance1 = logger_string_to_num(ginstance)) == 0) && 
		ginstance[0] != '\0')||
	       (((realm1 = logger_string_to_num(grealm)) == 0) && 
		grealm[0] != '\0'))
		{
		    /* There are no entries since we dont have the
		     * user in the string db.
		     */
		    return 0;
		}
	}

    if(start == 0) {
	userin.name = name;
	userin.instance = instance;
	userin.realm = realm;
	/* Look up the user then. User may not exist... */
	if(((userlook = logger_find_user(&userin)) == (User_db *) NULL) ||
	   (userlook->first == 0)) {
	    /* There are no entries for the user. */
	    return 0;
	}
	start = userlook->first;
    }
    /* We still have to verify that the first entry is a) in range & 
       b) Allowed access.
     */
    if(logger_journal_get_header(&jhead)) {
	syslog(LOG_INFO, "Unable to read header from journal db.");
	return QDBASEERROR;
    }

    if((start < 0) || (start > jhead.num_ent)) {
	return BAD_PARAMETER;
    }

    /* Ok - staring is now known to be in range. User name checks still
       needed */

    /* Memory for all - start shipping the stuff... */

    for(i=1, lent = LogEnts; i <= maxnum; i++, lent++) {
	if((ent = logger_journal_get_line((Pointer) start)) 
	   == (log_entity *) NULL){
	    /* We have an error - don't know why */
	    syslog(LOG_INFO, "LoggerJournal - could not read entry #%d", start);
	    /* Not quite the right error */
	    return BAD_PARAMETER;
	}

	if( (!authuser) && ((ent->user.name != name) || 
			    (ent->user.instance != instance) || 
			    (ent->user.realm != realm))) {
	    return QNOAUTH;
	}
	if (limited_group) {
	    if ((ent->func != LO_CHARGE) ||
		(ent->trans.charge.name != name1) || 
		(ent->trans.charge.inst != instance1) || 
		(ent->trans.charge.realm != realm1))
		continue;
	}
	
	/* Ok boys, package it up !!! */
	lent->time = ent->time;

	make_kname(logger_num_to_string(ent->user.name), 
		   logger_num_to_string(ent->user.instance), 
		   logger_num_to_string(ent->user.realm),
		   (char *) lent->name);

	if (is_group)
	    lent->account = qid->account;
	else
	    lent->account = 0;

	(void) strcpy((char *) lent->service, logger_num_to_string(ent->service));
	lent->next = ent->next;
	lent->prev = ent->prev;

	/* Protect extra structures */
	lent->offset.wname[0]='\0';
	lent->charge.wname[0]='\0';
	lent->charge.where[0]='\0';
	lent->group.uname[0]='\0';
	lent->group.aname[0]='\0';

	lent->func = ent->func;
	switch (ent->func) {
	case LO_ADD:
	case LO_SUBTRACT:
	case LO_SET:
	case LO_DELETEUSER:
	case LO_DISALLOW:
	case LO_ALLOW:
	case LO_ADJUST:
		lent->offset.amount = ent->trans.offset.amt;
		make_kname(logger_num_to_string(ent->trans.offset.name),
			   logger_num_to_string(ent->trans.offset.inst),
			   logger_num_to_string(ent->trans.offset.realm),
			   (char *) lent->offset.wname);
		break;
        case LO_CHARGE:
		lent->charge.ptime = ent->trans.charge.subtime;
		lent->charge.npages =  ent->trans.charge.npages;
		lent->charge.pcost = ent->trans.charge.med_cost;
		(void) strcpy((char *) lent->charge.where,
		       logger_num_to_string(ent->trans.charge.where));
		make_kname(logger_num_to_string(ent->trans.charge.name),
			   logger_num_to_string(ent->trans.charge.inst),
			   logger_num_to_string(ent->trans.charge.realm),
			   (char *) lent->charge.wname);
		break;
	case LO_ADD_ADMIN:
	case LO_DELETE_ADMIN:
	case LO_ADD_USER:
	case LO_DELETE_USER:
		make_kname(logger_num_to_string(ent->trans.group.uname),
			   logger_num_to_string(ent->trans.group.uinst),
			   logger_num_to_string(ent->trans.group.urealm),
			   (char *) lent->group.uname);
		make_kname(logger_num_to_string(ent->trans.group.aname),
			   logger_num_to_string(ent->trans.group.ainst),
			   logger_num_to_string(ent->trans.group.arealm),
			   (char *) lent->group.aname);
		break;
	}

	if(authuser && flags) start++;
	else start = ent->next;

	/* Check 0/eof */
	if((start == 0) || (start > jhead.num_ent)) break;
    }

    if((start !=0) && (start <= jhead.num_ent)) i--;
    *numtrans = i;    

    return 0;
}

/* Warning these must reflect the idl file */
static print_logger_v2$epv_t print_logger_v2$mgr_epv = {
    LoggerJournal
    };

register_logger_manager_v2()
{
        status_$t st;
	extern uuid_$t uuid_$nil;
        rpc_$register_mgr(&uuid_$nil, &print_logger_v2$if_spec, 
			  print_logger_v2$server_epv,
			  (rpc_$mgr_epv_t) &print_logger_v2$mgr_epv, &st);

    if (st.all != 0) {
	syslog(LOG_ERR, "Can't register - %s\n", error_text(st));
	exit(1);
    }
}
