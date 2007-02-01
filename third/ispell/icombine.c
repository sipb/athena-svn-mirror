#ifndef lint
static char Rcs_Id[] =
    "$Id: icombine.c,v 1.1.1.2 2007-02-01 19:50:06 ghudson Exp $";
#endif

#define MAIN

/*
 * icombine:  combine multiple ispell dictionary entries into a single 
 *            entry with the options of all entries
 *
 * The original version of this program was written by Gary Puckering at
 * Cognos, Inc.  The current version is a complete replacement, created by
 * reducing Pace Willisson's buildhash program.  By using routines common
 * to buildhash and ispell, we can be sure that the rules for combining
 * capitalizations are compatible.
 *
 * Copyright 1992, 1993, 1999, 2001, Geoff Kuenning, Claremont, CA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All modifications to the source code must be clearly marked as
 *    such.  Binary redistributions based on modified source code
 *    must be clearly marked as modified versions in the documentation
 *    and/or other materials provided with the distribution.
 * 4. The code that causes the 'ispell -v' command to display a prominent
 *    link to the official ispell Web site may not be removed.
 * 5. The name of Geoff Kuenning may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY GEOFF KUENNING AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL GEOFF KUENNING OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * $Log: not supported by cvs2svn $
 * Revision 2.33  2005/04/20 23:16:32  geoff
 * Rename some variables to make them more meaningful.
 *
 * Revision 2.32  2005/04/14 23:11:36  geoff
 * Add the -w switch.
 *
 * Revision 2.31  2005/04/14 14:38:23  geoff
 * Update license.
 *
 * Revision 2.30  2001/07/25 21:51:46  geoff
 * Minor license update.
 *
 * Revision 2.29  2001/07/23 20:24:03  geoff
 * Update the copyright and the license.
 *
 * Revision 2.28  2000/08/22 10:52:25  geoff
 * Fix some compiler warnings.
 *
 * Revision 2.27  1999/01/07 01:22:44  geoff
 * Update the copyright.
 *
 * Revision 2.26  1999/01/03  01:46:30  geoff
 * Add support for sgml and plain deformatter types.
 *
 * Revision 2.25  1997/12/02  06:24:45  geoff
 * Get rid of some compile options that really shouldn't be optional.
 *
 * Revision 2.24  1994/01/25  07:11:35  geoff
 * Get rid of all old RCS log lines in preparation for the 3.1 release.
 *
 */

#include <ctype.h>
#include "config.h"
#include "ispell.h"
#include "proto.h"
#include "msgs.h"

char *		Lfile;			/* Language-description file */

int		main P ((int argc, char * argv[]));
static void	usage P ((void));
VOID *		mymalloc P ((unsigned int size));
VOID *		myrealloc P ((VOID * ptr, unsigned int size,
		  unsigned int oldsize));
void		myfree P ((VOID * ptr));
static void	combinedict P ((void));
static void	combineout P ((void));

int main (argc, argv)
    int		argc;
    char *	argv[];
    {
    char *	argp;
    char *	preftype = NULL;
    char *	wchars = NULL;

    while (argc > 1  &&  argv[1][0] == '-')
	{
	argc--;
	argv++;
	switch (argv[0][1])
	    {
	    case 'T':
		argp = (*argv)+2;
		if (*argp == '\0')
		    {
		    argv++; argc--;
		    if (argc == 0)
			usage ();
		    argp = *argv;
		    }
		preftype = argp;
		break;
	    case 'w':
		wchars = (*argv) + 2;
		if (*wchars == '\0')
		    {
		    argv++;
		    argc--;
		    if (argc == 0)
			usage ();
		    wchars = *argv;
		    }
		break;
		break;
	    default:
		usage ();
		break;
	    }
	}

    if (argc > 1)			/* Figure out what language to use */
	Lfile = argv[1];
    else
	Lfile = DEFLANG;

    if (yyopen (Lfile))			/* Open the language file */
      return 1;
    yyinit ();				/* Set up for the parse */
    if (yyparse ())			/* Parse the language tables */
	exit (1);

    initckch (wchars);

    if (preftype != NULL)
	{
	defstringgroup = findfiletype (preftype, 1, (int *) NULL);
	if (defstringgroup < 0
	  &&  strcmp (preftype, "plain") != 0
	  &&  strcmp (preftype, "tex") != 0
	  &&  strcmp (preftype, "nroff") != 0
	  &&  strcmp (preftype, "sgml") != 0)
	    {
	    (void) fprintf (stderr, ICOMBINE_C_BAD_TYPE, preftype);
	    exit (1);
	    }
	}
    if (defstringgroup < 0)
	defstringgroup = 0;

    combinedict ();			/* Combine words */

    return 0;
    }

static void usage ()
    {

    (void) fprintf (stderr, ICOMBINE_C_USAGE);
    exit (1);
    }

VOID * mymalloc (size)
    unsigned int	size;
    {
    return malloc (size);
    }

/* ARGSUSED */
VOID * myrealloc (ptr, size, oldsize)
    VOID *		ptr;
    unsigned int	size;
    unsigned int	oldsize;
    {

    return realloc (ptr, size);
    }

void myfree (ptr)
    VOID *	ptr;
    {
    free (ptr);
    }

static void combinedict ()
    {
    struct dent		d;
    register struct dent * dp;
    unsigned char	lbuf[INPUTWORDLEN + MAXAFFIXLEN + 2 * MASKBITS];
    ichar_t		ucbuf[INPUTWORDLEN + MAXAFFIXLEN + 2 * MASKBITS];
    ichar_t		lastbuf[INPUTWORDLEN + MAXAFFIXLEN + 2 * MASKBITS];

    lastbuf[0] = '\0';
    hashtbl = (struct dent *) mymalloc (sizeof (struct dent));
    hashtbl->flagfield = 0;
    hashtbl->word = 0;
    while (fgets ((char *) lbuf, sizeof lbuf, stdin) != NULL)
	{
	if (ichartostr (lbuf, strtosichar (lbuf, 0), sizeof lbuf, 1))
	    (void) fprintf (stderr, WORD_TOO_LONG (lbuf));
	if (makedent (ichartosstr (strtosichar (lbuf, 0), 1),
	    ICHARTOSSTR_SIZE, &d)
	  < 0)
	    continue;

	if (strtoichar (ucbuf, d.word, sizeof ucbuf, 1))
	    (void) fprintf (stderr, WORD_TOO_LONG (lbuf));
	upcase (ucbuf);
	if (icharcmp (ucbuf, lastbuf) != 0)
	    {
	    /*
	    ** We have a new word.  Put the old one out.
	    */
	    combineout ();
	    (void) icharcpy (lastbuf, ucbuf);
	    }

	dp = hashtbl;
	if ((dp->flagfield & USED) == 0)
	    {
	    *dp = d;
	    /*
	    ** If it's a followcase word, we need to make this a
	    ** special dummy entry, and add a second with the
	    ** correct capitalization.
	    */
	    if (captype (d.flagfield) == FOLLOWCASE)
		{
		if (addvheader (dp))
		  exit (1);
		}
	    }
	else
	    {
	    /*
	    ** A different capitalization is already in
	    ** the dictionary.  Combine capitalizations.
	    */
	    if (combinecaps (dp, &d) < 0)
	      exit (1);
	    }
	}
    combineout ();
    }

static void combineout ()
    {
    register struct dent *	ndp;
    register struct dent *	tdp;

    /*
    ** Put out the dictionary entry on stdout in text format,
    ** freeing it as we go.
    **/
    if (hashtbl->flagfield & USED)
	{
	for (tdp = hashtbl;  tdp != NULL;  tdp = ndp)
	    {
	    toutent (stdout, tdp, 0);
	    myfree (tdp->word);
	    ndp = tdp->next;
	    while (tdp->flagfield & MOREVARIANTS)
		{
		if (tdp != hashtbl)
		    myfree ((char *) tdp);
		tdp = ndp;
		if (tdp->word)
		    myfree (tdp->word);
		ndp = tdp->next;
		}
	    if (tdp != hashtbl)
		myfree ((char *) tdp);
	    }
	}
    hashtbl->flagfield = 0;
    hashtbl->word = NULL;
    }
