/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/logger_server.c,v $
 *	$Author: epeisach $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/logger_server.c,v 1.2 1990-04-25 11:47:32 epeisach Exp $
 */

/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#if (!defined(lint) && !defined(SABER))
static char logger_server_rcsid[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/logger_server.c,v 1.2 1990-04-25 11:47:32 epeisach Exp $";
#endif (!defined(lint) && !defined(SABER))

#include "mit-copyright.h"
#include "quota.h"
#include <krb.h>
#include "quota_limits.h"
#include "quota_ncs.h"
#include "quota_err.h"
#include "quota_db.h"
#include "uuid.h"
#include "logger.h"
#include "logger_ncs.h"
char *set_service();




     
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
{

/* This is the meat of this whole user interface - we handle the world... */

    /* User making request */
    char uname[ANAME_SZ], uinstance[INST_SZ], urealm[REALM_SZ]; 
    /* Information request for */
    char rname[ANAME_SZ], rinstance[INST_SZ], rrealm[REALM_SZ];
    int authuser = 0;	/* If user is in the access list for special */
    String_num name, instance, realm;
    User_db *userlook;
    User_str userin;
    log_header jhead;
    log_entity *ent;
    LogEnt *lent;
    int i;
    AUTH_DAT ad;
    char name1[MAX_K_NAME_SZ];
    extern char qcurrency[];             /* The quota currency */
    /* Init for error returns */
    *numtrans = 0;

    strcpy(currency, qcurrency);

    CHECK_PROTECT();

/* First verify that the user is authenticated - you must be authenticated for
   any info */
    if(check_krb_auth(h, auth, &ad))
	return QBADTKTS;

    make_kname(ad.pname, ad.pinst, ad.prealm, name1);

    if(is_suser(name1)) authuser = 1;

    (void) strncpy(uname, ad.pname, ANAME_SZ);
    (void) strncpy(uinstance, ad.pinst, INST_SZ);
    (void) strncpy(urealm, ad.prealm, REALM_SZ);


    /* Find out who the info is about!!! */

    parse_username(qid->username, rname, rinstance, rrealm);

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
    if(
       (((name = logger_string_to_num(rname)) == 0) && rname[0] != '\0') ||
       (((instance = logger_string_to_num(rinstance)) == 0) && rinstance[0] != '\0')||
       (((realm = logger_string_to_num(rrealm)) == 0) && rrealm[0] != '\0'))
	    {
		/* There are no entries. We have already nulled out the info */
		return 0;
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
	if((ent = logger_journal_get_line(start)) == (log_entity *) NULL){
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

	/* Ok boys, package it up !!! */
	lent->time = ent->time;

	make_kname(ent->user.name, ent->user.instance, ent->user.realm,
		   lent->name);

	strcpy(lent->service, logger_num_to_string(ent->service));
	lent->next = ent->next;
	lent->prev = ent->prev;


	lent->func_union.func = ent->func;
	switch (ent->func) {
	case LO_ADD:
	case LO_SUBTRACT:
	case LO_SET:
	case LO_DELETEUSER:
	case LO_DISALLOW:
	case LO_ALLOW:
	case LO_ADJUST:
	    lent->func_union.tagged_union.offset.amount = ent->trans.offset.amt;
	    make_kname(logger_num_to_string(ent->trans.offset.name),
		       logger_num_to_string(ent->trans.offset.inst),
		       logger_num_to_string(ent->trans.offset.realm),
		       lent->func_union.tagged_union.offset.wname);
	    break;
	case LO_CHARGE:
	    lent->func_union.tagged_union.charge.ptime = ent->trans.charge.subtime;
	    lent->func_union.tagged_union.charge.npages =  ent->trans.charge.npages;
	    lent->func_union.tagged_union.charge.pcost = ent->trans.charge.med_cost;
	    strcpy(lent->func_union.tagged_union.charge.where,
		   logger_num_to_string(ent->trans.charge.where));

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
static print_logger_v1$epv_t print_logger_v1$mgr_epv = {
    LoggerJournal
    };

register_logger_manager_v1()
{
        status_$t st;
	extern uuid_$t uuid_$nil;
        rpc_$register_mgr(&uuid_$nil, &print_logger_v1$if_spec, 
			  print_logger_v1$server_epv,
			  (rpc_$mgr_epv_t) &print_logger_v1$mgr_epv, &st);

    if (st.all != 0) {
	syslog(LOG_ERR, "Can't register - %s\n", error_text(st));
	exit(1);
    }
}
