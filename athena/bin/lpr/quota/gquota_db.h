/* $Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/gquota_db.h,v 1.1 1990-11-08 09:52:38 epeisach Exp $ */
/* $Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/gquota_db.h,v $ */
/* $Author: epeisach $ */

/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#include "mit-copyright.h"


/*
 * Some useful defenitions for the Quota Server code
 */

#include "config.h"

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


/* Arguments to gquota_dbl_lock() */

#define GQUOTA_DBL_EXCLUSIVE 1
#define GQUOTA_DBL_SHARED 0

/* arguments to gquota_db_set_lockmode() */

#define GQUOTA_DBL_BLOCKING 0
#define GQUOTA_DBL_NONBLOCKING 1

typedef int     gquota_amount;
typedef int     gquota_allowed;
typedef u_long  gquota_time;
typedef int     gquota_deleted;

typedef struct {
	long           account;
	char 	       service[SERV_SZ];	    /* Printer service type */
	long           admin[GQUOTA_MAX_ADMIN + 1]; /* Max. no of admins */
	long           user[GQUOTA_MAX_USER + 1];   /* Max. no of users  */
	gquota_amount  quotaAmount;		    /* Total usage */
	gquota_amount  quotaLimit;		    /* Max usage */
	gquota_time    lastBilling;
	gquota_amount  lastCharge;
	gquota_amount  pendingCharge;
	gquota_amount  lastQuotaAmount;
	gquota_amount  yearToDateCharge;
	gquota_allowed allowedToPrint;
	gquota_deleted deleted;
	} gquota_rec;

extern int default_gquota;

#ifdef DEBUG
extern int gquota_debug;
#endif
