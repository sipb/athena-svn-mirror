/* Copyright 1985, 1998 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.

static const char rcsid[] = "$Id: just.c,v 1.5 1999-02-15 15:29:42 ghudson Exp $";

/*
 * fill/just filter
 *
 * Initial Coding 7 Feb 80, bbn:yba.
 * Version of 21 Sep 83, fsi:yba
 * Rehashed 18 Mar 85, mit:yba
 *
 *
 * Use:	fill [line_length]	(1 <= line_length <= 510.)
 *	just [line_length]
 *
 * Will fill lines or fill and justify lines to line_length (default
 * LINESIZE, below).
 *
 * Word longer than line_length are truncated!
 * Tabs are converted on input; double space at end of sentence is
 *     preserved.  Indentation for first line of paragraph preserved;
 *     subsequent lines have indentation of second line (if present).
 */

/* Next Page */
/* Globals */

/* Macros to convert Version 6 cc and BBN library to 4.2bsd and V7 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define seq(a,b)	(strcmp (a,b) == 0)
#define scopy(a,b,c)	(strchr (strcpy (b,a), '\0'))
#define slength		strlen
#define errmsg		perror

#define MAXLINE 512
#define LINESIZE	65	/* default line_length */

#define min(x,y)	((x) < (y) ? (x) : (y))
#define max(x,y)	((x) > (y) ? (x) : (y))

int     just;			/* bool: fill (false) or just (true)? */
int     nwords = 0;
int     line_width = LINESIZE;
int     inval = 0;		/* indentation from left margin */

char    inbuf[MAXLINE],
        outbuf[MAXLINE],
        wordbuf[MAXLINE];

char   *inptr = inbuf,
       *outptr = outbuf,
       *wordptr = wordbuf;

char   *me;

static int getword (void);
static void putword (void);
static void spread (void);
static void arg_err (void);
static int isbreak (int c);


/* Next Page */

int main (int argc, char **argv)
{
    char   *fgets ();


    int     nlp = 0;		/* # lines in current paragraph */
    int     i;
    char    c;

    me = strrchr (argv[0], '/');	/* fill or just? */
    if (me == NULL)		/* No prefix to strip? */
	me = argv[0];

    if (argc > 2)
	arg_err ();

    if (argc > 1)
	line_width = max (min (MAXLINE - 2, atoi (argv[1])), 1);

    just = seq (me, "just");

    outbuf[0] = '\0';
    while (fgets (inbuf, MAXLINE, stdin) != NULL) {
	inptr = inbuf;

	/* Check for blank lines and breaks (last line of paragraph, etc.) */

	while (*inptr == '\n')
	    if (nwords > 0) {	/* break? */
		puts (outbuf);	/* never justify this line */
		outptr = outbuf;
		outbuf[0] = '\0';
		nwords = 0;
		nlp = 0;
		}
	    else
		putchar (*inptr++);/* blank line */

	if (nlp == 0 && (*inptr != '\0')) {	/* 1st line of paragraph */
	    while (*inptr == ' ' || *inptr == '\t')
		if (*inptr == ' ')
		    *outptr++ = *inptr++;
		else {		/* convert tabs */
		    inptr++;
		    i = (*outptr = '\0', slength (outbuf));
		    do
			*outptr++ = ' ';
		        while (++i % 8 != 0 && (outptr < &outbuf[line_width - 1]));
		    }
	    *outptr = '\0';
	    /*
	     * Above we read from the buffered 1st line of the paragraph;
	     * below note that we are looking at stdin again,
	     * reading the second line (if any) of that paragraph
	     */
	    for (inval = 0; (c = getchar ()) == ' ' || (c == '\t');)
		if (c == '\t')
		    while (++inval % 8 != 0 && (inval < (line_width - 1)));
		else
		    inval++;

	    if (!feof (stdin))
		ungetc (c, stdin);
	    nlp++;
	    }

	while (getword ())
	    putword ();
	}

    if (outptr != outbuf)
	puts (outbuf);
    exit (0);
}

/* Next Page */

/*
 * getword -- returns true if it got a word; copies a word from inptr to outptr
 */

static int getword (void)
{
    wordptr = wordbuf;
    while (*inptr == ' ' || *inptr == '\t')
	inptr++;
    while (*inptr != ' ' && *inptr != '\t' && *inptr != '\n' && *inptr != '\0')
	*wordptr++ = *inptr++;

 /* add an extra space after end of sentence if it was present in source, *
    or if at the end of a line */

    if (wordptr != wordbuf && isbreak (*(inptr - 1)) && (*(inptr + 1) == ' ' || *inptr == '\n'))
	*wordptr++ = ' ';

    *wordptr = '\0';
    return slength (wordbuf);
}

/* Next Page */

/*
 * putword -- write a word into outbuf if it fits, pads and writes the
 *	buffer as appropriate
 */

static void putword (void)
{
    char   *strcpy ();
    int    i = inval;
    int    sl = slength (wordbuf) + slength (outbuf);

    if (*(wordptr - 1) == ' ' && sl == line_width) {
	*--wordptr = '\0';	/* get rid of space at end of line */
	sl--;			/* if it would put us over by one */
	}
    if (sl >= line_width) {
	while (*(outptr - 1) == ' ')/* no spaces at end of line */
	    *--outptr = '\0';
	if (just)
	    spread ();
	*outptr = '\n';
	*++outptr = '\0';
	fputs (outbuf, stdout);
	outptr = outbuf;
	nwords = 0;
	while (i--)
	    *outptr++ = ' ';
	}
    else
	if (outptr != outbuf && nwords)
	    *outptr++ = ' ';

 /* the scopy below places a null ('\0') at the end of the string * in
    outbuf */
    outptr = scopy (wordbuf, outptr, &outbuf[line_width]);
    nwords++;
}

/* Next Page */

/* spread -- spread out lines using the justification algorithm of
 *	Kernighan and Plauger, "Software Tools", Chap. 7, pg. 241
 *	(1976 edition)
 */

static void spread (void)
{
    static int  dir = 0;
    int     nextra = line_width - slength (outbuf),
            ne,
            nholes,
            nb;
    char   *ip,
           *jp;

    if (nextra > 0 && nwords > 1) {
	dir = 1 - dir;		/* alternate side to put extra space on */
	ne = nextra;
	nholes = nwords - 1;
	ip = outptr - 1;
	jp = min (&outbuf[line_width], (ip + ne));
	outptr = jp + 1;
	while (ip < jp) {
	    *jp = *ip;
	    if (*ip == ' ') {
		if (dir == 0)
		    nb = (ne - 1) / nholes + 1;
		else
		    nb = ne / nholes;
		ne -= nb;
		nholes--;
		for (; nb > 0; nb--)
		    *--jp = ' ';
		while (*(ip - 1) == ' ')
		    *--jp = *--ip;/* protected space */
		}
	    ip--;
	    jp--;
	    }
	}
}

/* Next Page */

/*
 * arg_err -- usage message; because this is used as an editor filter,
 *	copy the input to the output if being misued -- this prevents
 *	the user's entire input from disappearing in vi, pen, etc.
 */

static void arg_err (void)
{
    int     c;

    fprintf (stderr, "%s: Usage is %s [line width].\n", me, me);

    while ((c = getchar ()) != EOF)
	putchar (c);

    exit (1);
}

/* Next Page */
/*
 * isbreak -- is a character a member of the "ends a sentence" set?
 */

static int isbreak (int c)
{
    switch (c) {
	case '.': 
	case '!': 
	case '?': 
	case ':': 
	    return (c);		/* true */
	    break;		/* pro forma */
	default: 
	    return (0);		/* false */
	}
}
