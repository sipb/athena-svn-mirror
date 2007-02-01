#ifndef lint
static char Rcs_Id[] =
    "$Id: buildhash.c,v 1.1.1.2 2007-02-01 19:50:22 ghudson Exp $";
#endif

#define MAIN

/*
 * buildhash.c - make a hash table for ispell
 *
 * Pace Willisson, 1983
 *
 * Copyright 1992, 1993, 1999, 2001, 2005, Geoff Kuenning, Claremont, CA
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
 * Revision 1.74  2005/04/28 00:26:06  geoff
 * Re-count the dictionary file every time, rather than depending on a
 * file to hold the correct count.  This is necessary because the changes
 * for MS-DOS support caused us to generate non-unique count-file names,
 * which in turn could cause buildhash to fail.  There really isn't a
 * need for count files any more; they were only a performance
 * improvement and modern computers are fast enough that it doesn't
 * matter.
 *
 * Revision 1.73  2005/04/14 21:25:52  geoff
 * Declare ints-that-hold-pointers as unsigned, just for safety.
 *
 * Revision 1.72  2005/04/14 14:38:23  geoff
 * Update license and copyright.  Fix a count-file-naming bug introduced
 * in a recent delta.  Regenerate the count file if it has the same mtime
 * as the dictionary (otherwise you can have bugs on fast machines).  Fix
 * some small type bugs.
 *
 * Revision 1.71  2001/09/06 00:30:28  geoff
 * Many changes from Eli Zaretskii to support DJGPP compilation.
 *
 * Revision 1.70  2001/07/25 21:51:45  geoff
 * Minor license update.
 *
 * Revision 1.69  2001/07/23 20:24:02  geoff
 * Update the copyright and the license.
 *
 * Revision 1.68  2001/06/14 09:11:11  geoff
 * Use a non-conflicting macro for bcopy to avoid compilation problems on
 * smarter compilers.
 *
 * Revision 1.67  2000/08/22 10:52:25  geoff
 * Fix a whole bunch of signed/unsigned compiler warnings.
 *
 * Revision 1.66  1999/01/07 01:22:32  geoff
 * Update the copyright.
 *
 * Revision 1.65  1997/12/02  06:24:33  geoff
 * Get rid of an obsolete reference to "okspell".  Get rid of some
 * compile options that really shouldn't be optional.
 *
 * Revision 1.64  1995/01/08  23:23:26  geoff
 * Make the various file suffixes configurable for DOS purposes.
 *
 * Revision 1.63  1994/10/26  05:12:25  geoff
 * Get rid of some duplicate declarations.
 *
 * Revision 1.62  1994/07/28  05:11:33  geoff
 * Log message for previous revision: distinguish a zero count from a bad
 * count file.
 *
 * Revision 1.61  1994/07/28  04:53:30  geoff
 *
 * Revision 1.60  1994/01/25  07:11:18  geoff
 * Get rid of all old RCS log lines in preparation for the 3.1 release.
 *
 */

#include "config.h"
#include "ispell.h"
#include "proto.h"
#include "msgs.h"
#include "version.h"
#include <ctype.h>
#include <sys/stat.h>

int		main P ((int argc, char * argv[]));
static void	output P ((void));
static void	filltable P ((void));
VOID *		mymalloc P ((unsigned int size));
VOID *		myrealloc P ((VOID * ptr, unsigned int size,
		  unsigned int oldsize));
void		myfree P ((VOID * ptr));
static void	readdict P ((void));
static unsigned int
		newcount P ((void));

#define NSTAT	100		/* Size probe-statistics table */

char *		Dfile;		/* Name of dictionary file */
char *		Hfile;		/* Name of hash (output) file */
char *		Lfile;		/* Name of language file */

char		Sfile[MAXPATHLEN]; /* Name of statistics file */

static int silent = 0;		/* NZ to suppress count reports */

int main (argc, argv)
    int		argc;
    char *	argv[];
    {
    int		avg;
    char *	lastdot;
    FILE *	statf;
    int		stats[NSTAT];
    unsigned int i;
    int		j;

    while (argc > 1  &&  *argv[1] == '-')
	{
	argc--;
	argv++;
	switch (argv[0][1])
	    {
	    case 's':
		silent = 1;
		break;
	    }
	}
    if (argc == 4)
	{
	Dfile = argv[1];
	Lfile = argv[2];
	Hfile = argv[3];
	}
    else
	{
	(void) fprintf (stderr, BHASH_C_USAGE);
	return 1;
	}

    if (yyopen (Lfile))			/* Open the language file */
	return 1;
    yyinit ();				/* Set up for the parse */
    if (yyparse ())			/* Parse the language tables */
	exit (1);

    strcpy (Sfile, Dfile);
    lastdot = rindex (Sfile, '.');
    if (lastdot != NULL)
	*lastdot = '\0';
    strcat (Sfile, STATSUFFIX);
#ifdef MSDOS
    /*
    ** MS-DOS doesn't allow more than one dot in the filename part.
    ** If we have more than that, convert all the dots but the last into
    ** underscores.  The OS will truncate excess characters beyond 8+3.
    */
    lastdot = rindex (Sfile, '.');
    if (lastdot != NULL  &&  lastdot > Sfile)
	{
	while  (--lastdot >= Sfile)
	    {
	    if (*lastdot == '.')
		*lastdot = '_';
	    }
	}
#endif /* MSDOS */

    hashsize = newcount ();
    if (hashsize == 0)
	{
	(void) fprintf (stderr, BHASH_C_ZERO_COUNT);
	exit (1);
	}
    readdict ();

    if ((statf = fopen (Sfile, "w")) == NULL)
	{
	(void) fprintf (stderr, CANT_CREATE, Sfile, MAYBE_CR (stderr));
	exit (1);
	}

    for (i = 0; i < NSTAT; i++)
	stats[i] = 0;
    for (i = 0; i < hashsize; i++)
	{
	struct dent *   dp;

	dp = &hashtbl[i];
	if ((dp->flagfield & USED) != 0)
	    {
	    for (j = 0;  dp != NULL;  j++, dp = dp->next)
		{
		if (j >= NSTAT)
		    j = NSTAT - 1;
		stats[j]++;
		}
	    }
	}
    for (i = 0, j = 0, avg = 0;  i < NSTAT;  i++)
	{
	j += stats[i];
	avg += stats[i] * (i + 1);
	if (j == 0)
	    (void) fprintf (statf, "%d:\t%d\t0\t0.0\n", i + 1, stats[i]);
	else
	    (void) fprintf (statf, "%d:\t%d\t%d\t%f\n", i + 1, stats[i], j,
	      (double) avg / j);
	}
    (void) fclose (statf);

    filltable ();
    output ();
    return 0;
    }

static void output ()
    {
    register FILE *		houtfile;
    register struct dent *	dp;
    unsigned long		strptr;
    int				n;
    unsigned int		i;
    int				maxplen;
    int				maxslen;
    struct flagent *		fentry;

    if ((houtfile = fopen (Hfile, "wb")) == NULL)
	{
	(void) fprintf (stderr, CANT_CREATE, Hfile, MAYBE_CR (stderr));
	return;
	}
    hashheader.stringsize = 0;
    hashheader.lstringsize = 0;
    hashheader.tblsize = hashsize;
    (void) fwrite ((char *) &hashheader, sizeof hashheader, 1, houtfile);
    strptr = 0;
    /*
    ** Put out the strings from the flags table.  This code assumes that
    ** the size of the hash header is a multiple of the size of ichar_t,
    ** and that any long can be converted to an (ichar_t *) and back
    ** without damage (or, more accurately, without damaging those
    ** low-order bits necessary to represent the largest offset in the
    ** string table).
    */
    maxslen = 0;
    for (i = 0, fentry = sflaglist;  i < numsflags;  i++, fentry++)
	{
	if (fentry->stripl)
	    {
	    (void) fwrite ((char *) fentry->strip, fentry->stripl + 1,
	      sizeof (ichar_t), houtfile);
	    fentry->strip = (ichar_t *) strptr;
	    strptr += (fentry->stripl + 1) * sizeof (ichar_t);
	    }
	if (fentry->affl)
	    {
	    (void) fwrite ((char *) fentry->affix, fentry->affl + 1,
	      sizeof (ichar_t), houtfile);
	    fentry->affix = (ichar_t *) strptr;
	    strptr += (fentry->affl + 1) * sizeof (ichar_t);
	    }
	n = fentry->affl - fentry->stripl;
	if (n < 0)
	    n = -n;
	if (n > maxslen)
	    maxslen = n;
	}
    maxplen = 0;
    for (i = 0, fentry = pflaglist;  i < numpflags;  i++, fentry++)
	{
	if (fentry->stripl)
	    {
	    (void) fwrite ((char *) fentry->strip, fentry->stripl + 1,
	      sizeof (ichar_t), houtfile);
	    fentry->strip = (ichar_t *) strptr;
	    strptr += (fentry->stripl + 1) * sizeof (ichar_t);
	    }
	if (fentry->affl)
	    {
	    (void) fwrite ((char *) fentry->affix, fentry->affl + 1,
	      sizeof (ichar_t), houtfile);
	    fentry->affix = (ichar_t *) strptr;
	    strptr += (fentry->affl + 1) * sizeof (ichar_t);
	    }
	n = fentry->affl - fentry->stripl;
	if (n < 0)
	    n = -n;
	if (n > maxplen)
	    maxplen = n;
	}
    /*
    ** Write out the string character type tables.
    */
    hashheader.strtypestart = strptr;
    for (i = 0;  i < hashheader.nstrchartype;  i++)
	{
	n = strlen ((char *) chartypes[i].name) + 1;
	(void) fwrite ((char *) chartypes[i].name, n, 1, houtfile);
	strptr += n;
	n = strlen (chartypes[i].deformatter) + 1;
	(void) fwrite (chartypes[i].deformatter, n, 1, houtfile);
	strptr += n;
	for (n = 0;
	  chartypes[i].suffixes[n] != '\0';
	  n += strlen (&chartypes[i].suffixes[n]) + 1)
	    ;
	n++;
	(void) fwrite (chartypes[i].suffixes, n, 1, houtfile);
	strptr += n;
	}
    hashheader.lstringsize = strptr;
    /* We allow one extra byte because missingletter() may add one byte */
    maxslen += maxplen + 1;
    if (maxslen > MAXAFFIXLEN)
	{
	(void) fprintf (stderr,
	  BHASH_C_BAFF_1 (MAXAFFIXLEN, maxslen - MAXAFFIXLEN));
	(void) fprintf (stderr, BHASH_C_BAFF_2);
	}
    /* Put out the dictionary strings */
    for (i = 0, dp = hashtbl;  i < hashsize;  i++, dp++)
	{
	if (dp->word == NULL)
	    dp->word = (unsigned char *) -1;
	else
	    {
	    n = strlen ((char *) dp->word) + 1;
	    (void) fwrite (dp->word, n, 1, houtfile);
	    dp->word = (unsigned char *) strptr;
	    strptr += n;
	    }
	}
    /* Pad file to a struct dent boundary for efficiency. */
    n = (strptr + sizeof hashheader) % sizeof (struct dent);
    if (n != 0)
	{
	n = sizeof (struct dent) - n;
	strptr += n;
	while (--n >= 0)
	    (void) putc ('\0', houtfile);
	}
    /* Put out the hash table itself */
    for (i = 0, dp = hashtbl;  i < hashsize;  i++, dp++)
	{
	if (dp->next != 0)
	    {
	    unsigned long	x;
	    x = dp->next - hashtbl;
	    dp->next = (struct dent *)x;
	    }
	else
	    {
	    dp->next = (struct dent *)-1;
	    }
#ifdef PIECEMEAL_HASH_WRITES
	(void) fwrite ((char *) dp, sizeof (struct dent), 1, houtfile);
#endif /* PIECEMEAL_HASH_WRITES */
	}
#ifndef PIECEMEAL_HASH_WRITES
    (void) fwrite ((char *) hashtbl, sizeof (struct dent), hashsize, houtfile);
#endif /* PIECEMEAL_HASH_WRITES */
    /* Put out the language tables */
    (void) fwrite ((char *) sflaglist,
      sizeof (struct flagent), numsflags, houtfile);
    hashheader.stblsize = numsflags;
    (void) fwrite ((char *) pflaglist,
      sizeof (struct flagent), numpflags, houtfile);
    hashheader.ptblsize = numpflags;
    /* Finish filling in the hash header. */
    hashheader.stringsize = strptr;
    rewind (houtfile);
    (void) fwrite ((char *) &hashheader, sizeof hashheader, 1, houtfile);
    (void) fclose (houtfile);
    }

static void filltable ()
    {
    struct dent *freepointer, *nextword, *dp;
    struct dent *hashend;
    int i;
    int overflows;
    
    hashend = hashtbl + hashsize;
    for (freepointer = hashtbl;
      (freepointer->flagfield & USED)  &&  freepointer < hashend;
      freepointer++)
	;
    overflows = 0;
    for (nextword = hashtbl, i = hashsize; i != 0; nextword++, i--)
	{
	if ((nextword->flagfield & USED) == 0)
	    continue;
	if (nextword->next >= hashtbl  &&  nextword->next < hashend)
	    continue;
	dp = nextword;
	while (dp->next)
	    {
	    if (freepointer >= hashend)
		{
		overflows++;
		break;
		}
	    else
		{
		*freepointer = *(dp->next);
		dp->next = freepointer;
		dp = freepointer;

		while ((freepointer->flagfield & USED)
		  &&  freepointer < hashend)
		    freepointer++;
		}
	    }
	}
    if (overflows)
	(void) fprintf (stderr, BHASH_C_OVERFLOW, overflows);
    }

#if MALLOC_INCREMENT == 0
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

#else

VOID * mymalloc (size)		/* Fast, unfree-able variant of malloc */
    unsigned int	size;
    {
    VOID *		retval;
    static unsigned int	bytesleft = 0;
    static VOID *	nextspace;

    if (size < 4)
	size = 4;
    size = (size + 7) & ~7;	/* Assume doubleword boundaries are enough */
    if (bytesleft < size)
	{
	bytesleft = (size < MALLOC_INCREMENT) ? MALLOC_INCREMENT : size;
	nextspace = malloc ((unsigned) bytesleft);
	if (nextspace == NULL)
	    {
	    bytesleft = 0;
	    return NULL;
	    }
	}
    retval = nextspace;
    nextspace = (VOID *) ((char *) nextspace + size);
    bytesleft -= size;
    return retval;
    }

VOID * myrealloc (ptr, size, oldsize)
    VOID *		ptr;
    unsigned int	size;
    unsigned int	oldsize;
    {
    VOID *nptr;

    nptr = mymalloc (size);
    if (nptr == NULL)
	return NULL;
    (void) BCOPY (ptr, nptr, oldsize);
    return nptr;
    }

/* ARGSUSED */
void myfree (ptr)
    VOID *		ptr;
    {
    }
#endif

static void readdict ()
    {
    struct dent		d;
    register struct dent * dp;
    struct dent *	lastdp;
    unsigned char	lbuf[INPUTWORDLEN + MAXAFFIXLEN + 2 * MASKBITS];
    unsigned char	ucbuf[INPUTWORDLEN + MAXAFFIXLEN + 2 * MASKBITS];
    FILE *		dictf;
    int			i;
    int			h;

    if ((dictf = fopen (Dfile, "r")) == NULL)
	{
	(void) fprintf (stderr, BHASH_C_CANT_OPEN_DICT);
	exit (1);
	}

    hashtbl =
      (struct dent *) calloc ((unsigned) hashsize, sizeof (struct dent));
    if (hashtbl == NULL)
	{
	(void) fprintf (stderr, BHASH_C_NO_SPACE);
	exit (1);
	}

    i = 0;
    while (fgets ((char *) lbuf, sizeof lbuf, dictf) != NULL)
	{
	if (!silent  &&  (i % 1000) == 0)
	    {
	    (void) fprintf (stderr, "%d ", i);
	    (void) fflush (stdout);
	    }
	i++;

	if (makedent (lbuf, sizeof lbuf, &d) < 0)
	    continue;

	h = hash (strtosichar (d.word, 1), hashsize);

	dp = &hashtbl[h];
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
	    ** Collision.  Skip to the end of the collision
	    ** chain, or to a pre-existing entry for this
	    ** word.  Note that d.word always exists at
	    ** this point.
	    */
	    (void) strcpy ((char *) ucbuf, (char *) d.word);
	    chupcase (ucbuf);
	    while (dp != NULL)
		{
		if (strcmp ((char *) dp->word, (char *) ucbuf) == 0)
		    break;
		while (dp->flagfield & MOREVARIANTS)
		    dp = dp->next;
		dp = dp->next;
		}
	    if (dp != NULL)
		{
		/*
		** A different capitalization is already in
		** the dictionary.  Combine capitalizations.
		*/
		if (combinecaps (dp, &d) < 0)
		    exit (1);
		}
	    else
		{
		/* Insert a new word into the dictionary */
		for (dp = &hashtbl[h];  dp->next != NULL;  )
		    dp = dp->next;
		lastdp = dp;
		dp = (struct dent *) mymalloc (sizeof (struct dent));
		if (dp == NULL)
		    {
		    (void) fprintf (stderr, BHASH_C_COLLISION_SPACE);
		    exit (1);
		    }
		*dp = d;
		lastdp->next = dp;
		dp->next = NULL;
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
	    }
	}
    if (!silent)
	(void) fprintf (stderr, "\n");
    (void) fclose (dictf);
    }

static unsigned int newcount ()
    {
    unsigned char	buf[INPUTWORDLEN + MAXAFFIXLEN + 2 * MASKBITS];
    register unsigned int
			count;
    ichar_t		ibuf[INPUTWORDLEN + MAXAFFIXLEN + 2 * MASKBITS];
    register FILE *	d;
    ichar_t		lastibuf[sizeof ibuf / sizeof (ichar_t)];
    int			headercounted;
    int			followcase;
    register char *	cp;

    if (!silent)
	(void) fprintf (stderr, BHASH_C_COUNTING);

    if ((d = fopen (Dfile, "r")) == NULL)
	{
	(void) fprintf (stderr, BHASH_C_CANT_OPEN_DICT);
	exit (1);
	}

    headercounted = 0;
    lastibuf[0] = 0;
    for (count = 0;  fgets ((char *) buf, sizeof buf, d);  )
	{
	if ((++count % 1000) == 0  &&  !silent)
	    {
	    (void) fprintf (stderr, "%d ", count);
	    (void) fflush (stdout);
	    }
	cp = index ((char *) buf, hashheader.flagmarker);
	if (cp != NULL)
	    *cp = '\0';
	if (strtoichar (ibuf, buf, INPUTWORDLEN * sizeof (ichar_t), 1))
	    (void) fprintf (stderr, WORD_TOO_LONG (buf));
	followcase = (whatcap (ibuf) == FOLLOWCASE);
	upcase (ibuf);
	if (icharcmp (ibuf, lastibuf) != 0)
	    headercounted = 0;
	else if (!headercounted)
	    {
	    /* First duplicate will take two entries */
	    if ((++count % 1000) == 0  &&  !silent)
		{
		(void) fprintf (stderr, "%d ", count);
		(void) fflush (stdout);
		}
	    headercounted = 1;
	    }
	if (!headercounted  &&  followcase)
	    {
	    /* It's followcase and the first entry -- count again */
	    if ((++count % 1000) == 0  &&  !silent)
		{
		(void) fprintf (stderr, "%d ", count);
		(void) fflush (stdout);
		}
	    headercounted = 1;
	    }
	(void) icharcpy (lastibuf, ibuf);
	}
    (void) fclose (d);
    if (!silent)
	(void) fprintf (stderr, BHASH_C_WORD_COUNT, count);

    return count;
    }
