#ifndef lint
#define _NOTICE static char
_NOTICE N1[] = "Copyright (c) 1985,1987, 1992 Adobe Systems Incorporated";
_NOTICE N2[] = "GOVERNMENT END USERS: See Notice file in TranScript library directory";
_NOTICE N3[] = "-- probably /usr/lib/ps/Notice";
_NOTICE RCSID[]="$Header: /afs/dev.mit.edu/source/repository/third/transcript/src/lpcomm.c,v 1.1.1.1 1996-10-07 20:25:49 ghudson Exp $";
#endif
/* lpcomm.c, derived from pscomm.c
 *
 * Copyright (C) 1985,1987,1990,1991,1992 Adobe Systems Incorporated. All
 * rights reserved. 
 * GOVERNMENT END USERS: See Notice file in TranScript library directory
 * -- probably /usr/lib/ps/Notice
 *
 * parallel communications module.
 *
 * RCSLOG:
 * $Log: not supported by cvs2svn $
 * Revision 1.6  1994/04/08  23:27:38  snichols
 * added sigignore for SIGPIPE, surrounded by ifdef SYSV since
 * not all BSD systems have sigignore, and the problem doesn't
 * happen there anyway.
 *
 * Revision 1.5  1994/04/08  21:03:25  snichols
 * close fdinput as soon as we're done with it, to avoid SIGPIPE problems
 * on Solaris.
 *
 * Revision 1.4  1992/08/21  16:26:32  snichols
 * Release 4.0
 *
 * Revision 1.3  1992/07/29  21:41:28  snichols
 * get rid of _NFILES reference: don't need it anymore.
 *
 * Revision 1.2  1992/07/14  22:52:24  snichols
 * Updated copyright.
 *
 *
 */


#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <time.h>


/* Should be in system definitions.  Return value of wait(). */
/*
typedef struct {
    int w_termsig : 8;
    int w_retcode : 8;
    } stat_buf;
*/    

#include "transcript.h"
#include "psspool.h"
#include "config.h"

#ifdef BDEBUG
#define debugp(x) {fprintf x ; (void) fflush(stderr);}
#else
#define debugp(x)
#endif /* BDEBUG */

private char	*prog = "lpcomm";	/* invoking program name */
private int	progress, oldprogress;	/* finite progress counts */
private int	getstatus = FALSE;      /* TRUE = Query printer for status */
private int	newstatmsg = FALSE;     /* TRUE = We changed status message */
private int	jobaborted = FALSE;     /* TRUE = Aborting current job */
private long	startpagecount;         /* Page count at start of job */
private long	starttime;              /* Timer start. For status warnings */
private int	saveerror;		/* Place to save errno when exiting */

private char *verboselog;
private int VerboseLog;

/* Interrupt state variables */
typedef enum {                  /* Values for "die" interrupts, like SIGINT */
    init,                       /* Initialization */
    syncstart,                  /* Synchronize communications with printer */
    sending,                    /* Send info to printer */
    waiting,                    /* Waiting for listener to get EOF from prntr */
    lastpart,                   /* Final processing following user job */
    synclast,                   /* Syncronize communications at the end */
    ending,                     /* Cleaning up */
    croaking,			/* Abnormal exit, waiting for children to die */
    child                       /* Used ONLY for the child (listener) process */
} dievals;

private dievals intstate;       /* State of interrupts */
private flagsig;                /* TRUE = On signal receipt, just set flag */
#define DIE_INT    1            /* Got a "die" interrupt. like SIGINT */
#define ALARM_INT  2            /* Got an alarm */
#define EMT_INT    4            /* Got a SIGEMT signal (child-parent comm.) */
private int gotsig;             /* Mask that may take any of the above values */

private char abortbuf[] = "\003";	/* ^C abort */
private char statusbuf[] = "\024";	/* ^T status */
private char eofbuf[] = "\004";		/* ^D end of file */

/* global file descriptors (avoid stdio buffering!) */
private int fdsend;		/* to printer (from stdout) */
private int fdinput;		/* file to print (from stdin) */

private FILE *psin=NULL;        /* Buffered printer input */
private FILE *jobout;		/* special printer output log */

extern char *getenv();
extern int errno;

private VOID    GotDieSig();
private VOID    closedown();
private VOID    myexit1(),myexit2();
private VOID    croak();
private int resetprt();

/* The following are alarms settings for various states of this program */
#define SENDALARM 90		/* Status check while sending job to printer */
#define WAITALARM 90            /* Status check while waiting for job to end */
#define CROAKALARM 10           /* Waiting for child processes to die */
#define CHILDWAIT 60		/* Child waiting to die -- checking on parent */
#define ABORTALARM 90           /* Waiting for printer to respond to job abort */
#define SYNCALARM 30            /* Waiting for response to communications sync */
#define MAXSLEEP  15            /* Max time to sleep while sync'ing printer */

main(argc,argv)            /* MAIN ROUTINE */
	int argc;
	char *argv[];
{
    register char  *cp;
    register int cnt, wc;
    register char *mbp;

    char  **av;
    FILE *streamin;

    char mybuf[BUFSIZ];
    int format = 0;
    int i;


    /* initialize signal processing */
    flagsig = FALSE;           /* Process the signals */
    intstate = init;       /* We are initializing things now */
    VOIDC signal(SIGINT, GotDieSig);
    VOIDC signal(SIGHUP, GotDieSig);
    VOIDC signal(SIGTERM, GotDieSig);
#ifdef SYSV
    VOIDC sigignore(SIGPIPE);
#endif /* SYSV */		    

    /* parse command-line arguments */
    /* the argv (see header comments) comes from the spooler daemon */
    /* itself, so it should be canonical, but at least one 4.2-based */
    /* system uses -nlogin -hhost (insead of -n login -h host) so I */
    /* check for both */

    av = argv;
    if (prog = strrchr(*av,'/')) prog++;
    else prog = *av;

    debugp((stderr,"args: %s \n",prog));

    /* do printer-specific options processing */

    if (verboselog=envget("VERBOSELOG")) {
	VerboseLog=atoi(verboselog);
    }

    if (VerboseLog) {
	VOIDC time(&starttime);
	fprintf(stderr, "%s: start - %s", prog, ctime(&starttime));
	VOIDC fflush(stderr);
    }
    debugp((stderr,"%s: pid %d ppid %d\n",prog,getpid(),getppid()));
    debugp((stderr,"%s: options VL=%d\n",prog,VerboseLog));

    /* IMPORTANT: in the case of cascaded filters, */ 
    /* stdin may be a pipe! (and hence we cannot seek!) */


    streamin = stdin;
    fdinput = fileno(streamin); /* the file to print */

    fdsend = fileno(stdout);	/* the printer (write) *

    intstate = sending;

    progress = oldprogress = 0; /* finite progress on sender */


    VOIDC alarm(0); /* NO ALARMS. They screw up write() */

    while ((cnt = read(fdinput, mybuf, sizeof mybuf)) > 0) {
	mbp = mybuf;
	while ((cnt > 0) && ((wc = write(fdsend, mbp, (unsigned)cnt)) !=
			     cnt)) { 
	    if (wc < 0) {
		fprintf(stderr,"%s: error writing to printer",prog);
		perror("");
		sleep(10);
		croak(TRY_AGAIN);
	    }
	    mbp += wc;
	    cnt -= wc;
	    progress++;
	}
	progress++;
    }
    close(fdinput);
    /* Send the PostScript end-of-job character */
    debugp((stderr,"%s: done sending\n",prog));
    VOIDC write(fdsend, eofbuf, 1);
    intstate = ending;

    if (VerboseLog) {
	VOIDC time(&starttime);
	fprintf(stderr,"%s: end - %s",prog,ctime(&starttime));
	VOIDC fflush(stderr);
    }
    exit(0);
}

/* Close down without having done anything much */
private VOID closedown() {
    fprintf(stderr,"%s: abort (during startup)\n",prog);
    VOIDC fflush(stderr);
    croak(THROW_AWAY);
}

/* Abort the current job running on the printer */
private VOID abortjob() {
    if( jobaborted ) return;		/* Don't repeat work */
    jobaborted = TRUE;

    if(write(fdsend,abortbuf,1) != 1 ) {
	fprintf(stderr, "%s: error (abort job)", prog);
	perror("");
    }
}

/* Got a "die" signal, like SIGINT */
private VOID GotDieSig(sig)
    int sig;
    {
    debugp((stderr,"%s: Got 'die' signal=%d\n",prog,sig));
    VOIDC signal(sig, GotDieSig);
    if(flagsig) {
	gotsig |= DIE_INT;  /* We just say we got one and return */
	return;
	}
    switch( (int)intstate ) {
	case init:
	    VOIDC closedown();
	case sending:
	    fprintf(stderr,"%s: abort (sending job)\n",prog);
	    VOIDC fflush(stderr);
	    abortjob();
	    break;
	default:
	    break; /* Ignore it */
        }
}

static VOID croak(exitcode)
{
    exit(exitcode);
}
