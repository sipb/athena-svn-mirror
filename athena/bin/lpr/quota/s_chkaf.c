/*
 * s_chkaf.c
 *
 * This set of routines periodically checks the accounting files and reports
 * any changes to the quota server.
 *
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/s_chkaf.c,v 1.12 1992-04-19 21:29:09 epeisach Exp $
 */

/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

/* TODO:
 *
 *
 * The process() routine needs to be written (it should take into account
 * log files turning over and a one-cycle unavailability of a file).
 */

#ifndef lint
static char rcsid[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/s_chkaf.c,v 1.12 1992-04-19 21:29:09 epeisach Exp $";
#endif

/* We define this so it will be undefined later.. sys/dir.h has an error (sigh)*/
#define DIRSIZ this is garbage
#include "mit-copyright.h"
#include "config.h"
#include "lp.h"
#include <krb.h>
#include "quota.h"
#include "quota_limits.h"
#include "quota_ncs.h"
#include "pfm.h"
#include <strings.h>
#include <signal.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <com_err.h>
#include "quota_err.h"

#define MAXPRINTERS	30		/* Maximum number of printers */
#define WAKEUP		180		/* Interval for checking acct. file */
#define CACHEFILE	"/usr/spool/printer/klpd.cache"
#define CACHEFILENEW	"/usr/spool/printer/klpd.cache.new"
#define CACHEDIR	"/usr/spool/printer"

int init();
int wakeup();
int process();
int sendacct();
int create_binding();
char *error_text();
char *mktemp();
long time();

char	*prog;


struct p_stat {
    char	af[MAXPATHLEN];		/* Accounting file */
    char	rq[MAXHOSTNAMELEN];	/* Remote quota server */
    char	qs[SERV_SZ];		/* Service for printer */
    char	printer[BUFSIZ];	/* Printer name */
    FILE	*fp;			/* File pointer */
    long	val;			/* Unique identifier */
    long	pos;			/* File seek position */
} p_stat[MAXPRINTERS];

char myhostname[MAXHOSTNAMELEN];
char my_realm[REALM_SZ] = "\0";
char *tktfilename = "/tmp/tkprintXXXXXX";

main(argc,argv)
int argc;
char *argv[];
{
#if defined(ultrix) || defined(_IBMR2)
    void (* savealarm)();
    void cleanup();
#else
    int (* savealarm)();
    int cleanup();
#endif
    unsigned oldalarmtime;
    status_$t fst;
    static pfm_$cleanup_rec crec;
    char *p;

    prog = ((prog = rindex(argv[0],'/')) ? ++prog : argv[0]);
    if(argc != 1) {
	fprintf(stderr, "%s: invalid argument\n");
	exit(1);
    }
    if (init()) {
	exit(1);
    }

    initialize_quot_error_table();

    pfm_$init(pfm_$init_signal_handlers);
    fst = pfm_$cleanup(crec);
    (void) signal (SIGILL, SIG_DFL);
    (void) signal (SIGSEGV, SIG_DFL);
    (void) signal (SIGBUS, SIG_DFL);
    (void) signal (SIGINT, cleanup);
    (void) signal (SIGHUP, cleanup);
    (void) signal (SIGQUIT, cleanup);
    (void) signal (SIGTERM, cleanup);
    if (fst.all != pfm_$cleanup_set) {
	syslog(LOG_ERR, "*** Exception raised - %s\n", error_text(fst));
	syslog(LOG_ERR, "Error # %d\n", fst.all);
#if 0
	pfm_$rls_cleanup(crec, st);
#endif
	exit(2);
    }
    
    /* Set the kerberos ticket file */

    if(setenv("KRBTKFILE", mktemp(tktfilename), 1)) {
       fprintf(stderr, "Could not set KRBTKFILE environment\n");
       exit(1);
   }

    /* Get the host name, and make sure it's only the first comp. */
    if(gethostname(myhostname, MAXHOSTNAMELEN)) {
	syslog(LOG_ERR, "Unable to determine own hostname");	
	exit(1);
    }
    if ((p=index(myhostname, '.')) != NULL)
	*p = '\0';

    if(access(CACHEDIR, R_OK)) {
	syslog(LOG_ERR, "s_chkaf: Cannot access %s - exiting\n", CACHEDIR);
	exit(1);
    }

#ifndef NOFORK
    else {
	switch (fork()) {
	case -1:
	    syslog(LOG_ERR, "Unable to fork: %m");
	    exit(2);
	case 0:
	    break;
	default:
	    exit(0);
	}
    }
#endif

#if defined(LOG_DAEMON) &&  defined(LOG_LPR) && defined(LOG_NOWAIT)
    (void) openlog(prog, LOG_LPR|LOG_NOWAIT, LOG_DAEMON|LOG_INFO);
    (void) setlogmask(~0);
#else
    (void) openlog(prog, 0);
#endif

    while(1) {
	(void) process();
	
	oldalarmtime = alarm((unsigned) 0);
	savealarm = signal(SIGALRM, SIG_DFL);
        sleep(WAKEUP);
	(void) signal(SIGALRM, savealarm);
	(void) alarm(oldalarmtime);
    }
}

int
init() {
    int cnt = 0;
    int i, status, ret;

    char buf[BUFSIZ], buf1[BUFSIZ];
    char *bp = buf1;	/* A temp buffer for string work */

    char test_buf[BUFSIZ];
    char test_rq[BUFSIZ];
    char test_af[BUFSIZ];
    long test_val;
    long test_pos;
    
    char *RQ;
    char *AF;
    char *QS;

    FILE *cache_fp;
    struct stat sbuf;

    if ((cache_fp = fopen(CACHEFILE, "r")) == NULL) {
	syslog(LOG_INFO, "Cache file, %s, not found", CACHEFILE);
    }
    
    /* Loop through all the entries in the printcap file */
    while ((ret=getprent(buf)) && (ret != -1)) {
	bp = buf1;
	if ((RQ = pgetstr("rq", &bp)) != NULL
	    && (AF = pgetstr("af", &bp)) != NULL) {
	    /* Make sure we don't already have this entry */
	    for (status=0, i=0; i<cnt; i++) {
		if (!strcasecmp(RQ, p_stat[i].rq) &&
		    !strcmp(AF, p_stat[i].af)) {
		    status = 1;
		    break;
		}
	    }
	    if (status)
		continue;
	    if (cnt >= MAXPRINTERS) {
		syslog(LOG_ERR, "Too many printers (>%d) to monitor",
		       MAXPRINTERS);
		(void) fclose(cache_fp);
		return(1);
	    }

	    /* Add this entry to the table */
	    (void) strcpy(p_stat[cnt].rq, RQ);
	    (void) strcpy(p_stat[cnt].af, AF);

	    /* Get the service and media cost */
	    if ((QS = pgetstr("qs", &bp)) != NULL)
		(void) strcpy(p_stat[cnt].qs,QS);
	    else 
		p_stat[cnt].qs[0] = '\0';

	    /* Extract the printer name */
	    for (status=i=0; i < sizeof(p_stat[cnt].printer); i++) {
		switch(buf[i]) {
		case '|':
		case ':':
		case '\n':
		case '\0':
		    status = 1;
		    p_stat[cnt].printer[i] = '\0';
		    break;
		default:
		    p_stat[cnt].printer[i] = buf[i];
		    break;
		}
		if (status)
		    break;
	    }

	    /* Get a file pointer for the acct. file */
	    if ((p_stat[cnt].fp = fopen(AF, "r")) == NULL) {
		syslog(LOG_WARNING, "Unable to open accounting file %s", AF);
	    }

	    /* Set the seek position */
	    p_stat[cnt].val = 0L;
	    p_stat[cnt].pos = 0L;
	    if (cache_fp != NULL) {
		(void) rewind(cache_fp);
		while (fgets(test_buf, BUFSIZ, cache_fp) != (char *)NULL) {

		    if (test_buf[strlen(test_buf)-1] != '\n') {
			syslog(LOG_ERR, "Line too long in cachefile");
			return(1);
		    }
		    (void) sscanf(test_buf, "%s %s %d %d",
				  test_af, test_rq, &test_val, &test_pos);
		    if (!strcasecmp(RQ, test_rq) &&
			!strcmp(AF, test_af)) {

			if (fstat(fileno(p_stat[cnt].fp), &sbuf)) {
			    if (p_stat[cnt].fp) {
				/*
				 * We were able to open the file before,
				 * so we should be able to stat the fd.
				 * Since we can't, there's a problem!
				 */
				syslog(LOG_ERR, "Unable to stat %s", AF);
				return(1);
			    } else {
				/*
				 * The file wasn't available before.
				 * Perhaps files are being shifted around.
				 * Take the conservative approach and set
				 * the file position to its previous value.
				 * When things are back to normal, we will
				 * detect any changes.
				 */
				p_stat[cnt].val = test_val;
				p_stat[cnt].pos = test_pos;
			    }
			} else {
			    /*
			     * Make sure the file has not changed radically.
			     * Test the size to make sure it has not shrunk.
			     * Make sure the unique identifier matches the
			     * cached value.
			     */
			    if (test_pos <= sbuf.st_size &&
				test_val == p_stat[cnt].val) {

				p_stat[cnt].pos = test_pos;
				(void) fseek(p_stat[cnt].fp, test_pos, 0);
			    }
			}

			break;
		    }
		} /* while */
	    }
	    if(p_stat[cnt].fp) (void) fclose(p_stat[cnt].fp);
	    cnt++;
	}
    } /* while */
    
    /* Now we have all the positions in memory
     * check all entries with pos=0 and if an accounting
     * file already exists, set the position to the end
     * of that file 
     */
    for (i = 0; i < cnt; i++) {
	if ((p_stat[i].pos == (long)0) && 
	    (!fstat(fileno(p_stat[i].fp), &sbuf))) 
	    p_stat[i].pos = sbuf.st_size;
    }

    if(ret == -1) {
	syslog(LOG_ERR, "s_chkaf: printcap file not found");
	fprintf(stderr, "s_chkaf: printcap file not found\n");
	exit(1);
    }
    endprent();
    if(cache_fp != NULL) (void) fclose(cache_fp);

    while (cnt < MAXPRINTERS) {
	(void) bzero((char *)(&p_stat[cnt]), sizeof(struct p_stat));
	cnt++;
    }
    return(0);
}


int
process() {
    char *down_servers[MAXPRINTERS];
    FILE *cache_fp;
    int i, j, status, sdown;
    int down = 0;
    int mediacost, acctno, totalprinted;
    long timeprinted;

    char line[BUFSIZ];
    int st_pages;
    char st_user[BUFSIZ];

#if defined(DEBUG) && defined(NOFORK)
    fprintf(stderr, "Starting process\n");
#endif
    /* Assume all servers are back up now */
    for (i=0; i<MAXPRINTERS; i++) {
	down_servers[i] = (char *)NULL;
    }

    if ((cache_fp = fopen(CACHEFILENEW, "w+")) == NULL)
	syslog(LOG_ERR, "Unable to write to cachefile");
    
    /* Check to see if any of the acct. files have changed */
    for (i=0; i<MAXPRINTERS; i++) {
	if (!p_stat[i].af[0])
	    continue;

#if defined(DEBUG) && defined(NOFORK)
	fprintf(stderr, "Checking %s\n", p_stat[i].af);
#endif

	/* Get a file pointer for the acct. file */
	if ((p_stat[i].fp = fopen(p_stat[i].af, "r")) == NULL) {
#if defined(DEBUG) && defined(NOFORK)
	fprintf(stderr, "Unable to open %s\n", p_stat[i].af);
#endif
	    syslog(LOG_WARNING, "Unable to open accounting file %s", p_stat[i].af);
	    continue;
	}
	/* If file is smaller, then was moved out of the way */
	(void) fseek(p_stat[i].fp, p_stat[i].pos, 0);
	if(ftell(p_stat[i].fp) < p_stat[i].pos) {
	    p_stat[i].val = 0L;
	    p_stat[i].pos = 0L;
	    syslog(LOG_INFO, "Account file turned over %s\n", p_stat[i].af);
	}		
	sdown=0;
	if (p_stat[i].fp) {
	    for (status=0, j=0; j<down; j++)
		if (!strcasecmp(down_servers[j], p_stat[i].rq)) {
		    sdown = 1;
		    break;
		}
#if defined(DEBUG) && defined(NOFORK)
	fprintf(stderr, "About to read line from %s\n", p_stat[i].af);
#endif

	    /* Read any new data out of the file and send it... */
	    status = 0;
	    while (sdown==0 && fgets(line, BUFSIZ, p_stat[i].fp)) {
#if defined(DEBUG) && defined(NOFORK)
		    fprintf(stderr, "Read line:%s", line);
#endif
		j = strlen(line);
		if (line[j-1] != '\n') {
		    if (j >= BUFSIZ-1) {
			/* Line too long */
			/* BUG: this may be syslogged multiple times if the
			 * last line is too long AND incomplete.  The fix
			 * adds too much complexity... */
			if (!status)
			    syslog(LOG_ERR, "Acct. line too long: %s", line);
			status = 1;
			continue;
		    } else {
			/* Incomplete last line... restore the seek position */
			(void) fseek(p_stat[i].fp, p_stat[i].pos, 0);
			break;
		    }
		}
		if (status) {
		    /* We finally cleared out the huge line... */
		    p_stat[i].pos = ftell(p_stat[i].fp);
		    status = 0;
		    continue;
		}

#if 0
		if (sscanf(line, "%d%*[^:]:%s", &st_pages, st_user) != 2)
		    syslog(LOG_ERR, "Error parsing acct. line: %s", line);
#endif
		if (sscanf(line, "%d%*[^:]:%s%d%d%d%d",
			   &totalprinted, st_user, &timeprinted,
			   &acctno, &mediacost, &st_pages) != 6) {
		    syslog(LOG_ERR, "Error parsing acct. line: %s", line);
		    continue;
		}
#ifdef DEBUG
		fprintf(stderr,"Charging:%d:%s:%d:%d:%d:%d\n",
			   totalprinted, st_user, timeprinted,
			   acctno, mediacost, st_pages);
#endif
		if (status = sendacct(p_stat[i].rq, p_stat[i].printer,
				      mediacost, st_pages, st_user,
				      acctno, p_stat[i].qs, timeprinted)) {
		    /* Unable to send the data... restore the seek position */
		    (void) fseek(p_stat[i].fp, p_stat[i].pos, 0);
		    down_servers[down++] = p_stat[i].rq;
		    break;
		} else {
		    /* We sent the data... update the seek position */
		    p_stat[i].pos = ftell(p_stat[i].fp);
		}
		/* Reset location */
		/* There was an error, so don't check for a log turnover yet */
		if (status)
		    break;
	    } /* while */
	    
	    /* This is to hopefully convince that we are no longer at EOF */
	    (void) fseek(p_stat[i].fp, p_stat[i].pos, 0);
	}

	/* Write out the cachefile */
	if (cache_fp)
	    (void) fprintf(cache_fp, "%s %s %d %d\n",
			   p_stat[i].af, p_stat[i].rq,
			   p_stat[i].val, p_stat[i].pos);
	(void) fclose(p_stat[i].fp);

    }
    if (cache_fp) (void) fclose(cache_fp);
    if (cache_fp) (void) rename(CACHEFILENEW, CACHEFILE);
    for (i=0; i<MAXPRINTERS; i++) {
	if(p_stat[i].af && p_stat[i].fp) {
	    (void) fclose(p_stat[i].fp);
	    p_stat[i].fp = NULL;
	}
    }

#if defined(DEBUG) && defined(NOFORK)
    fprintf(stderr, "Returning from process\n");
#endif
    return(0);
}

struct {
    unsigned char hostname[MAXHOSTNAMELEN];
    handle_t handle;
    char realm[REALM_SZ];
} rpqauth[MAXPRINTERS];

int numquotaservers=0;

int
sendacct(rq, printer, mediacost, pages, principal, account, service, ptime)
char *rq;
char *printer;
int mediacost;
int pages;
char principal[];
int account;
char *service;
long ptime;
{
    /* Start by getting kerberos tickets for the rq. These are cached */
    /* We start by scanning the rqkauth for a particular host. */
    /* A case in insensitive match will be used */
    status_$t fst, st;
    pfm_$cleanup_rec crec;

    int i, ret;
    krb_ktext auth;
    char *realm;
    char *krb_realmofhost(), *krb_get_phost();
    quota_identifier qid;
    quota_report qrep;
    unsigned long chk;

    /* Fill in structures for report */
    (void) strcpy((char *) qid.username, principal);
    (void) strcpy((char *) qid.service, service);
    qid.account = account;

    qrep.pages = pages;
    qrep.pcost = mediacost;
    (void) strncpy((char *) qrep.pname, printer, PNAME_SZ);
    qrep.ptime = ptime;

    /* Don't bill for zero pages printed */
    if(pages == 0) return 0;

    /* First get my realm */
    if (!*my_realm)
	if ((ret = krb_get_lrealm(my_realm, 1)) != KSUCCESS) {
	    syslog(LOG_ERR,"krbrlm: %s",
		   krb_err_txt[ret]);
	    *my_realm = '\0';
	    return(1);
	}

    for(i=0; i < numquotaservers; i++) {
	if(!strcasecmp(rq, (char *) rpqauth[i].hostname)) break;
    }

    if(i == numquotaservers) {
	/* Add an entry into cache */
	(void) strcpy((char *) rpqauth[i].hostname, rq);
	if(!(realm = krb_realmofhost(rq))) {
	    syslog(LOG_ERR, "Cannot determine kerberos realm for %s", rq);
	    return 1;
	    }
	(void) strcpy(rpqauth[i].realm,realm);
	/* Create binding */
	if(create_binding((unsigned char *) rq, &rpqauth[i].handle))
	    /* Some sort of error, return w/o allocating */
	    return 1;
	numquotaservers++;
    }

    /* i is now the index of the entry */
    if(get_ktext(i, &auth))
	/* Something went wrong - return */
	return 1;

    /* We are now ready to make call to the remote server */

again:
    fst = pfm_$cleanup(crec);
    if (fst.all != pfm_$cleanup_set) {
	switch((int) fst.all) {
	case rpc_$comm_failure:
	    syslog(LOG_ERR, "Quota server down: %s", rpqauth[i].hostname);
	    goto failure;

	case rpc_$wrong_boot_time:
	    syslog(LOG_DEBUG, "Quota server restarted - rebinding");
	    rpc_$free_handle(rpqauth[i].handle, &st);
	    if(create_binding(rpqauth[i].hostname, &rpqauth[i].handle)) {
		syslog(LOG_CRIT, "Could not rebind to host %s - fatal|", rpqauth[i].hostname);
		exit(1);
	    }
	    /* Retry */
	    goto again;

	default:	
	    syslog(LOG_ERR, "*** Exception raised - %s\n", error_text(fst));
	    goto failure;
	}
    failure:
	pfm_$rls_cleanup(crec, st);
	return 1;
    }

    ret = QuotaReport(rpqauth[i].handle, &auth, &qid, &qrep, &chk);
    pfm_$rls_cleanup(crec, st);
    if(ret) {
	syslog(LOG_ERR, "Problem contacting quota server %s: %s", rpqauth[i].hostname, error_message(ret));
	return 1;
    }

    /* Check back auth */
    

    return(0);				/* Success */
}


char *error_text(st)
status_$t st;
{
    static char buff[200];
    extern char *error_$c_text();

    return (error_$c_text(st, buff, sizeof buff));
}

create_binding(hostname, h)
unsigned char *hostname;
handle_t *h;
{
    status_$t st;
    socket_$addr_t addr;
    unsigned long length = sizeof(addr);
    extern uuid_$t uuid_$nil;
    int port;
    struct servent *servname;

    servname = getservbyname(QUOTAQUERYENT,"udp");
    if(!servname) port = QUOTAQUERYPORT;
    else port = ntohs((u_short) servname->s_port);

    rpc_$name_to_sockaddr (hostname, strlen((char *) hostname), port, 
			   socket_$internet, &addr, &length, &st);
    if (st.all != 0) { 
	syslog(LOG_ERR, "Host not found: %s \n", hostname);
	return(1);
    }
    *h = rpc_$bind (&uuid_$nil, &addr, length, &st);
    if (st.all != 0) {
	syslog(LOG_ERR, "Couldn't bind handle: %s\n",
		error_text(st));
	return(1);
    }
    return 0;
}

get_ktext(ent, auth)
int ent;
krb_ktext *auth;
{
    int ret;
    CREDENTIALS cred;
    char hostname[MAXHOSTNAMELEN];

    (void) strcpy(hostname, krb_get_phost(rpqauth[ent].hostname));

    /* Lookup the credential for the said entry */
    if((ret = krb_get_cred(KLPQUOTA_SERVICE, hostname,
			   rpqauth[ent].realm, &cred)) == KSUCCESS) {
	/* The credential exists, see how long to live */
	if(cred.issue_date + (cred.lifetime * 5 * 60) > time((long *) 0)+120) {
	    /* Still valid */
	    /* John Kohl suggests the mk_req cause of static internals...*/
#if 0
	    bcopy(&(cred.ticket_st),auth,sizeof(KTEXT_ST));
	    fprintf(stderr, "I have valid tickets\n");
	    return 0;
#else
	    goto mkreq;
#endif
	}
    }

    /* We need a krb_mk_req - first check for new tgt */
    /* Make sure we have an up-to-date tgt */
    ret = krb_get_cred("krbtgt", my_realm, my_realm, &cred);
    if (ret != KSUCCESS || ((cred.issue_date + (cred.lifetime * 5 * 60)) <
			    time((long *) 0) + 120)) {
	/* If not, get one */
#ifdef DEBUG 
	syslog(LOG_DEBUG, "Requesting krbtgt.%s for rcmd.%s@%s",
	       my_realm, myhostname, my_realm);
#endif
	ret = krb_get_svc_in_tkt("rcmd", myhostname, my_realm,
				 "krbtgt", my_realm, 96, NULL);
	if (ret != KSUCCESS) {
	    syslog(LOG_ERR, "Could not get new tgt %s",krb_err_txt[ret]);
	    return 1;
	}
    }

mkreq:
    if((ret = krb_mk_req(auth, KLPQUOTA_SERVICE, hostname,
			 rpqauth[ent].realm, 0)) != KSUCCESS) {
	syslog(LOG_ERR, "krbmkreq to %s.%s@%s error %d", KLPQUOTA_SERVICE,
	       hostname ,rpqauth[ent].realm,ret);
	return 1;
    }
    /* auth filled in above */
    return 0;
}


/* Cleans up before exiting when a signal is trapped */
#if defined(ultrix) || defined(_IBMR2)
void 
#endif
cleanup()
{
    unlink(tktfilename);
    exit(0);
}
