#ifndef lint
static char Rcs_Id[] =
    "$Id: defmt.c,v 1.1.1.2 2007-02-01 19:50:06 ghudson Exp $";
#endif

/*
 * defmt.c - Handle formatter constructs, mostly by scanning over them.
 *
 * This code originally resided in ispell.c, but was moved here to keep
 * file sizes smaller.
 *
 * Copyright (c), 1983, by Pace Willisson
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
 *
 * The TeX code is originally by Greg Schaffer, with many improvements from
 * Ken Stevens.  The nroff code is primarily from Pace Willisson, although
 * other people have improved it.
 */

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.62  2005/04/20 23:16:32  geoff
 * Use inpossibilities to deal with the case where uppercase SS in German
 * causes ambiguities.
 *
 * Revision 1.61  2005/04/14 14:38:23  geoff
 * Update license.
 *
 * Revision 1.60  2001/11/28 22:39:26  geoff
 * Add Ken Stevens's fix for newlines in \verb
 *
 * Revision 1.59  2001/09/06 00:30:29  geoff
 * Many changes from Eli Zaretskii to support DJGPP compilation.
 *
 * Revision 1.58  2001/08/01 22:15:56  geoff
 * When processing quoted strings inside HTML tags, don't handle
 * ampersand sequences unless the quoted string is being spell-checked.
 * That way, ampersands inside HREF links won't confuse the deformatter,
 * but ampersand sequence inside ALT= tags will be handled correctly.
 * Also, when skipping ampersand sequences inside quoted strings, give up
 * the search for the semicolon if the closing quote is hit.  This change
 * makes us a bit more robust in the fact of HTML syntax errors.
 *
 * Revision 1.57  2001/07/25 21:51:47  geoff
 * Minor license update.
 *
 * Revision 1.56  2001/07/23 20:24:03  geoff
 * Update the copyright and the license.
 *
 * Revision 1.55  2000/08/22 10:52:25  geoff
 * Fix a whole bunch of signed/unsigned compiler warnings.
 *
 * Revision 1.54  2000/08/22 00:11:25  geoff
 * Add support for correct_verbose_mode.
 *
 * Revision 1.53  1999/11/04 08:16:54  geoff
 * Add a few more special TeX sequences
 *
 * Revision 1.52  1999/01/18  03:28:27  geoff
 * Turn many char declarations into unsigned char, so that we won't have
 * sign-extension problems.
 *
 * Revision 1.51  1999/01/07  01:57:53  geoff
 * Update the copyright.
 *
 * Revision 1.50  1999/01/03  01:46:28  geoff
 * Add support for external deformatters.
 *
 * Revision 1.49  1998/07/07  02:30:53  geoff
 * Implement the plus notation for adding to keyword lists
 *
 * Revision 1.48  1998/07/06  06:55:13  geoff
 * Generalize the HTML tag routines to be handy keyword lookup routines,
 * and fix some (but by no means all) of the TeX deformatting to take
 * advantage of these routines to allow a bit more flexibility in
 * processing private TeX commands.
 *
 * Revision 1.47  1998/07/06  05:34:10  geoff
 * Add a few more TeX commands
 *
 * Revision 1.46  1997/12/01  00:53:47  geoff
 * Add HTML support.  Fix the "\ " bug again, this time (hopefully) right.
 *
 * Revision 1.45  1995/11/08  05:09:29  geoff
 * Add the new interactive mode ("askverbose").
 *
 * Revision 1.44  1995/11/08  04:32:51  geoff
 * Modify the HTML support to be stylistically more consistent and to
 * interoperate more cleanly with the nroff and TeX modes.
 *
 * Revision 1.43  1995/10/25  04:05:31  geoff
 * html-mode code added by Gerry Tierney <gtierney@nova.ucd.ie> 14th of
 * Oct '95.
 *
 * Revision 1.42  1995/10/25  03:35:42  geoff
 * After skipping over a backslash sequence, make sure that we don't skip
 * over the first character of the following word.  Also, support the
 * verbatim environment of LaTex.
 *
 * Revision 1.41  1995/08/05  23:19:47  geoff
 * Get rid of an obsolete comment.  Add recognition of documentclass and
 * usepackage for Latex2e support.
 *
 * Revision 1.40  1995/03/06  02:42:43  geoff
 * Change TeX backslash processing so that it only assumes alpha
 * characters and the commonly-used "@" character are part of macro
 * names, and so that any other backslashed character (specifically
 * dollar signs) is skipped.
 *
 * Revision 1.39  1995/01/08  23:23:54  geoff
 * Fix typos in a couple of comments.
 *
 * Revision 1.38  1995/01/03  19:24:14  geoff
 * Add code to handle the LaTeX \verb command.
 *
 * Revision 1.37  1994/12/27  23:08:54  geoff
 * Fix a bug in TeX backslash processing that caused ispell to become
 * confused when it encountered an optional argument to a
 * double-backslash command.  Be a little smarter about scanning for
 * curly-brace matches, so that we avoid missing a math-mode transition
 * during the scan.
 *
 * Revision 1.36  1994/10/25  05:46:34  geoff
 * Recognize a few more Latex commands: pagestyle, pagenumbering,
 * setcounter, addtocounter, setlength, addtolength, settowidth.
 *
 * Revision 1.35  1994/10/18  04:03:19  geoff
 * Add code to skip hex numbers if they're preceded by '0x'.
 *
 * Revision 1.34  1994/10/04  03:51:24  geoff
 * Modify the parsing so that TeX commands are ignored even within
 * comments, but do not affect the overall parsing state.  (This is
 * slightly imperfect, in that some types of modality are ignored when
 * comments are entered.  But it should solve nearly all the problems
 * with commented-out TeX commands.)  This also fixes a couple of minor
 * bugs with TeX deformatting.
 *
 * Revision 1.33  1994/10/03  17:06:07  geoff
 * Remember to use contextoffset when reporting complete misses
 *
 * Revision 1.32  1994/08/31  05:58:41  geoff
 * Report the offset-within-line correctly in -a mode even if the line is
 * longer than BUFSIZ characters.
 *
 * Revision 1.31  1994/05/25  04:29:28  geoff
 * If two boundary characters appear in a row, consider it the end of the
 * word.
 *
 * Revision 1.30  1994/05/17  06:44:08  geoff
 * Add the new argument to all calls to good and compoundgood.
 *
 * Revision 1.29  1994/03/16  06:30:41  geoff
 * Don't lose track of math mode when an array environment is embedded.
 *
 * Revision 1.28  1994/03/15  05:31:57  geoff
 * Add TeX_strncmp, which allows us to handle AMS-TeX constructs like
 * \endroster without getting confused.
 *
 * Revision 1.27  1994/02/14  00:34:53  geoff
 * Pass length arguments to correct().
 *
 * Revision 1.26  1994/01/25  07:11:25  geoff
 * Get rid of all old RCS log lines in preparation for the 3.1 release.
 *
 */

#include <ctype.h>
#include "config.h"
#include "ispell.h"
#include "proto.h"
#include "msgs.h"

static unsigned char *
		skiptoword P ((unsigned char * bufp));
unsigned char *	skipoverword P ((unsigned char * bufp));
void		checkline P ((FILE * ofile));
static int	TeX_math_end P ((unsigned char ** bufp));
static int	TeX_math_begin P ((unsigned char ** bufp));
static int	TeX_LR_begin P ((unsigned char ** bufp));
static int	TeX_LR_check P ((int begin_p, unsigned char ** bufp));
static void	TeX_skip_args P ((unsigned char ** bufp));
static int	TeX_math_check P ((int cont_char, unsigned char ** bufp));
static void	TeX_skip_parens P ((unsigned char ** bufp));
static void	TeX_open_paren P ((unsigned char ** bufp));
static void	TeX_skip_check P ((unsigned char ** bufp));
static int	TeX_strncmp P ((unsigned char * a, char * b, int n));
int		init_keyword_table P ((char * rawtags, char * envvar,
		  char * deftags, int ignorecase, struct kwtable * keywords));
static int	keyword_in_list P ((unsigned char * string,
		  unsigned char * stringend, struct kwtable * keywords));
static int	tagcmp P ((unsigned char ** a, unsigned char ** b));

#define ISTEXTERM(c)   (((c) == TEXLEFTCURLY) || \
			((c) == TEXRIGHTCURLY) || \
			((c) == TEXLEFTSQUARE) || \
			((c) == TEXRIGHTSQUARE))
#define ISMATHCH(c)    (((c) == TEXBACKSLASH) || \
			((c) == TEXDOLLAR) || \
			((c) == TEXPERCENT))

static int	    TeX_comment = 0;

/*
 * The following variables are used to save the parsing state when
 * processing comments.  This allows comments to be parsed without
 * affecting the overall nesting.
 */
static int save_math_mode;
static char save_LaTeX_Mode;

/*
 * The following variables are used by the deformatter to keep
 * track of keywords that may indicate text to be ignored.
 */
static unsigned char *
		keywordbuf;	/* Scratch buffer for keyword comparison */
static unsigned int
		maxkeywordlen;	/* Length of longest keyword */

static unsigned char * skiptoword (bufp) /* Skip to beginning of a word */
    unsigned char *	bufp;
    {
    unsigned char *	htmltagstart;	/* Beginning of an HTML tag */
    unsigned char *	htmlsubfield = bufp;
					/* Ptr to start of subfield name */

    while (*bufp
      &&  ((!isstringch (bufp, 0)  &&  !iswordch (chartoichar (*bufp)))
	||  isboundarych (chartoichar (*bufp))
	||  (tflag == DEFORMAT_TEX  &&  ((math_mode & 1) || LaTeX_Mode != 'P'))
	||  (insidehtml & (HTML_IN_SPEC | HTML_ISIGNORED)) != 0
	||  ((insidehtml & HTML_IN_TAG) != 0
	  &&  (insidehtml
		& (HTML_IN_QUOTE | HTML_CHECKING_QUOTE))
	      != (HTML_IN_QUOTE | HTML_CHECKING_QUOTE))
	)
      )
	{
	/* 
	 * HTML deformatting
	 */
	if (tflag == DEFORMAT_SGML)
	    {
	    if ((insidehtml & HTML_IN_TAG) != 0  &&  *bufp == HTMLQUOTE)
		{
		if (insidehtml & HTML_IN_QUOTE)
		    insidehtml &= ~(HTML_IN_QUOTE | HTML_CHECKING_QUOTE);
		else
		    insidehtml |= HTML_IN_QUOTE;
		htmlsubfield = NULL;
		}
	    else if ((insidehtml & (HTML_IN_TAG | HTML_IN_QUOTE))
		== HTML_IN_TAG
	      &&  *bufp == HTMLTAGEND)
		insidehtml &=
		  ~(HTML_IN_TAG | HTML_IN_ENDTAG | HTML_CHECKING_QUOTE);
	    /*
	     * If we are checking an HTML file, we want to ignore any HTML
	     * tags.  These should start with a '<' and end with a '>', so
	     * we simply skip over anything between these two symbols.  If
	     * we reach the end of the line before finding a matching '>',
	     * we set 'insidehtml' appropriately.
	     */
	    else if ((insidehtml & HTML_IN_TAG) == 0
	      &&  *bufp == HTMLTAGSTART)
		{
		insidehtml |= HTML_IN_TAG;
		bufp++;
		if (*bufp == HTMLSLASH)
		    {
		    bufp++;
		    insidehtml |= HTML_IN_ENDTAG;
		    }
		htmltagstart = bufp;
		/*
		 * We found the start of an HTML tag.  Skip to the end
		 * of the tag.  We assume that all tags are made up of
		 * purely alphabetic characters.
		 */
		while (isalpha (*bufp))
		    bufp++;
		/*
		 * Check to see if this is an ignored tag, and set
		 * HTML_IGNORE properly.
		 */
		if (keyword_in_list (htmltagstart, bufp, &htmlignorelist))
		    {
		    /*
		     * Note that we use +/- here, rather than Boolean
		     * operators.  This is quite deliberate, because
		     * it allows us to properly handle nested HTML
		     * constructs that are supposed to be ignored.
		     */
		    if (insidehtml & HTML_IN_ENDTAG)
			insidehtml -= HTML_IGNORE;
		    else
			insidehtml += HTML_IGNORE;
		    }
		htmlsubfield = NULL;
		if (bufp > htmltagstart)
		    bufp--;
		}
	    else if ((insidehtml & (HTML_IN_TAG | HTML_IN_QUOTE))
	     == HTML_IN_TAG)
		{
		if (htmlsubfield == NULL  &&  isalpha (*bufp))
		    htmlsubfield = bufp;
		else if (htmlsubfield != NULL  &&  !isalpha (*bufp))
		    {
		    if (bufp != htmlsubfield
		      &&  keyword_in_list (htmlsubfield, bufp, &htmlchecklist))
			insidehtml |= HTML_CHECKING_QUOTE;
		    htmlsubfield = NULL;
		    }
		}
	    /*
	     * Skip over quoted entities such as "&quot;".  These all
	     * start with an ampersand and end with a semi-colon.  We
	     * do not need to worry about them extending over more
	     * than one line.  We also don't need to worry about them
	     * being string characters, because the isstringch() test
	     * above would have already broken us out of the enclosing
	     * loop in that case.
	     *
	     * A complication is that quoted entities are only
	     * interpreted in some quoted strings.  For example,
	     * they're valid in ALT= tags but not in HREF tags, where
	     * ampersands have an entirely different meaning.  We deal
	     * with the problem by only interpreting HTMLSPECSTART if
	     * we are checking the quoted string.
	     */
	    else if ((insidehtml & HTML_IN_SPEC) != 0
	      ||  (*bufp == HTMLSPECSTART
		&&  ((insidehtml & HTML_CHECKING_QUOTE)
		  ||  (insidehtml & HTML_IN_QUOTE) == 0)))
		{
		while (*bufp != HTMLSPECEND  &&  *bufp != '\0')
		    {
		    if ((insidehtml & HTML_IN_QUOTE)  &&  *bufp == HTMLQUOTE)
			{
			/*
			 * The quoted string ended before the
			 * ampersand sequence finished.  The HTML is
			 * probably incorrect, but it would be a
			 * mistake to keep skipping until we reach the
			 * next random semicolon.  Instead, just stop
			 * skipping right here.
			 */
			insidehtml &= ~(HTML_IN_QUOTE | HTML_CHECKING_QUOTE);
			break;
			}
		    bufp++;
		    }
		if (*bufp == '\0')
		    insidehtml |= HTML_IN_SPEC;
		else
		    insidehtml &= ~HTML_IN_SPEC;
		}
	    }
	else if (tflag == DEFORMAT_TEX) /* TeX or LaTeX stuff */
	    {
	    /* Odd numbers mean we are in "math mode" */
	    /* Even numbers mean we are in LR or */
	    /* paragraph mode */
	    if (*bufp == TEXPERCENT  &&  LaTeX_Mode != 'v')
		{
		if (!TeX_comment)
		    {
		    save_math_mode = math_mode;
		    save_LaTeX_Mode = LaTeX_Mode;
		    math_mode = 0;
		    LaTeX_Mode = 'P';
		    TeX_comment = 1;
		    }
		}
	    else if (math_mode & 1)
		{
		if ((LaTeX_Mode == 'e'  &&  TeX_math_check('e', &bufp))
		  || (LaTeX_Mode == 'm'  &&  TeX_LR_check(1, &bufp)))
		    math_mode--;    /* end math mode */
		else
		    {
		    while (*bufp  && !ISMATHCH(*bufp))
			bufp++;
		    if (*bufp == 0)
			break;
		    if (TeX_math_end(&bufp))
			math_mode--;
		    }
		if (math_mode < 0)
		    {
		    (void) fprintf (stderr, DEFMT_C_TEX_MATH_ERROR,
		      MAYBE_CR (stderr));
		    math_mode = 0;
		    }
		}
	    else
		{
		if (math_mode > 1
		  &&  *bufp == TEXRIGHTCURLY
		  &&  (math_mode < (math_mode & 127) * 128))
		    math_mode--;    /* re-enter math */
		else if (LaTeX_Mode == 'm'
		    || (math_mode && (math_mode >= (math_mode & 127) * 128)
		  &&  (TeX_strncmp(bufp, "\\end", 4)
		    == 0)))
		    {
		    if (TeX_LR_check(0, &bufp))
			math_mode--;
		    }
		else if (LaTeX_Mode == 'b'  &&  TeX_math_check('b', &bufp))
		    {
		    /* continued begin */
		    math_mode++;
		    }
		else if (LaTeX_Mode == 'r')
		    {
		    /* continued "reference" */
		    TeX_skip_parens(&bufp);
		    LaTeX_Mode = 'P';
		    }
		else if (LaTeX_Mode == 'v')
		    {
		    /* continued "verb" */
		    while (*bufp != save_LaTeX_Mode  &&  *bufp != '\0')
			bufp++;
		    if (*bufp != 0)
			LaTeX_Mode = 'P';
		    }
		else if (TeX_math_begin(&bufp))
		    /* checks references and */
		    /* skips \ commands */
		    math_mode++;
		}
	    if (*bufp == 0)
		break;
	    }
	else if (tflag == DEFORMAT_NROFF)	/* nroff deformatting */
	    {
	    if (*bufp == NRBACKSLASH)
		{
		switch ( bufp[1] )
		    {
		    case 'f':
			if(bufp[2] == NRLEFTPAREN)
			    {
			    /* font change: \f(XY */
			    bufp += 5;
			    }
			else
			    {
			    /* ) */
			    /* font change: \fX */
			    bufp += 3;
			    }
			continue;
		    case 's':
			/* size change */
			bufp += 2;
			if (*bufp == '+'  ||  *bufp == '-')
			    bufp++;
			/* This looks wierd 'cause we
			** assume *bufp is now a digit.
			*/
			bufp++;
			if (isdigit (*bufp))
			    bufp++;
			continue;
		    default:
			if (bufp[1] == NRLEFTPAREN)
			    {
			    /* extended char set */
			    /* escape:  \(XX */
			    /* ) */
			    bufp += 4;
			    continue;
			    }
			else if (bufp[1] == NRSTAR)
			    {
			    if (bufp[2] == NRLEFTPAREN)
				bufp += 5;
			    else
				bufp += 3;
			    continue;
			    }
			break;
		    }
		}
	    }
	/*
	 * Skip hex numbers, but not if we're in non-terse askmode.
	 * (In that case, we'd lose sync if we skipped hex.)
	 */
	if (*bufp == '0'
	  &&  (bufp[1] == 'x'  ||  bufp[1] == 'X')
	  &&  (terse  ||  !aflag))
	    {
	    bufp += 2;
	    while (isxdigit (*bufp))
		bufp++;
	    }
	else
	    bufp++;
	}
    if (*bufp == '\0')
	{
	if (TeX_comment)
	    {
	    math_mode = save_math_mode;
	    LaTeX_Mode = save_LaTeX_Mode;
	    TeX_comment = 0;
	    }
	}
    return bufp;
    }

unsigned char * skipoverword (bufp) /* Return pointer to end of a word */
    register unsigned char *
			bufp;	/* Start of word -- MUST BE A REAL START */
    {
    register unsigned char *
			lastboundary;
    register int	scharlen; /* Length of a string character */

    lastboundary = NULL;
    for (  ;  ;  )
	{
	if (*bufp == '\0')
	    {
	    if (TeX_comment)
		{
		math_mode = save_math_mode;
		LaTeX_Mode = save_LaTeX_Mode;
		TeX_comment = 0;
		}
	    break;
	    }
	else if (l_isstringch(bufp, scharlen, 0))
	    {
	    bufp += scharlen;
	    lastboundary = NULL;
	    }
	/*
	** Note that we get here if a character satisfies
	** isstringstart() but isn't in the string table;  this
	** allows string characters to start with word characters.
	*/
	else if (iswordch (chartoichar (*bufp)))
	    {
	    bufp++;
	    lastboundary = NULL;
	    }
	else if (isboundarych (chartoichar (*bufp)))
	    {
	    if (lastboundary == NULL)
		lastboundary = bufp;
	    else if (lastboundary == bufp - 1)
		break;			/* Double boundary -- end of word */
	    bufp++;
	    }
	else
	    break;			/* End of the word */
	}
    /*
    ** If the word ended in one or more boundary characters, 
    ** the address of the first of these is in lastboundary, and it
    ** is the end of the word.  Otherwise, bufp is the end.
    */
    return (lastboundary != NULL) ? lastboundary : bufp;
    }

void checkline (ofile)
    FILE *		ofile;
    {
    register unsigned char *
			p;
    register unsigned char *
			endp;
    int			hadlf;
    register int	len;
    register int	i;
    int			ilen;

    currentchar = filteredbuf;
    len = strlen ((char *) filteredbuf) - 1;
    hadlf = filteredbuf[len] == '\n';
    if (hadlf)
	{
	filteredbuf[len] = '\0';
	contextbufs[0][len] = '\0';
	}

    if (tflag == DEFORMAT_NROFF)
	{
	/* skip over .if */
	if (*currentchar == NRDOT
	  &&  (strncmp ((char *) currentchar + 1, "if t", 4) == 0
	    ||  strncmp ((char *) currentchar + 1, "if n", 4) == 0))
	    {
	    copyout (&currentchar,5);
	    while (*currentchar
	      &&  myspace (chartoichar (*currentchar)))
		copyout (&currentchar, 1);
	    }

	/* skip over .ds XX or .nr XX */
	if (*currentchar == NRDOT
	  &&  (strncmp ((char *) currentchar + 1, "ds ", 3) == 0 
	    ||  strncmp ((char *) currentchar + 1, "de ", 3) == 0
	    ||  strncmp ((char *) currentchar + 1, "nr ", 3) == 0))
	    {
	    copyout (&currentchar, 4);
	    while (*currentchar
	      &&  myspace (chartoichar (*currentchar)))
		copyout(&currentchar, 1);
	    while (*currentchar
	      &&  !myspace (chartoichar (*currentchar)))
		copyout(&currentchar, 1);
	    if (*currentchar == 0)
		{
		if (!lflag  &&  hadlf)
		    (void) putc ('\n', ofile);
		return;
		}
	    }
	}

    /* if this is a formatter command, skip over it */
    if (tflag == DEFORMAT_NROFF  &&  *currentchar == NRDOT)
	{
	while (*currentchar  &&  !myspace (chartoichar (*currentchar)))
	    {
	    if (!aflag && !lflag)
		(void) putc (*currentchar, ofile);
	    currentchar++;
	    }
	if (*currentchar == 0)
	    {
	    if (!lflag  &&  hadlf)
		(void) putc ('\n', ofile);
	    return;
	    }
	}
	
    for (  ;  ;  )
	{
	p = skiptoword (currentchar);
	if (p != currentchar)
	    copyout (&currentchar, p - currentchar);

	if (*currentchar == 0)
	    break;

	p = ctoken;
	endp = skipoverword (currentchar);
	while (currentchar < endp  &&  p < ctoken + sizeof ctoken - 1)
	    *p++ = *currentchar++;
	*p = 0;
	if (strtoichar (itoken, ctoken, INPUTWORDLEN * sizeof (ichar_t), 0))
	    (void) fprintf (stderr, WORD_TOO_LONG ((char *) ctoken));
	ilen = icharlen (itoken);

	if (lflag)
	    {
	    if (ilen > minword
	      &&  !good (itoken, 0, 0, 0, 0)
	      &&  !cflag  &&  !compoundgood (itoken, 0))
		(void) fprintf (ofile, "%s\n", (char *) ctoken);
	    }
	else
	    {
	    if (aflag)
		{
		if (ilen <= minword)
		    {
		    /* matched because of minword */
		    if (!terse)
			{
			if (askverbose)
			    (void) fprintf (ofile, "ok\n");
			else
			    {
			    if (correct_verbose_mode)
				(void) fprintf (ofile, "* %s\n", ctoken );
			    else
				(void) fprintf (ofile, "*\n");
			    }
			}
		    continue;
		    }
		if (good (itoken, 0, 0, 0, 0))
		    {
		    if (hits[0].prefix == NULL
		      &&  hits[0].suffix == NULL)
			{
			/* perfect match */
			if (!terse)
			    {
			    if (askverbose)
				(void) fprintf (ofile, "ok\n");
			    else
				{
				if (correct_verbose_mode)
				    (void) fprintf (ofile, "* %s\n", ctoken );
				else
				    (void) fprintf (ofile, "*\n");
				}
			    }
			}
		    else if (!terse)
			{
			/* matched because of root */
			if (askverbose)
			    (void) fprintf (ofile,
			      "ok (derives from root %s)\n",
			      (char *) hits[0].dictent->word);
			else
			    {
			    if (correct_verbose_mode)
				(void) fprintf (ofile, "+ %s %s\n",
				  ctoken, hits[0].dictent->word);
			    else
				(void) fprintf (ofile, "+ %s\n",
				  hits[0].dictent->word);
			    }
			}
		    }
		else if (compoundgood (itoken, 0))
		    {
		    /* compound-word match */
		    if (!terse)
			{
			if (askverbose)
			    (void) fprintf (ofile, "ok (compound word)\n");
			else
			    {
			    if (correct_verbose_mode)
				(void) fprintf (ofile, "- %s\n", ctoken);
			    else
				(void) fprintf (ofile, "-\n");
			    }
			}
		    }
		else
		    {
		    makepossibilities (itoken);
		    if (inpossibilities (ctoken)) /* Kludge for German, etc. */
			{
			/* might not be perfect match, but we'll lie */
			if (!terse)
			    {
			    if (askverbose)
				(void) fprintf (ofile, "ok\n");
			    else
				{
				if (correct_verbose_mode)
				    (void) fprintf (ofile, "* %s\n", ctoken );
				else
				    (void) fprintf (ofile, "*\n");
				}
			    }
			}
		    else if (pcount)
			{
			/*
			** print &  or ?, ctoken, then
			** character offset, possibility
			** count, and the possibilities.
			*/
			if (askverbose)
			    (void) fprintf (ofile, "how about");
			else
			    (void) fprintf (ofile, "%c %s %d %d",
			      easypossibilities ? '&' : '?',
			      (char *) ctoken,
			      easypossibilities,
			      (int) ((currentchar - filteredbuf)
				- strlen ((char *) ctoken)) + contextoffset);
			for (i = 0;  i < MAXPOSSIBLE;  i++)
			    {
			    if (possibilities[i][0] == 0)
				break;
			    (void) fprintf (ofile, "%c %s",
			      i ? ',' : ':', possibilities[i]);
			    }
			(void) fprintf (ofile, "\n");
			}
		    else
			{
			/*
			** No possibilities found for word TOKEN
			*/
			if (askverbose)
			    (void) fprintf (ofile, "not found\n");
			else
			    (void) fprintf (ofile, "# %s %d\n",
			      (char *) ctoken,
			      (int) ((currentchar - filteredbuf)
				- strlen ((char *) ctoken)) + contextoffset);
			}
		    }
		}
	    else
		{
		if (!quit)
		   correct (ctoken, sizeof ctoken, itoken, sizeof itoken,
		     &currentchar);
		}
	    }
	if (!aflag  &&  !lflag)
	   (void) fprintf (ofile, "%s", (char *) ctoken);
	}

    if (!lflag  &&  hadlf)
       (void) putc ('\n', ofile);
   }

/* must check for \begin{mbox} or whatever makes new text region. */
static int TeX_math_end (bufp)
    unsigned char **	bufp;
    {

    if (**bufp == TEXDOLLAR)
	{
	if ((*bufp)[1] == TEXDOLLAR)
	    (*bufp)++;
	return 1;
	}
    else if (**bufp == TEXPERCENT)
	{
	if (!TeX_comment)
	    {
	    save_math_mode = math_mode;
	    save_LaTeX_Mode = LaTeX_Mode;
	    math_mode = 0;
	    LaTeX_Mode = 'P';
	    TeX_comment = 1;
	    }
	return 0;
	}
    /* processing extended TeX command */
    (*bufp)++;
    if (**bufp == TEXRIGHTPAREN  ||  **bufp == TEXRIGHTSQUARE)
	return 1;
    if (TeX_LR_begin (bufp))	/* check for switch back to LR mode */
	return 1;
    if (TeX_strncmp (*bufp, "end", 3) == 0)
	/* find environment that is ending */
	return TeX_math_check ('e', bufp);
    else
	return 0;
    }

static int TeX_math_begin (bufp)
    unsigned char **	bufp;
    {
    int			didskip = 0;

    if (**bufp == TEXDOLLAR)
	{
	if ((*bufp)[1] == TEXDOLLAR)
	    (*bufp)++;
	return 1;
	}
    while (**bufp == TEXBACKSLASH)
	{
	didskip = 1;
	(*bufp)++; /* check for null char here? */
	if (**bufp == TEXLEFTPAREN  ||  **bufp == TEXLEFTSQUARE)
	    return 1;
	else if (!isalpha(**bufp)  &&  **bufp != '@')
	    {
	    (*bufp)++;
	    continue;
	    }
	else if (TeX_strncmp (*bufp, "begin", 5) == 0)
	    {
	    if (TeX_math_check ('b', bufp))
		return 1;
	    else
		(*bufp)--;
	    }
	else
	    {
	    TeX_skip_check (bufp);
	    return 0;
	    }
	}
      /*
       * Ignore references for the tib (1) bibliography system, that
       * is, text between a ``[.'' or ``<.'' and ``.]'' or ``.>''.
       * We don't care whether they match, tib doesn't care either.
       *
       * A limitation is that the entire tib reference must be on one
       * line, or we break down and check the remainder anyway.
       */ 
    if ((**bufp == TEXLEFTSQUARE  ||  **bufp == TEXLEFTANGLE)
      &&  (*bufp)[1] == TEXDOT)
	{
	(*bufp)++;
	while (**bufp)
	    {
	    if (*(*bufp)++ == TEXDOT
	      &&  (**bufp == TEXRIGHTSQUARE  ||  **bufp == TEXRIGHTANGLE))
		return TeX_math_begin (bufp);
	    }
	return 0;
	}
    else if (didskip)
	{
	/*
	 * If we've skipped over anything, it's possible that we are
	 * pointing at an important character.  If so, we need to back up
	 * one byte, because our caller will increment bufp.  Yes,
	 * this is a kludge.  This whole TeX deformatter is a mess.
	 */
	(*bufp)--;
	}
    return 0;
    }

static int TeX_LR_begin (bufp)
    unsigned char **	bufp;
    {

    if ((TeX_strncmp (*bufp, "mbox", 4) == 0)
      ||  (TeX_strncmp (*bufp, "makebox", 7) == 0)
      ||  (TeX_strncmp (*bufp, "text", 4) == 0)
      ||  (TeX_strncmp (*bufp, "intertext", 9) == 0)
      ||  (TeX_strncmp (*bufp, "fbox", 4) == 0)
      || (TeX_strncmp (*bufp, "framebox", 8) == 0))
	math_mode += 2;
    else if ((TeX_strncmp (*bufp, "parbox", 6) == 0)
      || (TeX_strncmp (*bufp, "raisebox", 8) == 0))
	{
	math_mode += 2;
	TeX_open_paren (bufp);
	if (**bufp)
	    (*bufp)++;
	else
	    LaTeX_Mode = 'r'; /* same as reference -- skip {} */
	}
    else if (TeX_strncmp (*bufp, "begin", 5) == 0)
	return TeX_LR_check (1, bufp);	/* minipage */
    else
	return 0;

    /* skip tex command name and optional or width arguments. */
    TeX_open_paren (bufp);
    return 1;
    }

static int TeX_LR_check (begin_p, bufp)
    int			begin_p;
    unsigned char **	bufp;
    {

    TeX_open_paren (bufp);
    if (**bufp == 0)	/* { */
	{
	LaTeX_Mode = 'm';
	return 0;	/* remain in math mode until '}' encountered. */
	}
    else
	LaTeX_Mode = 'P';
    if (strncmp ((char *) ++(*bufp), "minipage", 8) == 0)
	{
	TeX_skip_parens (bufp);
	if (**bufp)
	    (*bufp)++;
	if (begin_p)
	    {
	    TeX_skip_parens (bufp); /* now skip opt. args if on this line. */
	    math_mode += 2;
	    /* indicate minipage mode. */
	    math_mode += ((math_mode & 127) - 1) * 128;
	    }
	else
	    {
	    math_mode -= (math_mode & 127) * 128;
	    if (math_mode < 0)
		{
		(void) fprintf (stderr, DEFMT_C_LR_MATH_ERROR,
		  MAYBE_CR (stderr));
		math_mode = 1;
		}
	    }
	return 1;
	}
    (*bufp)--;
    return 0;
    }

/* Skips the begin{ARG}, and optionally up to two {PARAM}{PARAM}'s to
 *  the begin if they are required.  However, Only skips if on this line.
 */
static void TeX_skip_args (bufp)
    unsigned char **	bufp;
    {
    register int skip_cnt = 0; /* Max of 2. */

    if (strncmp((char *) *bufp, "tabular", 7) == 0
      ||  strncmp((char *) *bufp, "minipage", 8) == 0)
	skip_cnt++;
    if (strncmp((char *) *bufp, "tabular*", 8) == 0)
	skip_cnt++;
    TeX_skip_parens (bufp);	/* Skip to the end of the \begin{} parens */
    if (**bufp)
	(*bufp)++;
    else
	return;
    if (skip_cnt--)
	TeX_skip_parens (bufp);	/* skip 1st {PARAM}. */
    else
	return;
    if (**bufp)
	(*bufp)++;
    else
	return;
    if (skip_cnt)
	TeX_skip_parens (bufp);	/* skip to end of 2nd {PARAM}. */
    }

static int TeX_math_check (cont_char, bufp)
    int			cont_char;
    unsigned char **	bufp;
    {

    TeX_open_paren (bufp);
    /* Check for end of line, continue later. */
    if (**bufp == 0)
	{
	LaTeX_Mode = (char) cont_char;
	return 0;
	}
    else
	LaTeX_Mode = 'P';

    if (strncmp ((char *) ++(*bufp), "equation", 8) == 0
      ||  strncmp ((char *) *bufp, "eqnarray", 8) == 0
      ||  strncmp ((char *) *bufp, "displaymath", 11) == 0
      ||  strncmp ((char *) *bufp, "picture", 7) == 0
      ||  strncmp ((char *) *bufp, "gather", 6) == 0
      ||  strncmp ((char *) *bufp, "align", 5) == 0
      ||  strncmp ((char *) *bufp, "multline", 8) == 0
      ||  strncmp ((char *) *bufp, "flalign", 7) == 0
      ||  strncmp ((char *) *bufp, "alignat", 7) == 0
#ifdef IGNOREBIB
      ||  strncmp ((char *) *bufp, "thebibliography", 15) == 0
#endif
      ||  strncmp ((char *) *bufp, "verbatim", 8) == 0
      ||  strncmp ((char *) *bufp, "math", 4) == 0)
	{
	(*bufp)--;
	TeX_skip_parens (bufp);
	return 1;
	}
    if (cont_char == 'b')
	TeX_skip_args (bufp);
    else
	TeX_skip_parens (bufp);
    return 0;
    }

static void TeX_skip_parens (bufp)
    unsigned char **	bufp;
    {

    while (**bufp  &&  **bufp != TEXRIGHTCURLY  &&  **bufp != TEXDOLLAR)
	(*bufp)++;
    }

static void TeX_open_paren (bufp)
    unsigned char **	bufp;
    {
    while (**bufp  &&  **bufp != TEXLEFTCURLY  &&  **bufp != TEXDOLLAR)
	(*bufp)++;
    }

static void TeX_skip_check (bufp)
    unsigned char **	bufp;
    {
    unsigned char *	endp;
    int			skip_ch;

    for (endp = *bufp;  isalpha(*endp)  ||  *endp == '@';  endp++)
	;
    if (keyword_in_list (*bufp, endp, &texskip1list))
	{
	TeX_skip_parens (bufp);
	if (**bufp == 0)
	    LaTeX_Mode = 'r';
	}
    else if (keyword_in_list (*bufp, endp, &texskip2list)) /* skip two args. */
	{
	TeX_skip_parens (bufp);
	if (**bufp == 0)	/* Only skips one {} if not on same line. */
	    LaTeX_Mode = 'r';
	else			/* Skip second arg. */
	    {
	    (*bufp)++;
	    TeX_skip_parens (bufp);
	    if (**bufp == 0)
		LaTeX_Mode = 'r';
	    }
	}
    else if (TeX_strncmp (*bufp, "verb", 4) == 0)
	{
	skip_ch = (*bufp)[4];
	*bufp += 5;
	while (**bufp != skip_ch  &&  **bufp != '\0')
	    (*bufp)++;
	/* skip to end of verb field when not in a comment or math field */
	if (**bufp == 0 && !TeX_comment  &&  !(math_mode & 1))
	    {
	    LaTeX_Mode = 'v';
	    save_LaTeX_Mode = skip_ch;
	    }
	}
    else
	{
	/* Optional tex arguments sometimes should and
	** sometimes shouldn't be checked
	** (eg \section [C Programming] {foo} vs
	**     \rule [3em] {0.015in} {5em})
	** SO -- we'll always spell-check it rather than make a
	** full LaTeX parser.
	*/

	/* Must look at the space after the command. */
	while (isalpha(**bufp)  ||  **bufp == '@')
	    (*bufp)++;
	/*
	** Our caller expects to skip over a single character.  So we
	** need to back up by one.  Ugh.
	*/
	(*bufp)--;
	}
    }

/*
 * TeX_strncmp is like strncmp, except that it returns inequality if
 * the following character of a is alphabetic.  We do not use
 * iswordch here because TeX itself won't normally accept
 * nonalphabetics (except maybe on ISO Latin-1 installations?  I'll
 * have to look into that).  As a special hack, because LaTeX uses the
 * @ sign so much, we'll also accept that character.
 *
 * Properly speaking, the @ sign should be settable in the hash file
 * header, but I doubt that it varies, and I don't want to change the
 * syntax of affix files right now.
 *
 * Incidentally, TeX_strncmp uses unequal signedness for its arguments
 * because that's how it's always called, and it's easier to do one
 * typecast here than lots of casts in the calls.
 */
static int TeX_strncmp (a, b, n)
    unsigned char *	a;		/* Strings to compare */
    char *		b;		/* ... */
    int			n;		/* Number of characters to compare */
    {
    int			cmpresult;	/* Result of calling strncmp */

    cmpresult = strncmp ((char *) a, b, n);
    if (cmpresult == 0)
	{
	if (isascii (a[n])  &&  isalpha (a[n]))
	    return 1;		/* Force inequality if alpha follows */
	}
    return cmpresult;
    }

/*
 * Set up a table of keywords to be treated specially.  The input is
 * "rawtags", which is a comma-separated list of keywords to be
 * inserted into the table.  If rawtags is null, the environment
 * variable "envvar" is consulted; if this isn't set, the list in
 * "deftags" is used instead.
 *
 * If either rawtags or the contents of envvar begins with a + sign,
 * then this value is appended to the string in deftags, rather than
 * replacing it.  This effect is cumulative.  Thus, there are the
 * following possibilities:
 *
 *  rawtags envvar deftags  result
 *  xxx	    *	    *	    xxx
 *  +xxx    yyy	    *	    xxx,yyy
 *  +xxx    +yyy    zzz	    xxx,yyy,zzz
 *  null    yyy	    *	    yyy
 *  null    +yyy    zzz	    yyy,zzz
 *  null    null    zzz	    zzz
 *
 * The result of this routine is the initialization of a "struct
 * kwtable" that can be passed to keyword_in_list for lookup purposes.
 * Before the first time this routine is called, the "kwlist" pointer
 * in the struct kwtable must be set to NULL, to indicate that the
 * structure is uninitialized.
 *
 * Returns nonzero if the list has already been initialized, zero if
 * all is well.
 */
int init_keyword_table (rawtags, envvar, deftags, ignorecase, keywords)
    char *	rawtags;	/* Comma-separated list of tags to look up */
    char *	envvar;		/* Environment variable containing tag list */
    char *	deftags;	/* Default tag list */
    int		ignorecase;	/* NZ to ignore case in keyword matching */
    struct kwtable *
		keywords;	/* Where to put the keyword table */
    {
    char *	end;		/* End of current tag */
    char *	envtags;	/* Tags from envvar */
    unsigned char **
		nextkw;		/* Next keyword-table entry */
    char *	start;		/* Start of current tag */
    char *	wlist;		/* Modifiable copy of raw list */
    unsigned int wsize;		/* Size of wlist */

    if (keywords->kwlist != NULL)
	return 1;
    envtags = (envvar == NULL) ? NULL : getenv (envvar);
    if (rawtags != NULL  &&  rawtags[0] != '+')
	{
	envtags = NULL;
	deftags = NULL;
	}
    if (envtags != NULL  &&  envtags[0] != '+')
	deftags = NULL;

    /*
     * Allocate space for the modifiable tag list.  This may
     * over-allocate by up to 2 bytes if the "+" notation is used.
     */
    wsize = 0;
    if (rawtags != NULL)
	wsize += strlen (rawtags) + 1;
    if (envtags != NULL)
	wsize += strlen (envtags) + 1;
    if (deftags != NULL)
	wsize += strlen (deftags) + 1;
    wlist = malloc (wsize);
    if (wlist == NULL)
	{
	(void) fprintf (stderr, DEFMT_C_NO_SPACE, MAYBE_CR (stderr));
	exit (1);
	}
    wlist[0] = '\0';
    if (rawtags != NULL)
	{
	if (rawtags[0] == '+')
	    rawtags++;
	strcpy (wlist, rawtags);
	}
    if (envtags != NULL)
	{
	if (envtags[0] == '+')
	    envtags++;
	if (wlist[0] != '\0')
	    strcat (wlist, ",");
	strcat (wlist, envtags);
	}
    if (deftags != NULL)
	{
	if (wlist[0] != '\0')
	    strcat (wlist, ",");
	strcat (wlist, deftags);
	}

    /*
     * Count the keywords and allocate space for the pointers.
     */
    keywords->numkw = 1;
    keywords->forceupper = ignorecase;
    for (end = wlist;  *end != '\0';  ++end)
	{
	if (*end == ','  ||  *end == ':')
	    ++keywords->numkw;
	}
    keywords->kwlist =
      (unsigned char **) malloc (keywords->numkw * sizeof keywords->kwlist[0]);
    if (keywords->kwlist == NULL)
	{
	fprintf (stderr, DEFMT_C_NO_SPACE, MAYBE_CR (stderr));
	exit (1);
	}

    end = wlist;
    nextkw = keywords->kwlist;
    keywords->maxlen = 0;
    keywords->minlen = 0;
    while (nextkw < keywords->kwlist + keywords->numkw)
	{
	for (start = end;
	  *end != '\0'  &&  *end != ','  &&  *end != ':';
	  end++)
	    ;
	*end = '\0';
	if (ignorecase)
	    chupcase ((unsigned char *) start);
	if (end == start)
	    --keywords->numkw;
	else
	    {
	    *nextkw++ = (unsigned char *) start;
	    if ((unsigned) (end - start) > keywords->maxlen)
		keywords->maxlen = end - start;
	    if (keywords->minlen == 0
	      ||  (unsigned) (end - start) < keywords->minlen)
		keywords->minlen = end - start;
	    }
	end++;
	}
    qsort ((char *) keywords->kwlist, keywords->numkw,
      sizeof keywords->kwlist[0],
      (int (*) P ((const void *, const void *))) tagcmp);
  
    if (keywords->maxlen > maxkeywordlen)
	{
	maxkeywordlen = keywords->maxlen;
	if (keywordbuf != NULL)
	    free (keywordbuf);
	keywordbuf = (unsigned char *)
	  malloc ((maxkeywordlen + 1) * sizeof keywordbuf[0]);
	if (keywordbuf == NULL)
	    {
	    fprintf (stderr, DEFMT_C_NO_SPACE, MAYBE_CR (stderr));
	    exit(1);
	    }
	}
    return 0;
    }

/*
 * Decide whether a given keyword is in a list of those that need
 * special treatment.  The list of tags is so small that there's
 * little point in being fancy, but the code given to me used a binary
 * search and it seemed silly to throw it away.
 *
 * Returns nonzero if the keyword is in the chosen list.
 */
static int keyword_in_list (str, strend, keywords)
    unsigned char *
		str;		/* String to be tested (not null-terminated) */
    unsigned char *
		strend;		/* Character following end of test string */
    struct kwtable *
		keywords;	/* Table of keywords to be searched */
    {
    int		cmpresult;	/* Result of string comparison */
    unsigned int i;		/* Current binary-search position */
    int		imin;		/* Bottom of binary search */
    int		imax;		/* Top of binary search */

    i = strend - str;
    if (i < keywords->minlen  ||  i > keywords->maxlen)
	return 0;
    strncpy ((char *) keywordbuf, (char *) str, i);
    keywordbuf[i] = '\0';
    if (keywords->forceupper)
	chupcase (keywordbuf);

    /*
     * Binary search through the tags list
     */
    imin = 0;
    imax = keywords->numkw - 1;
    while (imin <= imax)
	{
	i = (imin + imax) >> 1;
	cmpresult = strcmp ((char *) keywordbuf, (char *) keywords->kwlist[i]);
	if (cmpresult == 0)
	    return 1;
	else if (cmpresult > 0 )
	    imin = i + 1;
	else
	    imax = i - 1;
	}
    return 0;			/* Not a tag to be ignored */
    }

/*
 * Compare two pointed-to strings
 */
static int tagcmp (a, b)
    unsigned char **	a;	    /* Strings to be compared */
    unsigned char **	b;	    /* ... */
    {
    return strcmp ((char *) *a, (char *) *b);
    }
