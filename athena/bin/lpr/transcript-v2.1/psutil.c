#ifndef lint
#define _NOTICE static char
_NOTICE N1[] = "Copyright (c) 1985,1987 Adobe Systems Incorporated";
_NOTICE N2[] = "GOVERNMENT END USERS: See Notice file in TranScript library directory";
_NOTICE N3[] = "-- probably /usr/lib/ps/Notice";
_NOTICE RCSID[]="$Id: psutil.c,v 1.5 1999-01-22 23:11:31 ghudson Exp $";
#endif
/* psutil.c
 *
 * Copyright (C) 1985,1987 Adobe Systems Incorporated. All rights reserved.
 * GOVERNMENT END USERS: See Notice file in TranScript library directory
 * -- probably /usr/lib/ps/Notice
 *
 * common utility subroutines
 *
 */

#include <stdio.h>
#ifdef POSIX
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#else
#include <strings.h>
#ifdef _AUX_SOURCE
#include <sys/types.h>
#endif
#include <sys/file.h>
#endif
#include "transcript.h"

extern char *getenv();

/* copy from file named fn to stream stm */
/* use read and write in hopes that it goes fast */
copyfile(fn, stm)
	char *fn;
	register FILE *stm;
{
    	int fd, fo;
	register int pcnt;
	char buf[BUFSIZ];

	VOIDC fflush(stm);
	fo = fileno(stm);
	if ((fd = open(fn, O_RDONLY, 0)) < 0) return(-1);
	while ((pcnt = read(fd, buf, sizeof buf)) > 0) {
	      if (write(fo, buf, (unsigned) pcnt) != pcnt) return(-2);
	}
	if (pcnt < 0) {perror("copyfile"); return (-1);}
	VOIDC close(fd);
	VOIDC fflush(stm);
	return(0);
}

/* exit with message and code */
VOID pexit(message, code)
char *message;
int code;
{
    perror(message);
    exit(code);
}
/* exit with message and code */
VOID pexit2(prog, message, code)
char *prog, *message;
int code;
{
    fprintf(stderr,"%s: ",prog);
    VOIDC perror(message);
    VOIDC exit(code);
}

/* concatenate s1 and s2 into s0 (of length l0), die if too long
 * returns a pointer to the null-terminated result
 */
char *mstrcat(s0,s1,s2,l0)
char *s0,*s1,*s2;
int	l0;
{
    if ((strlen(s1) + strlen(s2)) >= l0) {
	fprintf(stderr,"concatenate overflow %s%s\n",s1,s2);
	VOIDC exit(2);
    }
    return strcat(((s0 == s1) ? s0 : strcpy(s0,s1)),s2);
}

/* envget is a getenv
 * 	if the variable is not present in the environment or
 *	it has the null string as value envget returns NULL
 *	otherwise it returns the value from the environment
 */

char *envget(var)
char *var;
{
    register char *val;
    if (((val = getenv(var)) == NULL) || (*val == '\0'))
    	return ((char *) NULL);
    else return (val);
}
