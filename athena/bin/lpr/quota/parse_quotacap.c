/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/parse_quotacap.c,v $
 *	$Author: epeisach $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/parse_quotacap.c,v 1.1 1991-09-25 10:24:18 epeisach Exp $
 */

/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */


#if (!defined(lint) && !defined(SABER))
static char parse_quotacap_rcsid[] = "$Id: parse_quotacap.c,v 1.1 1991-09-25 10:24:18 epeisach Exp $";
#endif (!defined(lint) && !defined(SABER))

#include "mit-copyright.h"
#include "quota.h"
#include <sys/param.h>

short KA;    /* Kerberos Authentication */
short MA;    /* Mutual Authentication   */
int   DQ;    /* Default quota */
char *AF;    /* Acl File */
char *QF;    /* Master quota file */
char *GF;    /* Group quota file */
char *RF;    /* Report file for logger to grok thru */
char *QC;    /* Quota currency */
char *SA;    /* SAcl File */
int  QD;    /* Quota server "down" (i.e. allow to print) */

char aclname[MAXPATHLEN];       /* Acl filename */
char saclname[MAXPATHLEN];      /* Service Acl filename */
char qfilename[MAXPATHLEN];     /* Master quota database */
char gfilename[MAXPATHLEN];     /* Group quota database */
char rfilename[MAXPATHLEN];     /* Report file */
char qcapfilename[MAXPATHLEN];  /* Required by quotacap routines */
char qcurrency[30];             /* The quota currency */
char quota_name[256];           /* Quota server name (for quotacap) */
int  qdefault;                  /* Default quota */

read_quotacap()
{
    char buf[BUFSIZ];
    register char *cp;
    int status;
    int i = 0;
    char *qgetstr();

    if (quota_name[0] == '\0') {
	while (getqent(buf) > 0) {
	    i++;
	    for (cp = buf; *cp; cp++)
		if (*cp == '|' || *cp == ':') {
		    *cp = '\0';
		    break;
		}
	    endqent();
	}
	if (i > 1) {
	    syslog(LOG_ERR, "Multiple quota_name entries in quotacap");
	    return(1);
	}
	if (i == 0) {
	    syslog(LOG_ERR, "Configuration file not found");
	    return(1);
	}
	(void) strncpy(quota_name, buf, 256);
    }
    if ((status = qgetent(buf, quota_name)) < 0) {
	syslog(LOG_ERR, "Unable to open quotacap file");
	return(1);
    } else if (status == 0) {
	syslog(LOG_ERR, "Unknown quota server (%s) in quotacap", quota_name);
	return(1);
    }

    /* Now we have the right quotacap entry, now get all the info */
    bp = buf;

    KA = qgetnum("ka");
    MA = qgetnum("ma");
    DQ = qgetnum("dq");
    if (DQ == -1)
	DQ = qdefault= DEFQUOTA;
    else
	qdefault = DQ;

    QD = qgetnum("qd");
    if (QD == -1)
	QD = 0;

    if ((AF = (char *)qgetstr("af", &bp)) == NULL) {
	AF = aclname;
    }
    if ((SA = (char *)qgetstr("sa", &bp)) == NULL) {
	SA = saclname;
    }
    if ((QF = (char *)qgetstr("qf", &bp)) == NULL) {
	QF = qfilename;
    }
    if ((GF = (char *)qgetstr("gf", &bp)) == NULL) {
	GF = gfilename;
    }
    if ((RF = (char *)qgetstr("rf", &bp)) == NULL) {
	RF = rfilename;
    }
    if ((QC = (char *)qgetstr("qc", &bp)) == NULL) {
	QC = DEFCURRENCY;
	(void) strcpy(qcurrency, DEFCURRENCY);
    }
    return(0);
}
