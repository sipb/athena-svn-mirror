/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/qmain.c,v $
 *	$Author: epeisach $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/qmain.c,v 1.1 1990-04-16 16:29:05 epeisach Exp $
 */

#if (!defined(lint) && !defined(SABER))
static char qmain_rcsid[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/qmain.c,v 1.1 1990-04-16 16:29:05 epeisach Exp $";
#endif (!defined(lint) && !defined(SABER))

#include "quota.h"
#include <sys/param.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <krb.h>
#include "quota_limits.h"
#include "quota_ncs.h"
#include <pfm.h>
#include "quota_db.h"
#include "logger.h"
#include "logger_ncs.h"
#include "signal.h"

short KA;    /* Kerberos Authentication */
short MA;    /* Mutual Authentication   */
int   DQ;    /* Default quota */
char *AF;    /* Acl File */
char *QF;    /* Master quota file */
char *RF;    /* Report file for logger to grok thru */
char *QC;    /* Quota currency */
char *SA;    /* SAcl File */

char *progname;
char pbuf[BUFSIZ/2];
char *bp = pbuf;

#ifdef DEBUG
int quota_debug=0;
int logger_debug=0;
#endif

char aclname[MAXPATHLEN];       /* Acl filename */
char saclname[MAXPATHLEN];       /* Service Acl filename */
char qfilename[MAXPATHLEN];     /* Master quota database */
char rfilename[MAXPATHLEN];     /* Report file */
char qcapfilename[MAXPATHLEN];  /* Required by quotacap routines */
char qcurrency[30];             /* The quota currency */
char quota_name[256];           /* Quota server name (for quotacap) */
int  qdefault;                  /* Default quota */
char my_realm[REALM_SZ];

/* These should be configured at startup - hardcode for now */
static char *stringFile = "/usr/spool/quota/string.db";
static char *userFile = "/usr/spool/quota/user.db";
static char *jourFile = "/usr/spool/quota/journal.db";


main(argc, argv)
        int argc;
        char **argv;
{

#ifndef DEBUG
    int f;
#endif

/* Only argument acceptable is the acl filename. Default to something usefull 
   if not found */

    int qfd;	/* Query filedescriptor */
    int child, logger;

    progname = argv[0];
    argv++;
    aclname[0] = saclname[0] = qfilename[0] = rfilename[0] = quota_name[0] = '\0';
    strcpy(aclname, DEFACLFILE);
    strcpy(saclname, DEFSACLFILE);
    strcpy(qfilename, DBM_DEF_FILE);
    strcpy(qcapfilename, DEFCAPFILE);

    while(--argc) {
	if(argv[0][0] != '-')
	    usage(); /* Never returns */
	switch(argv[0][1]) {
	default:
	    usage(); /* Never returns */
	    break;
	case 'a':
	    if(!argv[0][2]) argc--, argv++;
	    if(argc) strcpy(aclname, argv[0]);
	    else usage();	/* Doesn't return */
	    break;
	case 's':
	    if(!argv[0][2]) argc--, argv++;
	    if(argc) strcpy(saclname, argv[0]);
	    else usage();	/* Doesn't return */
	    break;
	case 'n':
	    if(!argv[0][2]) argc--, argv++;
	    if(argc) strcpy(quota_name, argv[0]);
	    else usage();	/* Doesn't return */
	    break;
	case 'q':
	    if(!argv[0][2]) argc--, argv++;
	    if(argc) strcpy(qfilename, argv[0]);
	    else usage();	/* Doesn't return */
	    break;
	case 'c':
	    if(!argv[0][2]) argc--, argv++;
	    if(argc) strcpy(qcapfilename, argv[0]);
	    else usage();	/* Doesn't return */
	    break;
	}
	argv++;
    } /* while argc */

    if (read_quotacap()) {
	fprintf(stderr, "Unable to open/read quotacap file %s - FATAL\n",
		qcapfilename);
	exit(1);
    }
    
/* Arguments now parsed  - Iitialize */
#ifdef DEBUG 
    printf("SAclfilename: %s\n", saclname);
    printf("Aclfilename : %s\n", aclname);
    printf("Qfilename   : %s\n", qfilename);
    printf("Quotacap    : %s\n", qcapfilename);
#endif
    
    if(access(aclname, R_OK)) {
	fprintf(stderr,"%s: could not read %s\n", progname, aclname);
	exit(2);
    }

    if(access(saclname, R_OK)) {
	fprintf(stderr,"%s: could not read %s\n", progname, saclname);
	exit(2);
    }
    
    /* Should initialize database here XXX */
    if(quota_db_set_name(qfilename)) {
	fprintf(stderr,"%s: could not set database name %s\n", progname, qfilename);
	exit(2);
    }

    setpgrp(0, getpid());
    /* If this fails then we're not root... */
    krb_get_lrealm(my_realm, 1);
    
    openlog("lpqd", LOG_PID, LOG_LPR);
    
#ifndef DEBUG
    for (f = 0; f < 5; f++)
	(void) close(f);
    (void) open("/dev/null", O_RDONLY);
    (void) open("/dev/null", O_WRONLY);
    (void) dup(1);
#endif
    
    if(!(child=fork())) {
	if(create_query_socket(&qfd))  {
	    syslog(LOG_ERR, "Could not bind to socket");
	    exit(1);
	}
	handle_requests(qfd);
	/* Should never return */
	exit(1);
    }

    if(!(logger=fork())) {
	init_lncs();
	/* Should never return... */
	exit(2);
    }

    init_qncs();
    /* Should never return */
    exit(1);
}

read_quotacap()
{
    char buf[BUFSIZ];
    register char *cp;
    int status;
    int i = 0;

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
	strncpy(quota_name, buf, 256);	    
    } else {
	if ((status = qgetent(buf, quota_name)) < 0) {
	    syslog(LOG_ERR, "Unable to open quotacap file");
	    return(1);
	} else if (status == 0) {
	    syslog(LOG_ERR, "Unknown quota server (%s) in quotacap", quota_name);
	    return(1);
	}
    }

    /* Now we have the right quotacap entry, now get all the info */

    bp = buf;

    KA = qgetnum("ka");
    MA = qgetnum("ma");
    DQ = qgetnum("dq");
    if (DQ = -1)
	DQ = qdefault= DEFQUOTA;

    if ((AF = (char *)qgetstr("af", &bp)) == NULL) {
	AF = aclname;
    }

    if ((SA = (char *)qgetstr("sa", &bp)) == NULL) {
	SA = aclname;
    }

    if ((QF = (char *)qgetstr("qf", &bp)) == NULL) {
	QF = qfilename;
    }
    if ((RF = (char *)qgetstr("rf", &bp)) == NULL) {
	RF = rfilename;
    }
    if ((QC = (char *)qgetstr("qc", &bp)) == NULL) {
	QC = DEFCURRENCY;
	strcpy(qcurrency, DEFCURRENCY);
    }
    return(0);
}

usage()
{
    fprintf(stderr, "%s: [-a aclfile] [-q quota_database_file]\n", progname);
    fprintf(stderr, "    [-c quotacap] [-n quota_name] [-s saclfile]\n",
	    progname);
    exit(1);
}

create_query_socket(fd) 
int *fd;
{

    struct sockaddr_in sin_c;
    struct servent *servname;
    
    if((*fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	syslog(LOG_ERR,"Could not create socket - FATAL\n");
	return -1;
    }
    
    bzero((char *)&sin_c, sizeof(sin_c));
    servname = getservbyname(QUOTASERVENTNAME,"udp");
    if(!servname) 
	sin_c.sin_port = htons(QUOTASERVENT);
    else 
	sin_c.sin_port = servname->s_port;
    if (bind(*fd, &sin_c, sizeof(sin_c)) < 0) {
	syslog(LOG_ERR,"Could not bind - FATAL\n");
	return -1;
    }

    /* Everything worked. Return */
    return 0;
}

handle_requests(fd) 
int fd;
    {
	int addrlen;
	struct sockaddr addr;
	char buf[1024], outbuf[1024];
	fd_set	fdread;
	char serv[SERV_SZ], princ[ANAME_SZ], inst[INST_SZ], realm[REALM_SZ];
	char name[MAX_K_NAME_SZ];
	int acct, more, retval, ret;
	char *service, *set_service();
	quota_rec quotarec;

	while(1) {
	    FD_ZERO(&fdread);
	    FD_SET(fd, &fdread);
	    if(select(fd+1, &fdread, 0, 0, NULL) <=0) {
		syslog(LOG_NOTICE, "Select failed\n");
		continue;
	    }

	    if(!FD_ISSET(fd, &fdread)) {
		syslog(LOG_NOTICE, "Bogus response received to quotaserver\n");
		continue;
	    }

	    /* Handle the UDP packet from the lpd server. */
	    addrlen = sizeof(addr);
	    if (recvfrom(fd, buf, 1024, 0, &addr, &addrlen) < 0) {
		syslog(LOG_NOTICE, "Error in recvfrom\n");
		continue;
	    }
	    if(buf[0] == 0) {
		/* PING protocol */
		/* Copy vesion and sequence & timestamp back */
		bcopy(buf, outbuf, 13);	
		if(sendto(fd, outbuf, 13, 0, &addr, addrlen) != 13) {
		    syslog(LOG_NOTICE, "sendto ping failed\n");
		    continue;
		}
		continue;
	    }

	    if(buf[0] != UDPPROTOCOL) {
		syslog(LOG_ERR, "Wrong protocol sent.\n");
		continue;
	    }

	    /* Handle lookup request */
	    /* Start by copying to outbuf whatever can... */
	    /* Copy Protocol, seq #, printer name */
	    bcopy(buf, outbuf, 36);
	    bcopy(buf + 35, &acct, 4);
	    bcopy(buf + 39, serv, 20);
	    bcopy(buf + 59, princ, ANAME_SZ);
	    bcopy(buf + 59 + ANAME_SZ, inst, INST_SZ);
	    bcopy(buf + 59 + ANAME_SZ + INST_SZ, realm, REALM_SZ);
	    acct = htonl(acct);
	    service = set_service(serv);	    

	    /* Frob the realms and instance properly... */
	    make_kname(princ, inst, realm, name);
	    parse_username(name, princ, inst, realm);
	    /* Now lookup!!! */
	    retval = quota_db_get_principal(princ, inst,
					    service,
					    realm, &quotarec,
					    (unsigned int)1, &more);
	    
	    ret = ALLOWEDTOPRINT;
	    if(!retval) {
		/* User does not exist */
		ret = UNKNOWNUSER;
	    } else if(quotarec.allowedToPrint == FALSE)
		ret = NOALLOWEDTOPRINT;

	    outbuf[35] = (char) ret;
	    if(sendto(fd, outbuf, 36, 0, &addr, addrlen) != 36) {
		syslog(LOG_NOTICE, "sendto lookup failed\n");
	    }
	    continue;
		
	}
}		
	
Exit()
{
    killpg(getpid(), SIGKILL);
    exit(0);
}

char *error_text(st)
status_$t st;
{
    static char buff[200];
    extern char *error_$c_text();
    
    return (error_$c_text(st, buff, sizeof buff));
}

init_qncs() 
{
    status_$t st, fst;
    pfm_$cleanup_rec crec;
    socket_$addr_t loc;
    unsigned long llen = sizeof(loc);

    fst = pfm_$cleanup(crec);
    if (fst.all != pfm_$cleanup_set) {
	if (fst.all != status_$ok)
	    syslog(LOG_NOTICE, "*** Exception raised - %s\n", error_text(fst));
	pfm_$signal(fst);
    }
    rpc_$use_family_wk (socket_$internet, &print_quota_v1$if_spec,
			&loc, &llen, &st);
    
    if (st.all != 0) {
	syslog(LOG_ERR,"Can't use_family - %s\n", error_text(st));
	exit(1);
    }

    register_quota_manager_v1();

    signal(SIGHUP, Exit);
    signal(SIGINT, Exit);
    signal(SIGKILL, Exit);


#if notneeded
    rpc_$register (&print_quota_v1$if_spec, print_quota_v1$server_epv, &st);

    if (st.all != 0) {
	syslog(LOG_ERR, "Can't register - %s\n", error_text(st));
	exit(1);
    }
#endif
    rpc_$listen (1, &st);
}

init_lncs() 
{
    status_$t st, fst;
    pfm_$cleanup_rec crec;
    socket_$addr_t loc;
    unsigned long llen = sizeof(loc);
    void logger_periodic();

    if(logger_string_set_name(stringFile)) {
	syslog(LOG_INFO, "Unable to open string file database %s.", stringFile);
	exit(1);
    }

    if(logger_read_strings() < 0 ) {
	syslog(LOG_INFO, "Unable to read string file database %s: %s", 
	       stringFile, 
	       (errno > sys_nerr) ? (char *) "Unknown error" : sys_errlist[errno]);
	exit(1);
    }

    if(logger_user_set_name(userFile)) {
	syslog(LOG_INFO, "Unable to open user file database %s.", userFile);
	exit(1);
    }

    if(logger_journal_set_name(jourFile)) {
	syslog(LOG_INFO, "Unable to open user file database %s.", jourFile);
	exit(1);
    }


    fst = pfm_$cleanup(crec);
    if (fst.all != pfm_$cleanup_set) {
	if (fst.all != status_$ok)
	    syslog(LOG_NOTICE, "*** Exception raised - %s\n", error_text(fst));
	pfm_$signal(fst);
    }
    rpc_$use_family_wk (socket_$internet, &print_logger_v1$if_spec,
			&loc, &llen, &st);
    
    if (st.all != 0) {
	syslog(LOG_ERR,"Can't use_family - %s\n", error_text(st));
	exit(1);
    }

    rpc_$periodically(logger_periodic, "Periodic logger handler", PERIODICTIME);

    register_logger_manager_v1();

#ifdef notused
    rpc_$register (&print_logger_v1$if_spec, print_logger_v1$server_epv, &st);

    if (st.all != 0) {
	syslog(LOG_ERR, "Can't register - %s\n", error_text(st));
	exit(1);
    }
#endif
    rpc_$listen (1, &st);
    while(1) ;

}

static int protected = 0;
void PROTECT() { if(!(protected++)) pfm_$inhibit();}
void UNPROTECT() { if(!(--protected)) pfm_$enable();}
void CHECK_PROTECT() {
    if(protected) {
	syslog(LOG_INFO, "Integrity lock not disabled");
	protected = 0;
	pfm_$enable();
    }
}

#ifdef DEBUG	
test_udp(fd)
int fd;

    {	
	int addrlen;
	struct sockaddr addr;
	int cc;
	char buf[1024];
	fd_set	fdset;

	while( 1) {
	    bzero(buf, 1024);
	    FD_ZERO(&fdset);
	    FD_SET(fd, &fdset);
	    if(select(fd+1, &fdset, 0, 0, NULL) <=0) {
		syslog(LOG_ERR, "Select failed\n");
	    }

	    addrlen = sizeof(addr);
	    if ((cc = recvfrom(fd, buf, 1024, 0, &addr, &addrlen)) < 0) {
		syslog(LOG_ERR, "Error in recvfrom\n");
		continue;
	    }
	    
	}
    }
#endif
