/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char rcsid_ln01filter_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/filters/newln01.c,v 1.2 1986-07-09 15:28:33 treese Exp $";
#endif not lint

/*
 *	LN01 output filter: puts into appropriate mode; converts
 *	nroff-type output for underlining into appropriate start/stop
 *	sequences.
 */

#include <stdio.h>
#include <sys/file.h>
#include <signal.h>
#include <sgtty.h>

#define MAXWIDTH  132
#define MAXREP    10
#define BEGINUNDERLINE 0x0100
#define ENDUNDERLINE 0x0200


int	buf[MAXREP][MAXWIDTH];
int	maxcol[MAXREP] = {-1};
int	lineno;
int	width = 132;	/* default line length */
int	length = 66;	/* page length */
int	indent;		/* indentation length */
int	npages = 1;
int	literal;	/* print control characters */
char	*name;		/* user's login name */
char	*host;		/* user's machine name */
char	*acctfile;	/* accounting information file */
char *reset="\033c";			/* ris	*/
char *land_length="\033[2550t";
char *landscape="\033[10m";
char *port_length="\033[3300t";		/* form length = 11 inches */
char *portrait="\033[11m";
char *beginunderline = "\033[4m";
char *endunderline = "\033[0m";
char *mode, *page_length;

main(argc, argv) 
	int argc;
	char *argv[];
{
	register FILE *p = stdin, *o = stdout;
	register int i, col;
	register int *cp;
	int done, linedone, maxrep, inunderline = 0;
	char *arg0, *argp;
	int lookahead, ch, *limit;

	flock(1, LOCK_EX); /* if it fails, we can't do anything anyway... */

	arg0 = argv[0];

	while (--argc) {
		if (*(argp = *++argv) == '-') {
			switch (argp[1]) {
			case 'n':
				argc--;
				name = *++argv;
				break;

			case 'h':
				argc--;
				host = *++argv;
				break;

			case 'w':
				if ((i = atoi(&argp[2])) > 0 && i <= MAXWIDTH)
					width = i;
				break;

			case 'l':
				length = atoi(&argp[2]);
				break;

			case 'i':
				indent = atoi(&argp[2]);
				break;

			case 'c':	/* Print control chars */
				literal++;
				break;
			}
		} else
			acctfile = argp;
	}

	mode = portrait;
	page_length = port_length;

	if (strcmp(arg0, "ln01p") == 0)
	  {
	    width = 80;
	    length = 66;
	  }
	else if (strcmp(arg0, "ln01l") == 0) 
	  {
	    width = 132;
	    length = 66;
	    mode = landscape;
	    page_length = land_length;
	  }
	setup_output_modes();
	printf("%s%s%s", reset, mode, page_length);

	for (cp = buf[0], limit = buf[MAXREP]; cp < limit; *cp++ = ' ');
	done = 0;
	
	while (!done) {
		col = indent;
		maxrep = -1;
		linedone = 0;
		while (!linedone) {
			switch (ch = getc(p)) {
			      case EOF:
				linedone = done = 1;
				ch = '\n';
				break;

			      case '\f':
				lineno = length;
			      case '\n':
				if (maxrep < 0)
					maxrep = 0;
				linedone = 1;
				break;
				
			      case '_':
				lookahead = getc(p);
				if (inunderline) {
					if (lookahead != '\b') {
						inunderline = 0;
						ungetc(lookahead,p);
						cp = &buf[0][col];
						ch = getc(p) | ENDUNDERLINE;
						for (i = 0; i < MAXREP; i++) {
							if (i > maxrep)
							  maxrep = i;
							if (*cp == ' ') {
								*cp = ch;
								if (col > maxcol[i])
								  maxcol[i] = col;
								break;
							}
							cp += MAXWIDTH;
						}
						col++;
						break;
					} else ch = getc(p);
				} else { /* not in underline mode yet */
					if (lookahead != '\b') /* we don't want to underline */
					  ungetc(lookahead,p);
					else { /* go ahead and start underlining */
						inunderline = 1;
						cp = &buf[0][col];
						ch = getc(p) | BEGINUNDERLINE;
						for (i = 0; i < MAXREP; i++) {
							if (i > maxrep)
							  maxrep = i;
							if (*cp == ' ') {
								*cp = ch;
								if (col > maxcol[i])
								  maxcol[i] = col;
								break;
							}
							cp += MAXWIDTH;
						}
						col++;
						break;
					}
				}
				cp = &buf[0][col];
				for (i = 0; i < MAXREP; i++) {
					if (i > maxrep)
					  maxrep = i;
					if (*cp == ' ') {
						*cp = ch;
						if (col > maxcol[i])
						  maxcol[i] = col;
						break;
					}
					cp += MAXWIDTH;
				}
				col++;
				break;
			case '\b':
				if (--col < indent)
					col = indent;
				break;

			case '\r':
				col = indent;
				break;

			case '\t':
				col = ((col - indent) | 07) + indent + 1;
				break;

			case '\031':
				/*
				 * lpd needs to use a different filter to
				 * print data so stop what we are doing and
				 * wait for lpd to restart us.
				 */
				if ((ch = getchar()) == '\1') {
					fflush(stdout);
					flock(1, LOCK_UN);
					kill(getpid(), SIGSTOP);
					flock(1, LOCK_EX);
					printf("%s%s%s", reset, mode, page_length);
					break;
				} else {
					ungetc(ch, stdin);
					ch = '\031';
				}

			default:
				if (inunderline) {
					inunderline = 0;
					cp = &buf[0][col];
					ch |= ENDUNDERLINE;
					for (i = 0; i < MAXREP; i++) {
						if (i > maxrep)
						  maxrep = i;
						if (*cp == ' ') {
							*cp = ch;
							if (col > maxcol[i])
							  maxcol[i] = col;
							break;
						}
						cp += MAXWIDTH;
					}
					col++;
					break;
				}
				if (col >= width || !literal && ch < ' ') {
					col++;
					break;
				}
				cp = &buf[0][col];
				for (i = 0; i < MAXREP; i++) {
					if (i > maxrep)
						maxrep = i;
					if (*cp == ' ') {
						*cp = ch;
						if (col > maxcol[i])
							maxcol[i] = col;
						break;
					}
					cp += MAXWIDTH;
				}
				col++;
				break;
			}
		}

		/* print out lines */
		for (i = 0; i <= maxrep; i++) {
			for (cp = buf[i], limit = cp+maxcol[i]; cp <= limit;) {
				if ((*cp) & BEGINUNDERLINE) {
					fputs(beginunderline, o);
					putc((*cp) & 0377, o);
				}
				else if ((*cp) & ENDUNDERLINE) {
					fputs(endunderline, o);
					putc((*cp) & 0377, o);
				}
				else putc(*cp, o);
				*cp++ = ' ';
			}
			if (i < maxrep)
				putc('\r', o);
			else
				putc(ch, o);
			if (++lineno >= length) {
				fflush(o);
				npages++;
				lineno = 0;
			}
			maxcol[i] = -1;
		}
	}
	if (lineno) {		/* be sure to end on a page boundary */
		putchar('\f');
		npages++;
	}
	fflush(stdout);
	flock(1, LOCK_UN); /* nothing we can do if it fails */
	if (name && acctfile && access(acctfile, 02) >= 0 &&
	    freopen(acctfile, "a", stdout) != NULL) {
		printf("%7.2f\t%s:%s\n", (float)npages, host, name);
	}
	exit(0);
}

/*
 * fix output file descriptor so that it has the right modes. 
 * in particular, if it is in raw mode, flow control won't work
 */

setup_output_modes()
{
  struct sgttyb tty_buf;
  ioctl(fileno(stdout), TIOCGETP, &tty_buf);
  tty_buf.sg_flags &= ~RAW;
  ioctl(fileno(stdout), TIOCSETP, &tty_buf);
}
