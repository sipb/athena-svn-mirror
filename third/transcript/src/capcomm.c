#ifndef lint
#define _NOTICE static char
_NOTICE N1[] = "Copyright (c) 1985,1986,1987,1990,1991,1992 Adobe Systems Incorporated";
_NOTICE N2[] = "GOVERNMENT END USERS: See Notice file in TranScript library directory";
_NOTICE N3[] = "-- probably /usr/lib/ps/Notice";
_NOTICE RCSID[] = "$Header: /afs/dev.mit.edu/source/repository/third/transcript/src/capcomm.c,v 1.1.1.1 1996-10-07 20:25:47 ghudson Exp $";
#endif
/* pscomm.c
 *
 * Copyright (C) 1985,1986,1987,1990,1991,1992 Adobe Systems Incorporated.
 * All rights reserved.
 * GOVERNMENT END USERS: See Notice file in TranScript library
 * directory -- probably /usr/lib/ps/Notice
 *
 * RCSLOG:
 * $Log: not supported by cvs2svn $
 * Revision 1.6  1994/04/11  22:44:02  snichols
 * SGI is slightly different than other SYSV's.
 *
 * Revision 1.5  1994/04/08  23:27:38  snichols
 * added sigignore for SIGPIPE, surrounded by ifdef SYSV since
 * not all BSD systems have sigignore, and the problem doesn't
 * happen there anyway.
 *
 * Revision 1.4  1994/04/07  21:00:03  snichols
 * Solaris fixes.
 *
 * Revision 1.3  1993/04/06  22:09:54  snichols
 * shouldn't decrement argc on k option.
 *
 * Revision 1.2  1992/08/21  16:26:32  snichols
 * Release 4.0
 *
 * Revision 1.1  1992/08/19  00:27:08  snichols
 * Initial revision
 *
 *
 */

#include <unistd.h>
#include <ctype.h>
#include <setjmp.h>
#include <sgtty.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <netat/appletalk.h>
#undef TRUE
#undef FALSE

#include "transcript.h"
#include "psspool.h"
#include "config.h"    

#ifdef BDEBUG
#define debugp(x) {fprintf x ; (void) fflush(stderr);}
#else
#define debugp(x)
#endif BDEBUG

/* the following string is sent to the printer when we want it to report
   its current pagecount (for accounting) */

private char *getpages =
"\n(%%%%[ pagecount: )print statusdict/pagecount get exec(                )cvs \
print(, %d %d ]%%%%)= flush\n%s";

private jmp_buf initlabel, synclabel, sendlabel, croaklabel;

private char *prog = "capcomm";	/* invoking program name */
private char *name = " ";	/* user login name */
private char *host = " ";	/* host name */
private char *pname = " ";	/* printer name */
private char *accountingfile = " ";	/* file for printer accounting */
private int doactng;		/* true if we can do accounting */
private int progress, oldprogress;	/* finite progress counts */
private int getstatus = FALSE;	/* TRUE = Query printer for status */
private int newstatmsg = FALSE;	/* TRUE = We changed status message */
private int childdone = FALSE;	/* TRUE = Listener process finished */
private int jobaborted = FALSE;	/* TRUE = Aborting current job */
private long startpagecount;	/* Page count at start of job */
private long endpagecount;	/* Page count at end of job */
private long starttime;		/* Timer start. For status warnings */
private int saveerror;		/* Place to save errno when exiting */

private char	*applename;		/* Name of printer in Appletalk */
private char	*debugstr = NULL;	/* Debugging string for CAP routines */
private int	err,cno;
private int	eof,rlen,rcomp,wcomp,ocomp,paperr;
private int	seconds;
private PAPStatusRec papstatus;		/* Current status from printer */
#define FLOWSIZE 8			/* Use 8*512 for Appletalk transfers */
#define BUFMAX (512*FLOWSIZE)		/* Max size of Appletalk comm buffer */
#define OPENWAIT 300			/* Time to wait before open warning */
#define OPENSLEEP 30			/* Time to wait before retrying open */
#define IOSLEEP 4			/* Wait between I/O loops */
private char	rdbuf[BUFMAX+10];	/* Read buffer */
private int	rdlen;			/* Number of input chars */
private char	identbuf[100];		/* PostScript that sets jobname */
private int	identlen;		/* Number of chars in identbuf */

extern boolean dochecksum;

private char *bannerfirst;
private char *bannerlast;
private char *verboselog;
private int BannerFirst;
private int BannerLast;
private int UnlinkBannerLast;
private int VerboseLog;
private int pipeline = FALSE;

/* Interrupt state variables */
typedef enum {			/* Values for "die" interrupts, like
				   SIGINT */
    init,			/* Initialization */
    syncstart,			/* Synchronize communications with printer */
    sending,			/* Send info to printer */
    waiting,			/* Waiting for listener to get EOF from
				   prntr */
    lastpart,			/* Final processing following user job */
    synclast,			/* Syncronize communications at the end */
    ending,			/* Cleaning up */
    croaking,			/* Abnormal exit, waiting for children to
				   die */
    child			/* Used ONLY for the child (listener)
				   process */
}    dievals;
private dievals intstate;	/* State of interrupts */
private flagsig;		/* TRUE = On signal receipt, just set flag */
#define DIE_INT    1		/* Got a "die" interrupt. like SIGINT */
#define ALARM_INT  2		/* Got an alarm */
#define EMT_INT    4		/* Got a SIGEMT signal (child-parent
				   comm.) */
private int gotsig;		/* Mask that may take any of the above
				   values */

#define STARTCRIT() {gotsig=0; flagsig=TRUE;}	/* Start a critical region */
#define ENDCRIT()   {flagsig=FALSE;}	/* End a critical region */

/* WARNING: Make sure reapchildren() routine kills all processes we
   started */
#ifdef SYSV
#ifndef sgi
typedef struct {
    int w_termsig : 8;
    int w_retcode : 8;
} stat_buf;
#endif
#endif
private int cpid = 0;		/* listener pid */
private int mpid = 0;		/* current process pid */
private int wpid;		/* Temp pid */
#ifdef SYSV
#ifndef sgi
private int status;
#else
private union wait status;
#endif
#else
private union wait status;	/* Return value from wait() */
#endif    

private char abortbuf[] = "\003";	/* ^C abort */
private char statusbuf[] = "\024";	/* ^T status */
private char eofbuf[] = "\004";	/* ^D end of file */

/* global file descriptors (avoid stdio buffering!) */
private int fdsend;		/* to printer (from stdout) */
private int fdlisten;		/* from printer (same tty line) */
private int fdinput;		/* file to print (from stdin) */

private FILE *psin = NULL;	/* Buffered printer input */
private FILE *jobout;		/* special printer output log */

/* Return values from the NextCh() routine */
typedef enum {
    ChOk,			/* Everything is fine */
    ChIdle,			/* We got status="idle" from the printer */
    ChTimeout			/* The printer timed out */
}    ChStat;


extern char *getenv();
extern int errno;

private VOID GotDieSig();
private VOID GotAlarmSig();
private VOID GotEmtSig();
private VOID syncprinter();
private VOID listenexit();
private VOID closedown();
private VOID myexit1(), myexit2();
private VOID croak();
private VOID acctentry();
private char *FindPattern();
private ChStat NextCh();
private SendBanner();
private NextChInit();
private BackupStatus();
private RestoreStatus();
private Status();
private int resetprt();

static char *mapprtname();
static void writebuf();
static void readbuf();
static void parserdbuf();
static void CAPDebug();

/* The following are alarms settings for various states of this program */
#define SENDALARM 90		/* Status check while sending job to
				   printer */
#define WAITALARM 90		/* Status check while waiting for job to
				   end */
#define CROAKALARM 10		/* Waiting for child processes to die */
#define CHILDWAIT 60		/* Child waiting to die -- checking on
				   parent */
#define ABORTALARM 90		/* Waiting for printer to respond to job
				   abort */
#define SYNCALARM 30		/* Waiting for response to communications
				   sync */

#define MAXSLEEP  15		/* Max time to sleep while sync'ing
				   printer */

/* Exit values from the listener process */
#define LIS_NORMAL 0		/* No problems */
#define LIS_EOF    1		/* Listener got EOF on printer */
#define LIS_IDLE   2		/* Heard a "status: idle" from the printer */
#define LIS_DIE    3		/* Parent told listener to kill itself */
#define LIS_NOPARENT  4		/* Parent is no longer present */
#define LIS_ERROR  5		/* Unrecoverable error */
#define LIS_TIMEOUT 6		/* Printer timed out */



main(argc, argv)		/* MAIN ROUTINE */
    int argc;
    char *argv[];
{
    register char *cp;
    register int cnt, wc;
    register char *mbp;

    char **av;
    FILE *streamin;
    char *outname;		/* file name for job output */
    int havejobout = FALSE;	/* flag if jobout != stderr */

    char *q;

    int tmp;			/* fd for temporary when input is text
				   pipe */
    struct stat sbuf;

    char mybuf[BUFSIZ];
    int fdpipe[2];
    int i;

    setbuf(stderr, NULL);
    
    mpid = getpid();		/* Save the current process ID for later */

    /* initialize signal processing */
    flagsig = FALSE;		/* Process the signals */
    intstate = init;		/* We are initializing things now */
    VOIDC signal(SIGINT, GotDieSig);
    VOIDC signal(SIGHUP, GotDieSig);
    VOIDC signal(SIGTERM, GotDieSig);
    VOIDC signal(SIGALRM, GotAlarmSig);
    VOIDC signal(SIGEMT, GotEmtSig);
    VOIDC signal(SIGQUIT, CAPDebug);
#ifdef SYSV
    VOIDC sigignore(SIGPIPE);
#endif /* SYSV */    

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

		case 'n':	/* user name */
		    argc--;
		    name = *(++av);
		    break;

		case 'h':	/* host */
		    argc--;
		    host = *(++av);
		    break;

		case 'p':	/* prog */
		    argc--;
		    prog = *(++av);
		    break;
		case 'k':
		    dochecksum = 0;
		    break;
		case 'd':	/* debug CAP routines */
		    argc--;
		    debugstr = *(++av);
		    break;
		default:	/* unknown */
		    fprintf(stderr, "%s: unknown option: %s\n", prog, cp);
		    break;
	    }
	}
	else
	    accountingfile = cp;
    }

    debugp((stderr, "args: %s %s %s %s\n", prog, host, name,
	    accountingfile));

    sprintf( identbuf,"statusdict begin /jobname (%s/TranScript) def end\n",name );
    identlen = strlen(identbuf);

    /* Do CAP initialization work */
    applename = mapprtname(pname);
    if( applename == 0 ) {
	fprintf(stderr,"Can't find '%s' in /etc/cap.printers.\n",pname);
	croak(THROW_AWAY);
	}
    debugp((stderr,"UNIX printer name: '%s'; Appletalk name: '%s'\n",
	pname,applename));

    if( debugstr != NULL ) dbugarg(debugstr);
    abInit(FALSE);
    nbpInit();
    PAPInit();

    /* do printer-specific options processing */

    VerboseLog = 1;
    BannerFirst = BannerLast = 0;
    UnlinkBannerLast = 0;
    if (bannerfirst = envget("BANNERFIRST"))
	BannerFirst = atoi(bannerfirst);
    if (bannerlast = envget("BANNERLAST")) {
	switch (atoi(bannerlast)) {
	    case 0:
	    default:
		BannerLast = UnlinkBannerLast = 0;
		break;
	    case 1:
		BannerLast = 1;
		UnlinkBannerLast = 0;	/* No unlink banner */
		break;
	    case 2:
		BannerLast = UnlinkBannerLast = 1;	/* Unlink banner file */
		break;
	}
    }
    if (verboselog = envget("VERBOSELOG")) {
	VerboseLog = atoi(verboselog);
    }
    if (VerboseLog) {
#ifdef BSD
	VOIDC time(&starttime);
	fprintf(stderr, "%s: %s:%s %s start - %s", prog, host, name, pname,
	  ctime(&starttime));
	VOIDC fflush(stderr);
#endif /* BSD */	
    }
    if (q = envget("PIPELINE"))
	pipeline = atoi(q);
    debugp((stderr, "%s: pid %d ppid %d\n", prog, getpid(), getppid()));
    debugp((stderr, "%s: options BF=%d BL=%d VL=%d PL=%d \n", prog, BannerFirst,
	BannerLast, VerboseLog, pipeline));


    streamin = stdin;
    fdinput = fileno(streamin);	/* the file to print */

    doactng = name && *accountingfile != ' ' && (access(accountingfile,
							W_OK) == 0); 

    /* get control of the "status" message file. we copy the current one
       to ".status" so we can restore it on exit (to be clean). Our
       ability to use this is publicized nowhere in the 4.2 lpr
       documentation, so things might go bad for us. We will use it to
       report that printer errors condition has been detected, and the
       printer should be checked. Unfortunately, this notice may persist
       through the end of the print job, but this is no big deal. */
    BackupStatus(".status", "status");

    /* get jobout from environment if there, otherwise use stderr */
    if (((outname = envget("JOBOUTPUT")) == NULL)
    || ((jobout = fopen(outname,"w")) == NULL)) {
      jobout = stderr;
    }
    else havejobout = TRUE;

    intstate = syncstart;	/* start communications */

    rcomp = wcomp = 0;		/* No transactions outstanding */
    rdlen = 0;

    debugp((stderr, "%s: sync printer and get initial page count\n", prog));
    syncprinter(&startpagecount);	/* Make sure printer is listening */

    if (newstatmsg)
	RestoreStatus();	/* Put back old message -- we're OK now */

    STARTCRIT();		/* Child state change and setjmp() */
    intstate = sending;

    if (setjmp(sendlabel))
	goto donefile;
    ENDCRIT();		/* Child state change and setjmp() */
    if (gotsig & DIE_INT)
	closedown();
    
    debugp((stderr, "%s: printer responding\n", prog));
    progress = oldprogress = 0;	/* finite progress on sender */
    /* Send a string identifying us for others on Appletalk */
    VOIDC writebuf(identbuf,identlen,FALSE);

    /* initial break page ? */
    if (BannerFirst) {
	SendBanner();
	progress++;
	if (!BannerLast)
	    VOIDC unlink(".banner");
    }

    /* now ship the rest of the file */
    VOIDC readbuf(rdbuf,&rdlen);
    while (TRUE) {
	if (getstatus) {       /* Get printer status sometimes */
	    VOIDC write(fdsend, statusbuf, 1);
	    getstatus = FALSE;
	    progress++;
	}
	if( procrdbuf() ) {
	    parserdbuf(rdbuf,rdlen);
	    VOIDC readbuf(rdbuf,&rdlen);
	}
	if( procwrtbuf() ) {
	    cnt = read(fdinput, mybuf, sizeof mybuf);
	    if( cnt <= 0 ) break;
	    VOIDC writebuf(mybuf,cnt,FALSE);
	}
	abSleep(IOSLEEP,TRUE);
	progress++;
    }
    if (cnt < 0) {
	fprintf(stderr, "%s: error reading from stdin", prog);
	perror("");
	sleep(10);
	croak(TRY_AGAIN);
    }
    close(fdinput);

donefile:			/* Done sending the user's file */

	/* Send the PostScript end-of-job character */
	debugp((stderr, "%s: done sending\n", prog));
	while( !procwrtbuf() ) abSleep(IOSLEEP,TRUE);	/* Finish last write */
	STARTCRIT();		/* Only do end-of-job char once */
	VOIDC PAPWrite( cno,NULL,0,TRUE,&wcomp );	/* Send EOF */
	VOIDC write(fdsend, eofbuf, 1);
	intstate = waiting;	/* Waiting for end of user job */
	ENDCRIT();		/* Only do end-of-job char once */
        debugp((stderr, "%s: sent EOJ\n", prog));    
	if (gotsig & DIE_INT)
	    VOIDC kill(getpid(), SIGINT);
	while( !eof ) {		/* Wait for EOF back from printer */
	    if( procrdbuf() ) {
		parserdbuf(rdbuf,rdlen);
		VOIDC readbuf(rdbuf,&rdlen);
		}
	    abSleep(IOSLEEP,TRUE);
	    }
        debugp((stderr, "%s: read EOJ\n", prog));    
	if( cpid > 0 ) kill( cpid,SIGINT );	/* Make child go away */

	intstate = lastpart;
	if (BannerLast || doactng) {
	    if (BannerLast) {	/* final banner page */
		SendBanner();
		if (UnlinkBannerLast)
		    VOIDC unlink(".banner");
	    }
	    if (!pipeline) {
		intstate = synclast;
		syncprinter(&endpagecount);	/* Get communications in
						   sync again */
		intstate = ending;
		if (doactng)
		    VOIDC acctentry(startpagecount, endpagecount);
	    }
	}

	intstate = ending;

	if (VerboseLog) {
#ifdef BSD	    
	    VOIDC time(&starttime);
	    fprintf(stderr, "%s: end - %s", prog, ctime(&starttime));
	    VOIDC fflush(stderr);
#endif	    
	}
	RestoreStatus();
	if (havejobout) VOIDC fclose(jobout);
	PAPClose(cno);
	exit(0);
}

/* send the file ".banner" */
private SendBanner()
{
    register int banner;
    int cnt;
    char buf[BUFSIZ];

    if ((banner = open(".banner",O_RDONLY|O_NDELAY,0)) < 0) {
	debugp((stderr,"%s: No banner file\n",prog));
	return;
    }
    while( TRUE ) {
	if( procwrtbuf() ) {
	    cnt = read(banner,buf,sizeof buf);
	    if( cnt <= 0 ) break;
	    VOIDC writebuf(buf,cnt,FALSE);
	    }
	else {
	    abSleep(IOSLEEP,TRUE);
	    }
    }
    VOIDC close(banner);
}

/* search backwards from p in start for patt */
private char *FindPattern(p, start, patt)
register char *p;
         char *start;
         char *patt;
{
    int patlen;
    register char c;

    patlen = strlen(patt);
    c = *patt;
    
    p -= patlen;
    for (; p >= start; p--) {
	if (c == *p && strncmp(p, patt, patlen) == 0) return(p);
    }
    return ((char *)NULL);
}

/* Static variables for NextCh() routine */
static char linebuf[BUFSIZ];
static char *cp;
static enum {normal, onep, twop, inmessage,
	     close1, close2, close3, close4} st;
static int level;

/* Initialize the NextCh routine */
private NextChInit() {
    cp = linebuf;
    st = normal;
    level = 0;
}

/* Overflowed the NextCh() buffer */
private NextChErr()
{
    *cp = '\0';
    fprintf(stderr,"%s: Status message too long: (%s)\n",prog,linebuf);
    VOIDC fflush(stderr);
    st = normal;
    cp = linebuf;
}

/* Put one character in the status line buffer. Called by NextCh() */
#define NextChChar(c) if (cp <= linebuf+BUFSIZ-2) *cp++ = c; else NextChErr();

/* Process a char from the printer.  This picks out and processes status
 * and PrinterError messages.  The printer status file is handled IN THIS
 * ROUTINE when status messages are encountered -- there is no way to
 * control how these are handled outside this routine.
 */
private ChStat NextCh(c)
register int c;
{
    char *match, *last;

    switch ((int) st) {
	case normal:
	    if (c == '%') {
		st = onep;
		cp = linebuf;
		NextChChar(c);
		break;
	    }
	    putc(c, jobout);
	    VOIDC fflush(jobout);
	    break;
	case onep:
	    if (c == '%') {
		st = twop;
		NextChChar(c);
		break;
	    }
	    putc('%', jobout);
	    putc(c, jobout);
	    VOIDC fflush(jobout);
	    st = normal;
	    break;
	case twop:
	    if (c == '[') {
		st = inmessage;
		level++;
		NextChChar(c);
		break;
	    }
	    if (c == '%') {
		putc('%', jobout);
		VOIDC fflush(jobout);
		/* don't do anything to cp */
		break;
	    }
	    putc('%', jobout);
	    putc('%', jobout);
	    putc(c, jobout);
	    VOIDC fflush(jobout);
	    st = normal;
	    break;
	case inmessage:
	    NextChChar(c);
	    switch (c) {
		case ']':
		    st = close1;
		    level--;
		    break;
		case '[':
		    st = inmessage;
		    level++;
		    break;
		}
	    break;
	case close1:
	    NextChChar(c);
	    switch (c) {
		case '%':
		    st = close2;
		    break;
		case ']':
		    st = close1;
		    level--;
		    break;
		case ' ':
		    break;
		default:
		    st = inmessage;
		    break;
	    }
	    break;
	case close2:
	    NextChChar(c);
	    switch (c) {
		case '%':
		    st = close3;
		    break;
		case ']':
		    st = close1;
		    level--;
		    break;
		default:
		    st = inmessage;
		    break;
	    }
	    break;
	case close3:
	    switch (c) {
		case '\r':
		    st = close4;
		    NextChChar(c);
		    break;
		case ']':
		    st = close1;
		    NextChChar(c);
		    level--;
		    break;
		case '\n':
		    st = normal;
		    NextChChar(c);
		    break;
		default:
		    if (level > 0) {
			NextChChar(c);
			st = inmessage;
			break;
		    }
		    st = normal;
		    putc(c, jobout);
		    fflush(jobout);
		    break;
	    }
	    break;
	case close4:
	    switch (c) {
		case '\n':
		    st = normal;
		    NextChChar(c);
		    break;
		case ']':
		    st = close1;
		    level--;
		    NextChChar(c);
		    break;
		default:
		    if (level > 0) {
			st = inmessage;
			NextChChar(c);
			break;
		    }
		    st = normal;
		    putc(c,jobout);
		    fflush(jobout);
		    break;
	    }
	    if (st == normal) {
		/* parse complete message */
		last = cp;
		*cp = '\0';
		debugp((stderr, ">>%s", linebuf));
		if (match = FindPattern(cp, linebuf, " pagecount: ")) {
		    /* Do nothing */
		}
		else if (match = FindPattern(cp, linebuf, " PrinterError: ")) {
		    if (*(match - 1) != ':') {
			fprintf(stderr, "%s", linebuf);
			VOIDC fflush(stderr);
			*(last - 6) = 0;
			Status(match + 15);
		    }
		    else {
			last = strchr(match, ';');
			*last = '\0';
			Status(match + 15);
		    }
		}
		/* PrinterError's for certain (rare) printers */
		else if (match = FindPattern(cp, linebuf, " printer: ")) {
		    if (*(match - 1) != ':') {
			fprintf(stderr, "%s", linebuf);
			VOIDC fflush(stderr);
			*(last - 6) = 0;
			Status(match + 10);
		    }
		    else {
			last = strchr(match, ';');
			*last = '\0';
			Status(match + 10);
		    }
		}
		else if (match = FindPattern(cp, linebuf, " status: ")) {
		    match += 9;
		    if (strncmp(match, "idle", 4) == 0) {	/* Printer is idle */
			return (ChIdle);
		    }
		    else {
			/* one of: busy, waiting, printing, initializing */
			/* clear status message */
			RestoreStatus();
		    }
		}
		/* WARNING: Must NOT match "PrinterError: timeout" */
		else if (match = FindPattern(cp, linebuf, " Error: timeout")) {
		    return (ChTimeout);
		}
		else {
		    /* message not for us */
		    fprintf(jobout, "%s", linebuf);
		    VOIDC fflush(jobout);
		    st = normal;
		    break;
		}
	    }
	    break;
	default:
	    fprintf(stderr, "bad case;\n");
    }
    return (ChOk);
}

/* backup "status" message file in ".status", in case there is a PrinterError */
private BackupStatus(file1, file2)
char *file1, *file2;
{
#ifndef SYSV
    register int fd1, fd2;
    char buf[BUFSIZ];
    int cnt;

    VOIDC umask(0);
    fd1 = open(file1, O_WRONLY|O_CREAT, 0664);
    if ((fd1 < 0) || (flock(fd1,LOCK_EX) < 0)) {
	VOIDC unlink(file1);
	VOIDC flock(fd1,LOCK_UN);
	VOIDC close(fd1);
	fd1 = open(file1, O_WRONLY|O_CREAT, 0664);
    }
    if ((fd1 < 0) || (flock(fd1,LOCK_EX) <0)) {
	fprintf(stderr, "%s: writing %s; ",prog,file1);
	perror("");
	VOIDC close(fd1);
	return;
    }
    VOIDC ftruncate(fd1,0);
    if ((fd2 = open(file2, O_RDONLY,0)) < 0) {
	fprintf(stderr, "%s: error reading %s; ", prog, file2);
	perror("");
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

/* restore the "status" message from the backed-up ".status" copy */
private RestoreStatus() {
    BackupStatus("status",".status");
    newstatmsg = FALSE;		/* Say we went back to the old message */
}

/* report PrinterError via "status" message file */
private Status(msg)
register char *msg;
{
    register int fd;
    char msgbuf[1024];

    if ((fd = open("status",O_WRONLY|O_CREAT,0664)) < 0) return;
    VOIDC ftruncate(fd,0);
    sprintf(msgbuf,"Printer Error: may need attention! (%s)\n\0",msg);
    VOIDC write(fd,msgbuf,strlen(msgbuf));
    VOIDC close(fd);
    newstatmsg = TRUE;		/* Say we changed the status message */
}

/* Child has exited.  If there is a problem, this routine causes the program
 * to abort.  Otherwise, the routine just returns.
 */
#ifdef SYSV
#ifndef sgi
private VOID listenexit(exitval)
int exitval;
#else
private VOID listenexit(exitstatus)
union wait exitstatus;     /* Status returned by the child */
#endif /* sgi */
#else
private VOID listenexit(exitstatus)
union wait exitstatus;     /* Status returned by the child */
#endif
{
#ifdef SYSV
#ifndef sgi    
    stat_buf exitstatus;

    exitstatus.w_termsig = exitval & 0xFF;
    exitstatus.w_retcode = (exitval>>8) & 0xFF;
#endif
#endif    

    debugp((stderr, "%s: Listener return status: 0x%x\n", prog, exitstatus));
    if (exitstatus.w_termsig != 0) {	/* Some signal got the child */
	fprintf(stderr, "%s: Error: Listener process killed using signal=%d\n",
	  prog, exitstatus.w_termsig);
	VOIDC fflush(stderr);
	croak(TRY_AGAIN);
    }
    else {
	switch (exitstatus.w_retcode) {	/* Depends on child's exit status */
	    case LIS_IDLE:	/* Printer went idle during job. Probably
				   rebooted. */
		fprintf(stderr, "%s: ERROR: printer is idle. Giving up!\n", prog);
		VOIDC fflush(stderr);
		croak(THROW_AWAY);
	    case LIS_TIMEOUT:	/* Printer timed out during job. System
				   loaded? */
		fprintf(stderr, "%s: ERROR: printer timed out. Trying again.\n", prog);
		VOIDC fflush(stderr);
		croak(TRY_AGAIN);
	    case LIS_EOF:	/* Comm line down.  Somebody unplugged the
				   printer? */
		fprintf(stderr,
		  "%s: unexpected EOF from printer (listening)!\n", prog);
		VOIDC fflush(stderr);
		sleep(10);
		croak(TRY_AGAIN);
	    case LIS_ERROR:	/* Listener died */
		fprintf(stderr,
		  "%s: unrecoverable error from printer (listening)!\n", prog);
		VOIDC fflush(stderr);
		sleep(30);
		croak(TRY_AGAIN);
	    case LIS_NORMAL:	/* Normal exit. Keep going */
	    case LIS_DIE:	/* Parent said to die. */
	    default:
		break;
	}
    }
}

/* Reap the children.  This returns when all children are dead.
 * This routine ASSUMES we are dying.
 */
private VOID reapchildren() {
    intstate = croaking;    /* OK -- we are dying */
    VOIDC setjmp(croaklabel);    /* Get back here when we get an alarm */
    VOIDC unlink(".banner");         /* get rid of banner file */
    if (cpid != 0) VOIDC kill(cpid,SIGEMT);  /* This kills listener */
#ifdef SYSV
    while (wait((int *) 0) > 0);
#else    
    while (wait((union wait *) 0) > 0);
#endif    
    if (VerboseLog) {
	VOIDC time(&starttime);
	fprintf(stderr,"%s: end - %s",prog,ctime(&starttime));
	VOIDC fflush(stderr);
    }
}

/* Reap our children and die */
private VOID croak(exitcode)
int exitcode;
{
    PAPClose(cno);
    VOIDC reapchildren();
    RestoreStatus();
    exit(exitcode);
}

/* Exit and printer system error message with perror() */
private VOID myexit1(progname,exitcode)
char  *progname;
int    exitcode;
{
    PAPClose(cno);
    saveerror = errno;
    VOIDC reapchildren();
    RestoreStatus();
    errno = saveerror;
    pexit(progname,exitcode);
}

/* Exit and print system error message with perror() and printer a reason */
private VOID myexit2(progname,reason,exitcode)
char  *progname;
char  *reason;
int    exitcode;
{
    PAPClose(cno);
    saveerror = errno;
    VOIDC reapchildren();
    RestoreStatus();
    errno = saveerror;
    pexit2(progname, reason, exitcode);
}

/* Close down without having done anything much */
private VOID closedown() {
    fprintf(stderr,"%s: abort (during startup)\n",prog);
    VOIDC fflush(stderr);
    croak(THROW_AWAY);
}

/* On receipt of a job abort, we normally get the printer to abort the job
 * and still do the normal final banner page and accounting entry.  This
 * requires the printer to respond correctly.  If the printer is busted, it
 * will not respond.  In order to prevent this process from sitting around
 * until a working printer is connected to the printer port, this routine
 * gets called went it is deemed that the printer will not respond...
 */
private VOID dieanyway() {
    fprintf(stderr,"%s: No response from printer after abort.  Giving up!\n",
	prog);
    croak(THROW_AWAY);
}

/* Abort the current job running on the printer */
private VOID abortjob() {
    if (jobaborted) return;		/* Don't repeat work */
    jobaborted = TRUE;

    if (resetprt() || write(fdsend,abortbuf,1) != 1) {
	fprintf(stderr, "%s: ioctl error (abort job); ", prog);
	perror("");
    }
}

/* Got an EMT signal */
private VOID GotEmtSig() {

    debugp((stderr,"%s: Got SIGEMT signal, instate is %d\n",prog,(int) intstate));

    /* This signal does not need to obey the critical region rules */

    switch ((int)intstate) {
	case sending:
	case waiting:
	    while ((wpid=wait(&status)) > 0) {
		if(wpid==cpid) {
		    cpid = 0;
		    listenexit(status);
		    break;
		}
	    }
	    if (intstate == sending) {
		fprintf(stderr,"WARNING: Check spooled PostScript for control characters.\n");
		fflush(stderr);
	    }
	    childdone = TRUE;    /* Child exited somehow */
	    break;
	case child:
	    VOIDC kill(getppid(),SIGEMT);   /* Tell parent we are exiting */
	    exit(LIS_DIE);    /* Parent says die -- we exit early */
	default:
	    break;		/* Ignore it */
    }
}

/* Got an alarm signal. */
private VOID GotAlarmSig()
{
    char mybuf[BUFSIZ];

    debugp((stderr,"%s: Got alarm signal %d %d %d %d\n",
	prog,intstate,oldprogress,progress,getstatus));
    if(flagsig) {
	gotsig |= ALARM_INT;  /* We just say we got one and return */
	return;
    }
    switch ((int)intstate) {
	case syncstart:
	case synclast:
	    if (jobaborted) dieanyway(); /* If already aborted, just croak */
	    sprintf(mybuf, "Not Responding for %ld minutes",
		(time((long*)0)-starttime+30)/60);
	    Status(mybuf);
	    longjmp(synclabel,1);
	case sending:
	    if (progress == oldprogress) { /* Nothing written since last time */
		getstatus = TRUE;
	    }
	    else {
		oldprogress = progress;
		getstatus = FALSE;
	    }
	    VOIDC alarm(SENDALARM); /* reset the alarm and return */
	    break;
	case waiting:
	    if (jobaborted) dieanyway();   /* If already aborted, just croak */
	    VOIDC write(fdsend, statusbuf, 1);
	    if (kill(cpid,0) < 0) childdone = TRUE;  /* Missed exit somehow */
	    VOIDC alarm(WAITALARM); /* reset the alarm and return */
	    break;
	case lastpart:
	    if (jobaborted) dieanyway();   /* If already aborted, just croak */
	    break;
	case croaking:
	    longjmp(croaklabel,1);
	case child:
	    if (kill(getppid(),0) < 0) {  /* Missed death signal from parent */
		fprintf(stderr,
		    "%s: Error: Parent exited without signalling child\n",prog);
		VOIDC fflush(stderr);
		exit(LIS_NOPARENT);
	    }
	    alarm(CHILDWAIT);
	    break;
	default:
	    break; /* Ignore it */
	}
}


/* Got a "die" signal, like SIGINT */
private VOID GotDieSig(sig)
    int sig;
{
    debugp((stderr,"%s: Got 'die' signal=%d\n",prog,sig));
    if(flagsig) {
	gotsig |= DIE_INT;  /* We just say we got one and return */
	return;
    }
    switch ((int)intstate) {
	case init:
	    VOIDC closedown();
	case syncstart:
	    fprintf(stderr,"%s: abort (start communications)\n",prog);
	    VOIDC fflush(stderr);
	    abortjob();
	    VOIDC write(fdsend, eofbuf, 1);
	    croak(THROW_AWAY);
	case sending:
	    fprintf(stderr,"%s: abort (sending job)\n",prog);
	    VOIDC fflush(stderr);
	    abortjob();
	    longjmp(sendlabel,1);
	case waiting:
	    if (!jobaborted) {
	        fprintf(stderr,"%s: abort (waiting for job end)\n",prog);
	        VOIDC fflush(stderr);
	    }
	    abortjob();
	    break;
	case lastpart:
	case synclast:
	    if (jobaborted) dieanyway();   /* If already aborted, just croak */
	    fprintf(stderr,"%s: abort (post-job processing)\n",prog);
	    VOIDC fflush(stderr);
	    jobaborted = TRUE;
	    break;
	case child:
	    alarm(CHILDWAIT);   /* Wait a while, then see if parent is alive */
	    break;
	default:
	    break; /* Ignore it */
        }
}

/* Open the printer for listening.
 * This fcloses the old file as well, so it may be used to throw away
 * any buffered input.
 * NOTE: The printcap entry specifies "rw" and we get invoked with
 * stdout == the device, so we dup stdout, and reopen it for reading;
 * this seems to work fine...
 */
private VOID openprtread() 
{
    char mybuf[1024];
        /* CAP Open printer */
    if( psin != NULL ) return;		/* Do nothing if open already */
    seconds = OPENWAIT - 59;		/* Get first error after one minute */
    while( (err=PAPOpen(&cno,applename,FLOWSIZE,&papstatus,&ocomp)) != noErr) {
	if( err != -1 ) {
	    fprintf(stderr,"Error: PAPOpen return code: %d\n",err);
	    fflush(stderr);
	    }
	else {
	    if( seconds > OPENWAIT ) {
		sprintf(mybuf,"Can't open '%s' -- turn it on?",applename);
		Status(mybuf);
	        seconds = 0;
		}
	    }
	seconds += OPENSLEEP;
	sleep(OPENSLEEP);
	}
    while( TRUE ) {		/* Open is still executing */
	abSleep( 16,TRUE );
	if( ocomp <= 0 ) break;
	cpyp2cstr( mybuf,papstatus.StatusStr );
	Status(mybuf);
	}
    psin = (FILE *)1;				/* Flag printer as open */
}

/* Flush the I/O queues for the printer, and restart output to the
  printer if it has been XOFF'ed.
  Returns correct error stuff for perror() if a system call fails.
  NOTE: If this routine does nothing, one can get into a state where
  syncprinter() loops forever reading OLD responses to the acct message. */

private int resetprt()
{
    return (0);
}

/* Synchronize the input and output of the printer.   We use the
   accounting message, and include our process ID and
   a sequence number.  If the output doesn't match what we expect, we
   sleep a bit, flush the terminal buffers, and try again.
   WARNING: Make sure there are no pending alarms before calling this
   routine. */

private VOID syncprinter(pagecount)
    long *pagecount;		/* The current page count in the printer */
{
    static int synccount = 0;	/* Unique number for acct output.  This is
				   static so ALL calls will produce
				   different output */
    unsigned int sleeptime;	/* Number of seconds to sleep */
    char *mp;			/* Current pointer into mybuf */
    int gotpid;			/* The process ID returned from the
				   printer */
    int gotnum;			/* The synccount returned from the printer */
    register int r;		/* Place to put the chars we get */
    int sc;			/* Results from sscanf() */
    int errcnt;			/* Number of errors we have output */
    char mybuf[BUFSIZ];
    int len;			/* Number of chars read into mybuf */

    VOIDC time(&starttime);	/* Get current time for status warnings */
    errcnt = 0;
    sleeptime = 2;		/* Initial sleep interval */
    jobout = stderr;		/* Write extra stuff to the log file */
    NextChInit();
    openprtread();		/* Open the printer for reading */
    while (TRUE) {
	synccount++;
	if (setjmp(synclabel))
	    goto tryagain;	/* Got an alarm */
	while( !procwrtbuf() ) abSleep(IOSLEEP,TRUE);	/* Wait for old writes */
	VOIDC sprintf(mybuf, getpages, mpid, synccount, "");
	VOIDC writebuf(mybuf,strlen(mybuf),TRUE);
	VOIDC readbuf(rdbuf,&rdlen);
	while( TRUE ) {		/* Wait for EOF back from printer */
	    if( procrdbuf() ) {
		if( eof ) break;
		VOIDC readbuf(rdbuf,&len);
		}
	    abSleep(IOSLEEP,TRUE);
	    }
	debugp((stderr,"%s: sync reply (%s)\n",prog,rdbuf));
	mp = rdbuf + strlen(rdbuf) - 1;
	if (mp = FindPattern(mp, rdbuf, "%%[ pagecount: ")) {
	    sc = sscanf(mp, "%%%%[ pagecount: %ld, %d %d ]%%%%",
		pagecount,&gotpid,&gotnum);
	    }
	if (mp != NULL && sc == 3 && gotpid == mpid && gotnum == synccount)
	    break;
	errcnt++;
	if (errcnt <= 3 || errcnt % 10 == 0) {	/* Only give a few errors */
	    fprintf(stderr, "%s: printer sync problem [%d] (%s)\n",
	      prog, synccount, mybuf);
	    VOIDC fflush(stderr);
	}
		*pagecount = 0;
	break;			/* I/O queues can't get screwed up */
	tryagain:;
	}
}

/* Make an entry in the accounting file */
private VOID acctentry(start,end)
long start,end;         /* Starting and ending page counts for job */
{
    debugp((stderr, "%s: Make acct entry s=%ld, e=%ld\n", prog, start, end));
    if (start > end || start < 0 || end < 0) {
	fprintf(stderr, "%s: accounting error 3, %ld %ld\n", prog, start, end);
	fflush(stderr);
    }
    else if (freopen(accountingfile, "a", stdout) != NULL) {
	printf("%7.2f\t%s:%s\n", (float) (end - start), host, name);
	VOIDC fclose(stdout);
    }
}

/* Map the printer name to an Appletalk printer name.
 * This routine uses the /etc/cap.printers file.
 * Returns pointer to the mapped name, or NULL if not found.
 */
private char *mapprtname( unixname )
char  *unixname;		/* Name of the printer in UNIX */
{
    FILE *f;			/* Open file */
    static char line[256];	/* Temp line for reading file */
    char *pp;			/* UNIX name on current line */
    char *ap;			/* Appletalk printer name */
    char *p;

    if( (f=fopen("/etc/cap.printers","r")) == NULL ) {
	fprintf(stderr,"Can't open /etc/cap.printers\n");
	fflush(stderr);
	croak(THROW_AWAY);
	}
    while( fgets(line,sizeof(line),f) != NULL ) {
	line[strlen(line)-1] = '\0';		/* Get rid of newline */
	pp = line;
	while( isspace(*pp) ) pp++;	/* Skip white space */
	if( *pp == '#' || *pp == '\0' ) continue;	/* Comments and empty lines */
	if( (ap= (char *) strchr(pp,'=')) == NULL ) continue;	/* Probably bad line */
	for( p=ap; isspace(*(p-1)); p--);	/* skip backwards white space */
	*p = '\0';
	if( strcmp(unixname,pp) == 0 ) {
	    fclose(f);
	    return (ap+1);
	    }
	}
    fclose(f);
    return(NULL);
}


/* Write a buffer to the printer */
private VOID writebuf( buf,cnt,eof )
char  *buf;		/* Buffer to write */
int   cnt;		/* Number of chars to write */
int   eof;		/* TRUE = Write an EOF indication after the buffer */
{
    while( (err=PAPWrite(cno,buf,cnt,eof,&wcomp)) != noErr ) {
	if( err < 0 ) {
	    fprintf(stderr,"%s: PAPWrite error %d\n",prog,err);
	    fflush(stderr);
	    if( err == (-1101) ) {	/* noRelErr -- not really an error */
		croak(TRY_AGAIN);
		}
	    else {
		croak(THROW_AWAY);
		}
	    }
	}
}


/* Process a write buffer if we have one */
private int procwrtbuf()
{
    if( wcomp >= 0 ) return( wcomp == 0 );
    else {
	fprintf(stderr,"%s: PAPWrite error %d\n",prog,wcomp);
	fflush(stderr);
	croak(THROW_AWAY);
	}
}


/* Read a buffer from the printer */
private VOID readbuf( buf,cnt )
char  *buf;		/* Buffer to read */
int   *cnt;		/* Number of chars read */
{
    while( (err=PAPRead(cno,buf,cnt,&eof,&rcomp)) != noErr ) {
	if( err < 0 ) {
	    fprintf(stderr,"%s: PAPRead error %d\n",prog,err);
	    fflush(stderr);
	    croak(THROW_AWAY);
	    }
	}
}


/* Process a read buffer if we have one */
private int procrdbuf()
{
    if( rcomp >= 0 ) return( rcomp == 0 );
    else {
	fprintf(stderr,"%s: PAPRead error %d\n",prog,rcomp);
	fflush(stderr);
	croak(THROW_AWAY);
	}
}


/* Examine a read buffer that was received for printer errors */
private VOID parserdbuf( buf,cnt )
char  *buf;		/* Buffer read */
int   cnt;		/* Number of chars read */
{
    char *match, *last, *cp;

    if( cnt > 0 ) {
	cp = buf + cnt;
	*cp = '\0';
	debugp((stderr,">>%s",buf));
	if (match = FindPattern(cp, buf, "%%[ PrinterError: ")) {
		last = (char *)strchr(match+18,']') + 3;
		if( last == NULL ) {
		    /* message not for us */
		    fprintf(jobout,"%s",buf);
		    }
		else {
		    *last = '\0';
		    fprintf(stderr,"%s\n",match);
		    VOIDC fflush(stderr);
		    *(last-4) = '\0';
		    Status(match+18);
		    *match = '\0';
		    fprintf(jobout,"%s",buf);
		    fprintf(jobout,"%s",last+1);
		    }
		}
	else {
	    /* message not for us */
	    fprintf(jobout,"%s",buf);
	    }
	VOIDC fflush(jobout);
	}
}

private VOID CAPDebug()
{
    static int debugon = FALSE;

    if( !debugon ) dbugarg("dalnip");	/* Do all CAP debugging */
    else dbugarg("");

    debugon = !debugon;
}
