/* $Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/quota_db.h,v 1.1 1990-04-16 16:30:12 epeisach Exp $ */
/* $Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/quota_db.h,v $ */
/* $Author: epeisach $ */

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


/* typedef QuotaValue  PDquota_amount; */
/* typedef Integer     PDquota_allowed; */
/* typedef UTCTime     PDquota_time;    */

typedef int     quota_amount;
typedef int     quota_allowed;
typedef u_long  quota_time;
typedef int     quota_deleted;

typedef struct {
	char 		name[ANAME_SZ];		/* Kerb. name */
	char		instance[INST_SZ];	/* Kerb. instance */
	char 		realm[REALM_SZ];	/* Kerb. realm */
	char 		service[SERV_SZ];	/* Printer service type */
	quota_amount	quotaAmount;		/* Total usage */
	quota_amount	quotaLimit;		/* Max usage */
	quota_time    lastBilling;
	quota_amount  lastCharge;
	quota_amount  pendingCharge;
	quota_amount  lastQuotaAmount;
	quota_amount  yearToDateCharge;
	quota_allowed allowedToPrint;
	quota_deleted deleted;
	} quota_rec;

extern int default_quota;

#ifdef DEBUG
extern int quota_debug;
#endif
