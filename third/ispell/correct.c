#ifndef lint
static char Rcs_Id[] =
    "$Id: correct.c,v 1.1.1.2 2007-02-01 19:50:23 ghudson Exp $";
#endif

/*
 * correct.c - Routines to manage the higher-level aspects of spell-checking
 *
 * This code originally resided in ispell.c, but was moved here to keep
 * file sizes smaller.
 *
 * Copyright (c), 1983, by Pace Willisson
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
 * Revision 1.82  2005/04/28 14:46:51  geoff
 * Add code to log all corrections and replacements.
 *
 * Revision 1.81  2005/04/20 23:16:32  geoff
 * Add inpossibilities, and use it to deal with the case where uppercase
 * SS in German causes ambiguities.  Rename some variables to make them
 * more meaningful.
 *
 * Revision 1.80  2005/04/14 14:38:23  geoff
 * Update license.  Use i-prefixed names for terminal access functions to
 * prevent conflicts with libraries.  Fix a bug that caused a hang when
 * using external deformatters.
 *
 * Revision 1.79  2001/09/06 00:40:44  geoff
 * In ask mode, when extending long lines to ensure that a word hasn't
 * been broken, remember to do the extension inside both the context and
 * the filtered buffer.
 *
 * Revision 1.78  2001/09/06 00:30:29  geoff
 * Many changes from Eli Zaretskii to support DJGPP compilation.
 *
 * Revision 1.77  2001/07/25 21:51:47  geoff
 * Minor license update.
 *
 * Revision 1.76  2001/07/23 20:24:03  geoff
 * Update the copyright and the license.
 *
 * Revision 1.75  2001/06/14 09:11:11  geoff
 * Use a non-conflicting macro for bcopy to avoid compilation problems on
 * smarter compilers.
 *
 * Revision 1.74  2001/06/10 23:54:56  geoff
 * Fix an ask-mode bug that could cause hangs.
 *
 * Revision 1.73  2001/06/07 08:02:18  geoff
 * When copying out, be sure to use the information from contextbufs[0]
 * (the original file) rather then filteredbuf (the deformatted
 * information).
 *
 * Revision 1.72  2000/08/24 06:47:16  geoff
 * Fix a dumb error in merging the correct_verbose_mode patch
 *
 * Revision 1.71  2000/08/22 10:52:25  geoff
 * Fix a whole bunch of signed/unsigned compiler warnings.
 *
 * Revision 1.70  2000/08/22 00:11:25  geoff
 * Add support for correct_verbose_mode.
 *
 * Revision 1.69  1999/11/04 07:51:05  geoff
 * In verbose ask mode, put out a newline after the EOF so the TTY looks
 * nice.
 *
 * Revision 1.68  1999/11/04 07:19:59  geoff
 * Add "oktochange" to inserttoken, and set it to false on the first of
 * each paired call.
 *
 * Revision 1.67  1999/10/05 05:49:56  geoff
 * When processing the ~ command, don't pass a newline by accident!
 *
 * Revision 1.66  1999/01/18 03:28:24  geoff
 * Turn many char declarations into unsigned char, so that we won't have
 * sign-extension problems.
 *
 * Revision 1.65  1999/01/07  01:57:51  geoff
 * Update the copyright.
 *
 * Revision 1.64  1999/01/03  01:46:25  geoff
 * Add support for external deformatters.  Also display tab characters
 * correctly when showing context.
 *
 * Revision 1.63  1997/12/02  06:24:38  geoff
 * Get rid of some compile options that really shouldn't be optional.
 *
 * Revision 1.62  1997/12/01  00:53:43  geoff
 * Don't strip out the newline at the end of contextbufs on long lines.
 *
 * Revision 1.61  1995/11/08  05:09:26  geoff
 * Add the new interactive mode ("askverbose").  Modify the deformatting
 * support to allow html/sgml as a full equal of nroff and TeX.
 *
 * Revision 1.60  1995/10/25  04:05:28  geoff
 * Line added by Gerry Tierney to reset insidehtml flag for each new file
 * in case a tag was left open by a previous file.  10/14/95.
 *
 * Revision 1.59  1995/08/05  23:19:43  geoff
 * Fix a bug that caused offsets for long lines to be confused if the
 * line started with a quoting uparrow.
 *
 * Revision 1.58  1994/11/02  06:56:00  geoff
 * Remove the anyword feature, which I've decided is a bad idea.
 *
 * Revision 1.57  1994/10/26  05:12:39  geoff
 * Try boundary characters when inserting or substituting letters, except
 * (naturally) at word boundaries.
 *
 * Revision 1.56  1994/10/25  05:46:30  geoff
 * Fix an assignment inside a conditional that could generate spurious
 * warnings (as well as being bad style).  Add support for the FF_ANYWORD
 * option.
 *
 * Revision 1.55  1994/09/16  04:48:24  geoff
 * Don't pass newlines from the input to various other routines, and
 * don't assume that those routines leave the input unchanged.
 *
 * Revision 1.54  1994/09/01  06:06:41  geoff
 * Change erasechar/killchar to uerasechar/ukillchar to avoid
 * shared-library problems on HP systems.
 *
 * Revision 1.53  1994/08/31  05:58:38  geoff
 * Add code to handle extremely long lines in -a mode without splitting
 * words or reporting incorrect offsets.
 *
 * Revision 1.52  1994/05/25  04:29:24  geoff
 * Fix a bug that caused line widths to be calculated incorrectly when
 * displaying lines containing tabs.  Fix a couple of places where
 * characters were sign-extended incorrectly, which could cause 8-bit
 * characters to be displayed wrong.
 *
 * Revision 1.51  1994/05/17  06:44:05  geoff
 * Add support for controlled compound formation and the COMPOUNDONLY
 * option to affix flags.
 *
 * Revision 1.50  1994/04/27  05:20:14  geoff
 * Allow compound words to be formed from more than two components
 *
 * Revision 1.49  1994/04/27  01:50:31  geoff
 * Add support to correctly capitalize words generated as a result of a
 * missing-space suggestion.
 *
 * Revision 1.48  1994/04/03  23:23:02  geoff
 * Clean up the code in missingspace() to be a bit simpler and more
 * efficient.
 *
 * Revision 1.47  1994/03/15  06:24:23  geoff
 * Fix the +/-/~ commands to be independent.  Allow the + command to
 * receive a suffix which is a deformatter type (currently hardwired to
 * be either tex or nroff/troff).
 *
 * Revision 1.46  1994/02/21  00:20:03  geoff
 * Fix some bugs that could cause bad displays in the interaction between
 * TeX parsing and string characters.  Show_char now will not overrun
 * the inverse-video display area by accident.
 *
 * Revision 1.45  1994/02/14  00:34:51  geoff
 * Fix correct to accept length parameters for ctok and itok, so that it
 * can pass them to the to/from ichar routines.
 *
 * Revision 1.44  1994/01/25  07:11:22  geoff
 * Get rid of all old RCS log lines in preparation for the 3.1 release.
 *
 */

#include <ctype.h>
#include "config.h"
#include "ispell.h"
#include "proto.h"
#include "msgs.h"
#include "version.h"

void		givehelp P ((int interactive));
void		checkfile P ((void));
void		correct P ((unsigned char * ctok, int ctokl, ichar_t * itok,
		  int itokl, unsigned char ** curchar));
static void	show_line P ((unsigned char * line, unsigned char * invstart,
		  int invlen));
static int	show_char P ((unsigned char ** cp, int linew, int output,
		  int maxw));
static int	line_size P ((unsigned char * buf, unsigned char * bufend));
static void	inserttoken P ((unsigned char * buf, unsigned char * start,
		  unsigned char * tok, unsigned char ** curchar,
		  int oktochange));
static int	posscmp P ((unsigned char * a, unsigned char * b));
int		casecmp P ((unsigned char * a, unsigned char * b, int canonical));
void		makepossibilities P ((ichar_t * word));
int		inpossibilities P ((unsigned char * ctok));
static int	insert P ((ichar_t * word));
static void	wrongcapital P ((ichar_t * word));
static void	wrongletter P ((ichar_t * word));
static void	extraletter P ((ichar_t * word));
static void	missingletter P ((ichar_t * word));
static void	missingspace P ((ichar_t * word));
int		compoundgood P ((ichar_t * word, int pfxopts));
static void	transposedletter P ((ichar_t * word));
static void	tryveryhard P ((ichar_t * word));
static int	ins_cap P ((ichar_t * word, ichar_t * pattern));
static int	save_cap P ((ichar_t * word, ichar_t * pattern,
		  ichar_t savearea[MAX_CAPS][INPUTWORDLEN + MAXAFFIXLEN]));
int		ins_root_cap P ((ichar_t * word, ichar_t * pattern,
		  int prestrip, int preadd, int sufstrip, int sufadd,
		  struct dent * firstdent, struct flagent * pfxent,
		  struct flagent * sufent));
static void	save_root_cap P ((ichar_t * word, ichar_t * pattern,
		  int prestrip, int preadd, int sufstrip, int sufadd,
		  struct dent * firstdent, struct flagent * pfxent,
		  struct flagent * sufent,
		  ichar_t savearea[MAX_CAPS][INPUTWORDLEN + MAXAFFIXLEN],
		  int * nsaved));
static char *	getline P ((char * buf, int bufsize));
void		askmode P ((void));
void		copyout P ((unsigned char ** cc, int cnt));
static void	lookharder P ((unsigned char * string));
#ifdef REGEX_LOOKUP
static void	regex_dict_lookup P ((char * cmd, char * grepstr));
#endif /* REGEX_LOOKUP */

void givehelp (interactive)
    int		    interactive;	/* NZ for interactive-mode help */
    {
#ifdef COMMANDFORSPACE
    char ch;
#endif
    register FILE *helpout;	/* File to write help to */

    if (interactive)
	{
	ierase ();
	helpout = stdout;
	}
    else
	helpout = stderr;

    (void) fprintf (helpout, CORR_C_HELP_1, MAYBE_CR (helpout));
    (void) fprintf (helpout, CORR_C_HELP_2, MAYBE_CR (helpout));
    (void) fprintf (helpout, CORR_C_HELP_3, MAYBE_CR (helpout));
    (void) fprintf (helpout, CORR_C_HELP_4, MAYBE_CR (helpout));
    (void) fprintf (helpout, CORR_C_HELP_5, MAYBE_CR (helpout));
    (void) fprintf (helpout, CORR_C_HELP_6, MAYBE_CR (helpout));
    (void) fprintf (helpout, CORR_C_HELP_7, MAYBE_CR (helpout));
    (void) fprintf (helpout, CORR_C_HELP_8, MAYBE_CR (helpout));
    (void) fprintf (helpout, CORR_C_HELP_9, MAYBE_CR (helpout));

    (void) fprintf (helpout, CORR_C_HELP_COMMANDS, MAYBE_CR (helpout),
      MAYBE_CR (helpout), MAYBE_CR (helpout));

    (void) fprintf (helpout, CORR_C_HELP_R_CMD, MAYBE_CR (helpout));
    (void) fprintf (helpout, CORR_C_HELP_BLANK, MAYBE_CR (helpout));
    (void) fprintf (helpout, CORR_C_HELP_A_CMD, MAYBE_CR (helpout));
    (void) fprintf (helpout, CORR_C_HELP_I_CMD, MAYBE_CR (helpout));
    (void) fprintf (helpout, CORR_C_HELP_U_CMD, MAYBE_CR (helpout));
    (void) fprintf (helpout, CORR_C_HELP_0_CMD, MAYBE_CR (helpout));
    (void) fprintf (helpout, CORR_C_HELP_L_CMD, MAYBE_CR (helpout));
    (void) fprintf (helpout, CORR_C_HELP_X_CMD, MAYBE_CR (helpout),
      MAYBE_CR (helpout));
    (void) fprintf (helpout, CORR_C_HELP_Q_CMD, MAYBE_CR (helpout),
      MAYBE_CR (helpout));
    (void) fprintf (helpout, CORR_C_HELP_BANG, MAYBE_CR (helpout));
    (void) fprintf (helpout, CORR_C_HELP_REDRAW, MAYBE_CR (helpout));
    (void) fprintf (helpout, CORR_C_HELP_SUSPEND, MAYBE_CR (helpout));
    (void) fprintf (helpout, CORR_C_HELP_HELP, MAYBE_CR (helpout));

    if (interactive)
	{
	(void) fprintf (helpout, "\r\n");
	imove (li - 1, 0);  /* bottom line, no matter what screen size is */
	(void) fprintf (helpout, CORR_C_HELP_TYPE_SPACE);
	(void) fflush (helpout);
#ifdef COMMANDFORSPACE
	ch = GETKEYSTROKE ();
	if (ch != ' ' && ch != '\n' && ch != '\r')
	    (void) ungetc (ch, stdin);
#else
	while (GETKEYSTROKE () != ' ')
	    ;
#endif
	}
    }

void checkfile ()
    {
    int		bufno;
    unsigned int bufsize;
    int		ch;

    insidehtml = 0;
    math_mode = 0;
    LaTeX_Mode = 'P';

    for (bufno = 0;  bufno < contextsize;  bufno++)
	contextbufs[bufno][0] = '\0';

    for (  ;  ;  )
	{
	for (bufno = contextsize;  --bufno > 0;  )
	    (void) strcpy ((char *) contextbufs[bufno],
	      (char *) contextbufs[bufno - 1]);
	if (quit)	/* quit can't be set in l mode */
	    {
	    if (sourcefile == NULL)
		sourcefile = infile;
	    while (fgets ((char *) contextbufs[0], sizeof contextbufs[0],
		sourcefile)
	      != NULL)
		(void) fputs ((char *) contextbufs[0], outfile);
	    break;
	    }
	/*
	 * Only read in enough characters to fill half this buffer so that any
	 * corrections we make are not likely to cause an overflow.
	 */
	if (fgets ((char *) filteredbuf, sizeof filteredbuf / 2, infile)
	  == NULL)
	    {
	    if (sourcefile != NULL)
		{
		while (fgets ((char *) contextbufs[0], sizeof contextbufs[0],
		    sourcefile)
		  != NULL)
		    (void) fputs ((char *) contextbufs[0], outfile);
		}
	    break;
	    }
	/*
	 * If we didn't read to end-of-line, we may have ended the
	 * buffer in the middle of a word.  So keep reading until we
	 * see some sort of character that can't possibly be part of a
	 * word. (or until the buffer is full, which fortunately isn't
	 * all that likely).
	 */
	bufsize = strlen ((char *) filteredbuf);
	if (bufsize == sizeof filteredbuf / 2 - 1)
	    {
	    ch = (unsigned char) filteredbuf[bufsize - 1];
	    while (bufsize < sizeof filteredbuf - 1
	      &&  (iswordch ((ichar_t) ch)  ||  isboundarych ((ichar_t) ch)
	      ||  isstringstart (ch)))
		{
		ch = getc (infile);
		if (ch == EOF)
		    break;
		filteredbuf[bufsize++] = (char) ch;
		filteredbuf[bufsize] = '\0';
		}
	    }
	/*
	 * If we're not filtering, make a duplicate of filteredbuf in
	 * contextbufs[0].  Otherwise, read the same number of
	 * characters into contextbufs[0] from sourcefile.
	 */
	if (sourcefile == NULL)
	    (void) strcpy ((char *) contextbufs[0], (char *) filteredbuf);
	else
	    {
	    if (fread (contextbufs[0], 1, bufsize, sourcefile) != bufsize)
		{
		(void) fprintf (stderr, CORR_C_SHORT_SOURCE,
		  MAYBE_CR (stderr));
		(void) sleep ((unsigned) 2);
		xflag = 0;		/* Preserve file backup */
		break;
		}
	    contextbufs[0][bufsize] = '\0';
	    }
	checkline (outfile);
	}
    }

void correct (ctok, ctokl, itok, itokl, curchar)
    unsigned char *	ctok;
    int			ctokl;
    ichar_t *		itok;
    int			itokl;
    unsigned char **	curchar;    /* Pointer into filteredbuf */
    {
    register int	c;
    register int	i;
    int			col_ht;
    unsigned char *	curcontextchar; /* Pointer into contextbufs[0] */
    int			ncols;
    unsigned char *	start_l2;
    unsigned char *	begintoken;

    curcontextchar = contextbufs[0] + (*curchar - filteredbuf);
    begintoken = curcontextchar - strlen ((char *) ctok);

    if (icharlen (itok) <= minword)
	return;			/* Accept very short words */

checkagain:
    if (good (itok, 0, 0, 0, 0)  ||  compoundgood (itok, 0))
	return;

    makepossibilities (itok);
    if (inpossibilities (ctok))	/* Kludge for German and similar languages */
	return;

    ierase ();
    (void) printf ("    %s", (char *) ctok);
    if (currentfile)
	(void) printf (CORR_C_FILE_LABEL, currentfile);
    if (readonly)
	(void) printf (" %s", CORR_C_READONLY);
    (void) printf ("\r\n\r\n");

    /*
     * Make sure we have enough room on the screen to hold the
     * possibilities.  Reduce the list if necessary.  co / (maxposslen + 8)
     * is the maximum number of columns that will fit.  col_ht is the
     * height of the columns.  The constant 4 allows 2 lines (1 blank) at
     * the top of the screen, plus another blank line between the
     * columns and the context, plus a final blank line at the bottom
     * of the screen for command entry (R, L, etc).
     */
    col_ht = li - contextsize - 4 - minimenusize;
    ncols = co / (maxposslen + 8);
    if (pcount > ncols * col_ht)
	pcount = ncols * col_ht;

#ifdef EQUAL_COLUMNS
    /*
     * Equalize the column sizes.  The last column will be short.
     */
    col_ht = (pcount + ncols - 1) / ncols;
#endif

    for (i = 0; i < pcount; i++)
	{
#ifdef BOTTOMCONTEXT
	imove (2 + (i % col_ht), (maxposslen + 8) * (i / col_ht));
#else /* BOTTOMCONTEXT */
	imove (3 + contextsize + (i % col_ht), (maxposslen + 8) * (i / col_ht));
#endif /* BOTTOMCONTEXT */
	if (i >= easypossibilities)
	    (void) printf ("??: %s", possibilities[i]);
	else if (easypossibilities >= 10  &&  i < 10)
	    (void) printf ("0%d: %s", i, possibilities[i]);
	else
	    (void) printf ("%2d: %s", i, possibilities[i]);
	}

#ifdef BOTTOMCONTEXT
    imove (li - contextsize - 1 - minimenusize, 0);
#else /* BOTTOMCONTEXT */
    imove (2, 0);
#endif /* BOTTOMCONTEXT */
    for (i = contextsize;  --i > 0;  )
	show_line (contextbufs[i], contextbufs[i], 0);

    start_l2 = contextbufs[0];
    if (line_size (contextbufs[0], curcontextchar) > co - (sg << 1) - 1)
	{
	start_l2 = begintoken - (co / 2);
	while (start_l2 < begintoken)
	    {
	    i = line_size (start_l2, curcontextchar) + 1;
	    if (i + (sg << 1) <= co)
		break;
	    start_l2 += i - co;
	    }
	if (start_l2 > begintoken)
	    start_l2 = begintoken;
	if (start_l2 < contextbufs[0])
	    start_l2 = contextbufs[0];
	}
    show_line (start_l2, begintoken, (int) strlen ((char *) ctok));

    if (minimenusize != 0)
	{
	imove (li - 2, 0);
	(void) printf (CORR_C_MINI_MENU);
	}

    for (  ;  ;  )
	{
	(void) fflush (stdout);
	switch (c = GETKEYSTROKE ())
	    {
	    case 'Z' & 037:
		stop ();
		ierase ();
		goto checkagain;
	    case ' ':
		ierase ();
		(void) fflush (stdout);
		return;
	    case 'q': case 'Q':
		if (changes)
		    {
		    (void) printf (CORR_C_CONFIRM_QUIT);
		    (void) fflush (stdout);
		    c = GETKEYSTROKE ();
		    }
		else
		    c = 'y';
		if (c == 'y' || c == 'Y')
		    {
		    ierase ();
		    (void) fflush (stdout);
		    fclose (outfile);	/* So `done' may unlink it safely */
		    done (0);
		    }
		goto checkagain;
	    case 'i': case 'I':
		treeinsert (ichartosstr (strtosichar (ctok, 0), 1),
		 ICHARTOSSTR_SIZE, 1);
		ierase ();
		(void) fflush (stdout);
		changes = 1;
		return;
	    case 'u': case 'U':
		itok = strtosichar (ctok, 0);
		lowcase (itok);
		treeinsert (ichartosstr (itok, 1), ICHARTOSSTR_SIZE, 1);
		ierase ();
		(void) fflush (stdout);
		changes = 1;
		return;
	    case 'a': case 'A':
		treeinsert (ichartosstr (strtosichar (ctok, 0), 1),
		  ICHARTOSSTR_SIZE, 0);
		ierase ();
		(void) fflush (stdout);
		return;
	    case 'L' & 037:
		goto checkagain;
	    case '?':
		givehelp (1);
		goto checkagain;
	    case '!':
		{
		unsigned char	buf[200];

		imove (li - 1, 0);
		(void) putchar ('!');
		if (getline ((char *) buf, sizeof buf) == NULL)
		    {
		    (void) putchar (7);
		    ierase ();
		    (void) fflush (stdout);
		    goto checkagain;
		    }
		(void) printf ("\r\n");
		(void) fflush (stdout);
#ifdef	USESH
		shescape ((char *) buf);
#else
		(void) shellescape ((char *) buf);
#endif
		ierase ();
		goto checkagain;
		}
	    case 'r': case 'R':
		imove (li - 1, 0);
		if (readonly)
		    {
		    (void) putchar (7);
		    (void) printf ("%s ", CORR_C_READONLY);
		    }
		(void) printf (CORR_C_REPLACE_WITH);
		if (getline ((char *) ctok, ctokl) == NULL)
		    {
		    (void) putchar (7);
		    /* Put it back */
		    (void) ichartostr (ctok, itok, ctokl, 0);
		    }
		else
		    {
		    inserttoken (contextbufs[0],
		      begintoken, ctok, &curcontextchar, 0);
		    inserttoken (filteredbuf,
		      filteredbuf + (begintoken - contextbufs[0]),
		      ctok, curchar, 1);
		    if (strtoichar (itok, ctok, itokl, 0))
			{
			(void) putchar (7);
			(void) printf (WORD_TOO_LONG ((char *) ctok));
			}
		    changes = 1;
		    }
		ierase ();
		if (icharlen (itok) <= minword)
		    return;		/* Accept very short replacements */
		goto checkagain;
	    case '0': case '1': case '2': case '3': case '4':
	    case '5': case '6': case '7': case '8': case '9':
		i = c - '0';
		if (easypossibilities >= 10)
		    {
		    c = GETKEYSTROKE ();
		    if (c >= '0'  &&  c <= '9')
			i = i * 10 + c - '0';
		    else if (c != '\r'  &&  c != '\n')
			{
			(void) putchar (7);
			break;
			}
		    }
		if (i < easypossibilities)
		    {
		    (void) strcpy ((char *) ctok, possibilities[i]);
		    changes = 1;
		    inserttoken (contextbufs[0],
			begintoken, ctok, &curcontextchar, 0);
		    inserttoken (filteredbuf,
		      filteredbuf + (begintoken - contextbufs[0]),
		      ctok, curchar, 1);
		    ierase ();
		    if (readonly)
			{
			imove (li - 1, 0);
			(void) putchar (7);
			(void) printf ("%s", CORR_C_READONLY);
			(void) fflush (stdout);
			(void) sleep ((unsigned) 2);
			}
		    return;
		    }
		(void) putchar (7);
		break;
	    case '\r':	/* This makes typing \n after single digits */
	    case '\n':	/* ..less obnoxious */
		break;
	    case 'l': case 'L':
		{
		unsigned char	buf[100];
		imove (li - 1, 0);
		(void) printf (CORR_C_LOOKUP_PROMPT);
		if (getline ((char *) buf, sizeof buf) == NULL)
		    {
		    (void) putchar (7);
		    ierase ();
		    goto checkagain;
		    }
		(void) printf ("\r\n");
		(void) fflush (stdout);
		lookharder (buf);
		ierase ();
		goto checkagain;
		}
	    case 'x': case 'X':
		quit = 1;
		ierase ();
		(void) fflush (stdout);
		return;
	    default:
		(void) putchar (7);
		break;
	    }
	}
    }

static void show_line (line, invstart, invlen)
    unsigned char *	line;
    register unsigned char *
			invstart;
    register int	invlen;
    {
    register int	width = 0;
    int			maxwidth = co - 1;

    if (invlen != 0)
	maxwidth -= sg << 1;
    while (line < invstart  &&  width < maxwidth)
	width += show_char (&line, width, 1, invstart - line);
    if (invlen)
	{
	inverse ();
	invstart += invlen;
	while (line < invstart  &&  width < maxwidth)
	    width += show_char (&line, width, 1, invstart - line);
	normal ();
	}
    while (*line  &&  width < maxwidth)
	width += show_char (&line, width, 1, 0);
    (void) printf ("\r\n");
    }

static int show_char (cp, linew, output, maxw)
    register unsigned char **
			cp;
    int			linew;
    int			output;		/* NZ to actually do output */
    int			maxw;		/* NZ to limit width shown */
    {
    register int	ch;
    register int	i;
    int			len;
    ichar_t		ichar;
    register int	width;

    ch = **cp;
    if (l1_isstringch (*cp, len, 0))
	ichar = SET_SIZE + laststringch;
    else
	ichar = chartoichar (ch);
    if (!vflag  &&  iswordch (ichar)  &&  len == 1)
	{
	if (output)
	    (void) putchar (ch);
	(*cp)++;
	return 1;
	}
    if (ch == '\t')
	{
	if (output)
	    {
	    for (i = 8 - (linew & 0x07);  --i >= 0;  )
		(void) putchar (' ');
	    }
	(*cp)++;
	return 8 - (linew & 0x07);
	}
    /*
     * Character is non-printing, or it's ISO and vflag is set.  Display
     * it in "cat -v" form.  For string characters, display every element
     * separately in that form.
     */
    width = 0;
    if (maxw != 0  &&  len > maxw)
	len = maxw;			/* Don't show too much */
    for (i = 0;  i < len;  i++)
	{
	ch = (unsigned char) *(*cp)++;
	if (ch > '\177')
	    {
	    if (output)
		{
		(void) putchar ('M');
		(void) putchar ('-');
		}
	    width += 2;
	    ch &= 0x7f;
	    }
	if (ch < ' '  ||  ch == '\177')
	    {
	    if (output)
		{
		(void) putchar ('^');
		if (ch == '\177')
		    (void) putchar ('?');
		else
		    (void) putchar (ch + 'A' - '\001');
		}
	    width += 2;
	    }
	else
	    {
	    if (output)
		(void) putchar (ch);
	    width += 1;
	    }
	}
    return width;
    }

static int line_size (buf, bufend)
    unsigned char *	buf;
    register unsigned char *
			bufend;
    {
    register int	width;

    for (width = 0;  buf < bufend  &&  *buf != '\0';  )
	width += show_char (&buf, width, 0, bufend - buf);
    return width;
    }

static void inserttoken (buf, start, tok, curchar, oktochange)
    unsigned char *	buf;
    unsigned char *	start; 
    register unsigned char *
			tok;
    unsigned char **	curchar;	/* Where to do insertion (updated) */
    int			oktochange;	/* NZ if OK to modify tok */
    {
    unsigned char	copy[BUFSIZ];
    register unsigned char *
			p;
    register unsigned char *
			q;
    unsigned char *	ew;

    if (!oktochange  &&  logfile != NULL)
	{
	for (p = start;  p != *curchar;  p++)
	    (void) putc (*p, logfile);
	(void) putc (' ', logfile);
	(void) fputs (tok, logfile);
	(void) putc ('\n', logfile);
	(void) fflush (logfile);
	}

    (void) strcpy ((char *) copy, (char *) buf);

    p = start;
    q = copy + (*curchar - buf);
    ew = skipoverword (tok);
    while (tok < ew)
	*p++ = *tok++;
    *curchar = p;
    if (*tok)
	{

	/*
	** The token changed to two words.  Split it up and save the
	** second one for later.
	*/

	*p++ = *tok;
	if (oktochange)
	    *tok = '\0';
	tok++;
	while (*tok)
	    *p++ = *tok++;
	}
    while ((*p++ = *q++) != '\0')
	;
    }

static int posscmp (a, b)
    unsigned char *	a;
    unsigned char *	b;
    {

    return casecmp (a, b, 0);
    }

int casecmp (a, b, canonical)
    unsigned char *	a;
    unsigned char *	b;
    int			canonical;	/* NZ for canonical string chars */
    {
    register ichar_t *	ap;
    register ichar_t *	bp;
    ichar_t		inta[INPUTWORDLEN + 4 * MAXAFFIXLEN + 4];
    ichar_t		intb[INPUTWORDLEN + 4 * MAXAFFIXLEN + 4];

    (void) strtoichar (inta, a, sizeof inta, canonical);
    (void) strtoichar (intb, b, sizeof intb, canonical);
    for (ap = inta, bp = intb;  *ap != 0;  ap++, bp++)
	{
	if (*ap != *bp)
	    {
	    if (*bp == '\0')
		return hashheader.sortorder[*ap];
	    else if (mylower (*ap))
		{
		if (mylower (*bp)  ||  mytoupper (*ap) != *bp)
		    return (int) hashheader.sortorder[*ap]
		      - (int) hashheader.sortorder[*bp];
		}
	    else
		{
		if (myupper (*bp)  ||  mytolower (*ap) != *bp)
		    return (int) hashheader.sortorder[*ap]
		      - (int) hashheader.sortorder[*bp];
		}
	    }
	}
    if (*bp != '\0')
	return -(int) hashheader.sortorder[*bp];
    for (ap = inta, bp = intb;  *ap;  ap++, bp++)
	{
	if (*ap != *bp)
	    {
	    return (int) hashheader.sortorder[*ap]
	      - (int) hashheader.sortorder[*bp];
	    }
	}
    return 0;
    }

void makepossibilities (word)
    register ichar_t *	word;
    {
    register int	i;

    for (i = 0; i < MAXPOSSIBLE; i++)
	possibilities[i][0] = 0;
    pcount = 0;
    maxposslen = 0;
    easypossibilities = 0;

    wrongcapital (word);

/* 
 * according to Pollock and Zamora, CACM April 1984 (V. 27, No. 4),
 * page 363, the correct order for this is:
 * OMISSION = TRANSPOSITION > INSERTION > SUBSTITUTION
 * thus, it was exactly backwards in the old version. -- PWP
 */

    if (pcount < MAXPOSSIBLE)
	missingletter (word);		/* omission */
    if (pcount < MAXPOSSIBLE)
	transposedletter (word);	/* transposition */
    if (pcount < MAXPOSSIBLE)
	extraletter (word);		/* insertion */
    if (pcount < MAXPOSSIBLE)
	wrongletter (word);		/* substitution */

    if ((compoundflag != COMPOUND_ANYTIME)  &&  pcount < MAXPOSSIBLE)
	missingspace (word);	/* two words */

    easypossibilities = pcount;
    if (easypossibilities == 0  ||  tryhardflag)
	tryveryhard (word);

    if ((sortit  ||  (pcount > easypossibilities))  &&  pcount)
	{
	if (easypossibilities > 0  &&  sortit)
	    qsort ((char *) possibilities,
	      (unsigned) easypossibilities,
	      sizeof (possibilities[0]),
	      (int (*) P ((const void *, const void *))) posscmp);
	if (pcount > easypossibilities)
	    qsort ((char *) &possibilities[easypossibilities][0],
	      (unsigned) (pcount - easypossibilities),
	      sizeof (possibilities[0]),
	      (int (*) P ((const void *, const void *))) posscmp);
	}
    }

int inpossibilities (ctok)
    register unsigned char *	ctok;
    {
    register int		i;

    /*
     * This function is a horrible kludge, necessitated by a problem
     * that shows up in German (and possibly other languages).  The
     * problem in German is that the character "ess-zed" (which I'm
     * going to write as "|3" in these comments so I don't have to use
     * the ISO Latin-1 character set) doesn't have an uppercase
     * equivalent.  Instead, words that contain |3 are uppercased by
     * converting the character to SS.
     *
     * For ispell, this presents a problem.  Consider the two German
     * words "gro|3" (large) and "nass" (wet).  In lowercase, they are
     * represented differently both externally and internally.
     * Internally, the "ss" gets converted to a single ichar_t (for a
     * couple of good reasons I won't go into right now).  Internally,
     * there is also an uppercase representation for |3.  So when we
     * do things like dictionary lookups (which are in uppercase for
     * historical reasons) we can distinguish the correct "gro|3" and
     * "nass" from the incorrect "gross" and "na|3", even when they're
     * converted to uppercase.
     *
     * The problem arises when the user writes the words in uppercase:
     * "GROSS" and "NASS".  When parsing the character strings, ispell
     * would like to convert the "SS" sequence into the correct
     * ichar_t for the word.  However, the ichar_t that is needed for
     * the two S's in "GROSS" (uppercase version of |3) is different
     * from the ichar_t needed for the two S's in "NASS" (SS).  There
     * is no solution to the problem that can be based purely on
     * lexical information; the character representation simply
     * doesn't have the information needed.
     *
     * What ispell really needs is to base the chose of ichar_t
     * representation on the correct spelling of the word.  For
     * "GROSS" it should pick uppercase |3 because "gro|3" is in the
     * dictionary, and for "NASS" it should pick SS for the same
     * reason.
     *
     * That brings us to this function.  Lexically, ispell will always
     * choose one or the other of the internal representations.  (The
     * choice is fairly unpredictable; it depends on which one the
     * binary search happens to hit upon first.)  If good() says that
     * the spelling is OK, we must have chosen the right
     * representation (and we'll never get into this function.  But if
     * good() fails, makepossibilities() will wind up substituting the
     * correct character for the incorrect one in wrongletter().  Then
     * the reconversion to external string format will wind up
     * generating exactly the word that was originally spell-checked.
     *
     * So this function searches the possibilities list to see if the
     * requested word is found in the list.  If so, the caller will
     * accept the word just as if good() had succeeded.
     *
     * This is a pretty ugly solution.  It's also only partial.  If a
     * word contains TWO ambiguous characters, it won't find a
     * solution because wrongletter() can only correct a single error.
     * Fortunately, at least in German, that's not a huge problem.
     * The most popular German dictionary contains only three words
     * that have more than one ess-zed: Ku|3tengewa|3ser,
     * Vergro|3Serungsgla|3er, and gro|3Senordnungsma|3Sig/A.
     */
    for (i = 0;  i < pcount;  i++)
	{
	if (strcmp ((char *) ctok, possibilities[i]) == 0)
	    return 1;
	}
    return 0;
    }

static int insert (word)
    register ichar_t *	word;
    {
    register int	i;
    register unsigned char *
			realword;

    realword = ichartosstr (word, 0);
    for (i = 0; i < pcount; i++)
	{
	if (strcmp (possibilities[i], (char *) realword) == 0)
	    return (0);
	}

    (void) strcpy (possibilities[pcount++], (char *) realword);
    i = strlen ((char *) realword);
    if (i > maxposslen)
	maxposslen = i;
    if (pcount >= MAXPOSSIBLE)
	return (-1);
    else
	return (0);
    }

static void wrongcapital (word)
    register ichar_t *	word;
    {
    ichar_t		newword[INPUTWORDLEN + MAXAFFIXLEN];

    /*
    ** When the third parameter to "good" is nonzero, it ignores
    ** case.  If the word matches this way, "ins_cap" will recapitalize
    ** it correctly.
    */
    if (good (word, 0, 1, 0, 0))
	{
	(void) icharcpy (newword, word);
	upcase (newword);
	(void) ins_cap (newword, word);
	}
    }

static void wrongletter (word)
    register ichar_t *	word;
    {
    register int	i;
    register int	j;
    register int	n;
    ichar_t		savechar;
    ichar_t		newword[INPUTWORDLEN + MAXAFFIXLEN];

    n = icharlen (word);
    (void) icharcpy (newword, word);
    upcase (newword);

    for (i = 0; i < n; i++)
	{
	savechar = newword[i];
	for (j=0; j < Trynum; ++j)
	    {
	    if (Try[j] == savechar)
		continue;
	    else if (isboundarych (Try[j])  &&  (i == 0  ||  i == n - 1))
		continue;
	    newword[i] = Try[j];
	    if (good (newword, 0, 1, 0, 0))
		{
		if (ins_cap (newword, word) < 0)
		    return;
		}
	    }
	newword[i] = savechar;
	}
    }

static void extraletter (word)
    register ichar_t *	word;
    {
    ichar_t		newword[INPUTWORDLEN + MAXAFFIXLEN];
    register ichar_t *	p;
    register ichar_t *	r;

    if (icharlen (word) < 2)
	return;

    (void) icharcpy (newword, word + 1);
    for (p = word, r = newword;  *p != 0;  )
	{
	if (good (newword, 0, 1, 0, 0))
	    {
	    if (ins_cap (newword, word) < 0)
		return;
	    }
	*r++ = *p++;
	}
    }

static void missingletter (word)
    ichar_t *		word;
    {
    ichar_t		newword[INPUTWORDLEN + MAXAFFIXLEN + 1];
    register ichar_t *	p;
    register ichar_t *	r;
    register int	i;

    (void) icharcpy (newword + 1, word);
    for (p = word, r = newword;  *p != 0;  )
	{
	for (i = 0;  i < Trynum;  i++)
	    {
	    if (isboundarych (Try[i])  &&  r == newword)
		continue;
	    *r = Try[i];
	    if (good (newword, 0, 1, 0, 0))
		{
		if (ins_cap (newword, word) < 0)
		    return;
		}
	    }
	*r++ = *p++;
	}
    for (i = 0;  i < Trynum;  i++)
	{
	if (isboundarych (Try[i]))
	    continue;
	*r = Try[i];
	if (good (newword, 0, 1, 0, 0))
	    {
	    if (ins_cap (newword, word) < 0)
		return;
	    }
	}
    }

static void missingspace (word)
    ichar_t *		word;
    {
    ichar_t		firsthalf[MAX_CAPS][INPUTWORDLEN + MAXAFFIXLEN];
    int			firstno;	/* Index into first */
    ichar_t *		firstp;		/* Ptr into current firsthalf word */
    ichar_t		newword[INPUTWORDLEN + MAXAFFIXLEN + 1];
    int			nfirsthalf;	/* No. words saved in 1st half */
    int			nsecondhalf;	/* No. words saved in 2nd half */
    register ichar_t *	p;
    ichar_t		secondhalf[MAX_CAPS][INPUTWORDLEN + MAXAFFIXLEN];
    int			secondno;	/* Index into second */

    /*
    ** We don't do words of length less than 3;  this keeps us from
    ** splitting all two-letter words into two single letters.  We
    ** also don't do maximum-length words, since adding the space
    ** would exceed the size of the "possibilities" array.
    */
    nfirsthalf = icharlen (word);
    if (nfirsthalf < 3  ||  nfirsthalf >= INPUTWORDLEN + MAXAFFIXLEN - 1)
	return;
    (void) icharcpy (newword + 1, word);
    for (p = newword + 1;  p[1] != '\0';  p++)
	{
	p[-1] = *p;
	*p = '\0';
	if (good (newword, 0, 1, 0, 0))
	    {
	    /*
	     * Save_cap must be called before good() is called on the
	     * second half, because it uses state left around by
	     * good().  This is unfortunate because it wastes a bit of
	     * time, but I don't think it's a significant performance
	     * problem.
	     */
	    nfirsthalf = save_cap (newword, word, firsthalf);
	    if (good (p + 1, 0, 1, 0, 0))
		{
		nsecondhalf = save_cap (p + 1, p + 1, secondhalf);
		for (firstno = 0;  firstno < nfirsthalf;  firstno++)
		    {
		    firstp = &firsthalf[firstno][p - newword];
		    for (secondno = 0;  secondno < nsecondhalf;  secondno++)
			{
			*firstp = ' ';
			(void) icharcpy (firstp + 1, secondhalf[secondno]);
			if (insert (firsthalf[firstno]) < 0)
			    return;
			*firstp = '-';
			if (insert (firsthalf[firstno]) < 0)
			    return;
			}
		    }
		}
	    }
	}
    }

int compoundgood (word, pfxopts)
    ichar_t *		word;
    int			pfxopts;	/* Options to apply to prefixes */
    {
    ichar_t		newword[INPUTWORDLEN + MAXAFFIXLEN];
    register ichar_t *	p;
    register ichar_t	savech;
    long		secondcap;	/* Capitalization of 2nd half */

    /*
    ** If compoundflag is COMPOUND_NEVER, compound words are never ok.
    */
    if (compoundflag == COMPOUND_NEVER)
	return 0;
    /*
    ** Test for a possible compound word (for languages like German that
    ** form lots of compounds).
    **
    ** This is similar to missingspace, except we quit on the first hit,
    ** and we won't allow either member of the compound to be a single
    ** letter.
    **
    ** We don't do words of length less than 2 * compoundmin, since
    ** both halves must at least compoundmin letters.
    */
    if (icharlen (word) < 2 * hashheader.compoundmin)
	return 0;
    (void) icharcpy (newword, word);
    p = newword + hashheader.compoundmin;
    for (  ;  p[hashheader.compoundmin - 1] != 0;  p++)
	{
	savech = *p;
	*p = 0;
	if (good (newword, 0, 0, pfxopts, FF_COMPOUNDONLY))
	    {
	    *p = savech;
	    if (good (p, 0, 1, FF_COMPOUNDONLY, 0)
	      ||  compoundgood (p, FF_COMPOUNDONLY))
		{
		secondcap = whatcap (p);
		switch (whatcap (newword))
		    {
		    case ANYCASE:
		    case CAPITALIZED:
		    case FOLLOWCASE:	/* Followcase can have l.c. suffix */
			return secondcap == ANYCASE;
		    case ALLCAPS:
			return secondcap == ALLCAPS;
		    }
		}
	    }
	else
	    *p = savech;
	}
    return 0;
    }

static void transposedletter (word)
    register ichar_t *	word;
    {
    ichar_t		newword[INPUTWORDLEN + MAXAFFIXLEN];
    register ichar_t *	p;
    register ichar_t	temp;

    (void) icharcpy (newword, word);
    for (p = newword;  p[1] != 0;  p++)
	{
	temp = *p;
	*p = p[1];
	p[1] = temp;
	if (good (newword, 0, 1, 0, 0))
	    {
	    if (ins_cap (newword, word) < 0)
		return;
	    }
	temp = *p;
	*p = p[1];
	p[1] = temp;
	}
    }

static void tryveryhard (word)
    ichar_t *		word;
    {
    (void) good (word, 1, 0, 0, 0);
    }

/* Insert one or more correctly capitalized versions of word */
static int ins_cap (word, pattern)
    ichar_t *		word;
    ichar_t *		pattern;
    {
    int			i;		/* Index into savearea */
    int			nsaved;		/* No. of words saved */
    ichar_t		savearea[MAX_CAPS][INPUTWORDLEN + MAXAFFIXLEN];

    nsaved = save_cap (word, pattern, savearea);
    for (i = 0;  i < nsaved;  i++)
	{
	if (insert (savearea[i]) < 0)
	    return -1;
	}
    return 0;
    }

/* Save one or more correctly capitalized versions of word */
static int save_cap (word, pattern, savearea)
    ichar_t *		word;		/* Word to save */
    ichar_t *		pattern;	/* Prototype capitalization pattern */
    ichar_t		savearea[MAX_CAPS][INPUTWORDLEN + MAXAFFIXLEN];
					/* Room to save words */
    {
    int			hitno;		/* Index into hits array */
    int			nsaved;		/* Number of words saved */
    int			preadd;		/* No. chars added to front of root */
    int			prestrip;	/* No. chars stripped from front */
    int			sufadd;		/* No. chars added to back of root */
    int			sufstrip;	/* No. chars stripped from back */

    if (*word == 0)
	return 0;

    for (hitno = numhits, nsaved = 0;  --hitno >= 0  &&  nsaved < MAX_CAPS;  )
	{
	if (hits[hitno].prefix)
	    {
	    prestrip = hits[hitno].prefix->stripl;
	    preadd = hits[hitno].prefix->affl;
	    }
	else
	    prestrip = preadd = 0;
	if (hits[hitno].suffix)
	    {
	    sufstrip = hits[hitno].suffix->stripl;
	    sufadd = hits[hitno].suffix->affl;
	    }
	else
	    sufadd = sufstrip = 0;
	save_root_cap (word, pattern, prestrip, preadd,
	    sufstrip, sufadd,
	    hits[hitno].dictent, hits[hitno].prefix, hits[hitno].suffix,
	    savearea, &nsaved);
	}
    return nsaved;
    }

int ins_root_cap (word, pattern, prestrip, preadd, sufstrip, sufadd,
  firstdent, pfxent, sufent)
    register ichar_t *	word;
    register ichar_t *	pattern;
    int			prestrip;
    int			preadd;
    int			sufstrip;
    int			sufadd;
    struct dent *	firstdent;
    struct flagent *	pfxent;
    struct flagent *	sufent;
    {
    int			i;		/* Index into savearea */
    ichar_t		savearea[MAX_CAPS][INPUTWORDLEN + MAXAFFIXLEN];
    int			nsaved;		/* Number of words saved */

    nsaved = 0;
    save_root_cap (word, pattern, prestrip, preadd, sufstrip, sufadd,
      firstdent, pfxent, sufent, savearea, &nsaved);
    for (i = 0;  i < nsaved;  i++)
	{
	if (insert (savearea[i]) < 0)
	    return -1;
	}
    return 0;
    }

/* ARGSUSED */
static void save_root_cap (word, pattern, prestrip, preadd, sufstrip, sufadd,
  firstdent, pfxent, sufent, savearea, nsaved)
    register ichar_t *	word;		/* Word to be saved */
    register ichar_t *	pattern;	/* Capitalization pattern */
    int			prestrip;	/* No. chars stripped from front */
    int			preadd;		/* No. chars added to front of root */
    int			sufstrip;	/* No. chars stripped from back */
    int			sufadd;		/* No. chars added to back of root */
    struct dent *	firstdent;	/* First dent for root */
    struct flagent *	pfxent;		/* Pfx-flag entry for word */
    struct flagent *	sufent;		/* Sfx-flag entry for word */
    ichar_t		savearea[MAX_CAPS][INPUTWORDLEN + MAXAFFIXLEN];
					/* Room to save words */
    int *		nsaved;		/* Number saved so far (updated) */
    {
    register struct dent * dent;
    int			firstisupper;
    ichar_t		newword[INPUTWORDLEN + 4 * MAXAFFIXLEN + 4];
    register ichar_t *	p;
    int			len;
    int			i;
    int			limit;

    if (*nsaved >= MAX_CAPS)
	return;
    (void) icharcpy (newword, word);
    firstisupper = myupper (pattern[0]);
#define flagsareok(dent)    \
    ((pfxent == NULL \
	||  TSTMASKBIT (dent->mask, pfxent->flagbit)) \
      &&  (sufent == NULL \
	||  TSTMASKBIT (dent->mask, sufent->flagbit)))

    dent = firstdent;
    if ((dent->flagfield & (CAPTYPEMASK | MOREVARIANTS)) == ALLCAPS)
	{
	upcase (newword);	/* Uppercase required */
	(void) icharcpy (savearea[*nsaved], newword);
	(*nsaved)++;
	return;
	}
    for (p = pattern;  *p;  p++)
	{
	if (mylower (*p))
	    break;
	}
    if (*p == 0)
	{
	upcase (newword);	/* Pattern was all caps */
	(void) icharcpy (savearea[*nsaved], newword);
	(*nsaved)++;
	return;
	}
    for (p = pattern + 1;  *p;  p++)
	{
	if (myupper (*p))
	    break;
	}
    if (*p == 0)
	{
	/*
	** The pattern was all-lower or capitalized.  If that's
	** legal, insert only that version.
	*/
	if (firstisupper)
	    {
	    if (captype (dent->flagfield) == CAPITALIZED
	      ||  captype (dent->flagfield) == ANYCASE)
		{
		lowcase (newword);
		newword[0] = mytoupper (newword[0]);
		(void) icharcpy (savearea[*nsaved], newword);
		(*nsaved)++;
		return;
		}
	    }
	else
	    {
	    if (captype (dent->flagfield) == ANYCASE)
		{
		lowcase (newword);
		(void) icharcpy (savearea[*nsaved], newword);
		(*nsaved)++;
		return;
		}
	    }
	while (dent->flagfield & MOREVARIANTS)
	    {
	    dent = dent->next;
	    if (captype (dent->flagfield) == FOLLOWCASE
	      ||  !flagsareok (dent))
		continue;
	    if (firstisupper)
		{
		if (captype (dent->flagfield) == CAPITALIZED)
		    {
		    lowcase (newword);
		    newword[0] = mytoupper (newword[0]);
		    (void) icharcpy (savearea[*nsaved], newword);
		    (*nsaved)++;
		    return;
		    }
		}
	    else
		{
		if (captype (dent->flagfield) == ANYCASE)
		    {
		    lowcase (newword);
		    (void) icharcpy (savearea[*nsaved], newword);
		    (*nsaved)++;
		    return;
		    }
		}
	    }
	}
    /*
    ** Either the sample had complex capitalization, or the simple
    ** capitalizations (all-lower or capitalized) are illegal.
    ** Insert all legal capitalizations, including those that are
    ** all-lower or capitalized.  If the prototype is capitalized,
    ** capitalized all-lower samples.  Watch out for affixes.
    */
    dent = firstdent;
    p = strtosichar (dent->word, 1);
    len = icharlen (p);
    if (dent->flagfield & MOREVARIANTS)
	dent = dent->next;	/* Skip place-holder entry */
    for (  ;  ;  )
	{
	if (flagsareok (dent))
	    {
	    if (captype (dent->flagfield) != FOLLOWCASE)
		{
		lowcase (newword);
		if (firstisupper  ||  captype (dent->flagfield) == CAPITALIZED)
		    newword[0] = mytoupper (newword[0]);
		(void) icharcpy (savearea[*nsaved], newword);
		(*nsaved)++;
		if (*nsaved >= MAX_CAPS)
		    return;
		}
	    else
		{
		/* Followcase is the tough one. */
		p = strtosichar (dent->word, 1);
		(void) BCOPY ((char *) (p + prestrip),
		  (char *) (newword + preadd),
		  (len - prestrip - sufstrip) * sizeof (ichar_t));
		if (myupper (p[prestrip]))
		    {
		    for (i = 0;  i < preadd;  i++)
			newword[i] = mytoupper (newword[i]);
		    }
		else
		    {
		    for (i = 0;  i < preadd;  i++)
			newword[i] = mytolower (newword[i]);
		    }
		limit = len + preadd + sufadd - prestrip - sufstrip;
		i = len + preadd - prestrip - sufstrip;
		p += len - sufstrip - 1;
		if (myupper (*p))
		    {
		    for (p = newword + i;  i < limit;  i++, p++)
			*p = mytoupper (*p);
		    }
		else
		    {
		    for (p = newword + i;  i < limit;  i++, p++)
		      *p = mytolower (*p);
		    }
		(void) icharcpy (savearea[*nsaved], newword);
		(*nsaved)++;
		if (*nsaved >= MAX_CAPS)
		    return;
		}
	    }
	if ((dent->flagfield & MOREVARIANTS) == 0)
	    break;		/* End of the line */
	dent = dent->next;
	}
    return;
    }

static char * getline (s, len)
    register char *	s;
    register int	len;
    {
    register char *	p;
    register int	c;

    p = s;

    for (  ;  ;  )
	{
	(void) fflush (stdout);
	c = GETKEYSTROKE ();
	/*
	** Don't let them overflow the buffer.
	*/
	if (p >= s + len - 1)
	    {
	    *p = 0;
	    return s;
	    }
	if (c == '\\')
	    {
	    (void) putchar ('\\');
	    (void) fflush (stdout);
	    c = GETKEYSTROKE ();
	    backup ();
	    (void) putchar (c);
	    *p++ = (char) c;
	    }
	else if (c == ('G' & 037))
	    return (NULL);
	else if (c == '\n' || c == '\r')
	    {
	    *p = 0;
	    return (s);
	    }
	else if (c == uerasechar)
	    {
	    if (p != s)
		{
		p--;
		backup ();
		(void) putchar (' ');
		backup ();
		}
	    }
	else if (c == ukillchar)
	    {
	    while (p != s)
		{
		p--;
		backup ();
		(void) putchar (' ');
		backup ();
		}
	    }
	else
	    {
	    *p++ = (char) c;
	    (void) putchar (c);
	    }
	}
    }

void askmode ()
    {
    unsigned int	bufsize;	/* Length of contextbufs[0] */
    int			ch;		/* Next character read from input */
    register unsigned char *
			cp1;
    register unsigned char *
			cp2;
    ichar_t *		itok;		/* Ichar version of current word */
    int			hadnl;		/* NZ if \n was at end of line */

    if (fflag)
	{
	if (freopen (askfilename, "w", stdout) == NULL)
	    {
	    (void) fprintf (stderr, CANT_CREATE, askfilename,
	      MAYBE_CR (stderr));
	    exit (1);
	    }
	}

    (void) printf ("%s\n", Version_ID[0]);

    contextoffset = 0;
    while (1)
	{
	if (askverbose)
	    (void) printf ("word: ");
	(void) fflush (stdout);
	/*
	 * Only read in enough characters to fill half this buffer so that any
	 * corrections we make are not likely to cause an overflow.
	 */
	if (contextoffset == 0)
	    {
	    if (xgets ((char *) filteredbuf, (sizeof filteredbuf) / 2, stdin)
	      == NULL)
		break;
	    }
	else
	    {
	    if (fgets ((char *) filteredbuf, (sizeof filteredbuf) / 2, stdin)
	      == NULL)
		break;
	    }
	/*
	 * Make a copy of the line in contextbufs[0] so copyout works.
	 */
	(void) strcpy ((char *) contextbufs[0], (char *) filteredbuf);
	/*
	 * If we didn't read to end-of-line, we may have ended the
	 * buffer in the middle of a word.  So keep reading until we
	 * see some sort of character that can't possibly be part of a
	 * word. (or until the buffer is full, which fortunately isn't
	 * all that likely).
	 */
	bufsize = strlen ((char *) filteredbuf);
	hadnl = filteredbuf[bufsize - 1] == '\n';
	if (bufsize == (sizeof filteredbuf) / 2 - 1)
	    {
	    ch = (unsigned char) filteredbuf[bufsize - 1];
	    while (bufsize < sizeof filteredbuf - 1
	      &&  (iswordch ((ichar_t) ch)  ||  isboundarych ((ichar_t) ch)
	      ||  isstringstart (ch)))
		{
		ch = getc (stdin);
		if (ch == EOF)
		    break;
		contextbufs[0][bufsize] = (char) ch;
		filteredbuf[bufsize++] = (char) ch;
		contextbufs[0][bufsize] = '\0';
		filteredbuf[bufsize] = '\0';
		}
	    }
	/*
	** *line is like `i', @line is like `a', &line is like 'u'
	** `#' is like `Q' (writes personal dictionary)
	** `+' sets tflag, `-' clears tflag
	** `!' sets terse mode, `%' clears terse
	** `~' followed by a filename sets parameters according to file name
	** `^' causes rest of line to be checked after stripping 1st char
	*/
	if (askverbose  ||  contextoffset != 0)
	    checkline (stdout);
	else
	    {
	    if (filteredbuf[0] == '*'  ||  filteredbuf[0] == '@')
		treeinsert(ichartosstr (strtosichar (filteredbuf + 1, 0), 1),
		  ICHARTOSSTR_SIZE,
		  filteredbuf[0] == '*');
	    else if (filteredbuf[0] == '&')
		{
		itok = strtosichar (filteredbuf + 1, 0);
		lowcase (itok);
		treeinsert (ichartosstr (itok, 1), ICHARTOSSTR_SIZE, 1);
		}
	    else if (filteredbuf[0] == '#')
		{
		treeoutput ();
		insidehtml = 0;
		math_mode = 0;
		LaTeX_Mode = 'P';
		}
	    else if (filteredbuf[0] == '!')
		terse = 1;
	    else if (filteredbuf[0] == '%')
		{
		terse = 0;
		correct_verbose_mode = 0;
		}
	    else if (filteredbuf[0] == '-')
		{
		insidehtml = 0;
		math_mode = 0;
		LaTeX_Mode = 'P';
		tflag = DEFORMAT_NROFF;
		}
	    else if (filteredbuf[0] == '+')
		{
		insidehtml = 0;
		math_mode = 0;
		LaTeX_Mode = 'P';
		if (strcmp ((char *) &filteredbuf[1], "plain") == 0
		  ||  strcmp ((char *) &filteredbuf[1], "none") == 0)
		    tflag = DEFORMAT_NONE;
		else if (strcmp ((char *) &filteredbuf[1], "nroff") == 0
		  ||  strcmp ((char *) &filteredbuf[1], "troff") == 0)
		    tflag = DEFORMAT_NROFF;
		else if (strcmp ((char *) &filteredbuf[1], "tex") == 0
		  ||  strcmp ((char *) &filteredbuf[1], "latex") == 0
		  ||  filteredbuf[1] == '\0') /* Backwards compatibility */
		    tflag = DEFORMAT_TEX;
		else if (strcmp ((char *) &filteredbuf[1], "html") == 0
		  ||  strcmp ((char *) &filteredbuf[1], "sgml") == 0)
		    tflag = DEFORMAT_SGML;
		else
		    tflag = DEFORMAT_TEX;	/* Backwards compatibility */
		}
	    else if (filteredbuf[0] == '~')
		{
		if (hadnl)
		    filteredbuf[bufsize - 1] = '\0';
		defstringgroup =
		  findfiletype ((char *) &filteredbuf[1], 1, (int *) NULL);
		if (defstringgroup < 0)
		    defstringgroup = 0;
		if (hadnl)
		    filteredbuf[bufsize - 1] = '\n';
		}
	    else if (filteredbuf[0] == '`')
		correct_verbose_mode = 1;
	    else
		{
		if (filteredbuf[0] == '^')
		    {
		    /* Strip off leading uparrow */
		    for (cp1 = filteredbuf, cp2 = filteredbuf + 1;
		      (*cp1++ = *cp2++) != '\0';
		      )
			;
		    contextoffset++;
		    bufsize--;
		    }
		checkline (stdout);
		}
	    }
	if (hadnl)
	    contextoffset = 0;
	else
	    contextoffset += bufsize;
#ifndef USG
	if (sflag)
	    {
	    stop ();
	    if (fflag)
		{
		rewind (stdout);
		(void) creat (askfilename, 0666);
		}
	    }
#endif
	}
    if (askverbose)
	(void) printf ("\n");
    }

/*
 * Copy up to "cnt" characters to the output file.  For historical
 * reasons, cc points to a characer in "filteredbuf", but the copying
 * must be done from "contextbufs[0]".  As a side effect this function
 * advances cc by the number of characters actually copied.
 */
void copyout (cc, cnt)
    unsigned char **	cc;		/* Char in filteredbuf to start at */
    register int	cnt;		/* Number of chars to copy */
    {
    register char *	cp;		/* Char in contextbufs[0] to copy */
    
    cp = (char *) &contextbufs[0][*cc - filteredbuf];
    *cc += cnt;
    while (--cnt >= 0)
	{
	if (*cp == '\0')
	    {
	    *cc -= cnt + 1;		/* Compensate for short copy */
	    break;
	    }
	if (!aflag && !lflag)
	    (void) putc (*cp, outfile);
	cp++;
	}
    }

static void lookharder (string)
    unsigned char *	string;
    {
    char		cmd[150];
    char		grepstr[100];
    register char *	g;
    register unsigned char *
			s;
#ifndef REGEX_LOOKUP
    register int	wild = 0;
#ifdef LOOK
    static int		look = -1;
#endif /* LOOK */
#endif /* REGEX_LOOKUP */

    g = grepstr;
    for (s = string; *s != '\0'; s++)
	{
	if (*s == '*')
	    {
#ifndef REGEX_LOOKUP
	    wild++;
#endif /* REGEX_LOOKUP */
	    *g++ = '.';
	    *g++ = '*';
	    }
	else
	    *g++ = *s;
	}
    *g = '\0';
    if (grepstr[0])
	{
#ifdef REGEX_LOOKUP
	regex_dict_lookup (cmd, grepstr);
#else /* REGEX_LOOKUP */
#ifdef LOOK
	/* now supports automatic use of look - gms */
	if (!wild && look)
	    {
	    /* no wild and look(1) is possibly available */
	    (void) sprintf (cmd, "%s %s %s", LOOK, grepstr, WORDS);
	    if (shellescape (cmd))
		return;
	    else
		look = 0;
	    }
#endif /* LOOK */
	/* string has wild card chars or look not avail */
	if (!wild)
	    (void) strcat (grepstr, ".*");	/* work like look */
	(void) sprintf (cmd, "%s ^%s$ %s", EGREPCMD, grepstr, WORDS);
	(void) shellescape (cmd);
#endif /* REGEX_LOOKUP */
	}
    }

#ifdef REGEX_LOOKUP
static void regex_dict_lookup (cmd, grepstr)
    char *		cmd;
    char *		grepstr;
    {
    char *		rval;
    int			whence = 0;
    int			quitlookup = 0;
    int			count = 0;
    int			ch;

    (void) sprintf (cmd, "^%s$", grepstr);
    while (!quitlookup  &&  (rval = do_regex_lookup (cmd, whence)) != NULL)
	{
	whence = 1;
        (void) printf ("%s\r\n", rval);;
	if ((++count % (li - 1)) == 0)
	    {
	    inverse ();
	    (void) printf (CORR_C_MORE_PROMPT);
	    normal ();
	    (void) fflush (stdout);
	    if ((ch = GETKEYSTROKE ()) == 'q'
	      ||  ch == 'Q'  ||  ch == 'x'  ||  ch == 'X' )
	         quitlookup = 1;
	    /*
	     * The following line should blank out the -- more -- even on
	     * magic-cookie terminals.
	     */
	    (void) printf (CORR_C_BLANK_MORE);
	    (void) fflush (stdout);
	    }
	}
    if ( rval == NULL )
	{
	inverse ();
	(void) printf (CORR_C_END_LOOK);
	normal ();
	(void) fflush (stdout);
	(void) GETKEYSTROKE ();    
	}
    }

#endif /* REGEX_LOOKUP */
