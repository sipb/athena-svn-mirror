/*
 * ispell.c - An interactive spelling corrector.
 *
 * Copyright (c), 1983, by Pace Willisson
 * Permission for non-profit use is hereby granted.
 * All other rights reserved.
 *
 * 1987, Robert McQueer, added:
 *	-w option & handling of extra legal word characters
 *	-d option for alternate dictionary file
 *	-p option & WORDLIST variable for alternate personal dictionary
 *	-x option to suppress .bak files.
 *	8 bit text & config.h parameters
 * 1987, Geoff Kuenning, added:
 *	-c option for creating suffix suggestions from raw words
 *	suffixes in personal dictionary file
 *	hashed personal dictionary file
 *	-S option for unsorted word lists
 * 1987, Greg Schaffer, added:
 *	-T option (for TeX and LaTeX instead of troff) [later changed to -t]
 *	   passes over \ till next whitespace.
 *	   does not recognize % (comment)
 */
#ifdef SYSV
#define USG
#endif

#include <stdio.h>
#include <ctype.h>
#include <sys/param.h>
#include "ispell.h"
#include "config.h"

FILE *infile;
FILE *outfile;

char hashname[MAXPATHLEN];

extern struct dent *treeinsert();

/*
** we use extended character set range specifically to allow intl.
** character set characters.  We are being REALLY paranoid about indexing
** this array - explicitly cast into unsigned INTEGER, then mask
** If NO8BIT is set, text will be masked to ascii range.
*/
static int Trynum;
#ifdef NO8BIT
static char Try[128];
static char Checkch[128];
#define iswordch(X) (Checkch[((unsigned)(X))&0x7f])
#else
static char Try[256];
static char Checkch[256];
#define iswordch(X) (Checkch[((unsigned)(X))&0xff])
#endif

static int sortit = 1;

givehelp ()
{
	erase ();
	printf ("Whenever a word is found that is not in the dictionary, it is printed on\r\n");
	printf ("the first line of the screen.  If the dictionary contains any similar\r\n");
	printf ("words, they are listed with a single digit next to each one.  You have\r\n");
	printf ("the option of replacing the word completely, or choosing one of the\r\n");
	printf ("suggested words.\r\n");
	printf ("\r\n");
	printf ("Commands are:\r\n\r\n");
	printf ("0-9     Replace with one of the suggested words.\r\n");
	printf ("<space> Accept the word this time only\r\n");
	printf ("a       Accept the word for the rest of this file.\r\n");
	printf ("i       Accept the word, and put it in your private dictionary.\r\n");
	printf ("r       Replace the misspelled word completely.\r\n");
	printf ("q       Write the rest of this file, ignoring misspellings, ");
	printf (         "and start next file.\r\n");
	printf ("x or ^C Exit immediately.  Asks for confirmation.  ");
	printf (         "Leaves file unchanged.\r\n");
	printf ("^L      Redraw screen.\r\n");
	printf ("!       Shell escape.\r\n");
	printf ("l       Look up words in system dictionary.\r\n");
	printf ("\r\n\r\n");
	printf ("-- Type space to continue --");
	(void) fflush (stdout);
	getchar ();
}


char *getline();

int cflag = 0;
int lflag = 0;
int aflag = 0;
int fflag = 0;
#ifndef USG
int sflag = 0;
#endif
int xflag = 0;
int tflag = 0;

char *askfilename;

static char *Cmd;

usage ()
{
	fprintf (stderr, "Usage: %s [-dfile | -pfile | -wchars | -t | -x | -S] file .....\n",Cmd);
	fprintf (stderr, "       %s [-dfile | -pfile | -wchars | -t] -l\n",Cmd);
#ifdef USG
	fprintf (stderr, "       %s [-dfile | -pfile | -ffile | -t | -s] -a\n",Cmd);
#else
	fprintf (stderr, "       %s [-dfile | -pfile | -ffile | -t] -a\n",Cmd);
#endif
	fprintf (stderr, "       %s [-wchars] -c\n");
	exit (1);
}

static initckch()
{
	int c;

	Trynum = 0;
#ifdef NO8BIT
	for (c = 0; c < 128; ++c) {
#else
	for (c = 0; c < 256; ++c) {
#endif
		if (myalpha((char) c)) {
			Checkch[c] = (char) 1;
			if (myupper((char) c)) {
				Try[Trynum] = (char) c;
				++Trynum;
			}
		}
		else
			Checkch[c] = (char) 0;
	}
}

main (argc, argv)
char **argv;
{
	char *p;
	char *cpd;
	char num[4];
	unsigned mask;

	Cmd = *argv;

	initckch();
	(void) sprintf(hashname,"%s/%s",LIBDIR,DEFHASH);

	cpd = NULL;

	argv++;
	argc--;
	while (argc && **argv == '-') {
		switch ((*argv)[1]) {
		case 't':
			tflag++;
			break;
		case 'a':
			aflag++;
			break;
		case 'c':
			cflag++;
			lflag++;
			break;
		case 'x':
			xflag++;
			break;
		case 'f':
			fflag++;
			p = (*argv)+2;
			if (*p == '\0') {
				argv++; argc--;
				if (argc == 0)
					usage ();
				p = *argv;
			}
			askfilename = p;
			break;
		case 'l':
			lflag++;
			break;
#ifndef USG
		case 's':
			sflag++;
			break;
#endif
		case 'S':
			sortit = 0;
			break;
		case 'p':
			cpd = (*argv)+2;
			if (*cpd == '\0') {
				argv++; argc--;
				if (argc == 0)
					usage ();
				cpd = *argv;
			}
			break;
		case 'd':
			p = (*argv)+2;
			if (*p == '\0') {
				argv++; argc--;
				if (argc == 0)
					usage ();
				p = *argv;
			}
			if (*p == '/')
				(void) strcpy(hashname,p);
			else
				(void) sprintf(hashname,"%s/%s",LIBDIR,p);
			break;
		case 'w':
			num[3] = '\0';
#ifdef NO8BIT
			mask = 0x7f;
#else
			mask = 0xff;
#endif
			p = (*argv)+2;
			if (*p == '\0') {
				argv++; argc--;
				if (argc == 0)
					usage ();
				p = *argv;
			}
			while (Trynum <= mask && *p != '\0') {
				if (*p != 'n') {
					Checkch[((unsigned)(*p))&mask] = (char) 1;
					Try[Trynum] = *p & mask;
					++p;
				}
				else {
					++p;
					num[0] = *p; ++p;
					num[1] = *p; ++p;
					num[2] = *p; ++p;
					Try[Trynum] = atoi(num) & mask;
					Checkch[atoi(num)&mask] = (char) 1;
				}
				++Trynum;
			}
			break;
		default:
			usage();
		}
		argv++; argc--;
	}

	if (!argc && !lflag && !aflag)
		usage ();

	if (linit () < 0)
		exit (0);

	treeinit (cpd);

	if (aflag) {
		askmode ();
		exit (0);
	}

	if (lflag) {
		infile = stdin;
		checkfile ();
		exit (0);
	}

	terminit ();

	while (argc--)
		dofile (*argv++);

	done ();
}

char firstbuf[BUFSIZ], secondbuf[BUFSIZ];
char *currentchar;
char token[BUFSIZ];

int quit;

char *currentfile = NULL;

dofile (filename)
char *filename;
{
	int c;
	char	bakfile[256];

	currentfile = filename;

	if ((infile = fopen (filename, "r")) == NULL) {
		fprintf (stderr, "Can't open %s\r\n", filename);
		sleep (2);
		return(-1);
	}

	if (access (filename, 2) < 0) {
		fprintf (stderr, "Can't write to %s\r\n", filename);
		sleep (2);
		return(-1);
	}

	(void) strcpy(tempfile, TEMPNAME);
	(void) mktemp (tempfile);
	if ((outfile = fopen (tempfile, "w")) == NULL) {
		fprintf (stderr, "Can't create %s\r\n", tempfile);
		sleep (2);
		return(-1);
	}

	quit = 0;

	checkfile ();

	(void) fclose (infile);
	(void) fclose (outfile);

	if (!cflag)
		treeoutput ();

	if ((infile = fopen (tempfile, "r")) == NULL) {
		fprintf (stderr, "temporary file disappeared (%s)\r\n", tempfile);	
		sleep (2);
		return(-1);
	}

	(void) sprintf(bakfile, "%s.bak", filename);
	if(link(filename, bakfile) == 0)
		(void) unlink(filename);

	/* if we can't write new, preserve .bak regardless of xflag */
	if ((outfile = fopen (filename, "w")) == NULL) {
		fprintf (stderr, "can't create %s\r\n", filename);
		sleep (2);
		return(-1);
	}

	while ((c = getc (infile)) != EOF)
		(void) putc (c, outfile);

	(void) fclose (infile);
	(void) fclose (outfile);

	(void) unlink (tempfile);
	if (xflag)
		(void) unlink(bakfile);
}

checkfile ()
{
	char *p;
	int len;

	secondbuf[0] = 0;
	currentchar = secondbuf;

	while (1) {
		(void) strcpy (firstbuf, secondbuf);
		if (quit) {	/* quit can't be set in l mode */
			while (fgets (secondbuf, sizeof secondbuf, infile) != NULL)
				fputs (secondbuf, outfile);
			break;
		}

		if (fgets (secondbuf, sizeof secondbuf, infile) == NULL)
			break;
		currentchar = secondbuf;
		
		len = strlen (secondbuf) - 1;

		/* skip over .if */
		if (strncmp(currentchar,".if t",5) == 0 
		||  strncmp(currentchar,".if n",5) == 0) {
			copyout(&currentchar,5);
			while (*currentchar && isspace(*currentchar)) 
				copyout(&currentchar, 1);
		}

		/* skip over .ds XX or .nr XX */
		if (strncmp(currentchar,".ds ",4) == 0 
		||  strncmp(currentchar,".de ",4) == 0
		||  strncmp(currentchar,".nr ",4) == 0) {
			copyout(&currentchar, 3);
			while (*currentchar && isspace(*currentchar)) 
				copyout(&currentchar, 1);
			while (*currentchar && !isspace(*currentchar))
				copyout(&currentchar, 1);
			if (*currentchar == 0) {
				if (!lflag) (void) putc ('\n', outfile);
				continue;
			}
		}

		if (secondbuf [ len ] == '\n')
			secondbuf [ len ] = 0;

		/* if this is a formatter command, skip over it */
		if (!tflag && *currentchar == '.') {
			while (*currentchar && !myspace (*currentchar)) {
				if (!lflag)
					(void) putc (*currentchar, outfile);
				currentchar++;
			}
			if (*currentchar == 0) {
				if (!lflag)
					(void) putc ('\n', outfile);
				continue;
			}
		}

		while (1) {
			while (*currentchar && !iswordch(*currentchar)) {
			    if (tflag)		/* TeX or LaTeX stuff */
			    {
				if (*currentchar == '\\') {
				    /* skip till whitespace */
				    while (*currentchar && 
					!isspace(*currentchar)) {
					    if (!lflag)
						(void) putc(*currentchar, outfile);
					    currentchar++;
					}
				    continue;
				}
			    }
			    else
			    {
				/* formatting escape sequences */
				if (*currentchar == '\\') {
				    if(currentchar[1] == 'f') {
					/* font change: \fX */
					copyout(&currentchar, 3);
					continue;
				    }
				    else if(currentchar[1] == 's') {
					/* size change */
					if(currentchar[2] < 6 &&
					   currentchar[2] != 0)
						/* two digit size */
						copyout(&currentchar, 4);
					else
						/* one digit size */
						copyout(&currentchar, 3);
					continue;
				    }
				    else if(currentchar[1] == '(') {
					/* extended char set escape: \(XX */
					copyout(&currentchar, 4);
					continue;
				    }
				}
			    }

				if (!lflag)
					(void) putc (*currentchar, outfile);
				currentchar++;
			}

			if (*currentchar == 0)
				break;

			p = token;
			while (iswordch(*currentchar) ||
			       (*currentchar == '\'' &&
				iswordch(*(currentchar + 1))))
			  *p++ = *currentchar++;
			*p = 0;
			if (lflag) {
				if (!good (token)  &&  !cflag)
					if (lflag) printf ("%s\n", token);
					else       printf ("%s\r\n", token);
			} else {
				if (!quit)
				correct (token, &currentchar);
			}
			if (!lflag)
				fprintf (outfile, "%s", token);
		}
		if (!lflag)
			(void) putc ('\n', outfile);
	}
}

#define MAXPOSSIBLE	99	/* Max no. of possibilities to generate */

char possibilities[MAXPOSSIBLE][BUFSIZ];
int pcount;
int maxposslen;

correct (token, currentchar)
char *token;
char **currentchar;
{
	int c;
	int i;
	int col_ht;
	int ncols;
	char *p;
	int len;
	char *begintoken;

	len = strlen (token);
	begintoken = *currentchar - len;

checkagain:
	if (good (token))
		return;

	erase ();
	printf ("    %s", token);
	if (currentfile)
		printf ("              File: %s", currentfile);
	printf ("\r\n\r\n");

	makepossibilities (token);

	/*
	 * Make sure we have enough room on the screen to hold the
	 * possibilities.  Reduce the list if necessary.  80 / (maxposslen + 8)
	 * is the maximum number of columns that will fit.
	 */
	col_ht = li - 6;		/* Height of columns of words */
	ncols = 80 / (maxposslen + 8);
	if (pcount > ncols * col_ht)
		pcount = ncols * col_ht;

#if 0
	/*
	 * Equalize the column sizes.  The last column will be short.
	 */
	col_ht = (pcount + ncols - 1) / ncols;
#endif

	for (i = 0; i < pcount; i++) {
		move (2 + (i % col_ht), (maxposslen + 8) * (i / col_ht));
		printf ("%2d: %s", i, possibilities[i]);
	}

	move (li - 3, 0);
	printf ("%s\r\n", firstbuf);

	for (p = secondbuf; p != begintoken; p++)
		putchar (*p);
	inverse ();
	for (i = strlen (token); i > 0; i--)
		putchar (*p++);
	normal ();
	while (*p)
		putchar (*p++);
	printf ("\r\n");

	while (1) {
		switch (c = (getchar () & NOPARITY)) {
#ifndef USG
		case 'Z' & 037:
			stop ();
			erase ();
			goto checkagain;
#endif
		case ' ':
			erase ();
			return;
		case 'C' & 037:
		case 'x': case 'X':
			printf ("Are you sure you want to throw away your changes? ");
			c = (getchar () & NOPARITY);
			if (c == 'y' || c == 'Y') {
				erase ();
				done ();
			}
			putchar (7);
			goto checkagain;
		case 'i': case 'I':
			treeinsert (token, 1);
			erase ();
			return;
		case 'a': case 'A':
			treeinsert (token, 0);
			erase ();
			return;
		case 'L' & 037:
			goto checkagain;
		case '?':
			givehelp ();
			goto checkagain;
		case '!':
			{
				char buf[200];
				move (li - 1, 0);
				putchar ('!');
				if (getline (buf) == NULL) {
					putchar (7);
					erase ();
					goto checkagain;
				}
				printf ("\r\n");
				shellescape (buf);
				erase ();
				goto checkagain;
			}
		case 'r': case 'R':
			move (li - 1, 0);
			printf ("Replace with: ");
			if (getline (token) == NULL) {
				putchar (7);
				erase ();
				goto checkagain;
			}
			inserttoken (secondbuf, begintoken, token, currentchar);
			erase ();
			goto checkagain;
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			i = c - '0';
			if (pcount > 10
			    &&  i > 0  &&  i <= (pcount - 1) / 10) {
				c = getchar () & NOPARITY;
				if (c >= '0'  &&  c <= '9')
					i = i * 10 + c - '0';
				else if (c != '\r'  &&  c != '\n') {
					putchar (7);
					break;
				}
			}
			if (i < pcount) {
				(void) strcpy (token, possibilities[i]);
				inserttoken (secondbuf, begintoken,
				    token, currentchar);
				erase ();
				return;
			}
			putchar (7);
			break;
		case '\r':	/* This makes typing \n after single digits */
		case '\n':	/* ..less obnoxious */
			break;
		case 'l': case 'L':
			{
				char buf[100];
				move (li - 1, 0);
				printf ("Lookup string ('*' is wildcard): ");
				if (getline (buf) == NULL) {
					putchar (7);
					erase ();
					goto checkagain;
				}
				printf ("\r\n\r\n");
				lookharder (buf);
				erase ();
				goto checkagain;
			}
		case 'q': case 'Q':
			quit = 1;
			erase ();
			return;
		default:
			putchar (7);
			break;
		}
	}
}

inserttoken (buf, start, token, currentchar)
char *buf, *start, *token;
char **currentchar;
{
	char copy[BUFSIZ];
	char *p, *q;

	(void) strcpy (copy, buf);

	for (p = buf, q = copy; p != start; p++, q++)
		*p = *q;
	while (*token)
		*p++ = *token++;
	q += *currentchar - start;
	*currentchar = p;
	while (*p++ = *q++)
		;
}


makepossibilities (word)
char word[];
{
	int i;
	extern int strcmp ();

	for (i = 0; i < MAXPOSSIBLE; i++)
		possibilities[i][0] = 0;
	pcount = 0;
	maxposslen = 0;

	if (pcount < MAXPOSSIBLE) wrongletter (word);
	if (pcount < MAXPOSSIBLE) extraletter (word);
	if (pcount < MAXPOSSIBLE) missingletter (word);
	if (pcount < MAXPOSSIBLE) transposedletter (word);

	if (sortit  &&  pcount)
		qsort ((char *) possibilities, pcount,
		    sizeof (possibilities[0]), strcmp);
}

char *cap();

insert (word)
char *word;
{
	int i;

	for (i = 0; i < pcount; i++)
		if (strcmp (possibilities[i], word) == 0)
			return (0);

	(void) strcpy (possibilities[pcount++], word);
	i = strlen (word);
	if (i > maxposslen)
		maxposslen = i;
	if (pcount >= MAXPOSSIBLE)
		return (-1);
	else
		return (0);
}

wrongletter (word)
char word[];
{
	int i, j, n;
	char newword[BUFSIZ];

	n = strlen (word);
	(void) strcpy (newword, word);

	for (i = 0; i < n; i++) {
		for (j=0; j < Trynum; ++j) {
			newword[i] = Try[j];
			if (good (newword)) {
				if (insert (cap (newword, word)) < 0)
					return;
			}
		}
		newword[i] = word[i];
	}
}

extraletter (word)
char word[];
{
	char newword[BUFSIZ], *p, *s, *t;

	if (strlen (word) < 3)
		return;

	for (p = word; *p; p++) {
		for (s = word, t = newword; *s; s++)
			if (s != p)
				*t++ = *s;
		*t = 0;
		if (good (newword)) {
			if (insert (cap (newword, word)) < 0)
				return;
		}
	}
}

missingletter (word)
char word[];
{
	char newword[BUFSIZ], *p, *r, *s, *t;
	int i;

	for (p = word; p == word || p[-1]; p++) {
		for (s = newword, t = word; t != p; s++, t++)
			*s = *t;
		r = s++;
		while (*t)
			*s++ = *t++;
		*s = 0;
		for (i=0; i < Trynum; ++i) {
			*r = Try[i];
			if (good (newword)) {
				if (insert (cap (newword, word)) < 0)
					return;
			}
		}
	}
}

transposedletter (word)
char word[];
{
	char newword[BUFSIZ];
	int t;
	char *p;

	(void) strcpy (newword, word);
	for (p = newword; p[1]; p++) {
		t = p[0];
		p[0] = p[1];
		p[1] = t;
		if (good (newword)) {
			if (insert (cap (newword, word)) < 0)
				return;
		}
		t = p[0];
		p[0] = p[1];
		p[1] = t;
	}
}

char *
cap (word, pattern)
char word[], pattern[];
{
	static char newword[BUFSIZ];
	char *p, *q;

	if (*word == 0)
		return;

	if (myupper (pattern[0])) {
		if (myupper (pattern[1])) {
			for (p = word, q = newword; *p; p++, q++) {
				if (mylower (*p))
					*q = toupper (*p);
				else
					*q = *p;
			}
			*q = 0;
		} else {
			if (mylower (word [0]))
				newword[0] = toupper (word[0]);
			else
				newword[0] = word[0];

			for (p = word + 1, q = newword + 1; *p; p++, q++)
				if (myupper (*p))
					*q = tolower (*p);
				else
					*q = *p;

			*q = 0;
		}
	} else {
		for (p = word, q = newword; *p; p++, q++)
			if (myupper (*p))
				*q = tolower (*p);
			else
				*q = *p;
		*q = 0;
	}
	return (newword);
}

char *
getline (s)
char *s;
{
	char *p;
	int c;

	p = s;

	while (1) {
		c = (getchar () & NOPARITY);
		if (c == '\\') {
			putchar ('\\');
			c = (getchar () & NOPARITY);
			backup ();
			putchar (c);
			*p++ = c;
		} else if (c == ('G' & 037)) {
			return (NULL);
		} else if (c == '\n' || c == '\r') {
			*p = 0;
			return (s);
		} else if (c == erasechar) {
			if (p != s) {
				p--;
				backup ();
				putchar (' ');
				backup ();
			}
		} else if (c == killchar) {
			while (p != s) {
				p--;
				backup ();
				putchar (' ');
				backup ();
			}
		} else {
			*p++ = c;
			putchar (c);
		}
	}
}

askmode ()
{
	char buf[BUFSIZ];
	int i;

	if (fflag) {
		if (freopen (askfilename, "w", stdout) == NULL) {
			fprintf (stderr, "Can't create %s\n", askfilename);
			exit (1);
		}
	}

	setbuf (stdin, NULL);
	setbuf (stdout, NULL);

	while (gets (buf) != NULL) {
		/* *line is like `i', @line is like `a' */
		if (buf[0] == '*' || buf[0] == '@') {
			treeinsert(buf + 1, buf[0] == '*');
			printf("*\n");
			treeoutput ();
		} else if (good (buf)) {
			if (rootword[0] == 0) {
				printf ("*\n");	/* perfect match */
			} else {
				printf ("+ %s\n", rootword);
			}
		} else {
			makepossibilities (buf);
			if (possibilities[0][0]) {
				printf ("& ");
				for (i = 0; i < MAXPOSSIBLE; i++) {
					if (possibilities[i][0] == 0)
						break;
					printf ("%s ", possibilities[i]);
				}
				printf ("\n");
			} else {
				printf ("#\n");
			}
		}
#ifndef USG
		if (sflag) {
			stop ();
			if (fflag) {
				rewind (stdout);
				(void) creat (askfilename, 0666);
			}
		}
#endif
	}
}


copyout(cc, cnt)
char	**cc;
{
	while (--cnt >= 0) {
		if (*(*cc) == 0)
			break;
		if (!lflag)
			(void) putc (*(*cc), outfile);
		(*cc)++;
	}

}

lookharder(string)
char *string;
{
	char cmd[150];
	char *g, *s, grepstr[100];
	int wild = 0;

	g = grepstr;
	for (s = string; *s != '\0'; s++)
		if (*s == '*') {
			wild++;
			*g++ = '.';
			*g++ = '*';
		} else
			*g++ = *s;
	*g = '\0';
	if (grepstr[0]) {
#ifdef LOOK
		if (wild)
			/* string has wild card characters */
			(void) sprintf (cmd, "%s '^%s$' %s", EGREPCMD, grepstr, WORDS);
		else
			/* no wild, use look(1) */
			(void) sprintf (cmd, "%s %s %s", LOOK, grepstr, WORDS);
#else
		(void) sprintf (cmd, "%s '^%s$' %s", EGREPCMD, grepstr, WORDS);
#endif
		shellescape (cmd);
	}
}
