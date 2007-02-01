#ifndef lint
static char Rcs_Id[] =
    "$Id: tgood.c,v 1.1.1.2 2007-02-01 19:49:55 ghudson Exp $";
#endif

/*
 * Copyright 1987-1989, 1992, 1993, 1999, 2001, 2005, Geoff Kuenning,
 * Claremont, CA
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
 * Table-driven version of good.c.
 *
 * Geoff Kuenning, July 1987
 */

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.43  2005/04/14 14:38:23  geoff
 * Update license.  Incorporate Ed Avis's changes, which primarily
 * involve making the expansion table a proper structure (almost a
 * C++-style object) and moving the code to handle it into a separate
 * file.
 *
 * Revision 1.42  2001/07/25 21:51:46  geoff
 * Minor license update.
 *
 * Revision 1.41  2001/07/23 22:09:15  geoff
 * Change the expansion code so that the scratch table is allocated
 * dynamically.  This cures a bug that caused munchlist to fail for
 * languages that used affixes to encode parts of speech, rather than
 * string transformations, and generated huge numbers of affixes.  While
 * fixing this bug, I also eliminated an obsolete unused parameter from
 * several functions.
 *
 * Revision 1.40  2001/07/23 20:24:04  geoff
 * Update the copyright and the license.
 *
 * Revision 1.39  2000/11/14 07:27:04  geoff
 * Use messages from msgs.h for the "too many expansions" errors.
 *
 * Revision 1.38  2000/08/22 10:52:25  geoff
 * Fix a whole bunch of signed/unsigned discrepancies.
 *
 * Revision 1.37  1999/01/18 03:28:43  geoff
 * Turn many char declarations into unsigned char, so that we won't have
 * sign-extension problems.
 *
 * Revision 1.36  1999/01/07  01:57:43  geoff
 * Update the copyright.
 *
 * Revision 1.35  1997/12/21  23:10:24  geoff
 * Fix the generation of expansions so that only one unique expansion is
 * produced for any given root/affix combination.  For example, if both
 * the A and the N flags can add "n" to the end of a word (as in the
 * deutsch.aff file), expanding foo/AN will produce "foon" only once.
 * This has the side effect that the "-e4" option will generate a more
 * accurate ratio for flag productivity, which in turn means that
 * munchlist will do a better job of choosing an optimal set of roots and
 * flags.
 *
 * Revision 1.34  1997/12/03  05:28:13  geoff
 * Fix a bug that caused suffix expansions to be printed with the wrong
 * capitalization when all but one character of the root was stripped.
 *
 * Revision 1.33  1997/12/02  06:25:09  geoff
 * Get rid of some compile options that really shouldn't be optional.
 *
 * Revision 1.32  1994/11/02  06:56:16  geoff
 * Remove the anyword feature, which I've decided is a bad idea.
 *
 * Revision 1.31  1994/10/25  05:46:25  geoff
 * Add support for the FF_ANYWORD (affix applies to all words, even if
 * flag bit isn't set) flag option.
 *
 * Revision 1.30  1994/05/24  06:23:08  geoff
 * Don't create a hit if "allhits" is clear and capitalization
 * mismatches.  This cures a bug where a word could be in the dictionary
 * and yet not found.
 *
 * Revision 1.29  1994/05/17  06:44:21  geoff
 * Add support for controlled compound formation and the COMPOUNDONLY
 * option to affix flags.
 *
 * Revision 1.28  1994/01/25  07:12:13  geoff
 * Get rid of all old RCS log lines in preparation for the 3.1 release.
 *
 */

#include <ctype.h>
#include "config.h"
#include "ispell.h"
#include "msgs.h"
#include "proto.h"
#include "exp_table.h"

void		chk_aff P ((ichar_t * word, ichar_t * ucword, int len,
		  int ignoreflagbits, int allhits, int pfxopts, int sfxopts));
static void	pfx_list_chk P ((ichar_t * word, ichar_t * ucword,
		  int len, int optflags, int sfxopts, struct flagptr * ind,
		  int ignoreflagbits, int allhits));
static void	chk_suf P ((ichar_t * word, ichar_t * ucword, int len,
		  int optflags, struct flagent * pfxent, int ignoreflagbits,
		  int allhits));
static void	suf_list_chk P ((ichar_t * word, ichar_t * ucword, int len,
		  struct flagptr * ind, int optflags, struct flagent * pfxent,
		  int ignoreflagbits, int allhits));
int		expand_pre P ((unsigned char * croot, ichar_t * rootword,
		  MASKTYPE mask[], int option, unsigned char * extra));
static void	gen_pre_expansion P ((ichar_t * rootword,
		  struct flagent * flent, MASKTYPE mask[],
		  struct exp_table * exptable));
int		expand_suf P ((unsigned char * croot, ichar_t * rootword,
		  MASKTYPE mask[], int optflags, int option,
		  unsigned char * extra));
static void	expand_suf_into_table P ((ichar_t * rootword,
		  MASKTYPE mask[], int optflags,
		  struct exp_table * exptable, MASKTYPE existing_flags[]));
static void	gen_suf_expansion P ((ichar_t * rootword,
		  struct flagent * flent, struct exp_table * exptable,
		  MASKTYPE existing_flags[]));
static void	forcelc P ((ichar_t * dst, int len));
static char *	flags_str P ((MASKTYPE flags));
static int	output_expansions P ((struct exp_table * exptable, int option,
		  unsigned char * croot, unsigned char * extra));

/* Check possible affixes */
void chk_aff (word, ucword, len, ignoreflagbits, allhits, pfxopts, sfxopts)
    ichar_t *		word;		/* Word to be checked */
    ichar_t *		ucword;		/* Upper-case-only copy of word */
    int			len;		/* The length of word/ucword */
    int			ignoreflagbits;	/* Ignore whether affix is legal */
    int			allhits;	/* Keep going after first hit */
    int			pfxopts;	/* Options to apply to prefixes */
    int			sfxopts;	/* Options to apply to suffixes */
    {
    register ichar_t *	cp;		/* Pointer to char to index on */
    struct flagptr *	ind;		/* Flag index table to test */

    pfx_list_chk (word, ucword, len, pfxopts, sfxopts, &pflagindex[0],
      ignoreflagbits, allhits);
    cp = ucword;
    ind = &pflagindex[*cp++];
    while (ind->numents == 0  &&  ind->pu.fp != NULL)
	{
	if (*cp == 0)
	    return;
	if (ind->pu.fp[0].numents)
	    {
	    pfx_list_chk (word, ucword, len, pfxopts, sfxopts, &ind->pu.fp[0],
	      ignoreflagbits, allhits);
	    if (numhits  &&  !allhits  &&  !cflag  &&  !ignoreflagbits)
		return;
	    }
	ind = &ind->pu.fp[*cp++];
	}
    pfx_list_chk (word, ucword, len, pfxopts, sfxopts, ind, ignoreflagbits,
      allhits);
    if (numhits  &&  !allhits  &&  !cflag  &&  !ignoreflagbits)
	return;
    chk_suf (word, ucword, len, sfxopts, (struct flagent *) NULL,
      ignoreflagbits, allhits);
    }

/* Check some prefix flags */
static void pfx_list_chk (word, ucword, len, optflags, sfxopts, ind,
  ignoreflagbits, allhits)
    ichar_t *		word;		/* Word to be checked */
    ichar_t *		ucword;		/* Upper-case-only word */
    int			len;		/* The length of ucword */
    int			optflags;	/* Options to apply */
    int			sfxopts;	/* Options to apply to suffixes */
    struct flagptr *	ind;		/* Flag index table */
    int			ignoreflagbits;	/* Ignore whether affix is legal */
    int			allhits;	/* Keep going after first hit */
    {
    int			cond;		/* Condition number */
    register ichar_t *	cp;		/* Pointer into end of ucword */
    struct dent *	dent;		/* Dictionary entry we found */
    int			entcount;	/* Number of entries to process */
    register struct flagent *
			flent;		/* Current table entry */
    int			preadd;		/* Length added to tword2 as prefix */
    register int	tlen;		/* Length of tword */
    ichar_t		tword[INPUTWORDLEN + 4 * MAXAFFIXLEN + 4]; /* Tmp cpy */
    ichar_t		tword2[sizeof tword]; /* 2nd copy for ins_root_cap */

    for (flent = ind->pu.ent, entcount = ind->numents;
      entcount > 0;
      flent++, entcount--)
	{
	/*
	 * If this is a compound-only affix, ignore it unless we're
	 * looking for that specific thing.
	 */
	if ((flent->flagflags & FF_COMPOUNDONLY) != 0
	  &&  (optflags & FF_COMPOUNDONLY) == 0)
	    continue;
	/*
	 * In COMPOUND_CONTROLLED mode, the FF_COMPOUNDONLY bit must
	 * match exactly.
	 */
	if (compoundflag == COMPOUND_CONTROLLED
	  &&  ((flent->flagflags ^ optflags) & FF_COMPOUNDONLY) != 0)
	    continue;
	/*
	 * See if the prefix matches.
	 */
	tlen = len - flent->affl;
	if (tlen > 0
	  &&  (flent->affl == 0
	    ||  icharncmp (flent->affix, ucword, flent->affl) == 0)
	  &&  tlen + flent->stripl >= flent->numconds)
	    {
	    /*
	     * The prefix matches.  Remove it, replace it by the "strip"
	     * string (if any), and check the original conditions.
	     */
	    if (flent->stripl)
		(void) icharcpy (tword, flent->strip);
	    (void) icharcpy (tword + flent->stripl, ucword + flent->affl);
	    cp = tword;
	    for (cond = 0;  cond < flent->numconds;  cond++)
		{
		if ((flent->conds[*cp++] & (1 << cond)) == 0)
		    break;
		}
	    if (cond >= flent->numconds)
		{
		/*
		 * The conditions match.  See if the word is in the
		 * dictionary.
		 */
		tlen += flent->stripl;
		if (cflag)
		    flagpr (tword, BITTOCHAR (flent->flagbit), flent->stripl,
		      flent->affl, -1, 0);
		else if (ignoreflagbits)
		    {
		    if ((dent = lookup (tword, 1)) != NULL)
			{
			cp = tword2;
			if (flent->affl)
			    {
			    (void) icharcpy (cp, flent->affix);
			    cp += flent->affl;
			    *cp++ = '+';
			    }
			preadd = cp - tword2;
			(void) icharcpy (cp, tword);
			cp += tlen;
			if (flent->stripl)
			    {
			    *cp++ = '-';
			    (void) icharcpy (cp, flent->strip);
			    }
			(void) ins_root_cap (tword2, word,
			  flent->stripl, preadd,
			  0, (cp - tword2) - tlen - preadd,
			  dent, flent, (struct flagent *) NULL);
			}
		    }
		else if ((dent = lookup (tword, 1)) != NULL
		  &&  TSTMASKBIT (dent->mask, flent->flagbit))
		    {
		    if (numhits < MAX_HITS)
			{
			hits[numhits].dictent = dent;
			hits[numhits].prefix = flent;
			hits[numhits].suffix = NULL;
			numhits++;
			}
		    if (!allhits)
			{
			if (cap_ok (word, &hits[0], len))
			    return;
			numhits = 0;
			}
		    }
		/*
		 * Handle cross-products.
		 */
		if (flent->flagflags & FF_CROSSPRODUCT)
		    chk_suf (word, tword, tlen, sfxopts | FF_CROSSPRODUCT,
		      flent, ignoreflagbits, allhits);
		}
	    }
	}
    }

/* Check possible suffixes */
static void chk_suf (word, ucword, len, optflags, pfxent, ignoreflagbits,
  allhits)
    ichar_t *		word;		/* Word to be checked */
    ichar_t *		ucword;		/* Upper-case-only word */
    int			len;		/* The length of ucword */
    int			optflags;	/* Affix option flags */
    struct flagent *	pfxent;		/* Prefix flag entry if cross-prod */
    int			ignoreflagbits;	/* Ignore whether affix is legal */
    int			allhits;	/* Keep going after first hit */
    {
    register ichar_t *	cp;		/* Pointer to char to index on */
    struct flagptr *	ind;		/* Flag index table to test */

    suf_list_chk (word, ucword, len, &sflagindex[0], optflags, pfxent,
      ignoreflagbits, allhits);
    cp = ucword + len - 1;
    ind = &sflagindex[*cp];
    while (ind->numents == 0  &&  ind->pu.fp != NULL)
	{
	if (cp == ucword)
	    return;
	if (ind->pu.fp[0].numents)
	    {
	    suf_list_chk (word, ucword, len, &ind->pu.fp[0],
	      optflags, pfxent, ignoreflagbits, allhits);
	    if (numhits != 0  &&  !allhits  &&  !cflag  &&  !ignoreflagbits)
		return;
	    }
	ind = &ind->pu.fp[*--cp];
	}
    suf_list_chk (word, ucword, len, ind, optflags, pfxent,
      ignoreflagbits, allhits);
    }
    
static void suf_list_chk (word, ucword, len, ind, optflags, pfxent,
  ignoreflagbits, allhits)
    ichar_t *		word;		/* Word to be checked */
    ichar_t *		ucword;		/* Upper-case-only word */
    int			len;		/* The length of ucword */
    struct flagptr *	ind;		/* Flag index table */
    int			optflags;	/* Affix option flags */
    struct flagent *	pfxent;		/* Prefix flag entry if crossonly */
    int			ignoreflagbits;	/* Ignore whether affix is legal */
    int			allhits;	/* Keep going after first hit */
    {
    register ichar_t *	cp;		/* Pointer into end of ucword */
    int			cond;		/* Condition number */
    struct dent *	dent;		/* Dictionary entry we found */
    int			entcount;	/* Number of entries to process */
    register struct flagent *
			flent;		/* Current table entry */
    int			preadd;		/* Length added to tword2 as prefix */
    register int	tlen;		/* Length of tword */
    ichar_t		tword[INPUTWORDLEN + 4 * MAXAFFIXLEN + 4]; /* Tmp cpy */
    ichar_t		tword2[sizeof tword]; /* 2nd copy for ins_root_cap */

    (void) icharcpy (tword, ucword);
    for (flent = ind->pu.ent, entcount = ind->numents;
      entcount > 0;
      flent++, entcount--)
	{
	if ((optflags & FF_CROSSPRODUCT) != 0
	  &&  (flent->flagflags & FF_CROSSPRODUCT) == 0)
	    continue;
	/*
	 * If this is a compound-only affix, ignore it unless we're
	 * looking for that specific thing.
	 */
	if ((flent->flagflags & FF_COMPOUNDONLY) != 0
	  &&  (optflags & FF_COMPOUNDONLY) == 0)
	    continue;
	/*
	 * In COMPOUND_CONTROLLED mode, the FF_COMPOUNDONLY bit must
	 * match exactly.
	 */
	if (compoundflag == COMPOUND_CONTROLLED
	  &&  ((flent->flagflags ^ optflags) & FF_COMPOUNDONLY) != 0)
	    continue;
	/*
	 * See if the suffix matches.
	 */
	tlen = len - flent->affl;
	if (tlen > 0
	  &&  (flent->affl == 0
	    ||  icharcmp (flent->affix, ucword + tlen) == 0)
	  &&  tlen + flent->stripl >= flent->numconds)
	    {
	    /*
	     * The suffix matches.  Remove it, replace it by the "strip"
	     * string (if any), and check the original conditions.
	     */
	    (void) icharcpy (tword, ucword);
	    cp = tword + tlen;
	    if (flent->stripl)
		{
		(void) icharcpy (cp, flent->strip);
		tlen += flent->stripl;
		cp = tword + tlen;
		}
	    else
		*cp = '\0';
	    for (cond = flent->numconds;  --cond >= 0;  )
		{
		if ((flent->conds[*--cp] & (1 << cond)) == 0)
		    break;
		}
	    if (cond < 0)
		{
		/*
		 * The conditions match.  See if the word is in the
		 * dictionary.
		 */
		if (cflag)
		    {
		    if (optflags & FF_CROSSPRODUCT)
			flagpr (tword, BITTOCHAR (pfxent->flagbit),
			  pfxent->stripl, pfxent->affl,
			  BITTOCHAR (flent->flagbit), flent->affl);
		    else
			flagpr (tword, -1, 0, 0,
			  BITTOCHAR (flent->flagbit), flent->affl);
		    }
		else if (ignoreflagbits)
		    {
		    if ((dent = lookup (tword, 1)) != NULL)
			{
			cp = tword2;
			if ((optflags & FF_CROSSPRODUCT)
			  &&  pfxent->affl != 0)
			    {
			    (void) icharcpy (cp, pfxent->affix);
			    cp += pfxent->affl;
			    *cp++ = '+';
			    }
			preadd = cp - tword2;
			(void) icharcpy (cp, tword);
			cp += tlen;
			if ((optflags & FF_CROSSPRODUCT)
			  &&  pfxent->stripl != 0)
			    {
			    *cp++ = '-';
			    (void) icharcpy (cp, pfxent->strip);
			    cp += pfxent->stripl;
			    }
			if (flent->stripl)
			    {
			    *cp++ = '-';
			    (void) icharcpy (cp, flent->strip);
			    cp += flent->stripl;
			    }
			if (flent->affl)
			    {
			    *cp++ = '+';
			    (void) icharcpy (cp, flent->affix);
			    cp += flent->affl;
			    }
			(void) ins_root_cap (tword2, word,
			  (optflags & FF_CROSSPRODUCT) ? pfxent->stripl : 0,
			  preadd,
			  flent->stripl, (cp - tword2) - tlen - preadd,
			  dent, pfxent, flent);
			}
		    }
		else if ((dent = lookup (tword, 1)) != NULL
		  &&  TSTMASKBIT (dent->mask, flent->flagbit)
		  &&  ((optflags & FF_CROSSPRODUCT) == 0
		    || TSTMASKBIT (dent->mask, pfxent->flagbit)))
		    {
		    if (numhits < MAX_HITS)
			{
			hits[numhits].dictent = dent;
			hits[numhits].prefix = pfxent;
			hits[numhits].suffix = flent;
			numhits++;
			}
		    if (!allhits)
			{
			if (cap_ok (word, &hits[0], len))
			    return;
			numhits = 0;
			}
		    }
		}
	    }
	}
    }

/*
 * Expand a dictionary prefix entry
 */
int expand_pre (croot, rootword, mask, option, extra)
    unsigned char *		croot;		/* Char version of rootword */
    ichar_t *			rootword;	/* Root word to expand */
    register MASKTYPE		mask[];		/* Mask bits to expand on */
    int				option;		/* Option, see expandmode */
    unsigned char *		extra;		/* Extra info to add to line */
    {
    int				entcount;	/* No. of entries to process */
    int				explength;	/* Length of expansions */
    static struct exp_table     exptable;       /* Table of expansions */
    register struct flagent *
				flent;		/* Current table entry */

    exp_table_init(&exptable, rootword);
    for (flent = pflaglist, entcount = numpflags;
	 entcount > 0;
	 flent++, entcount--)
	{
	if (TSTMASKBIT (mask, flent->flagbit))
	    gen_pre_expansion (rootword, flent, mask, &exptable);
	}
    explength = output_expansions (&exptable, option, croot, extra);
    (void) exp_table_empty (&exptable);
    return explength;
    }

/* Generate a prefix expansion, modifying the table passed in */
static void gen_pre_expansion (rootword, flent, mask, exptable)
    register ichar_t *		rootword;	/* Root word to expand */
    register struct flagent *	flent;		/* Current table entry */
    MASKTYPE			mask[];		/* Mask bits to expand on */
    struct exp_table *          exptable;	/* Ptr to tbl of expansions */
    {
    MASKTYPE                    applied[MASKSIZE]; /* Flag being applied */
    int				cond;		/* Current condition number */
    register ichar_t *		nextc;		/* Next case choice */
    register unsigned char *	result;		/* Result of expansion */
    int				tlen;		/* Length of tword */
    ichar_t			tword[INPUTWORDLEN + MAXAFFIXLEN]; /* Temp */

    tlen = icharlen (rootword);
    if (flent->numconds > tlen)
	return;
    tlen -= flent->stripl;
    if (tlen <= 0)
	return;
    tlen += flent->affl;
    for (cond = 0, nextc = rootword;  cond < flent->numconds;  cond++)
	{
	if ((flent->conds[mytoupper (*nextc++)] & (1 << cond)) == 0)
	    return;
	}
    /*
     * The conditions are satisfied.  Copy the word, add the prefix,
     * and make it the proper case.   This code is carefully written
     * to match that ins_cap and cap_ok.  Note that the affix, as
     * inserted, is uppercase.
     *
     * There is a tricky bit here:  if the root is capitalized, we
     * want a capitalized result.  If the root is followcase, however,
     * we want to duplicate the case of the first remaining letter
     * of the root.  In other words, "Loved/U" should generate "Unloved",
     * but "LOved/U" should generate "UNLOved" and "lOved/U" should
     * produce "unlOved".
     */
    if (flent->affl)
	{
	(void) icharcpy (tword, flent->affix);
	nextc = tword + flent->affl;
	}
    (void) icharcpy (nextc, rootword + flent->stripl);
    if (myupper (rootword[0]))
	{
	/* We must distinguish followcase from capitalized and all-upper */
	for (nextc = rootword + 1;  *nextc;  nextc++)
	    {
	    if (!myupper (*nextc))
		break;
	    }
	if (*nextc)
	    {
	    /* It's a followcase or capitalized word.  Figure out which. */
	    for (  ;  *nextc;  nextc++)
		{
		if (myupper (*nextc))
		    break;
		}
	    if (*nextc)
		{
		/* It's followcase. */
		if (!myupper (tword[flent->affl]))
		    forcelc (tword, flent->affl);
		}
	    else
		{
		/* It's capitalized */
		forcelc (tword + 1, tlen - 1);
		}
	    }
	}
    else
	{
	/* Followcase or all-lower, we don't care which */
	if (!myupper (*nextc))
	    forcelc (tword, flent->affl);
	}
    /*
     * We've generated the result.  If it's not already in the
     * expansion table, add it at the end.
     */
    result = ichartosstr (tword, 1);
    BZERO (applied, sizeof applied);
    SETMASKBIT (applied, flent->flagbit);
    add_expansion_copy(exptable, (char *) result, applied);

    if (flent->flagflags & FF_CROSSPRODUCT)
      expand_suf_into_table (tword, mask, FF_CROSSPRODUCT, exptable,
			     applied);
    }

/*
 * Expand a dictionary suffix entry
 */
int expand_suf (croot, rootword, mask, optflags, option, extra)
    unsigned char *		croot;		/* Char version of rootword */
    ichar_t *			rootword;	/* Root word to expand */
    register MASKTYPE		mask[];		/* Mask bits to expand on */
    int				optflags;	/* Affix option flags */
    int				option;		/* Option, see expandmode */
    unsigned char *		extra;		/* Extra info to add to line */
    {
    int				explength;	/* Length of expansions */
    struct exp_table            exptable;       /* Table of expansions */
    int				i;		/* For zeroing flags */
    MASKTYPE                    nullflags[MASKSIZE]; /* Flag being applied */

    for (i = 0;  i < MASKSIZE;  i++)
	nullflags[i] = 0;
    exp_table_init (&exptable, rootword);
    expand_suf_into_table(rootword, mask, optflags, &exptable, nullflags);
    explength = output_expansions (&exptable, option, croot, extra);
    (void) exp_table_empty (&exptable);
    return explength;
    }

/*
 * Expand a dictionary suffix entry and insert it into an expansion table
 */
static void expand_suf_into_table (rootword, mask, optflags, exptable,
				   existing_flags)
    ichar_t *			rootword;	/* Root word to expand */
    register MASKTYPE		mask[];		/* Mask bits to expand on */
    int				optflags;	/* Affix option flags */
    struct exp_table *          exptable;	/* Table of expansions */
    MASKTYPE                    existing_flags[]; /* Flags already applied */
    {
    int				entcount;	/* No. of entries to process */
    register struct flagent *
				flent;		/* Current table entry */

    for (flent = sflaglist, entcount = numsflags;
      entcount > 0;
      flent++, entcount--)
	{
	if (TSTMASKBIT (mask, flent->flagbit))
	    {
	    if ((optflags & FF_CROSSPRODUCT) == 0
	      ||  (flent->flagflags & FF_CROSSPRODUCT))
		(void) gen_suf_expansion (rootword, flent, exptable,
					  existing_flags);
	    }
	}
    }

/* Generate a suffix expansion */
static void gen_suf_expansion (rootword, flent, exptable, existing_flags)
    register ichar_t *		rootword;	/* Root word to expand */
    register struct flagent *	flent;		/* Current table entry */
    struct exp_table *		exptable;	/* Table of expansions */
    MASKTYPE *			existing_flags; /* Flags already applied */
    {
    int				cond;		/* Current condition number */
    register ichar_t *		nextc;		/* Next case choice */
    register unsigned char *	result;		/* Result of expansion */
    int				tlen;		/* Length of tword */
    ichar_t			tword[INPUTWORDLEN + MAXAFFIXLEN]; /* Temp */
    int				wascapitalized;	/* NZ if capitalized root */

    tlen = icharlen (rootword);
    cond = flent->numconds;
    if (cond > tlen)
	return;
    if (tlen - flent->stripl <= 0)
	return;
    wascapitalized = myupper (rootword[0]);
    for (nextc = rootword + tlen;  --cond >= 0;  )
	{
	if ((flent->conds[mytoupper (*--nextc)] & (1 << cond)) == 0)
	    return;
	if (nextc > rootword  &&  myupper (*nextc))
	    wascapitalized = 0;
	}
    while (--nextc > rootword)
	{
	if (myupper (*nextc))
	    wascapitalized = 0;
	}
    /*
     * The conditions are satisfied.  Copy the word, add the suffix,
     * and make it match the case of the last remaining character of the
     * root.  Again, this code carefully matches ins_cap and cap_ok.
     *
     * There is one tricky case to watch out for here.  If the root
     * was simply capitalized, and the stripping removed all but the
     * initial character of the root, the above algorithm will
     * generate an all-caps result.  However, ins_cap and cap_ok would
     * generate a simple capitalization, because a captype of
     * CAPITALIZED would override FOLLOWCASE and ALLCAPS.  So we have
     * to deal with this as a special case.
     */
    (void) icharcpy (tword, rootword);
    nextc = tword + tlen - flent->stripl;
    if (flent->affl)
	{
	(void) icharcpy (nextc, flent->affix);
	if ((nextc == tword + 1  &&  wascapitalized)
	  ||  !myupper (nextc[-1]))
	    forcelc (nextc, flent->affl);
	}
    else
	*nextc = 0;
    /*
     * We've generated the result.  If it's not already in the
     * expansion table, add it at the end.
     */
    result = ichartosstr (tword, 1);
    SETMASKBIT (existing_flags, flent->flagbit);
    (void) add_expansion_copy(exptable, (char *) result, existing_flags);
    }

static void forcelc (dst, len)			/* Force to lowercase */
    register ichar_t *		dst;		/* Destination to modify */
    register int		len;		/* Length to copy */
    {

    for (  ;  --len >= 0;  dst++)
	*dst = mytolower (*dst);
    }

/*
 * Convert a mask of flags to a string of flag chars, using a static
 * buffer. 
 *
 * Assume 26 letters in the alphabet :-).  In practice you probably
 * won't get more than two flags at once.
 */
static char * flags_str (flags)
    MASKTYPE			flags;
    {
    static char			flags_str_buf[MASKSIZE + 1];
    int				i;
    int				length = 0;

    for (i = 0;  i < MASKSIZE;  i++)
	{
	if (TSTMASKBIT (&flags, i))
	    flags_str_buf[length++] = BITTOCHAR (i);
	}

    flags_str_buf[length] = '\0';
    return flags_str_buf;
    }


/*
 * Print an expansion table in the manner required by ispell's various
 * -e options (or at least do some of the printing required).  Returns
 * the total length of all expansions.
 *
 * P.S. Yes, I'm embarrassed by the magic numbers.
 */
static int output_expansions (exptable, option, croot, extra)
    struct exp_table *		exptable;	/* Expansions to print */
    int				option;		/* Num. of -e option */
    unsigned char *		croot;		/* Original "word/flags" */
    unsigned char *		extra;		/* Extra info to add to line */
    {
    int				explength = 0;	/* Expansion size (chars) */
    int				i;		/* Loop index */
    const ichar_t *		rootword;	/* Root of the expansion */

    rootword = get_orig_word (exptable);

    /* Print in reverse order to match old behavior */
    for (i = num_expansions (exptable);  --i >= 0;  )
	{
	const char * expansion = get_expansion (exptable, i);

	if (option == 3)
	    (void) printf ("\n%s", (char *) croot);
	else if (option == 5)
	    {
	    char * fs = flags_str (get_flags (exptable, i));

	    if (strlen (fs) != 0)
		(void) printf ("\n%s+%s", ichartosstr (rootword, 1), fs);
	    else
		(void) printf ("\n%s", ichartosstr (rootword, 1));
	    }

	if (option != 4)
	    (void) printf (" %s%s", expansion, (char *) extra);

	explength += strlen (expansion);
	}

    return explength;
    }
