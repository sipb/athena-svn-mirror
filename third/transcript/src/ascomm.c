#ifndef lint
#define _NOTICE static char
_NOTICE N1[] = "Copyright (c) 1993 Adobe Systems Incorporated";
_NOTICE RCSID[] = "$Header: /afs/dev.mit.edu/source/repository/third/transcript/src/ascomm.c,v 1.1.1.1 1996-10-07 20:25:47 ghudson Exp $";
#endif
/*
 * ascomm.c
 *
 * Copyright (C) 1993 Adobe Systems Incorporated.
 * All rights reserved.
 *
 * lpr/lpd communications filter for Adobe network printer.
 *
 * ascomm gets called with:
 *   stdin = the file to print
 *   stdout = the printer device (ignored)
 *   stderr = the printer log file
 *   cwd = the spool directory
 *   argv = set up by interface shell script:
 *   filtername -P printer -p filtername -n username -h host [acctfile]
 *
 *
 */
/*
  RCSLOG
  $Log: not supported by cvs2svn $
 * Revision 1.6  1994/04/08  23:27:38  snichols
 * added sigignore for SIGPIPE, surrounded by ifdef SYSV since
 * not all BSD systems have sigignore, and the problem doesn't
 * happen there anyway.
 *
 * Revision 1.5  1994/04/08  23:15:14  snichols
 * more fixes for Solaris.
 *
 * Revision 1.4  1994/04/08  21:03:25  snichols
 * close fdinput as soon as we're done with it, to avoid SIGPIPE problems
 * on Solaris.
 *
 * Revision 1.3  1994/03/09  18:15:32  snichols
 * added support for not using the control protocol and not querying status.
 *
 * Revision 1.2  1994/02/16  22:41:54  snichols
 * ported to Solaris, plus some typos corrected.
 *
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <string.h>

#include "transcript.h"
#include "config.h"

#define TRUE 1
#define FALSE 0


static char *prog ="ascomm";
static char *host = " ";
static char *name = " ";
static char *pname = " ";
static char *accountingfile;

static long startpagecount;
static long endpagecount;

static char *getpages = "(%%[ pagecount: )print statusdict/pagecount \
get exec (                      ) cvs print ( ]%%)= flush";

static int socklen;
static struct sockaddr_in datasock;
static struct sockaddr_in statussock;
static struct sockaddr_in self;
static struct hostent *hp;

static char *hostname;
static int ppid;

static int fdsend;
static int fdinput;
static int fdcontrol;
static int fdstatus;

int control = 1;
int status = 1;

struct controlpacket {
    unsigned char ctrlchar;
    unsigned char dummy;
    u_short port;
} controldata;

static fd_set readfds;
static fd_set statfds;

static int doactng;
static int bannerfirst = 0;
static int bannerlast = 0;


/* job state variables */
typedef enum {
    init,			/* initialization */
    syncstart,			/* first acc'ting job */
    sending,			/* sending main job to printer */
    waiting,			/* waiting for EOF from printer */
    lastpart,			/* sending banner after job */
    synclast,			/* last acc'ting job */
    ending,			/* cleaning up */
    croaking,			/* abnormal exit */
    child,			/* child */
} states;

static states currentstate;

static int statusp;

static void JobControl();

#define PRINTER_PORT 9100
#define STATUS_PORT 9101
#define MYBUFSIZE 10240

#ifdef BDEBUG
#define debugp(x) {fprintf x ; (void) fflush(stderr);}
extern int errno;
#else
#define debugp(x)
#endif /* BDEBUG */

static int dataport = PRINTER_PORT;
static int statusport = STATUS_PORT;
static int localport;

static void cleanupandexit(code)
    int code;
{
    /* we want to clean and and leave; closing any open file descriptors,
       and possibly trying to send an end of session to the printer */

    debugp((stderr, "cleanupanexit send %d status %d control %d\n", fdsend,
	    fdstatus, fdcontrol));
    if (currentstate >= syncstart && currentstate < ending) {
	/* send a ctrl-D to clean up */
	controldata.ctrlchar = 0x04;
	controldata.port = htons(0);
	JobControl(controldata);
	
    }
    if (fdsend) close(fdsend);
    if (fdinput) close(fdinput);
    if (fdstatus) close(fdstatus);
    if (fdcontrol) close(fdcontrol);
    exit(code);
}

#ifdef BDEBUG
handler(sig)
    int sig;
{
    char buf[20];
    sprintf(buf, "terminated with signal %d\n", sig);
    write(2,buf, strlen(buf));
    _exit(2);
}
#endif

static void giveup(sig)
    int sig;
{
    /* child told us to die */
    debugp((stderr, "Got EMT sig! Die!.\n"));
    cleanupandexit(2);
}

static long ParseAnswer(p)
    char *p;
{
    char *r;
    char *q;

    r = strchr(p, ':');
    if (r == NULL)
	return 0;
    r++;
    while (*r == ' ') r++;
    /* now r should point to count */
    q = strchr(r, ' ');
    if (q == NULL)
	return 0;
    *q = '\0';
    return atoi(r);
}

static void AccountingJob(pagecount)
    long *pagecount;
{
    int cnt;
    char buf[BUFSIZ];
    int foo;
    int r;
    int percents = 0;
    struct linger linger;
    char *p;

    /* NOTE: this closes the connection! */

    foo = 1;
    if (setsockopt(fdsend, IPPROTO_TCP, TCP_NODELAY, (char *)&foo,
		   sizeof(foo)) < 0) {
	perror("sockcomm: setsockopt (no delay)");
	cleanupandexit(2);
    }

    debugp((stderr, "sending accounting job\n"));
    cnt = write(fdsend, getpages, strlen(getpages));
    linger.l_onoff = 0;
    linger.l_linger = 0;
    if (setsockopt(fdsend, SOL_SOCKET, SO_LINGER, (char *)&linger,
		   sizeof(linger)) 
	< 0) {
	perror("sockcomm: setsockopt(linger)");
	cleanupandexit(2);
    }
    shutdown(fdsend, 1);
    debugp((stderr, "sent %d bytes\n", cnt));
    p = buf;
    while ((cnt = read(fdsend, p, BUFSIZ)) > 0) {
	p += cnt;
    }
    *p = '\0';
    debugp((stderr, "%s\n", buf));
    *pagecount = ParseAnswer(buf);
    close(fdsend);
}

static void AcctEntry(start, end)
    long start, end;
{
    FILE *fp;

    debugp((stderr, "startpagecount %d endpagecount %d\n", startpagecount,
	    endpagecount));
    if (start > end || start < 0 || end < 0) {
	fprintf(stderr, "%s: accounting error: start %d end %d\n", prog,
		start, end);
	fflush(stderr);
    }
    else if ((fp = fopen(accountingfile, "a")) != NULL) {
	fprintf(fp, "%7.2f\t%s:%s\n", (float) (end - start), host, name);
	fclose(fp);
    }
}



static void ReopenConn()
{
    struct linger linger;
    struct sockaddr_in sockname;
    int namelen = sizeof(struct sockaddr_in);

    if ((fdsend = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	fprintf(stderr, "%s: error creating data socket\n", prog);
	cleanupandexit(2);
    }
    debugp((stderr, "after data socket call\n"));
    debugp((stderr, "TCP file descriptor (data) %d\n", fdsend));

    linger.l_onoff = 1;
    linger.l_linger = 0;
    if (setsockopt(fdsend, SOL_SOCKET, SO_LINGER, (char *)&linger,
		   sizeof(linger)) 
	< 0) {
	perror("sockcomm: setsockopt(linger)");
	cleanupandexit(2);
    }

    memset(&self, 0, sizeof(self));
    self.sin_addr.s_addr = htonl(INADDR_ANY);
    self.sin_family = AF_INET;
    self.sin_port = htons(0);
    if (bind(fdsend, (struct sockaddr *)&self, sizeof(self)) < 0) {
	fprintf(stderr, "%s: bind of local address failed (data)\n", prog);
	cleanupandexit(2);
    }
    getsockname(fdsend, (struct sockaddr *)&sockname, &namelen);
    debugp((stderr, "local port is %d\n", sockname.sin_port));
    localport = sockname.sin_port;
    controldata.ctrlchar = 0x02;
    controldata.port = localport;
    JobControl(controldata);

    while (1) {
	if (connect(fdsend, (struct sockaddr *)&datasock, socklen) < 0) {
	    continue;
	}
	break;
    }
}

static void SendBanner(done)
    int done;
{
    int banner;
    int cnt;
    char buf[MYBUFSIZE];

    debugp((stderr, "in SendBanner\n"));
    if ((banner = open(".banner", O_RDONLY|O_NDELAY, 0)) < 0) {
	fprintf(stderr, "%s: no banner file\n", prog);
	return;
    }
    while ((cnt = read(banner, buf, sizeof(buf))) > 0) {
	debugp((stderr, "read %d bytes for banner.\n", cnt));
	write(fdsend, buf, cnt);
    }
    /* hang around until banner is done */
    if (done) {
	shutdown(fdsend, 1);
	while ((cnt = read(fdsend, buf, sizeof(buf))) > 0);
    }
    close(banner);
}

    
static int CheckStatus(who)
    char *who;
{
    /* send a packet down a UDP port to the printer; expect
       interpreter status in return */

    char readbuf[MYBUFSIZE];
    int nfound = 0;
    int cnt;
    int retries = 0;
    struct sockaddr from;
    int fromlen;
    int answered = FALSE;
    struct timeval timeout;

    if (!status) {
	return 1;
    }
    /* set up select */
    FD_ZERO(&statfds);
    FD_SET(fdstatus, &statfds);
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    cnt = sendto(fdstatus, " ", 1, 0, (struct sockaddr *)&statussock,
		 sizeof(statussock)); 
    debugp((stderr, "%s: sent status query\n", who));

    while (1) {
	nfound = select(fdstatus+1, &statfds, (fd_set *) 0, (fd_set *) 0,
			&timeout);
	if (nfound > 0) {
	    cnt = recvfrom(fdstatus, readbuf, MYBUFSIZE, 0,
			   (struct sockaddr *)&from, (int *) &fromlen);
	    if (cnt > 0) {
		debugp((stderr, "%s: fdstatus read cnt %d\n", who, cnt));
		readbuf[cnt] = '\0';
		debugp((stderr, "stat: %s\n", readbuf));
		answered = TRUE;
	    }
	    else if (cnt == 0) {
		break;
	    }
	    else {
		fprintf(stderr, "%s: fatal error reading status\n", prog);
		perror("");
		return -1;
	    }
	}
	else if (nfound == 0) {
	    if (answered) {
		/* we received an answer, so this timeout probably means
		   we're done receiving this answer */
		break;
	    }
	    debugp((stderr,
		    "checkstatus: no response from printer, retrying %d.\n",
		    ++retries));
	    if (retries > 3) {
		fprintf(stderr,
			"%s: no response from printer! giving up.\n", prog);
		return 0;
	    }
	    cnt = sendto(fdstatus, "\024", 1, 0, (struct sockaddr *)&statussock,
			 sizeof(statussock)); 
	    FD_ZERO(&statfds);
	    FD_SET(fdstatus, &statfds);
	    timeout.tv_sec = 5;
	    timeout.tv_usec = 0;
	}
    }
    return 1;
}

static void Listener()
{
    /* child process; listens to the data port.  When
       it receives EOF on the data port, it exits.  */

    static char readbuf[MYBUFSIZE];
    int cs, done = FALSE;
    int nfound = 0;
    int cnt;
    struct timeval timeout;
    
    /* child */
    debugp((stderr, "child started\n"));

    while (!done) {
	debugp((stderr, "child: calling select\n"));

	/* set up select so we can listen on data sockets */
	
	FD_ZERO(&readfds);
	FD_SET(fdsend, &readfds);

	timeout.tv_sec = 15;
	timeout.tv_usec = 0;
	nfound = select(fdsend+1, &readfds, (fd_set *) 0,
			(fd_set *) 0, &timeout);
	if (nfound < 0 || nfound > 2) {
	    /* select failed */
	    fprintf(stderr, "%s: select failed\n", prog);
	    perror("");
	    kill(ppid, SIGEMT);
	    cleanupandexit(2);
	}
	else {
	    if (nfound == 0) {
		/* timeout! */
		debugp((stderr,"child: timeout\n"));
		cs = CheckStatus("child");
		if (cs < 1) {
		    fprintf(stderr, "Giving up!\n");
		    kill(ppid, SIGEMT);
		    cleanupandexit(2);
		}
	    }
	    if (nfound > 0) {
		debugp((stderr, "child: data ready! %d\n", nfound));
		/* data ready! */
		if (FD_ISSET(fdsend, &readfds)) {
		    debugp((stderr, "child: data ready on fdsend\n"));
		    cnt = read(fdsend, readbuf, MYBUFSIZE);
		    debugp((stderr, "child: fdsend read cnt %d\n",
			    cnt)); 
		    if (cnt < 0) {
			fprintf(stderr,
				"%s: fatal error reading from data port!\n",
				prog);
			perror("");
			kill(ppid, SIGEMT);
			cleanupandexit(2);
		    }
		    readbuf[cnt] = '\0';
		    if (cnt > 0) {
			fprintf(stderr, "%s", readbuf);
		    }
		    else {
			done = TRUE;
			break;
		    }
		}
	    }
	}
    }
    cleanupandexit(2);
}

static void JobControl(msg)
    struct controlpacket msg;
{
    /* sends msg as a TCP packet down the control port, listens for
       response; this will change to TCP  */ 
    int cnt;
    int nfound = 0;
    char readbuf[MYBUFSIZE];
    int retries = 0;
    struct sockaddr from;
    int fromlen;
    char *p;
    
    if (!control) {
	/* if we're not doing job control, bail */
	return;
    }
    /* send msg */
    cnt = write(fdcontrol, &msg, sizeof(msg));
    debugp((stderr, "%d: sent %x\n", currentstate, msg.ctrlchar));
    cnt = read(fdcontrol, readbuf, sizeof(readbuf));
    if (cnt > 0) {
	debugp((stderr, "%d: received %x\n", currentstate, readbuf[0]));
	if (readbuf[0] != msg.ctrlchar) {
	    fprintf(stderr, "%s: unexpected response from printer!\n", prog);
	    cleanupandexit(2);
	}
    }
}


static void abortjob(sig)
    int sig;
{
    debugp((stderr, "%s: Got 'die' signal=%d, state=%d\n", prog, sig,
	    currentstate)); 
    controldata.ctrlchar = 0x03;
    controldata.port = htons(0);
    switch (currentstate) {
    case init:
	cleanupandexit(2);
	break;
    case syncstart:
	fprintf(stderr, "%s: abort (start communications)\n", prog);
	fflush(stderr);
	JobControl(controldata);
	cleanupandexit(2);
	break;
    case sending:
	fprintf(stderr, "%s: abort (sending job)\n", prog);
	fflush(stderr);
	JobControl(controldata);
	cleanupandexit(2);
	break;
    case lastpart:
    case synclast:
	fprintf(stderr, "%s: abort (post-job processing)\n", prog);
	fflush(stderr);
	JobControl(controldata);
	cleanupandexit(2);
	break;
    default:
	break;
    }
}

static void SendFile()
{
    int cnt, wc;
    char *mbp;
    char mybuf[MYBUFSIZE];
    int pid, childpid;
    char readbuf[MYBUFSIZE];
    int foo = 1;
    int retries = 0;
    struct sockaddr_in sockname;
    int namelen = sizeof(struct sockaddr_in);
    struct linger linger;

    /* create status, control, and data connectons */
    if ((fdstatus = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	fprintf(stderr, "%s: error creating status socket.\n", prog);
	perror("");
	cleanupandexit(2);
    }
    debugp((stderr, "after status socket call\n"));
    debugp((stderr, "status fd is %d\n", fdstatus));

    memset(&self, 0, sizeof(self));
    self.sin_addr.s_addr = htonl(INADDR_ANY);
    self.sin_family = AF_INET;
    self.sin_port = htons(9101);

    if (bind(fdstatus, (struct sockaddr *)&self, sizeof(self)) < 0) {
	fprintf(stderr, "%s: bind of local address failed (status)\n", prog);
	cleanupandexit(2);
    }

    if ((fdcontrol = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	fprintf(stderr, "%s: error creating control socket\n", prog);
	cleanupandexit(2);
    }

    if (setsockopt(fdcontrol, IPPROTO_TCP, TCP_NODELAY, (char *)&foo,
		   sizeof(foo)) < 0) {
	perror("sockcomm: setsockopt (no delay)");
	cleanupandexit(2);
    }
    memset(&self, 0, sizeof(self));
    self.sin_addr.s_addr = htonl(INADDR_ANY);
    self.sin_family = AF_INET;
    self.sin_port = htons(0);
    if (bind(fdcontrol, (struct sockaddr *)&self, sizeof(self)) < 0) {
	fprintf(stderr, "%s: bind of local address failed (control)\n", prog);
	cleanupandexit(2);
    }
    socklen = sizeof(statussock);
    if (control && status) {
	if (connect(fdcontrol, (struct sockaddr *)&statussock, socklen) < 0 ) {
	    fprintf(stderr, "error connecting\n");
	    perror("");
	    cleanupandexit(1);
	}
	debugp((stderr, "after connect call (control)\n"));
    }

    currentstate = syncstart;

    if ((fdsend = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	fprintf(stderr, "%s: error creating data socket\n", prog);
	cleanupandexit(2);
    }
    debugp((stderr, "after data socket call\n"));
    debugp((stderr, "TCP file descriptor (data) %d\n", fdsend));
    
    memset(&self, 0, sizeof(self));
    self.sin_addr.s_addr = htonl(INADDR_ANY);
    self.sin_family = AF_INET;
    /* let system pick it, then we'll tell printer */
    self.sin_port = htons(0);
    if (bind(fdsend, (struct sockaddr *)&self, sizeof(self)) < 0) {
	fprintf(stderr, "%s: bind of local address failed (data)\n", prog);
	cleanupandexit(2);
    }

    getsockname(fdsend, (struct sockaddr *)&sockname, &namelen);
    localport = sockname.sin_port;
    debugp((stderr, "local port is %d\n", ntohs(localport)));
    memset(&controldata, 0, sizeof(controldata));
    controldata.ctrlchar = 0x01;
    JobControl(controldata);

    controldata.ctrlchar = 0x02;
    controldata.port = localport;
    JobControl(controldata);

    debugp((stderr, "trying to connect to data socket...\n"));

    socklen = sizeof(datasock);
    if (connect(fdsend, (struct sockaddr *)&datasock, socklen) < 0 ) {
	fprintf(stderr, "%s: error connecting (data)\n", prog);
	perror("");
	cleanupandexit(2);
    }
    debugp((stderr, "after connect call \n"));

    /* do accounting, if necessary */

    if (doactng) {
	AccountingJob(&startpagecount);
	ReopenConn();
    }

    /* fork a child for listening */
    if ((pid = fork()) < 0) {
	fprintf(stderr, "fork failed \n");
	perror("");
	cleanupandexit(2);
    }
    if (pid == 0) {
	currentstate = child;
	Listener();
	/* should never get here */
	exit(2);
    }
    /* parent */

    /* send file */
    currentstate = sending;
    if (bannerfirst) {
	SendBanner(0);
	if (!bannerlast) unlink(".banner");
    }
    debugp((stderr, "sending file\n"));
    while ((cnt = read(fdinput, mybuf, sizeof(mybuf))) > 0) {
	mbp = mybuf;
	debugp((stderr, "parent: read %d bytes\n", cnt));
	while ((cnt > 0) && ((wc = write(fdsend, mbp, cnt)) != cnt)) {
	    if (wc < 0) {
		fprintf(stderr, "%s: error writing to printer\n", prog);
		perror("");
		sleep(10);
		kill(pid, SIGEMT);
		cleanupandexit(1);
	    }
	    debugp((stderr, "parent: wrote %d bytes\n", wc));
	    mbp += wc;
	    cnt -= wc;
	}
	debugp((stderr, "parent: wrote it all, read some more\n"));
    }
    linger.l_onoff = 0;
    linger.l_linger = 0;
    if (setsockopt(fdsend, SOL_SOCKET, SO_LINGER, (char *)&linger,
		   sizeof(linger)) 
	< 0) {
	perror("sockcomm: setsockopt(linger)");
	cleanupandexit(2);
    }
    close(fdinput);
    shutdown(fdsend, 1);
    debugp((stderr, "after shutdown\n"));
    if (cnt < 0) {
	fprintf(stderr, "%s: error reading input file\n", prog);
	perror("");
	sleep(10);
	kill(pid, SIGEMT);
	cleanupandexit(1);
    }
    debugp((stderr, "sent file\n"));
    currentstate = waiting;
    while ((childpid=wait(&statusp)) > 0) {
	debugp((stderr, "parent: wait returned pid %d status %d\n",
		childpid, statusp));
	if (childpid == pid)
	    break;
    }
    if (bannerlast) {
	currentstate = lastpart;
	ReopenConn();
	SendBanner(1);
	close(fdsend);
	if (bannerlast > 1) unlink(".banner");
    }
    if (doactng) {
	currentstate = synclast;
	ReopenConn();
	AccountingJob(&endpagecount);
	AcctEntry(startpagecount, endpagecount);
    }
    currentstate = ending;
    controldata.ctrlchar = 0x04;
    controldata.port = htons(0);
    JobControl(controldata);
}
    

#define ARGS "P:n:h:p:"
extern char *optarg;
extern int optind;

main(argc, argv)
    int argc;
    char **argv;
{
    int i;
    int argp;
    long starttime;
    char *tmp;
    long addr;
    

    currentstate = init;
    while ((argp = getopt(argc, argv, ARGS)) != EOF) {
	switch(argp) {
	case 'P':
	    /* printer name */
	    pname = optarg;
	    break;
	case 'n':
	    name = optarg;
	    break;
	case 'h':
	    host = optarg;
	    break;
	case 'p':
	    prog = optarg;
	    break;
	}
    }

    if (argc > optind)
	accountingfile = argv[optind];

    doactng = name && accountingfile && (access(accountingfile, W_OK) == 0);

    if (tmp = getenv("BANNERFIRST"))
	bannerfirst = atoi(tmp);

    if (tmp = getenv("BANNERLAST"))
	bannerlast = atoi(tmp);

    if (tmp = getenv("ASCONTROL"))
	control = atoi(tmp);

    if (tmp = getenv("ASSTATUS"))
	status = atoi(tmp);

    /* init signal processing */
#ifdef BDEBUG
    for (i=1; i<31; i++)
	if (i != SIGCHLD)
	    signal(i, handler);
#endif    
    signal(SIGEMT, giveup);
    signal(SIGINT, abortjob);
    signal(SIGHUP, abortjob);
    signal(SIGTERM, abortjob);
#ifdef SYSV
    sigignore(SIGPIPE);
#endif /* SYSV */    

#ifdef BSD
    time(&starttime);
    fprintf(stderr, "%s: %s:%s %s start - %s", prog, host, name, pname,
	    ctime(&starttime));
    fflush(stderr);
#endif /* BSD */    
    
    ppid = getpid();

    fdinput = fileno(stdin);

    memset(&statussock, 0, sizeof(statussock));
    memset(&datasock, 0, sizeof(datasock));
    datasock.sin_addr.s_addr = inet_addr(pname);
    statussock.sin_addr.s_addr = inet_addr(pname);
    if (datasock.sin_addr.s_addr == -1) {
	if ((hp = gethostbyname(pname)) == NULL) {
	    fprintf(stderr, "%s: couldn't find host %s.\n", prog, pname);
	    cleanupandexit(2);
	}
	debugp((stderr, "after gethostbyname\n"));
	debugp((stderr,
		"printer hostent fields: name %s\n addr %s length 0x%x\n",
		hp->h_name, inet_ntoa((struct in_addr *) (hp->h_addr)),
		hp->h_length));
	memcpy(&datasock.sin_addr, hp->h_addr, hp->h_length);
	memcpy(&statussock.sin_addr, hp->h_addr, hp->h_length);
    }

    debugp((stderr, "port no. %d\n", dataport));

    datasock.sin_family = AF_INET;
    datasock.sin_port = htons(dataport);

    statussock.sin_family = AF_INET;
    statussock.sin_port = htons(statusport);

    debugp((stderr, "status port no %d\n", statusport));

    SendFile();

#ifdef BSD
    time(&starttime);
    fprintf(stderr, "%s: end - %s", prog, ctime(&starttime));
    fflush(stderr);
#endif /* BSD */    
    cleanupandexit(0);
}
