/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/lpc.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/lpc.c,v 1.7 1995-07-11 19:25:54 miki Exp $
 */

#ifndef lint
static char *rcsid_lpc_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/lpc.c,v 1.7 1995-07-11 19:25:54 miki Exp $";
#endif lint

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "@(#)lpc.c	5.2 (Berkeley) 11/17/85";
#endif not lint

#ifdef OPERATOR
#define GETUID geteuid
#else
#define GETUID getuid
#endif

/*
 * lpc -- line printer control program
 */
#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <setjmp.h>
#include <syslog.h>
#include <netdb.h>
  
#ifdef OPERATOR
#include <pwd.h>
#endif


#include "lpc.h"
#if defined(POSIX) && !defined(ultrix)
#include "posix.h"  
#endif

int	fromatty;

char	cmdline[200];
int	margc;
char	*margv[20];
int	top;
void	intr();
struct	cmd *getcmd();
extern struct cmd cmdtab[];

jmp_buf	toplevel;

main(argc, argv)
	char *argv[];
{
	register struct cmd *c;
	extern char *name;

	name = argv[0];
#ifdef LOG_AUTH
#ifdef OPERATOR
	openlog("lpc", 0, LOG_AUTH);
#else
	openlog("lpd", 0, LOG_LPR);
#endif
#else /* not LOG_AUTH */
#ifdef OPERATOR
	openlog("lpc", 0);
#else
	openlog("lpd", 0);
#endif
#endif /* LOG_AUTH */
	if (--argc > 0) {
		c = getcmd(*++argv);
		if (c == (struct cmd *)-1) {
			printf("?Ambiguous command\n");
			exit(1);
		}
		if (c == 0) {
			printf("?Invalid command\n");
			exit(1);
		}
		if (c->c_priv && GETUID()) {
			printf("?Privileged command\n");
			exit(1);
		}
#ifdef OPERATOR
		log_cmdline(argc, argv);
#endif
		(*c->c_handler)(argc, argv);
		exit(0);
	}
	fromatty = isatty(fileno(stdin));
	top = setjmp(toplevel) == 0;
	if (top)
		signal(SIGINT, intr);
	for (;;) {
		cmdscanner(top);
		top = 1;
	}
}

void
intr()
{
	if (!fromatty)
		exit(0);
	longjmp(toplevel, 1);
}

/*
 * Command parser.
 */
cmdscanner(top)
	int top;
{
	register struct cmd *c;
	extern int help();

	if (!top)
		putchar('\n');
	for (;;) {
		if (fromatty) {
			printf("lpc> ");
			fflush(stdout);
		}
		if (gets(cmdline) == 0)
			quit();
		if (cmdline[0] == 0)
			break;
		makeargv();
		c = getcmd(margv[0]);
		if (c == (struct cmd *)-1) {
			printf("?Ambiguous command\n");
			continue;
		}
		if (c == 0) {
			printf("?Invalid command\n");
			continue;
		}
		if (c->c_priv && GETUID()) {
			printf("?Privileged command\n");
			continue;
		}
#ifdef OPERATOR
		log_cmdline(margc, margv);
#endif
		(*c->c_handler)(margc, margv);
	}
	longjmp(toplevel, 0);
}

struct cmd *
getcmd(name)
	register char *name;
{
	register char *p, *q;
	register struct cmd *c, *found;
	register int nmatches, longest;

	longest = 0;
	nmatches = 0;
	found = 0;
	for (c = cmdtab; p = c->c_name; c++) {
		for (q = name; *q == *p++; q++)
			if (*q == 0)		/* exact match? */
				return(c);
		if (!*q) {			/* the name was a prefix */
			if (q - name > longest) {
				longest = q - name;
				nmatches = 1;
				found = c;
			} else if (q - name == longest)
				nmatches++;
		}
	}
	if (nmatches > 1)
		return((struct cmd *)-1);
	return(found);
}

/*
 * Slice a string up into argc/argv.
 */
makeargv()
{
	register char *cp;
	register char **argp = margv;

	margc = 0;
	for (cp = cmdline; *cp;) {
		while (isspace(*cp))
			cp++;
		if (*cp == '\0')
			break;
		*argp++ = cp;
		margc += 1;
		while (*cp != '\0' && !isspace(*cp))
			cp++;
		if (*cp == '\0')
			break;
		*cp++ = '\0';
	}
	*argp++ = 0;
}

#define HELPINDENT (sizeof ("directory"))

/*
 * Help command.
 */
help(argc, argv)
	int argc;
	char *argv[];
{
	register struct cmd *c;

	if (argc == 1) {
		register int i, j, w;
		int columns, width = 0, lines;
		extern int NCMDS;

		printf("Commands may be abbreviated.  Commands are:\n\n");
		for (c = cmdtab; c < &cmdtab[NCMDS]; c++) {
		        int len;
			if (!c->c_name)
			    continue;
			len = strlen(c->c_name);

			if (len > width)
				width = len;
		}
		width = (width + 8) &~ 7;
		columns = 80 / width;
		if (columns == 0)
			columns = 1;
		lines = (NCMDS + columns - 1) / columns;
		for (i = 0; i < lines; i++) {
			for (j = 0; j < columns; j++) {
				c = cmdtab + j * lines + i;
				if(c->c_name) printf("%s", c->c_name);
				if (c + lines >= &cmdtab[NCMDS]) {
					printf("\n");
					break;
				}
				w = strlen(c->c_name);
				while (w < width) {
					w = (w + 8) &~ 7;
					putchar('\t');
				}
			}
		}
		return;
	}
	while (--argc > 0) {
		register char *arg;
		arg = *++argv;
		c = getcmd(arg);
		if (c == (struct cmd *)-1)
			printf("?Ambiguous help command %s\n", arg);
		else if (c == (struct cmd *)0)
			printf("?Invalid help command %s\n", arg);
		else
			printf("%-*s\t%s\n", HELPINDENT,
				c->c_name, c->c_help);
	}
	return;
}

#ifdef OPERATOR
log_cmdline(argc, argv)
     int argc;
     char *argv[];
{
	struct passwd 	*pwentry;
	char		*name;
	char 		logbuf[512];
	register int	i;
					/* note getuid() returns real uid */
	pwentry = getpwuid(getuid()); /* get password entry for invoker */
	name = pwentry->pw_name; /* get his name */

	sprintf(logbuf,"%s LPC: ",name);
	for (i=0 ;i < argc; i++) {
		strcat(logbuf,argv[i]);
		strcat(logbuf," ");
	}
	syslog(LOG_INFO, logbuf);
	return(0);
}

#endif
