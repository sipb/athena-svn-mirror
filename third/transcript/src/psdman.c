#ifndef lint
#define _NOTICE static char
_NOTICE N1[] = "Copyright (c) 1990,1991,1992 Adobe Systems Incorporated";
_NOTICE N2[] = "GOVERNMENT END USERS: See Notice file in TranScript library directory";
_NOTICE N3[] = "-- probably /usr/lib/ps/Notice";
_NOTICE RCSID[]="$Header: /afs/dev.mit.edu/source/repository/third/transcript/src/psdman.c,v 1.1.1.1 1996-10-07 20:25:51 ghudson Exp $";
#endif
/* psdman.c
 *
 *
 * Copyright (C) 1990,1991,1992 Adobe Systems Incorporated. All rights
 * reserved. 
 * GOVERNMENT END USERS: See Notice file in TranScript library directory
 * -- probably /usr/lib/ps/Notice
 *
 *
 *  PostScript document manager - provides services for PostScript files
 *  that conform to the Document Structure Conventions.
 *
 *
 * $Log: not supported by cvs2svn $
 * Revision 3.5  1993/04/28  17:30:29  snichols
 * make compiler happy.
 *
 * Revision 3.4  1992/08/21  16:26:32  snichols
 * Release 4.0
 *
 * Revision 3.3  1992/07/14  22:41:44  snichols
 * Updated copyright.
 *
 * Revision 3.2  1992/05/26  21:53:27  snichols
 * comments after endif
 *
 * Revision 3.1  1992/05/18  20:06:26  snichols
 * Change the default behavior to proceed despite errors.  Add switch
 * (-s) to override this.
 *
 * Revision 3.0  1991/06/17  16:46:10  snichols
 * Release3.0
 *
 * Revision 1.28  1991/06/12  00:08:50  snichols
 * should include time.h, not sys/time.h.
 *
 * Revision 1.27  1991/05/08  23:09:45  snichols
 * set control.force to FALSE *prior* to calling ParseArgs, so an arg
 * can override it.
 *
 * Revision 1.26  1991/04/30  00:07:42  snichols
 * another typo.
 *
 * Revision 1.25  1991/04/30  00:05:17  snichols
 * typo.
 *
 * Revision 1.24  1991/04/30  00:02:26  snichols
 * use psdman instead of prog on error messages, to distinguish from pscomm.
 *
 * Revision 1.23  1991/04/29  23:52:07  snichols
 * make enscript's default behavior more like a line printer, when invoked
 * from psdman.
 *
 * Revision 1.22  1991/04/29  23:34:07  snichols
 * modify args string to accept new arguments.
 *
 * Revision 1.21  1991/04/29  23:12:37  snichols
 * re-ordered things so that the default values didn't override command
 * line values.
 *
 * Revision 1.20  1991/04/29  23:09:48  snichols
 * typo.
 *
 * Revision 1.19  1991/04/29  23:05:44  snichols
 * add more command-line options for controlling what psdman does; the
 * underlying interface was already there, just needed to expose it.
 *
 * Revision 1.18  1991/04/29  04:51:44  snichols
 * Use TempDir, rather than tempdir.
 *
 * Revision 1.17  1991/03/27  00:17:44  snichols
 * removed -X from enscript call.
 *
 * Revision 1.16  1991/03/25  21:27:16  snichols
 * changed the way SEEK_SET/L_SET problem is handled.
 *
 * Revision 1.15  1991/03/25  20:21:30  snichols
 * should be fcntl.h, not sys/fcntl.h
 *
 * Revision 1.14  1991/03/01  14:06:13  snichols
 * Use new -X switch of enscript to proceed despite certain errors.
 *
 * Revision 1.13  91/01/23  16:31:33  snichols
 * Added support for landscape, cleaned up some defaults, and
 * handled *PageRegion features better.
 * 
 * Revision 1.12  91/01/21  14:51:04  snichols
 * Fixed mistake in args for enscript in SYSV; handle args from SYSV
 * correctly.
 * 
 * Revision 1.11  91/01/17  13:22:53  snichols
 * extra debugging stuff.
 * 
 * Revision 1.10  91/01/16  15:51:50  snichols
 * SysV args are different, so handle them differently.
 * 
 * Revision 1.9  91/01/16  14:14:00  snichols
 * Added support for LZW compression and ascii85 encoding for Level 2 printers.
 * 
 * Revision 1.8  91/01/04  13:13:14  snichols
 * temporary fix to use global ResourceDir... this will all change
 * when new resource stuff is available.
 * 
 * Revision 1.7  91/01/03  15:32:53  snichols
 * include sys/fcntl.h
 * 
 * Revision 1.6  91/01/02  16:35:11  snichols
 * conditional compile for XPG3 to use SEEK_SET instead of L_SET in lseek.
 * 
 * Revision 1.5  91/01/02  16:12:54  snichols
 * sys/types.h needs to be included before sys/file.h in sysv.
 * 
 * Revision 1.4  90/12/12  10:25:41  snichols
 * new configuration stuff.
 * 
 * Revision 1.3  90/11/20  15:47:19  snichols
 * correctly interprets environment to decide on reversal now.
 * 
 * Revision 1.2  90/11/16  14:35:49  snichols
 * Removed unnecessary code; handles spooling to a temp file from a pipe
 * better; fixed bug with text to PS conversion.
 * 
 *
 */


#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#ifdef XPG3
#include <unistd.h>
#endif 
#include <sys/types.h> 
#include <sys/file.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>   

#include "transcript.h"
#include "psspool.h"
#include "psparse.h"
#include "config.h"

#ifdef DEBUG
#define debugp(x) {fprintf x ; (void) fflush(stderr);}
#else
#define debugp(x)
#endif 


extern char *getenv();
extern int errno;


char *pname;
char *name;
char *host;
char *prog;
char *accountingfile;
char *reverse;

char *compress;

struct controlswitches control;

private int verboseLog;
private long starttime;
private int saveerror;

extern char *optarg;
extern int optind;

private int fpid = 0;                  /* formatter pid */
private int mpid = 0;                  /* current process pid */

private VOID myexit1(), myexit2();
private VOID croak();

private int fdsend;
private int fdinput;

private char magic[14];
private char magiccnt;

int noReverse = 0;

FILE *streamin;

char mybuf[BUFSIZ];

int fdpipe[2];



#define ARGS "P:n:p:h:rfGFaLs"

static void ParseArgs(ac,av)
    int ac;
    char **av;
{
    int c;
    char *envalue;

    /* parse command line */

    while ((c = getopt(ac,av,ARGS)) != EOF) {
	switch (c) {
	case 'P' :
	    pname = optarg;
	    break;
	case 'n' :
	    name = optarg;
	    break;
	case 'h' :
	    host = optarg;
	    break;
	case 'p' :
	    prog = optarg;
	    break;
	case 'r' :
	    noReverse = TRUE;
	    break;
	case 's':
	    control.force = FALSE;
	    break;
	case 'f':
	    control.force = TRUE;
	    break;
	case 'G':
	    control.norearrange = TRUE;
	    break;
	case 'F':
	    control.noparse = TRUE;
	    break;
	case 'a':
	    control.strip = TRUE;
	    break;
	case 'L':
	    control.landscape = TRUE;
	    break;
	case '?' :
	    break;
	default: break;
	}
    }
    /* process the rest of args */

    if (optind < ac)
	accountingfile = av[optind];

    /* printer specific stuff */

    verboseLog = 1;
    if (envalue = getenv("VERBOSELOG"))
	verboseLog = atoi(envalue);
    
    if (!noReverse)
	reverse = getenv("REVERSE");
    if (reverse) {
	if (*reverse == '\0')
	    reverse = NULL;
    }
    compress = getenv("COMPRESS");
    if (compress) {
	if (*compress == '\0')
	    compress = NULL;
    }
}

private int Text2PS()
{
    int i;
    int tmp;
    int cnt;
    char converter[1024];

    if ((magiccnt = read(fdinput,magic,sizeof(magic))) > 0) {
	for (i = 0; i < magiccnt; i++) {
	    if (!isascii(magic[i]) || (!isprint(magic[i]) &&
				       !isspace(magic[i]))) {
		fprintf(stderr, "psdman: Error: spooled binary file rejected.\n");
		VOIDC fflush(stderr);
		
		sprintf(mybuf, "%s/bogusmsg.ps", envget("PSLIBDIR"));
		
		if ((streamin = freopen(mybuf, "r", stdin)) == NULL) {
		    croak(THROW_AWAY);
		}
		return 0;
	    }
	}
    }

#ifdef DEBUG    
    time(&starttime);
    fprintf(stderr, "psdman: ready to do enscript at %s\n", ctime(&starttime));
#endif
    strncpy(converter,bindir,1024);
    strncat(converter,"enscript",1024);
    lseek(fdinput, 0L, SEEK_SET);
    rewind(streamin);
    if (pipe(fdpipe))
	myexit2("psdman", "format pipe", THROW_AWAY);
    debugp((stderr, "psdman: Calling fork\n"));
    if ((fpid = fork()) < 0)
	myexit2("psdman", "format fork", THROW_AWAY);
    debugp((stderr, "psdman: After fork, fpid is %d (process %d)\n", fpid, getpid()));
    if (fpid == 0) {
	if (close(1) || (dup(fdpipe[1]) != 1) || close(fdpipe[1]) ||
	  close(fdpipe[0]))
	    myexit2("psdman", "format child", THROW_AWAY);
	debugp((stderr, "psdman: Exec'ing enscript (process %d).\n", getpid()));
#ifdef SYSV	
	execl(converter,"enscript","-p","-","-q", "-l", "-d", pname, 0);
#else
	execl(converter,"enscript","-p","-","-q", "-l", "-P", pname, 0);
#endif
	debugp((stderr, "psdman: Should not see this.\n"));
	myexit2("psdman", "format exec", THROW_AWAY);
    }
    /* parent continues */
    /* set up stdin to be pipe */
#ifdef DEBUG    
    fprintf(stderr, "Fell through here, on way to reversal if needed (process %d).\n", getpid());
    time(&starttime);
    fprintf(stderr, "After enscript, time is %s.\n",ctime(&starttime));
#endif
    if (close(0) || (dup(fdpipe[0]) != 0) || close(fdpipe[0]) || close(fdpipe[1]))
	myexit2("psdman", "format parent", THROW_AWAY);

    /* fall through with new stdin */
    /* can't seek here but we should be at right place */
    streamin = fdopen(0, "r");
    return 1;
}

static void FixPipe(fd)
    int fd;
{
    struct stat sbuf;
    char *tmp;
    char *envalue;
    char *tempfilename;
    int fdtmp;
    int cnt;

    /* if input is pipe, copy to temp file */
    (void) fstat(fd, &sbuf);
    if ((sbuf.st_mode & S_IFMT) != S_IFREG) {
	if ((envalue = envget("PSTEMPDIR")) != NULL)
	    tmp = envalue;
	else
	    tmp = TempDir;
	if ((tempfilename = (char *)tempnam(tmp,"TS")) == NULL) 
	    myexit2("psdman","creating temp file name", THROW_AWAY);
	if ((fdtmp = open(tempfilename, O_WRONLY | O_CREAT, 0600)) == -1)
	    myexit2("psdman", "open temp file", THROW_AWAY);
	while ((cnt = read(fdinput, mybuf, sizeof(mybuf))) > 0) {
	    if (write(fdtmp,mybuf,cnt) != cnt)
		perror("psdman");
	}
	if (cnt < 0)
	    myexit2("psdman","copying file", THROW_AWAY);
	close(fdtmp);
	close(fd);
	streamin = freopen(tempfilename,"r",stdin);
	fdinput = fileno(streamin);
	unlink(tempfilename);
    }
}
	    



main(argc,argv)
    int argc;
    char **argv;
{
    register int cnt;
    int tmp;
    int i;
    long last;
    enum status fs;
    enum status textfs;
    int status;
    FILE *fpipe;
    int fdout;
    int filtering;

    mpid = getpid();
#ifdef SYSV
    if (prog = (char *)strrchr(argv[0],'/')) prog++;
    else prog = argv[0];
#endif
    control.norearrange = FALSE;
    control.noparse = FALSE;
    control.strip = FALSE;
    control.nfeatures = 0;
    control.force = TRUE;
    ParseArgs(argc, argv);

#ifdef DEBUG
    fprintf(stderr,"args to psdman: ");
    for (i = 0; i<argc; i++)
	fprintf(stderr,"%s ", argv[i]);
    fprintf(stderr,"\n");
#endif /* DEBUG   */
    if (verboseLog) {
	VOIDC time(&starttime);
#ifdef SYSV
	fprintf(stderr,"psdman: start %s - %s", pname, ctime(&starttime));
#else	
	fprintf(stderr, "psdman: %s:%s %s start - %s",
	  host, name, pname, ctime(&starttime));
#endif
	VOIDC fflush(stderr);
    }
    streamin = stdin;
    fdinput = fileno(streamin);

    /* check to see if input is pipe */
    FixPipe(fdinput);

    if (reverse)
	control.reverse = TRUE;
    else control.reverse = FALSE;
    if (compress)
	control.compress = TRUE;
    else control.compress = FALSE;

#ifdef DEBUG
    time(&starttime);
    fprintf(stderr,"psdman: calling HandleComments at %s.\n",ctime(&starttime));
#endif
    fs = HandleComments(streamin,pname,ResourceDir,control);
#ifdef DEBUG
    time(&starttime);
    fprintf(stderr,"psdman: after HandleComments at %s.\n",ctime(&starttime));
    fprintf(stderr,"psdman: status is %d.\n",fs);
#endif
    switch (fs) {
    case handled:
    case notcon:
	rewind(streamin);
	lseek(fdinput, 0L, SEEK_SET);
	fdsend = fileno(stdout);
	if ((filtering = SetupCompression(&fpipe,fdsend,control)))
	    fdout = fileno(fpipe);
	else
	    fdout = fdsend;
	while ((cnt = read(fdinput,mybuf,BUFSIZ)) > 0) 
	    write(fdout,mybuf,cnt);
	close(fdout);
	if (filtering)
	    wait(&status);
	break;
    case notps:
	rewind(streamin);
	lseek(fdinput, 0L, SEEK_SET);
	if (!Text2PS()) {
	    fseek(streamin,0L,SEEK_SET);
	    lseek(fdinput, 0L, SEEK_SET);
	    fdsend = fileno(stdout);
	    while ((cnt =  read(fdinput,mybuf,BUFSIZ)) > 0)
		write(fdsend,mybuf,cnt);
	}
	else {
	    FixPipe(fdinput);
	    /* we were successful, now run through HandleComments again */
	    textfs = HandleComments(streamin,pname,ResourceDir,control);
	    if (textfs != success) {
		rewind(streamin);
		lseek(fdinput, 0L, SEEK_SET);
		fdsend = fileno(stdout);
		while ((cnt = read(fdinput,mybuf,BUFSIZ)) > 0)
		    write(fdsend,mybuf,cnt);
	    }
	}
    case success:
	rewind(streamin);
#ifdef DEBUG
	time(&starttime);
	fprintf(stderr,"psdman: calling HandleOutput at %s.\n",ctime(&starttime));
#endif
	HandleOutput(streamin,stdout,pname,control);
#ifdef DEBUG
	time(&starttime);
	fprintf(stderr,"psdman: after HandleComments at %s.\n",ctime(&starttime));
#endif
	break;
    }
}

/* Reap the children.  This returns when all children are dead.
 * This routine ASSUMES we are dying.
 */
private VOID reapchildren() {
    VOIDC unlink(".banner");         /* get rid of banner file */
    if( fpid != 0 ) VOIDC kill(fpid,SIGINT);
    if (verboseLog) {
	VOIDC time(&starttime);
	fprintf(stderr,"psdman: end - %s",ctime(&starttime));
	VOIDC fflush(stderr);
    }
}

/* Reap our children and die */
private VOID croak(exitcode)
int exitcode;
{
    VOIDC reapchildren(); 
    exit(exitcode);
}

/* Exit and print system error message with perror() and print a reason */
private VOID myexit2(progname,reason,exitcode)
char  *progname;
char  *reason;
int    exitcode;
{
    saveerror = errno; 
    VOIDC reapchildren(); 
    errno = saveerror;
    pexit2(progname,reason,exitcode);
}

    
