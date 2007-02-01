#ifndef lint
static char Rcs_Id[] =
    "$Id: defmt-sh.c,v 1.1.1.1 2007-02-01 19:49:41 ghudson Exp $";
#endif

/*
 * Simple deformatter for sh/bash scripts.
 *
 * Copyright 2001, Geoff Kuenning, Claremont, CA
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
 */

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2001/07/25 21:51:48  geoff
 * *** empty log message ***
 *
 * Revision 1.3  2001/07/23 20:43:38  geoff
 * *** empty log message ***
 *
 * Revision 1.2  2001/06/07 08:03:54  geoff
 * Don't interpret $# as a comment introduction.
 *
 * Revision 1.1  2001/06/07 07:23:54  geoff
 * Initial revision
 *
 */

#include <stdio.h>
#ifndef NO_FCNTL_H
#include <fcntl.h>
#if defined(O_BINARY) && O_BINARY
#include <io.h>
#define SET_BINARY(fd)	do \
			    { \
			    if (!isatty (fd)) \
				setmode (fd, O_BINARY); \
			    } while (0)
#else
#define SET_BINARY(fd) /* Nothing needed */
#endif /* O_BINARY */
#endif /* NO_FCNTL_H */

int		main ();	/* Filter to select sh/bash comments */
static int	igetchar ();	/* Read one character from stdin */
static int	do_comment ();	/* Handle comments */
static int	do_quote ();	/* Handle quoted strings */

int main (argc, argv)
    int		argc;		/* Argument count */
    char *	argv[];		/* Argument vector */
    {
    int		c;		/* Next character read from stdin */

    /*
     * Since the deformatter needs to produce exactly one character
     * of output for each character of input, we need to preserve
     * the end-of-line format (Unix Newline or DOS CR-LF) of the
     * input file.  This means we must do binary I/O.
     */
    SET_BINARY (fileno (stdin));
    SET_BINARY (fileno (stdout));

    while ((c = igetchar ()) != EOF)
	{
	if (c == '\\')
	    {
	    putchar (' ');
	    if ((c = igetchar ()) == EOF)
		break;
	    if (c == '\n' || c == '\r')
		putchar (c);
	    else
		putchar (' ');
	    }
	else if (c == '#')
	    {
	    if (do_comment())
		break;
	    }
	else if (c == '\''  ||  c == '"')
	    {
	    if (do_quote(c))
		break;
	    }
	else if (c == '$')
	    {
	    /*
	     * $ might be followed by #, in which case it's not a comment
	     * start.  So we skip the character immediately after the $.
	     */
	    putchar (' ');
	    if ((c = igetchar ()) == EOF)
		break;
	    putchar (' ');
	    }
	else if (c == '\n' || c == '\r')
	    putchar (c);
	else
	    putchar (' ');
	}

    return 0;
    }

/*
 * Like getchar, except on MSDOS, where it knows about ^Z.
 */
static int igetchar ()
    {
    int c = getchar ();
#ifdef MSDOS
    if (c == '\032')	/* ^Z is a kind of ``software EOF'' */
	c = EOF;;
#endif
    return c;
    }

/*
 * Handle shell comments, passing their contents through unchanged.
 */
static int do_comment ()
    {
    int			c;	/* Next character from stdin */

    putchar (' ');		/* Create blank to cover for pound sign */

    while ((c = igetchar ()) != EOF)
	{
	putchar (c);
	if (c == '\n')
	    return 0;		/* End of comment, continue deformatting */
	}

    return 1;			/* EOF hit, caller must terminate loop */
    }

/*
 * Handle quoted strings, passing their contents through unchanged.
 */
static int do_quote(qc)
    int			qc;	/* Character that started the quote section */
    {
    int			c;	/* Next character from stdin */

    putchar (' ');		/* Create blank to cover for the quote */

    while ((c = igetchar ()) != EOF)
	{
	if (c == qc)
	    {
	    putchar (' ');
	    return 0;		/* End of quotes, continue deformatting */
	    }
	else if (c == '\\')
	    {
	    /*
	     * Backslashed stuff is tricky to handle, because it might
	     * contain a magic nroff or TeX character, but in that case
	     * a doubled backslash would have to be converted to single.
	     * That's too hard, so we'll settle for just passing the double
	     * backslash through.  If you want to spell-check that kind of
	     * sequence, you'll have to create a new character-set type in
	     * your affix file.
	     */
	    putchar ('\\');
	    if ((c = igetchar ()) == EOF)
		return 1;
	    putchar (c);
	    }
	else
	    putchar (c);
	}

    return 1;			/* EOF hit, caller must terminate loop */
    }

