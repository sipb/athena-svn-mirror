#ifndef lint
#define _NOTICE static char
_NOTICE N1[] = "Copyright (c) 1985,1987 Adobe Systems Incorporated";
_NOTICE N2[] = "GOVERNMENT END USERS: See Notice file in TranScript library directory";
_NOTICE N3[] = "-- probably /usr/lib/ps/Notice";
_NOTICE RCSID[]="$Id: psrv.c,v 1.3 1999-01-22 23:11:29 ghudson Exp $";
#endif
/* psrv.c
 *
 * Copyright (C) 1985,1987 Adobe Systems Incorporated. All rights reserved.
 * GOVERNMENT END USERS: See Notice file in TranScript library directory
 * -- probably /usr/lib/ps/Notice
 *
 * page-reversal (only) filter to be invoked by spooler communications filter
 *
 */

#include <stdio.h>
#ifdef POSIX
#include <fcntl.h>
#else
#include <sys/file.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include "psspool.h"
#include "transcript.h"

private char buf[BUFSIZ];
private char *prog;
private int fdin, fdout;
private char TempName[100];


private VOID writesection(first,last)
	register long first, last;
{
    register long len;
    register int pcnt;
    register unsigned nb;

    len = last - first;
    VOIDC lseek(fdin,first,0);
    while (len > 0) {
	nb = (unsigned) (len > sizeof(buf)) ? sizeof(buf) : len;
	len -= pcnt = read(fdin, buf, nb);
	if (write(fdout, buf, (unsigned) pcnt) != pcnt)
	    pexit2(prog,"write",2);
    }
}


/* page-reverse stdin to stdout -- errors to stderr */
main(argc, argv)
int argc;
char **argv;
{
    struct stat statb;
    long ptable[2000];
    int page = 0;
    long endpro = -1;
    long trailer = -1;
    long lastchar;
    int isregular;
    register int i;
    register int cnt, c;
    register long linestart;
    register char *bp;
    FILE *stmin;
    char *tempdir;

    fdin = fileno(stdin);
    fdout = fileno(stdout);
    stmin = stdin;

    prog = *argv;

    if (fstat(fdin, &statb) == -1) pexit2(prog,"fstat", THROW_AWAY);
    isregular = ((statb.st_mode & S_IFMT) == S_IFREG);

    if (!isregular) {
	/* copy stdin to a temp file */
	if ((tempdir = envget("PSTEMPDIR")) == NULL) tempdir = TempDir;
	VOIDC mktemp(mstrcat(TempName, tempdir, REVTEMP,sizeof TempName));
	
	if ((i = open(TempName,O_WRONLY|O_CREAT,0600)) == -1) {
	    pexit2(prog,"open temp file",THROW_AWAY);
	}
	while ((cnt = read(fdin, buf, sizeof buf)) > 0) {
	    if (write(i,buf,cnt) != cnt) {
		perror(prog);
	    }
	}
	if (cnt < 0) {
	    pexit2(prog,"copying file",THROW_AWAY);
	}
	VOIDC close(i);
	VOIDC close(fdin);
	stmin = freopen(TempName,"r",stdin);
	VOIDC unlink(TempName);
    }

    while (1) {
	cnt = BUFSIZ - 1;
	bp = buf;
	if ((linestart = ftell(stmin)) == -1) pexit2(prog,"ftell",2);
	while ((--cnt > 0) && ((*bp++ = (c = getchar())) != '\n')) {
	    if (c == EOF) {
		*--bp = '\0';
		break;
	    }
	}
	if (c != EOF) {
	    while (c != '\n') {
		if ((c = getchar()) == EOF) break;
	    }
	}
	*bp = '\0';
	if ((c == EOF) && (bp == buf)) break;
	else if (*buf != '%') continue;
	else if (strncmp(buf,"%%Page:",7) == 0) {
	    ptable[page++] = linestart;
	    if (endpro == -1) endpro = linestart;
	}
	else if (strncmp(buf,"%%Trailer",9) == 0) {
	    trailer = linestart;
	    ptable[page] = linestart;
	}
    }
    lastchar = ftell(stmin);
    if (trailer == -1) ptable[page] = lastchar;

    VOIDC fseek(stmin,0L,0); VOIDC lseek(fdin, 0L, 0);

#ifdef BSD
    VOIDC kill(getppid(),SIGEMT);
#endif

    if ((endpro == -1) || (page == 0)) {
	fprintf(stderr,"%s: file not reversible\n",prog);
	writesection(0L,lastchar);
    }
    else {
	/* do prologue */
	writesection(0L, endpro);

	/* do pages */
	for (i = page; i > 0; i--) {
	    writesection(ptable[i-1], ptable[i]);
	}

	/* do trailer */
	if (trailer != -1) writesection(trailer, lastchar);
    }
    exit(0);
}
