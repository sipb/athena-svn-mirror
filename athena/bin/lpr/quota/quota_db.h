/* $Id: quota_db.h,v 1.4 1999-01-22 23:11:09 ghudson Exp $ */

/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#include "mit-copyright.h"


/*
 * Some useful defenitions for the Quota Server code
 */

#include <sys/types.h>

#ifdef KERBEROS
#include <krb.h>
#endif

#include "quota_limits.h"

#ifdef KERBEROS
extern char AuthenticName[];
extern char AuthenticInstance[];
extern char AuthenticRealm[];
extern char my_realm[];
#else !(KERBEROS)
extern char AuthenticName[];
#endif !(KERBEROS)


/* Arguments to quota_dbl_lock() */
#define QUOTA_DBL_EXCLUSIVE 1
#define QUOTA_DBL_SHARED 0

/* arguments to quota_db_set_lockmode() */
#define QUOTA_DBL_BLOCKING 0
#define QUOTA_DBL_NONBLOCKING 1

/* Constants used in quota_db_generate_uid */
#define QUOTA_UID_NAME     "_uid_"
#define QUOTA_UID_INSTANCE "_uid_"
#define QUOTA_UID_REALM    "_uid_"
#define QUOTA_UID_SERVICE  "_uid_"

/* typedefs used in user quota struct */
typedef int     quota_amount;
typedef int     quota_allowed;
typedef u_long  quota_time;
typedef int     quota_deleted;

/* User quota structure */
typedef struct {
	char 		name[ANAME_SZ];		/* Kerb. name */
	char		instance[INST_SZ];	/* Kerb. instance */
	char 		realm[REALM_SZ];	/* Kerb. realm */
	char 		service[SERV_SZ];	/* Printer service type */
	long            uid;                    /* Unique ID */
	quota_amount	quotaAmount;		/* Total usage */
	quota_amount	quotaLimit;		/* Max usage */
	quota_time    lastBilling;
	quota_amount  lastCharge;		/* Amount on statement */
	quota_amount  pendingCharge;		/* Amount supposed to 
						   have been billed, but
						   couldn't for $5 min */
	quota_amount  lastQuotaAmount;		/* Quota amt at statement */
	quota_amount  yearToDateCharge;
	quota_allowed allowedToPrint;
	quota_deleted deleted;
	} quota_rec;

extern int default_quota;

#ifdef DEBUG
extern int quota_debug;
#endif
