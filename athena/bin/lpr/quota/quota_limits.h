/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#include "mit-copyright.h"

#ifndef _QUOTA_LIMIT_READ_
#define _QUOTA_LIMIT_READ_

#define SERV_SZ	20	/* Size of service name */
#define PNAME_SZ 30     /* Size of printer name */
#define CURRENCY_SZ 10  /* Size of currency */
#define MESSAGE_SZ 80   /* Size of message returned from quota query */

/* Limits for group accounting */
#define GQUOTA_MAX_ADMIN    20      /* No. of admins per group */
#define GQUOTA_MAX_USER     100     /* No. of users per group  */
#define G_ADMINMAXRETURN    20      /* Max no. of admin to return per rpc call */
#define G_USERMAXRETURN     50      /* Max no. of users to return per rpc call */

/* krb.h */
#ifndef MAX_K_NAME_SZ
#define MAX_K_NAME_SZ 122 /* From krb.h */
#endif

#define LOGMAXRETURN	100	/* Return a maximum of 100 entries/rpc call */
				/* This insures other requests get in... */

#define QUOTAQUERYPORT 3702
#define QUOTAQUERYENT "lpquery"

#define QUOTALOGGERPORT 3703
#define QUOTALOGENT "lplog"

/* Used for returning limits per rpc call for group account query */
#define GQUOTA_NONE        0
#define GQUOTA_MORE_USER   1
#define GQUOTA_MORE_ADMIN  2
#define GQUOTA_BOTH        3  /* equals GQUOTA_MORE_USER + GQUOTA_MORE_ADMIN */

#endif _QUOTA_LIMIT_READ_
