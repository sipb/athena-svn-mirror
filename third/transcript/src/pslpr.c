#ifndef lint
#define _NOTICE static char
_NOTICE N1[] = "Copyright (c) 1990,1991,1992 Adobe Systems Incorporated";
_NOTICE N2[] = "GOVERNMENT END USERS: See Notice file in TranScript library directory";
_NOTICE N3[] = "-- probably /usr/lib/ps/Notice";
_NOTICE RCSID[]="$Header: /afs/dev.mit.edu/source/repository/third/transcript/src/pslpr.c,v 1.1.1.1 1996-10-07 20:25:51 ghudson Exp $";
#endif
/* pslpr.c
 *
 * Copyright (C) 1990,1991,1992 Adobe Systems Incorporated. All rights
 * reserved. 
 * GOVERNMENT END USERS: See Notice file in TranScript library directory
 * -- probably /usr/lib/ps/Notice
 *
 *
 *
 * $Log: not supported by cvs2svn $
 * Revision 3.6  1993/04/28  17:31:30  snichols
 * make compiler happy.
 *
 * Revision 3.5  1992/08/21  16:26:32  snichols
 * Release 4.0
 *
 * Revision 3.4  1992/07/14  22:40:47  snichols
 * Updated copyright.
 *
 * Revision 3.3  1992/05/18  20:10:33  snichols
 * New switches for specifying shrink to fit and over-translating on landscape.
 *
 * Revision 3.2  1992/05/05  22:10:11  snichols
 * added switch for adding showpage to end of file.
 *
 * Revision 3.1  1992/05/05  21:22:26  snichols
 * support for handling multiple files on the command line.
 *
 * Revision 3.0  1991/06/17  16:46:10  snichols
 * Release3.0
 *
 * Revision 1.19  1991/03/25  21:28:20  snichols
 * some clean up.
 *
 * Revision 1.18  1991/03/25  20:21:30  snichols
 * should be fcntl.h, not sys/fcntl.h
 *
 * Revision 1.17  1991/03/06  00:37:01  snichols
 * leave resourcepath null unless explicitly set on command line.  PSres
 * package will handle any enviroment variables.
 *
 * Revision 1.16  1991/02/19  15:52:18  snichols
 * used "-t" instead of "-o" when building the "-o" arg list.
 *
 * Revision 1.15  91/02/19  15:51:10  snichols
 * The SysV spooler switch "-o" takes an argument.
 * 
 * Revision 1.14  91/02/04  13:56:26  snichols
 * Since we don't always have a input file name, don't use it in the error 
 * message.
 * 
 * Revision 1.13  91/01/24  14:47:16  snichols
 * need to use -d printer instead of -P printer for SysV.
 * 
 * Revision 1.12  91/01/23  16:32:21  snichols
 * Added support for landscape, cleaned up some defaults, and
 * handled *PageRegion features better.
 * 
 * Revision 1.11  91/01/21  14:52:05  snichols
 * Use tempnam() to create a temp file name.
 * 
 * Revision 1.10  91/01/16  14:14:03  snichols
 * Added support for LZW compression and ascii85 encoding for Level 2 printers.
 * 
 * Revision 1.9  91/01/07  09:22:03  snichols
 * added sys/fcntl.h, let transcript.h handle malloc declaration.
 * 
 * Revision 1.8  91/01/04  13:13:43  snichols
 * temporary fix to use global ResourceDir... this will all change
 * when new resource stuff is available.
 * 
 * Revision 1.7  90/12/17  13:48:20  snichols
 * Uses lp defined in config.h for spooler.
 * 
 * Revision 1.6  90/12/12  10:26:39  snichols
 * new configuration stuff.
 * 
 * Revision 1.5  90/11/16  15:48:48  snichols
 * Use string.h instead of strings.h.  Fixed -r switch to actually remove 
 * input file.
 * 
 * Revision 1.4  90/11/16  14:39:01  snichols
 * Added code to wait on lpr process, because mh-e likes to kill the child 
 * process if the parent goes away.  Added support for range printing.
 * Yet another clean up of command-line switchs (this time for sure!)
 * 
 * Revision 1.2  90/09/17  17:49:47  snichols
 * added header and copyright notices, support for stripping out
 * comments and support for forging ahead rather than exiting when
 * errors encountered.
 * 
 *
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "transcript.h"
#include "psparse.h"
#include "config.h"

#ifdef DEBUG
#include <signal.h>
#endif
#ifndef XPG3
extern char *getenv();
#endif

FILE *output, *input;


extern char *optarg;
extern int optind;

char *argstr[200];
int nargs = 0;

int nranges = 0;

Usage()
{
    fprintf(stderr,"Usage: pslpr [-pfile][-lpath][-a][-f][-F][-G][-R][-z][-L] ");
    fprintf(stderr,"[-Sfeature=value] ");
#ifdef SYSV
    fprintf(stderr,"[-dprinter][-ncopies][-o][-s][-w][-m][-ttitle][-h][-c] ");
#else
    fprintf(stderr,"[-Pprinter][-#copies][-Jjob][-Cclass][-m][-r][-h] ");
#endif
    fprintf(stderr,"filename\n");
    exit(0);
}
#ifdef SYSV
#define ARGS "o:sn:wmcd:t:hfFRGS:al:p:i:zLequ"
#else
#define ARGS "P:#:C:J:rmhfFRGS:al:p:i:zLequ"
#endif

static char *printer;
static char *resourcepath;
static enum status fs;
static char buf[BUFSIZ];
static int c;
static int fdpipe[2];
static int lpid = 0;
static char temparg[200];
static int tfd,pfd;
static int fdin, fdout, out;
static int cnt;
static struct controlswitches control;
static char *outname;
static char *inname;
static int spooling = TRUE;
static int status;

static int removefile = FALSE;

static void addarg(arg)
    char *arg;
{
    char *p;
    if ((p = malloc(strlen(arg) + 1)) == NULL) {
	fprintf(stderr,"pslpr: malloc failure.\n");
	exit(1);
    }
    strcpy(p,arg);
    argstr[nargs++] = p;
    argstr[nargs] = NULL;
}

static void HandleFile()
{
    int fstatus;
    FILE *tmp;
    int filtering;

    /* deal with the comments */
    fs = HandleComments(input, printer, resourcepath, control);
    switch (fs) {
	case notps:
	    fprintf(stderr, "pslpr: input is not a PostScript file.  Run enscript first.\n");
	    exit(1);
	    break;
	case notcon:
	    /* notcon and handled do same thing */
	    fprintf(stderr, "pslpr: input is not a conforming file; sending as is.\n");
	case handled:
	    rewind(input);
	    fdin = fileno(input);
	    fdout = fileno(output);
	    if ((filtering = SetupCompression(&tmp,fdout,control)))
		out = fileno(tmp);
	    else
		out = fdout;
	    while ((cnt = read(fdin, buf, BUFSIZ)) > 0)
		write(out, buf, cnt);
	    if (filtering) {
		wait(&fstatus);
	    }
	    break;
	case success:
	    /* ready to send output */
	    rewind(input);
	    HandleOutput(input, output, printer, control);
	    break;
	default:
	    fprintf(stderr, "pslpr: unknown error!\n");
	    break;
    }
    if (inname && removefile) {
	if (unlink(inname) != 0) {
	    fprintf(stderr, "pslpr: unlink of input file failed.\n");
	    perror("");
	}
    }
}

#ifdef DEBUG
handler(sig)
    int sig;
{
    char buf[20];
    sprintf(buf, "terminated with signal %d\n",sig);
    write(2,buf,strlen(buf));
    _exit(1);
}
#endif

static void SetupChild()
{
    if (pipe(fdpipe)) {
	fprintf(stderr, "pslpr: pipe to lpr failed.\n");
	exit(1);
    }
    if ((lpid = fork()) < 0) {
	fprintf(stderr, "pslpr: fork failed.\n");
	exit(1);
    }
#ifdef DEBUG
    fprintf(stderr, "process id: %d\n", lpid);
#endif
    if (lpid == 0) {
	if (close(0) || (dup(fdpipe[0]) != 0) || close(fdpipe[0]) ||
	    close(fdpipe[1])) {
	    fprintf(stderr, "pslpr: child failed.\n");
	    exit(1);
	}
#ifdef DEBUG
	{
	    int cnter;
	    fprintf(stderr, "args for lpr: ");
	    for (cnter = 0; cnter < nargs; cnter++)
		fprintf(stderr, "%s ", argstr[cnter]);
	    fprintf(stderr, "\n");
	}
#endif
	execvp(lp, argstr);
	fprintf(stderr, "pslpr: returned from exec - bad news!\n");
	exit(1);
    }
    if (close(1) || (dup(fdpipe[1]) != 1) || close(fdpipe[1]) ||
	close(fdpipe[0])) {
	perror("");
	fprintf(stderr, "pslpr: parent failed.\n");
	exit(1);
    }
    if ((output = fdopen(1, "w")) == NULL) {
	fprintf(stderr, "pslpr: error opening communication pipe.\n");
	exit(1);
    }
}


main(argc,argv)
    int argc;
    char **argv;
{
    int i,j;
    char *p,*q;
    struct stat sbuf;
    char *tempname;

#ifdef DEBUG
    for (i=1; i<31; i++)
	if (i != SIGCHLD)
	    signal(i,handler);
    fprintf(stderr,"pslpr: ");
    for (i=0;i<argc;i++)
	fprintf(stderr,"%s ",argv[i]);
    fprintf(stderr,"\n");
#endif
    for (i = 0; i < MAXRANGES; i++)
	control.ranges[i].begin = control.ranges[i].end = 0;
    outname = inname = printer = resourcepath = NULL;
    addarg(lp);

    while ((c = getopt(argc,argv,ARGS)) != EOF) {
	switch (c) {
	case 'p':
	    outname = optarg;
	    break;
	case 'l':
	    resourcepath = optarg;
	    break;
	case 'f':
	    control.force = TRUE;
	    break;
	case 'R':
	    control.reverse = TRUE;
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
	case 'z':
	    control.compress = TRUE;
	    break;
	case 'L':
	    control.landscape = TRUE;
	    break;
	case 'e':
	    control.addshowpage = TRUE;
	    break;
	case 'q':
	    control.squeeze = TRUE;
	    break;
	case 'u':
	    control.upper = TRUE;
	    break;
	case 'i':
	    p = optarg;
	    q = strchr(optarg,'-');
	    if (q) {
		*q = '\0';
		q++;
		control.ranges[nranges].begin = atoi(p);
		control.ranges[nranges].end = atoi(q);
	    }
	    else {
		control.ranges[nranges].begin = control.ranges[nranges].end
		    = atoi(p);
	    }
	    nranges++;
	    break;
	case 'S':
	    p = optarg;
	    q = strchr(optarg,'=');
	    if (q) {
		*q = '\0'; q++;
		strncpy(control.features[control.nfeatures].option,q,50);
	    }
	    else
		strncpy(control.features[control.nfeatures].option,"True",50);
	    control.features[control.nfeatures].keyword[0] = '*';
	    strncat(control.features[control.nfeatures].keyword,p,50);
	    control.nfeatures++;
	    break;
#ifdef SYSV
	case 'd':
	    printer = optarg;
	    sprintf(temparg,"-d%s",optarg);
	    addarg(temparg);
	    break;
	case 'c':
	    addarg("-c");
	    break;
	case 's':
	    addarg("-s");
	    break;
	case 'w':
	    addarg("-w");
	    break;
	case 'm':
	    addarg("-m");
	    break;
	case 'n':
	    sprintf(temparg,"-n%s",optarg);
	    addarg(temparg);
	    break;
	case 't':
	    sprintf(temparg,"-t%s",optarg);
	    addarg(temparg);
	    break;
	case 'o':
	    sprintf(temparg,"-o%s",optarg);
	    addarg(temparg);
	    break;
	case 'h':
	    addarg("-h");
	    break;
#else
	case 'P':
	    printer = optarg;
	    sprintf(temparg,"-P%s",optarg);
	    addarg(temparg);
	    break;
	case '#':
	    sprintf(temparg,"-#%s",optarg);
	    addarg(temparg);
	    break;
	case 'C':
	    sprintf(temparg,"-C%s",optarg);
	    addarg(temparg);
	    break;
	case 'J':
	    sprintf(temparg,"-J%s",optarg);
	    addarg(temparg);
	    break;
	case 'm':
	    addarg("-m");
	    break;
	case 'r':
	    removefile = TRUE;
	    break;
	case 'h':
	    addarg("-h");
	    break;
#endif
	default:
	    Usage();
	    break;
	}
    }

    if (outname == NULL)
	SetupChild();
    else {
	spooling = FALSE;
	if (*outname != '-') {
	    if ((output = fopen(outname, "w")) == NULL) {
		fprintf(stderr, "Couldn't open output file %s\n", outname);
		exit(1);
	    }
	}
	else
	    output = stdout;
    }

    if (!printer && !outname) 
	printer = getenv("PRINTER");
	
    if (argc > optind) {
	for (i = optind ; i < argc; i++) {
	    inname = argv[i];
	    if ((input = fopen(inname, "r")) == NULL) {
		fprintf(stderr, "Couldn't open input file %s\n", inname);
		exit(1);
	    }
	    HandleFile();
	    fclose(input);
	}
    }
    else {
	input = stdin;

	if ((fstat(fileno(input),&sbuf)) < 0) {
	    fprintf(stderr,"pslpr: couldn't stat input!\n");
	    exit(1);
	}
	if ((sbuf.st_mode & S_IFMT) != S_IFREG) {
	    /* input is pipe, copy to temp file */
	    pfd = fileno(input);
	    tempname = (char *)tempnam(TempDir,"TS");
	    if (!tempname) {
		fprintf(stderr,"pslpr: couldn't create temp file name.\n");
		exit(1);
	    }
	    if ((tfd = open(tempname,O_WRONLY | O_CREAT, 0600)) < 0) {
		fprintf(stderr,"pslpr: couldn't create temp file.\n");
		exit(1);
	    }
	    while ((cnt = read(pfd,buf,BUFSIZ)) > 0) {
		if (write(tfd,buf,cnt) != cnt) {
		    fprintf(stderr,"pslpr: error writing temp file.\n");
		    exit(1);
		}
	    }
	    if (cnt < 0) {
		fprintf(stderr,"pslpr: error writing temp file.\n");
		exit(1);
	    }
	    close(tfd);
	    if (freopen(tempname,"r",input) == NULL) {
		fprintf(stderr,"pslpr: couldn't read temp file.\n");
		exit(1);
	    }
	    if (unlink(tempname)) 
		fprintf(stderr,"pslpr: couldn't unlink temp file.\n");
	}
	HandleFile();
    }
    fclose(output);
    if (spooling) {
	wait(&status);
    }
}
