#ifndef lint
#define _NOTICE static char
_NOTICE N1[] = "Copyright (c) 1985,1987 Adobe Systems Incorporated";
_NOTICE N2[] = "GOVERNMENT END USERS: See Notice file in TranScript library directory";
_NOTICE N3[] = "-- probably /usr/lib/ps/Notice";
_NOTICE RCSID[]="$Id: psbanner.c,v 1.1 1993-10-12 05:28:12 probe Exp $";
#endif

/* psbanner.c
 *
 * Copyright (C) 1985,1987 Adobe Systems Incorporated. All rights reserved.
 * GOVERNMENT END USERS: See Notice file in TranScript library directory
 * -- probably /usr/lib/ps/Notice
 *
 * 4.2BSD lpr/lpd banner break page filter for PostScript printers
 * (formerly "psof" in TranScript release 1.0)
 *
 * psbanner is used ONLY to print the "break page" --
 * the job seperator/banner page; the "if" filter
 * which invokes (pscomm) MUST be in
 * the printcap database for handling communications.
 *
 * This filter does not actually print anything; it sends
 * nothing to the printer (stdout).  Instead, it writes a
 * file in the current working directory (which is the
 * spooling directory) called ".banner" which, depending
 * on the value of printer options BANNERFIRST and
 * BANNERLAST, may get printed by pscomm filter when
 * it is envoked.
 *
 * psbanner parses the standard input (the SHORT banner string)
 * into PostScript format as the argument to a "Banner"
 * PostScript procedure (defined in $BANNERPRO, from the environment)
 * which will set the job name on the printer and print a
 * banner-style job break page.
 *
 * NOTE that quoting characters into PostScript form is
 * necessary.
 *
 * psbanner gets called with:
 *	stdin	== the short banner string to print (see below)
 *	stdout	== the printer (not used here)
 *	stderr	== the printer log file
 *	argv	== (empty)
 *	cwd	== the spool directory
 *	env	== VERBOSELOG, BANNERPRO
 *
 * An example of the "short banner string" input is:
adobe:shore  Job: test.data  Date: Tue Sep 18 16:22:33 1984
^Y^A
 * where the ^Y^A are actual control characters signalling
 * the end of the banner input and the time to do SIGSTOP
 * ourselves so that the controlling lpd will
 * invoke the next filter. (N.B. this is, regretably, NOT
 * documented in any of the lpr/printcap/spooler manual pages.)
 *
 *
 * I have decided to let the PostScript side try to parse the
 * string if possible, since it's easier to change the PS code
 * then to do so in C.  If you don't like the way the
 * banner page looks, change the BANNERPRO file to
 * do something else.
 *
 * RCSLOG:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  93/08/23  16:32:21  probe
 * Initial revision
 * 
 * Revision 1.2  89/05/22  10:50:36  epeisach
 * Fixed syntax which cc on RT complained. '\(' caused failure.
 * 
 * Revision 1.1  89/05/22  10:47:20  epeisach
 * Initial revision
 * 
 * Revision 2.2  87/11/17  16:50:39  byron
 * Release 2.1
 * 
 * Revision 2.1.1.5  87/11/12  13:40:38  byron
 * Changed Government user's notice.
 * 
 * Revision 2.1.1.4  87/06/18  16:05:00  byron
 * Removed unlink of banner file from interrupt processing.  pscomm may
 * need it after the interrupt, and will delete it itself.
 * 
 * Revision 2.1.1.3  87/06/09  14:10:46  byron
 * Only do the .banner file if pscomm will use it.
 * Also, fixed a couple of bugs in the previous revision.
 * 
 * Revision 2.1.1.2  87/06/04  10:55:37  byron
 * Added error handling for banner buffer overflow.  Buffer could overflow
 * before, producing random (bad) problems and psbanner crashes.
 * 
 * Revision 2.1.1.1  87/04/23  10:25:48  byron
 * Copyright notice.
 * 
 * Revision 2.2  86/11/02  14:19:27  shore
 * Product Update
 * 
 * Revision 2.1  85/11/24  11:49:49  shore
 * Product Release 2.0
 * 
 * Revision 1.1  85/11/20  01:02:26  shore
 * Initial revision
 */

#include <stdio.h>
#include <signal.h>
#include <strings.h>
#include "transcript.h"
#include "psspool.h"

#ifndef BPRO_ENV
#define BPRO_ENV "BANNERPRO"
#endif

#ifdef BDEBUG
#define debugp(x) fprintf x; fflush(stderr);
#else
#define debugp(x)
#endif BDEBUG

private char	*prog;	/* invoking program name (i.e., psbanner) */
private char	*pname;	/* intended printer ? */

#define BANBUFSIZE 500     /* Size of banner string input buffer */
private char	bannerbuf[BANBUFSIZE];	/* PS'd short banner string */
private int     bannerfull;     /* TRUE when bannerbuf is full */
private char	*verboselog;	/* do log file reporting (from env) */
private char	*bannerpro;	/* prolog file name (from env) */
private int	VerboseLog;
private int     dobanner;       /* True = We are printing banners */

private VOID	on_int();

main(argc,argv)
	int argc;
	char *argv[];
{
    register int c;
    register char *bp;
    register FILE *in = stdin;
    int done = 0;
    char  *p;

    VOIDC signal(SIGINT, on_int);

    VOIDC fclose(stdout); /* we don't talk to the printer */
    prog = *argv; /* argv[0] == program name */
    pname = *(++argv);

    VerboseLog = 1;
    if (verboselog = envget("VERBOSELOG")) {
	VerboseLog = atoi(verboselog);
    }

    dobanner = 0;   /* Assume we will not be doing banners */
    p = envget("BANNERFIRST");
    if( p != NULL && atoi(p) != 0 ) dobanner = 1;
    p = envget("BANNERLAST");
    if( p != NULL && atoi(p) != 0 ) dobanner = 1;

    bannerpro = envget(BPRO_ENV);

    bp = bannerbuf;
    bannerfull = 0;
    while (!done) {
	switch (c = getc (in)) {
	    case EOF: /* this doesn't seem to happen */
		done = 1;
		break;
	    case '\n':
	    case '\f':
	    case '\r': 
		break;
	    case '\t': 
		if (!bannerfull) *bp++ = ' ';
		break;
	    case '\\': case '(': case ')': /* quote chars for POSTSCRIPT */
	        if (!bannerfull) {
		    *bp++ = '\\';
		    *bp++ = c;
		}
		break;
	    case '\031': 
		if ((c = getc (in)) == '\1') {
		    *bp = '\0';
		    /* done, ship the banner line */
		    if (bp != bannerbuf) {
			if(dobanner) DoBanner();  /* Do it if we will use it */
			if (verboselog) {
			    fprintf(stderr, "%s: %s\n", prog, bannerbuf);
			    VOIDC fflush(stderr);
			}
		    }
		    VOIDC kill (getpid (), SIGSTOP);
		    /* do we get continued for next job ? */
		    debugp((stderr,"%s: continued %d\n",
		    	prog,bp == bannerbuf));
		    bp = bannerbuf;
		    *bp = '\0';
		    bannerfull = 0;
		    break;
		}
		else {
		    VOIDC ungetc(c, in);
		    if (!bannerfull) *bp++ = ' ';
		    break;
		}
	    default: 		/* simple text */
		if (!bannerfull) *bp++ = c;
		break;
	}
	if (!bannerfull && bp >= &bannerbuf[BANBUFSIZE-3]) {
	    *bp++ = '\n'; *bp++ = '\0';
	    fprintf (stderr,"%s: banner buffer overflow\n",prog);
	    VOIDC fflush(stderr);
	    bannerfull = 1;
	}
    }
 /* didn't expect to get here, just exit */
 debugp((stderr,"%s: done\n",prog));
 VOIDC unlink(".banner");
 exit (0);
}

#define	STRSIZE	128

private DoBanner() {
    register FILE *out;
    char user[9];
    char host[STRSIZE];
    char jname[STRSIZE];
    char command[256];
    char *laps_cmd;
    char *date;
    register char *cp;
    register int i;

#if defined(_AIX) || defined(SOLARIS)
    memset(user, 0, 9);
    memset(host, 0, STRSIZE);
    memset(jname, 0, STRSIZE);
#else
    bzero(user, 9);
    bzero(host, STRSIZE);
    bzero(jname, STRSIZE);
#endif

    for (i = 0, cp = bannerbuf; (i < STRSIZE) && (*cp != ':'); i++, cp++)
	host[i] = *cp;
    host[STRSIZE - 1] = '\0';

    if (i == STRSIZE)
	goto dooutput;

    cp++;				/* past the colon */
    for (i = 0; i < 8 && !isspace(*cp); i++, cp++)
	user[i] = *cp;

    while (isspace(*cp))		/* get to whitespace (end of name) */
	cp++;
    while (isspace(*cp))		/* get past whitespace */
	cp++;
    while (!isspace(*cp))		/* get to whitespace (past Job:)*/
	cp++;
    while (isspace(*cp))		/* get past whitespace */
	cp++;
    /* now we are looking at the job name */
    for (i = 0; i < STRSIZE && !isspace(*cp); i++, cp++)
	jname[i] = *cp;
    jname[STRSIZE - 1] = '\0';

    if (i == STRSIZE)
	goto dooutput;

    while (isspace(*cp))		/* get past whitespace */
	cp++;
    while (!isspace(*cp))		/* get to whitespace (past Date:)*/
	cp++;
    while (isspace(*cp))		/* get past whitespace */
	cp++;

    date = cp;
    
dooutput:
    if ((out = fopen(".banner","w")) == NULL) {
	fprintf(stderr,"%s: can't open .banner", prog);
	VOIDC fflush(stderr);
	exit (THROW_AWAY);
    }
    if (verboselog) {
	fprintf(stderr, "%s: %s\n", prog, bannerbuf);
	VOIDC fflush(stderr);
    }
    if (copyfile(bannerpro,out)) {
	/* error copying file, don't print a break page */
	fprintf(stderr,"%s: trouble writing .banner\n",prog);
	VOIDC fflush(stderr);
	VOIDC unlink(".banner");
	return;
    }
    /* order of fields is:
       note	(unused)
       client	(host:user)
       jfname	(filename)
       filespec (filename path)
       account	(username)
       submitdate (date)
       printq 	(queue printed on) (printer name)

       */
    fprintf(out, "()(%s)(%s)(%s)(%s@%s)(%s)(%s) do_flagpage end\n",
	    user,
	    jname,
	    jname,
	    user,
	    host,
	    date,
	    pname);
    VOIDC fclose(out);	/* this does a flush */

    laps_cmd = envget("LAPS");
    if (laps_cmd) {
	sprintf(command, "%s %s <.banner", laps_cmd, pname);
	system(command);
    }
}

private VOID on_int() {
    /* NOTE: Let pscomm unlink banner -- it may want to use it */
    exit (THROW_AWAY);
}
