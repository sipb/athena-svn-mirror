/* $Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/config.h,v 1.4 1993-05-10 13:41:41 vrt Exp $ */
/* $Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/config.h,v $ */
/* $Author: vrt $ */

/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#include "mit-copyright.h"

/* The following is to allow a consistant Makefile accross platform */
#ifndef __CONFIG_H__
#define __CONFIG_H__

/* Define NDBM if your system uses NDBM */
#if (!defined(ultrix) && (defined(vax) || defined(ibm032))) || defined(Ultrix40) || defined(_IBMR2) || defined(SOLARIS)
#define NDBM 
#endif

#define DEFACLFILE	"/usr/spool/quota/acl"
#define DEFSACLFILE	"/usr/spool/quota/sacl"
#define DBM_DEF_FILE	"/usr/spool/quota/quota_db"
#define DBM_GDEF_FILE	"/usr/spool/quota/gquota_db"
#define DEFREPORTFILE	"/usr/spool/quota/report"
#define DEFCAPFILE      "/usr/spool/quota/quotacap"
#define LOGTRANSFILE    "/usr/spool/quota/logtrans"
#define SHUTDOWNFILE	"/usr/spool/quota/shutdown_for_backup"
#define DEFQUOTA        500
#define DEFCURRENCY     "cents"
#define DEFSERVICE	"athena"

#define QUOTASERVENT	3701	
#define QUOTASERVENTNAME "lpallow"

#define KLPQUOTA_SERVICE "rcmd"

#define STR_DB_MODE	0755	/* Mode for opening the strings database */
#define USER_DB_MODE	0755	/* Mode for opening the user database */
#define JOUR_DB_MODE	0755	/* Mode for opening the journal database */

#define PERIODICTIME	1	/* # sec's between checks on Quota DB. */
#define SAVETIME	3600	/* # sec's before deleting trans. log */

void PROTECT(), UNPROTECT(), CHECK_PROTECT();

#endif  /* __CONFIG__H__ */
