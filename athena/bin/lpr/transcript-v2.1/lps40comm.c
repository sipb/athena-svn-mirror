/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/transcript-v2.1/lps40comm.c,v $
 *	$Author: miki $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/transcript-v2.1/lps40comm.c,v 1.6 1995-07-11 21:13:30 miki Exp $
 */

#ifndef lint
static char *rcsid_lps40_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/transcript-v2.1/lps40comm.c,v 1.6 1995-07-11 21:13:30 miki Exp $";
#endif lint

/* lps40comm.c
 *
 * The equivalent of pscomm for talking to a DEC LPS40 via a laps process.
 * 
 * Mostly Copyright (C) 1985 Adobe Systems Incorporated
 *
 * pscomm gets called with:
 *	stdin	== the file to print (may be a pipe!)
 *	stdout	== the printer
 *	stderr	== the printer log file
 *	cwd	== the spool directory
 *	argv	== set up by interface shell script:
 *	  filtername	-P printer
 *			-p filtername
 *			[-r]		(don't ever reverse)
 *			-n login
 *			-h host
 *			-a account
 *			-m mediacost
 *			[accntfile]
 *
 *	environ	== various environment variable effect behavior
 *		VERBOSELOG	- do verbose log file output
 *		PSLIBDIR	- transcript library directory
 *		PSTEXT		- simple text formatting filter
 */

#include <ctype.h>
#include <setjmp.h>
#include <sgtty.h>
#include <signal.h>
#include <stdio.h>
#include <strings.h>
#ifdef POSIX
#include <unistd.h>
#include "../posix.h"
#include <fcntl.h>
#endif
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "transcript.h"
#include "psspool.h"

#ifdef BDEBUG
#define debugp(x) {fprintf x ; (void) fflush(stderr);}
#else
#define debugp(x)
#endif BDEBUG

/*
 * the following string is sent to the printer when we want it to
 * report its current pagecount (for accounting)
 */

private char *getpages = "\n(%%%%[ pagecount: )print \
statusdict/pagecount get exec(                )cvs print( ]%%%%)= flush\n%s";

private jmp_buf waitonreverse, startstatus, dwait, sendint;

private char	*prog;			/* invoking program name */
private char	*name;			/* user login name */
private char	*host;			/* host name */
private char	*pname;			/* printer name */
private char	*account = NULL;	/* account number */
private char	*mediacost = "0";	/* mediacost */
private char	*accountingfile;	/* file for printer accounting */
private int	doactng;		/* true if we can do accounting */
private int	progress, oldprogress;	/* finite progress counts */
private int	getstatus = FALSE;
private int	revdone = FALSE;	/* reverse done, send new */
private int	goahead = FALSE;	/* got initial status back */
private int	gotemt = FALSE;		/* got ^D ack from listener */
private int	sendend = TRUE;		/* send an ^D */

private char *bannerfirst;
private char *bannerlast;
private char *verboselog;
private char *reverse;
private int BannerFirst;
private int BannerLast;
private int VerboseLog;

private int	fpid = 0;	/* formatter pid */
private int	cpid = 0;	/* listener pid */
private int	lpid = 0;	/* laps pid */

private int	intrup = FALSE;	/* interrupt flag */

private char abortbuf[] = "\003";	/* ^C abort */
private char statusbuf[] = "\024";	/* ^T status */
private char eofbuf[] = "\004";		/* ^D end of file */

private char EOFerr[] = "%s: unexpected EOF from printer (%s)!\n";

/* global file descriptors (avoid stdio buffering!) */
private int fdsend;		/* to printer (from stdout) */
private int fdlisten;		/* from printer (same tty line) */
private int fdinput;		/* file to print (from stdin) */

private FILE *jobout;		/* special printer output log */

private int flg = FREAD|FWRITE;	 /* ioctl FLUSH arg */


extern int errno;
extern char *getenv();

private VOID	intinit();
private VOID	intsend();
private VOID	intwait();
private VOID	salarm();
private VOID	walarm();
private VOID	falarm();
private VOID	reverseready();
private VOID	readynow();
private VOID	emtdead();
private VOID	emtdone();
private VOID	open_lps40();
private VOID	badfile();
private char 	*FindPattern();
private VOID    acctentry();

main(argc,argv)
	int argc;
	char *argv[];
{
    register char  *cp;
    register int cnt, wc;
    register char *mbp;

    char  **av;
    long clock;		/* for log timestamp */
    char magic[11];	/* first few bytes of stdin ?magic number and type */
    int  noReverse = 0; /* flag if we should never page reverse */
    int  canReverse = 0;/* flag if we can page-reverse the ps file */
    int  reversing = 0;
    FILE *streamin;

    char mybuf[BUFSIZ];
    int wpid;
#if defined(POSIX) && !defined(ultrix)
    int status;	        /* Return value from wait() */
#else
    union wait status;
#endif
    int fdpipe[2];
    int format = 0;
    int i;
    register FILE *psin;
    register int r;

    char pbuf[BUFSIZ]; /* buffer for pagecount info */
    char *pb;		/* pointer for above */
    int pc1, pc2; 	/* page counts before and after job */
    int sc;		/* pattern match count for sscanf */
#if defined(POSIX) && !defined(ultrix)
    struct sigaction sa;
#endif

#if defined(POSIX) && !defined(ultrix)

    (void) sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = intinit;
    (void) sigaction(SIGINT, &sa, (struct sigaction *)0);
    (void) sigaction(SIGHUP, &sa, (struct sigaction *)0);
    (void) sigaction(SIGQUIT, &sa, (struct sigaction *)0);
    (void) sigaction(SIGTERM, &sa, (struct sigaction *)0);
#else
    VOIDC signal(SIGINT, intinit);
    VOIDC signal(SIGHUP, intinit);
    VOIDC signal(SIGQUIT, intinit);
    VOIDC signal(SIGTERM, intinit);
#endif
    /* parse command-line arguments */
    /* the argv (see header comments) comes from the spooler daemon */
    /* itself, so it should be canonical, but at least one 4.2-based */
    /* system uses -nlogin -hhost (insead of -n login -h host) so I */
    /* check for both */

    av = argv;
    prog = *av;

    while (--argc) {
	if (*(cp = *++av) == '-') {
	    switch (*(cp + 1)) {
		case 'P':	/* printer name */
		    argc--;
		    pname = *(++av);
		    break;

		case 'n': 	/* user name */
		    argc--;
		    name = *(++av);
		    break;

		case 'h': 	/* host */
		    argc--;
		    host = *(++av);
		    break;

		case 'p':	/* prog */
		    argc--;
		    prog = *(++av);
		    break;

		case 'r':	/* never reverse */
		    argc--;
		    break;

		case 'a':	/* account number */
		    argc--;
		    account = *(++av);
		    break;

		case 'm':	/* media cost */
		    argc--;
		    mediacost = *(++av);
		    break;

		default:	/* unknown */
		    fprintf(stderr,"%s: unknown option: %s\n",prog,cp);
		    break;
	    }
	}
	else
	    accountingfile = cp;
    }
    noReverse = 1;

    debugp((stderr,"args: %s %s %s %s\n",prog,host,name,accountingfile));

    /* do printer-specific options processing */

    VerboseLog = 1;
    BannerFirst = BannerLast = 0;
    reverse = NULL;
    if (verboselog=envget("VERBOSELOG")) {
	VerboseLog=atoi(verboselog);
    }

    if (VerboseLog) {
	fprintf(stderr, "%s: %s:%s %s start - %s", prog, host, name, pname,
            (VOIDC time(&clock), ctime(&clock)));
	VOIDC fflush(stderr);
    }
    debugp((stderr,"%s: pid %d ppid %d\n",prog,getpid(),getppid()));
    debugp((stderr,"%s: options BF %d BL %d VL %d R %s\n",prog,BannerFirst,
    	BannerLast, VerboseLog, ((reverse == NULL) ? "norev": reverse)));

    if ((cnt = read(fileno(stdin),magic,11)) != 11) badfile();
    debugp((stderr,"%s: magic number is %11.11s\n",prog,magic));
    streamin = stdin;

    if (strncmp(magic,"%!",2) != 0) {
	    /* here is where you might test for other file type
	     * e.g., PRESS, imPRESS, DVI, Mac-generated, etc.
	     */

	    /* final sanity check on the text file, to guard
	     * against arbitrary binary data
	     */

	    for (i = 0; i < 11; i++) {
		    if (!isascii(magic[i]) ||
			(!isprint(magic[i]) && !isspace(magic[i]))){
			    fprintf(stderr,
				    "%s: spooled binary file rejected\n",prog);
			    VOIDC fflush(stderr);
			    sprintf(mybuf,"%s/bogusmsg.ps",envget("PSLIBDIR"));
			    if ((streamin = freopen(mybuf,"r",stdin))
				== NULL) {
				    exit(THROW_AWAY);
			    }
			    format = 1;
			    goto lastchance;
		    }
	    }
    
	    /* exec text formatter to make a listing */
	    debugp((stderr,"formatting\n"));
	    format = 1;
	    VOIDC lseek(0,0L,0);
	    rewind(stdin);
	    if (pipe (fdpipe)) pexit2(prog, "format pipe",THROW_AWAY);
	    if ((fpid = fork()) < 0) pexit2(prog, "format fork",THROW_AWAY);
	    if (fpid == 0) { /* child */
		    /* set up child stdout to feed parent stdin */
		    if (close(1) || (dup(fdpipe[1]) != 1)
			|| close(fdpipe[1]) || close(fdpipe[0])) {
			    pexit2(prog, "format child",THROW_AWAY);
		    }
		    /* 
		     * How do we send the magic number to pstext?
		     * We already read it out of stdin which may be a pipe
		     */
		    execl(envget("PSTEXT"), "pstext", pname, 0);
		    pexit2(prog,"format exec",THROW_AWAY);
	    }
	    /* parent continues */
	    /* set up stdin to be pipe */
	    if (close(0) || (dup(fdpipe[0]) != 0)
		|| close(fdpipe[0]) || close(fdpipe[1])) {
		    pexit2(prog, "format parent",THROW_AWAY);
	    }

	    /* fall through to spooler with new stdin */
	    /* can't seek here but we should be at the right place */
	    streamin = fdopen(0,"r");
    }
    /* we don't do page reversal on the LPS40, it stacks face down
       by default */
    lastchance:;

    /* establish a laps to the printer */

    open_lps40();

    fdinput = fileno(streamin); /* the file to print */

    doactng = name && accountingfile && (access(accountingfile, W_OK) == 0);
    BackupStatus(".status","status");
    VOIDC setjmp(sendint);

    if (intrup) {
	    /* we only get here if there was an interrupt */

	    fprintf(stderr,"%s: abort (sending)\n",prog);
	    VOIDC fflush(stderr);

	    /* flush and restart output to printer,
	     * send an abort (^C) request and wait for the job to end
	     */
	    if (kill(lpid, SIGINT)) {
		    RestoreStatus();
		    pexit(prog,THROW_AWAY);
	    }
	    debugp((stderr,"%s: sent interrupt - waiting\n",prog));
	    intrup = 0;
	    goto donefile; /* sorry ewd! */
    }
#if defined(POSIX) && !defined(ultrix)
    (void) sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = intsend;
    (void) sigaction(SIGINT, &sa, (struct sigaction *)0);
    (void) sigaction(SIGHUP, &sa, (struct sigaction *)0);
    (void) sigaction(SIGQUIT, &sa, (struct sigaction *)0);
    (void) sigaction(SIGTERM, &sa, (struct sigaction *)0);

#else
    VOIDC signal(SIGINT, intsend);
    VOIDC signal(SIGHUP, intsend);
    VOIDC signal(SIGQUIT, intsend);
    VOIDC signal(SIGTERM, intsend);
#endif
    RestoreStatus();
    /* initial page accounting (BEFORE break page) */
    if (doactng) {
	    sprintf(mybuf, getpages, "");
	    VOIDC write(fdsend, mybuf, strlen(mybuf));
	    if (close(fdsend))		/* send EOF */
		    debugp((stderr,"%s: laps close failure 1:%s\n",prog,errno));
	    progress++;
	    pc1 = pc2 = -1; /* bogus initial values */
	    if ((psin = fdopen(fdlisten, "r")) == NULL) {
		    pexit(prog, THROW_AWAY);
	    }
	    pb = pbuf;
	    *pb = '\0';
	    while (TRUE) {
		    r = getc(psin);
		    if (r == EOF) {
			    break;
		    }
		    *pb++ = r;
	    }
	    *pb = '\0';
	    fclose(psin);
	    close(fdlisten);
	    open_lps40();
	    if ((psin = fdopen(fdlisten, "r")) == NULL) {
		    pexit(prog, THROW_AWAY);
	    }
	    if (pb = FindPattern(pb, pbuf, "%%[ pagecount: ")) {
		    sc = sscanf(pb, "%%%%[ pagecount: %d ]%%%%\r", &pc1);
	    }
	    if ((pb == NULL) || (sc != 1)) {
		    fprintf(stderr, "%s: accounting error 1 (%s)\n", prog,pbuf);
		    VOIDC fflush(stderr);
	    }
	    debugp((stderr,"%s: accounting 1 (%s)\n",prog,pbuf));
    }
    /* ship the magic number! */
    if ((!format) && (!reversing)) {
	    VOIDC write(fdsend,magic,11);
	    progress++;
    }

    /* now ship the rest of the file */
    while ((cnt = read(fdinput, mybuf, sizeof mybuf)) > 0) {
	    if (intrup == TRUE) break;

	    mbp = mybuf;
	    while ((cnt > 0) && ((wc = write(fdsend, mbp, cnt)) != cnt)) {
		    /* this seems necessary but not sure why */
		    if (wc < 0) {
			    fprintf(stderr,"%s: error writing to printer:\n",prog);
			    perror(prog);
			    RestoreStatus();
			    sleep(10);
			    exit(TRY_AGAIN);
		    }
		    mbp += wc;
		    cnt -= wc;
	    }
    }
    if (cnt < 0) {
	    fprintf(stderr,"%s: error reading from stdin: \n", prog);
	    perror(prog);
	    RestoreStatus();
	    sleep(10);
    	    exit(TRY_AGAIN);	/* kill the listener? */
    }

    if (close(fdsend))			/* send EOF */
	    debugp((stderr,"%s: laps close failure 2:%d\n",prog,errno));

    /* listen for the user job */
    while (TRUE) {
	    r = getc(psin);
	    if (r == EOF) {
		    break;
	    }
	    GotChar(r);
    }

 donefile:

    VOIDC close(fdsend);		/* send EOF in case we are jumping
					   here */
    VOIDC fclose(psin);
    VOIDC close(fdlisten);

    open_lps40();

    if ((psin = fdopen(fdlisten, "r")) == NULL) {
	    pexit(prog, THROW_AWAY);
    }
    /* final page accounting */
    if (doactng) {
	    sprintf(mybuf, getpages, "");
	    VOIDC write(fdsend, mybuf, strlen(mybuf));
	    if (close(fdsend))		/* send EOF */
		    debugp((stderr,"%s: laps close failure 4:%d\n",prog, errno));
	    pb = pbuf;
	    *pb = '\0';	/* ignore the previous pagecount */
	    while (TRUE) {
		    r = getc(psin);
		    if (r == EOF) {
			    break;
		    }
		    *pb++ = r;
	    }
	    *pb = '\0';
	    debugp((stderr,"%s: accounting 2 (%s)\n",prog,pbuf));
	    if (pb = FindPattern(pb, pbuf, "%%[ pagecount: ")) {
		    sc = sscanf(pb, "%%%%[ pagecount: %d ]%%%%\r", &pc2);
	    }
	    if ((pb == NULL) || (sc != 1)) {
		    fprintf(stderr,
			    "%s: accounting error 2 (%s)\n", prog,pbuf);
		    VOIDC fflush(stderr);
	    }
	    else if ((pc2 < pc1) || (pc1 < 0) || (pc2 < 0)) {
		    fprintf(stderr,
			    "%s: accounting error 3 %d %d\n", prog,pc1,pc2);
		    VOIDC fflush(stderr);
	    }
	    else if (freopen(accountingfile, "a", stdout) != NULL) {
		VOIDC acctentry(pc1, pc2, account, mediacost);
/*		    printf("%7.2f\t%s:%s\n", (float)(pc2 - pc1), host, name);*/
		    VOIDC fclose(stdout);
	    }
    }

    VOIDC close(fdlisten);
    VOIDC fclose(psin);
#if defined(POSIX) && !defined(ultrix)
    (void) sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = SIG_IGN;
    (void) sigaction(SIGINT, &sa, (struct sigaction *)0);
    (void) sigaction(SIGHUP, &sa, (struct sigaction *)0);
    (void) sigaction(SIGQUIT, &sa, (struct sigaction *)0);
    (void) sigaction(SIGTERM, &sa, (struct sigaction *)0);
    (void) sigaction(SIGEMT, &sa, (struct sigaction *)0);
#else
    VOIDC signal(SIGINT, SIG_IGN);
    VOIDC signal(SIGHUP, SIG_IGN);
    VOIDC signal(SIGQUIT, SIG_IGN);
    VOIDC signal(SIGTERM, SIG_IGN);
    VOIDC signal(SIGEMT, SIG_IGN);
#endif
    if (VerboseLog) {
	    fprintf(stderr,"%s: end - %s",prog,
		    (VOIDC time(&clock),ctime(&clock)));
	    VOIDC fflush(stderr);
    }
    RestoreStatus();
    exit(0);
}

/* initial interrupt handler - before communications begin, so
 * nothing to be sent to printer
 */
private VOID intinit() {
    long clock;

    /* get rid of banner file */
    VOIDC unlink(".banner");

    fprintf(stderr,"%s: abort (during setup)\n",prog);
    VOIDC fflush(stderr);

    /* these next two may be too cautious */
    VOIDC kill(0,SIGINT);
    while (wait((union wait *) 0) > 0);

    if (VerboseLog) {
	fprintf (stderr, "%s: end - %s", prog, (time(&clock), ctime(&clock)));
	VOIDC fflush(stderr);
    }

    exit(THROW_AWAY);
}

private VOID badfile() {
        fprintf(stderr,"%s: bad magic number, EOF\n", prog);
	VOIDC fflush(stderr);
	exit(THROW_AWAY);
}

private VOID open_lps40() {
	int tolaps[2];
	int fromlaps[2];
	if (pipe (tolaps)) pexit2(prog, "laps pipe",THROW_AWAY);
	if (pipe (fromlaps)) pexit2(prog, "laps pipe 2",THROW_AWAY);
	if ((lpid = fork()) < 0) pexit2(prog, "laps fork",THROW_AWAY);
	if (lpid == 0) { /* child */
		/* set up child stdout & stderr to feed parent pipe */
		if (tolaps[0] < 3 || tolaps[1] < 3
		    || fromlaps[0] < 3 || fromlaps[1] < 3) {
			pexit2(prog, "laps pipe bad fds", THROW_AWAY);
		}
		if ((dup2(tolaps[0], 0) != 0) || (dup2(fromlaps[1], 1) != 1)
		    || (dup2(1, 2) != 2)
		    || close(tolaps[0]) || close(tolaps[1])
		    || close(fromlaps[0]) || close(fromlaps[1])) {
			pexit2(prog, "laps child",THROW_AWAY);
		}
		execl(envget("LAPS"), "laps", pname, 0);
		pexit2(prog,"laps exec",THROW_AWAY);
	}
	/* set up fdsend & fdlisten */
	if (((fdsend = dup(tolaps[1])) < 0)
	    || ((fdlisten = dup(fromlaps[0])) < 0)
	    || close(tolaps[0]) || close(tolaps[1])
	    || close(fromlaps[0]) || close(fromlaps[1])) {
		pexit2(prog, "laps pipe cleanup", THROW_AWAY);
	}
}

/* interrupt during sending phase to sender process */

private VOID intsend() {
    /* set flag */
    intrup = TRUE;
    longjmp(sendint, 0);
}
/* search backwards from p in start for patt */
private char *FindPattern(p, start, patt)
char *p;
char *start;
char *patt;
{
    int patlen;
    patlen = strlen(patt);
    
    p -= patlen;
    for (; p >= start; p--) {
	if (strncmp(p, patt, patlen) == 0) return(p);
    }
    return ((char *)NULL);
}

private GotChar(c)
register int c;
{
    static char linebuf[BUFSIZ];
    static char *cp = linebuf;
    static enum State {normal, onep, twop, inmessage,
    			close1, close2, close3, close4} st = normal;
    char *match, *last;

    switch (st) {
	case normal:
	    if (c == '%') {
		st = onep;
		cp = linebuf;
		*cp++ = c;
		break;
	    }
	    putc(c,jobout);
	    VOIDC fflush(jobout);
	    break;
	case onep:
	    if (c == '%') {
		st = twop;
		*cp++ = c;
		break;
	    }
	    putc('%',jobout);
	    putc(c,jobout);
	    VOIDC fflush(jobout);
	    st = normal;
	    break;
	case twop:
	    if (c == '[') {
		st = inmessage;
		*cp++ = c;
		break;
	    }
	    if (c == '%') {
		putc('%',jobout);
		VOIDC fflush(jobout);
		/* don't do anything to cp */
		break;
	    }
	    putc('%',jobout);
	    putc('%',jobout);
	    VOIDC fflush(jobout);
	    st = normal;
	    break;
	case inmessage:
	    *cp++ = c;
	    if (c == ']') st = close1;
	    break;
	case close1:
	    *cp++ = c;
	    switch (c) {
		case '%': st = close2; break;
		case ']': st = close1; break;
		default: st = inmessage; break;
	    }
	    break;
	case close2:
	    *cp++ = c;
	    switch (c) {
		case '%': st = close3; break;
		case ']': st = close1; break;
		default: st = inmessage; break;
	    }
	    break;
	case close3:
	    *cp++ = c;
	    switch (c) {
		case '\r': st = close4; break;
		case ']': st = close1; break;
		default: st = inmessage; break;
	    }
	    break;
	case close4:
	    *cp++ = c;
	    switch(c) {
		case '\n': st = normal; break;
		case ']': st = close1; break;
		default: st = inmessage; break;
	    }
	    if (st == normal) {
		/* parse complete message */
		last = cp;
		*cp = 0;
		debugp((stderr,">>%s",linebuf));
		if (match = FindPattern(cp, linebuf, " PrinterError: ")) {
		    if (*(match-1) != ':') {
			fprintf(stderr,"%s",linebuf);
			VOIDC fflush(stderr);
			*(last-6) = 0;
			Status(match+15);
		    }
		    else {
			last = index(match,';');
			*last = 0;
			Status(match+15);
		    }
		}
		else if (match = FindPattern(cp, linebuf, " status: ")) {
		    match += 9;
		    if (strncmp(match,"idle",4) == 0) {
			/* we are hopelessly lost, get everyone to quit */
			fprintf(stderr,"%s: ERROR: printer is idle, giving up!\n",prog);
			VOIDC fflush(stderr);
			VOIDC kill(getppid(),SIGKILL); /* will this work */
			exit(THROW_AWAY);
		    }
		    else {
			/* one of: busy, waiting, printing, initializing */
			/* clear status message */
			RestoreStatus();
		    }
		}
		else {
		    /* message not for us */
		    fprintf(jobout,"%s",linebuf);
		    VOIDC fflush(jobout);
		    st = normal;
		    break;
		}
	    }
	    break;
	default:
	    fprintf(stderr,"bad case;\n");
    }
    return;
}

/* restore the "status" message from the backed-up ".status" copy */
private RestoreStatus() {
    BackupStatus("status",".status");
}

/* report PrinterError via "status" message file */
private Status(msg)
register char *msg;
{
    register int fd;
    char msgbuf[100];

    if ((fd = open("status",O_WRONLY|O_CREAT,0664)) < 0) return;
    VOIDC ftruncate(fd,0);
    sprintf(msgbuf,"Printer Error: may need attention! (%s)\n\0",msg);
    VOIDC write(fd,msgbuf,strlen(msgbuf));
    VOIDC close(fd);
}

/* backup "status" message file in ".status",
 * in case there is a PrinterError
 */

private BackupStatus(file1, file2)
char *file1, *file2;
{
    register int fd1, fd2;
    char buf[BUFSIZ];
    int cnt;

#if defined(POSIX) && !defined(ultrix)
    register int status;
    struct flock fl;
    
    VOIDC umask(0);
    fd1 = open(file1, O_WRONLY|O_CREAT, 0664);
    if (fd1 < 0) {
	VOIDC unlink(file1);
	fd1 = open(file1, O_WRONLY|O_CREAT, 0664);
    }
    status = (fd1 < 0);
    if (!status) {
	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 0;
	fl.l_pid = getpid();
	status = fcntl(fd1, F_SETLKW, &fl);
    }
    if (status) {
	fprintf(stderr, "%s: writing %s",prog,file1);
	perror("");
	VOIDC close(fd1);
	return;
    }
    VOIDC ftruncate(fd1,0);
    if ((fd2 = open(file2, O_RDONLY,0)) < 0) {
	fprintf(stderr, "%s: error reading %s", prog, file2);
	perror("");
	VOIDC close(fd1);
	return;
    }
    cnt = read(fd2,buf,BUFSIZ);
    VOIDC write(fd1,buf,cnt);

    fl.l_type = F_UNLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_pid = getpid();
    VOIDC fcntl(fd1, F_SETLKW, &fl);
	
    VOIDC close(fd1);
    VOIDC close(fd2);

#else


    VOIDC umask(0);
    fd1 = open(file1, O_WRONLY|O_CREAT, 0664);
    if ((fd1 < 0) || (flock(fd1,LOCK_EX) < 0)) {
	VOIDC unlink(file1);
	VOIDC flock(fd1,LOCK_UN);
	VOIDC close(fd1);
	fd1 = open(file1, O_WRONLY|O_CREAT, 0664);
    }
    if ((fd1 < 0) || (flock(fd1,LOCK_EX) <0)) {
	fprintf(stderr, "%s: writing %s:\n",prog,file1);
	perror(prog);
	VOIDC close(fd1);
	return;
    }
    VOIDC ftruncate(fd1,0);
    if ((fd2 = open(file2, O_RDONLY,0)) < 0) {
	fprintf(stderr, "%s: error reading %s:\n", prog, file2);
	perror(prog);
	VOIDC close(fd1);
	return;
    }
    cnt = read(fd2,buf,BUFSIZ);
    VOIDC write(fd1,buf,cnt);
    VOIDC flock(fd1,LOCK_UN);
    VOIDC close(fd1);
    VOIDC close(fd2);
#endif
}

/* Make an entry in the accounting file */
private VOID acctentry(start,end,account,mediacost)
long start,end;         /* Starting and ending page counts for job */
char *mediacost, account;
{
    struct timeval timestamp;
    debugp((stderr,"%s: Make acct entry s=%ld, e=%ld\n",prog,start,end));

    /* The following is cause you might not really print the banner page if
       the job is aborted and you get a negative number. Ugly.... */
    if( start > end || start < 0 || end < 0 ) {
	fprintf(stderr,"%s: accounting error 3, %ld %ld\n",prog, start,end);
	fflush(stderr);
	}
    else if( freopen(accountingfile,"a",stdout) != NULL ) {
	gettimeofday(&timestamp, NULL);
	if (account == NULL) 
	    printf("%d\t%s:%s\t%ld\t0\t%s\t%d\n",(end-start),host,name,
		   timestamp.tv_sec,mediacost);
	else if (mediacost == NULL) 
	    printf("%d\t%s:%s\t%ld\t%s\t0\t%d\n",(end-start),host,name,
		   timestamp.tv_sec,account, (end-start));
	else printf("%d\t%s:%s\t%ld\t%s\t%s\t%d\n",(end-start),host,name,
		    timestamp.tv_sec,account,mediacost, (end-start));
	}
}
