/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/quota_logger.c,v $
 *	$Author: epeisach $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/quota_logger.c,v 1.1 1990-04-16 16:30:51 epeisach Exp $
 */

#if (!defined(lint) && !defined(SABER))
static char quota_logger_rcsid[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/quota_logger.c,v 1.1 1990-04-16 16:30:51 epeisach Exp $";
#endif (!defined(lint) && !defined(SABER))

#include "quota.h"
#include <stdio.h>
#include <krb.h>
#include "quota_limits.h"
#include "quota_ncs.h"
#include "quota_err.h"
#include "quota_db.h"

char *set_service();

/* These routines handle the printing out to the logger server info file */
/* Fatal errors exit. */

static FILE *fout;

static void open_acct()
{

  if ((fout = fopen(LOGTRANSFILE, "a")) == NULL) {
      syslog(LOG_INFO, "Unable to open accounting database called %s - FATAL", LOGTRANSFILE);
      exit(2);
  }
}

static void close_acct()
{
    if(fclose(fout) == EOF) {
      syslog(LOG_INFO, "Unable to close accounting database called %s - FATAL", LOGTRANSFILE);
      exit(2); 
      /* The device was probably full */
  }
}


void Quota_modify_log(qid, ad, qtype, qamount)
quota_identifier	*qid;
AUTH_DAT		*ad;
modify_user_type	qtype;
quota_value		qamount;
{
    char qprincipal[ANAME_SZ], qinstance[INST_SZ], qrealm[REALM_SZ]; /* Whose data was changed?*/	
    char *service;

    service = set_service(qid->service);
    parse_username(qid->username, qprincipal, qinstance, qrealm);

    open_acct();
    PROTECT();
    fprintf(fout, "t=%ld n=%s",time((long *)0), qprincipal);

#define ADDSTR(ch, str) if(str != NULL && str[0]!= '\0') \
    fprintf(fout, " %c=%s", ch, str);

    ADDSTR('i', qinstance);
    ADDSTR('r', qrealm);
    ADDSTR('s', service);
    ADDSTR('N', ad->pname);
    ADDSTR('I', ad->pinst);
    ADDSTR('R', ad->prealm);
    fprintf(fout, " a=%d", qamount);
    switch ((int) qtype) {
    case U_SET:
	ADDSTR('f', "SET");
	break;
    case U_ADD:
	ADDSTR('f', "ADD");
	break;
    case U_SUBTRACT:
	ADDSTR('f', "SUBTRACT");
	break;
    case U_ALLOW_PRINT:
	ADDSTR('f', "ALLOW");
	break;
    case U_DISALLOW_PRINT:
	ADDSTR('f', "DISALLOW");
	break;
    case U_DELETE:
	ADDSTR('f', "DELETE");
	break;
    case U_ADJUST:
	ADDSTR('f', "ADJUST");
	break;
	
#undef ADDSTR
    }
    fputc('\n', fout);
    close_acct();
    UNPROTECT();
}

void Quota_report_log(qid, qreport)
quota_identifier	*qid;
quota_report	  	*qreport;
{

    char qprincipal[ANAME_SZ], qinstance[INST_SZ], qrealm[REALM_SZ]; /* Whose data was changed?*/	
    char *service;

    service = set_service(qid->service);

    parse_username(qid->username, qprincipal, qinstance, qrealm);

    open_acct();
PROTECT();
    fprintf(fout, "t=%ld n=%s",time((long *) 0), qprincipal);

#define ADDSTR(ch, str) if(str != NULL && str[0]!= '\0') \
    fprintf(fout, " %c=%s", ch, str);
#define ADDNUM(ch, num) if(num != 0) fprintf(fout, " %c=%d", ch, num);

    ADDSTR('i', qinstance);
    ADDSTR('r', qrealm);
    ADDSTR('s', service);
    ADDSTR('f', "CHARGE");
    ADDNUM('p', qreport->pages);
    ADDNUM('q', qreport->ptime);
    ADDNUM('c', qreport->pcost);
    ADDSTR('w', qreport->pname);
    
    fputc('\n', fout);
    close_acct();
    UNPROTECT();
}
