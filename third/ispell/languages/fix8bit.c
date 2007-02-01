#ifndef lint
static char Rcs_Id[] =
    "$Id: fix8bit.c,v 1.1.1.2 2007-02-01 19:49:42 ghudson Exp $";
#endif

/*
 * Copyright 1993, 1999, 2001, Geoff Kuenning, Claremont, CA
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
 * This is a stupid little program that can be used to convert 8-bit
 * characters to and from backslashed escape sequences.  It is usually
 * more efficient to do this to affix files than to uuencode them for
 * transport.  Ispell will read affix files in either format, so it is
 * merely personal preference as to which form to use.
 *
 * Usage:
 *
 *	fix8bit {-7 | -8} < infile > outfile
 *
 * One of -7 and -8 must be specified.  If -7 is given, any character
 * sequence that is not standard printable ASCII will be converted
 * into a backslashed octal sequence.  If -8 is given, any backslashed
 * octal or hex sequence will be converted into the equivalent 8-bit
 * character.
 *
 * This program is not very smart.  In particular, it makes no attempt
 * to understand comments, quoted strings, or similar constructs in
 * which you might not want conversion to take place.  I suggest that
 * you "diff" the input against the output, and if you don't like the
 * result, correct it by hand.
 */

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.8  2005/04/13 23:52:42  geoff
 * Clean up a bit of style, fix push_back to be tolerant of pushes while
 * there is still stuff on the queue, and make the octal coding conform
 * to C standards (i.e., don't require 3 digits), ill-advised though that
 * may be.
 *
 * Revision 1.7  2005/04/13 23:08:18  geoff
 * Ed Avis's improvements
 *
 * Revision 1.3  2001/11/07 18:59:26  epa98
 * Rewrite of fix8bit.c prompted by SuSE's ispell-3.2.06-languages.patch.
 * I wanted to make sure the patch wouldn't break anything, so I wrote a
 * test suite, but doing that I found lots of other things that were
 * wrong, so I started trying to fix those...
 *
 * Makefile: fixed dependencies for fix8bit, added 'test' target.  The
 * test suite checks fix8bit's pushback routines, runs test_fix8bit (see
 * below) and checks a couple of additional properties: fix8bit -8 |
 * fix8bit -7 == cat; fix8bit -7 | fix8bit -7 == fix8bit -7.
 *
 * fix8bit.c: rewrote to8bit() to better handle cases when the
 * backslashed sequence turns out to be illegal.  The initial backslash
 * is printed and the remaining characters are pushed back to be read
 * again.  This means that for example \\x41 will print as \A, in the
 * same way that !\x41 produces !A.  It also handles escape sequences
 * cut off by EOF properly (again they are printed unchanged).  This uses
 * a mini pushback library which has a test suite if you give main() the
 * argument --test-pushback.  Also fixed the original problem with hex
 * sequences being miscomputed, which SuSE wrote the patch for.  Added a
 * warning if the input already contains 8-bit chars (that would stop
 * -8 | -7 being identity).
 *
 * test_fix8bit: new file.  This is a Perl script to run fix8bit -7 and
 * fix8bit -8 on every input file in test_data/ and check the results
 * against the expected results also in that directory.
 *
 * test_data/: new directory.  Contains test cases, some written by hand
 * and some randomly generated by rand_gen.  rand_gen tries to make 'well
 * behaved' input that doesn't muck up fix8bit -8 | fix8bit -7 or other
 * commands - but there are three flags you can use to tell it not to.
 * The random test cases have not been checked by hand, so they should be
 * used in addition to human-written ones.
 *
 * Revision 1.2  2001/10/05 14:22:30  epa98
 * Imported 3.2.06.epa1 release.  This was previously developed using
 * sporadic RCS for certain files, but I'm not really bothered about
 * rolling back beyond this release.
 *
 * Revision 1.6  2001/07/25 21:51:47  geoff
 * *** empty log message ***
 *
 * Revision 1.5  2001/07/23 20:43:38  geoff
 * *** empty log message ***
 *
 * Revision 1.4  1999/01/07 06:07:52  geoff
 * Update the copyright.
 *
 * Revision 1.3  1994/01/25  07:12:26  geoff
 * Get rid of all old RCS log lines in preparation for the 3.1 release.
 *
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

int		main ();	/* Convert to/from 8-bit sequences */
static void	usage ();	/* Issue a usage message */
static void	to7bit ();	/* Convert from 8-bit sequences */
static void	to8bit ();	/* Convert to 8-bit sequences */
static int	get_char ();    /* Get char with pushback */
static void	push_back ();   /* Push back character */
static void	warn_not_8bit (); /* Warn if input isn't 8-bit */

extern void	exit ();	/* Terminate program */

/*
 * Maximum number of characters that get_char can push back
 */
#define MAX_PUSHED_BACK 3

static int	num_pushed_back = 0;
				/* Amount of data in pushed_back */
static int	pushed_back[MAX_PUSHED_BACK];
				/* Characters that get_char has pushed back */

int main (argc, argv)		/* Convert to/from 8-bit sequences */
    int			argc;	/* Argument count */
    char *		argv[];	/* Argument vector */
    {
    
    if (argc != 2)
	usage ();
    if (strcmp (argv[1], "-7") == 0)
	to7bit ();
    else if (strcmp (argv[1], "-8") == 0)
	to8bit ();
    else
	usage ();
    return 0;
    }

static void usage ()		/* Issue a usage message */
    {

    (void) fprintf (stderr, "Usage:  fix8bit {-7 | -8} < infile > outfile\n");
    exit (1);
    }

static void to7bit ()		/* Convert from 8-bit sequences */
    {
    int			ch;	/* Next character read from input */

    while ((ch = getchar ()) != EOF)
	{
	ch &= 0xFF;
	if (ch >= 0x80)
	    (void) printf ("\\%3.3o", (unsigned) ch);
	else
	    (void) putchar (ch);
	}
    }

static void to8bit ()		/* Convert to 8-bit sequences */
    {
    int			backch;	/* Backslashed character being built */
    int			ch;	/* Next character read from input */
    int			ch_1;	/* First character after backslash */
    int			ch_2;	/* Second character after backslash */
    int			ch_3;	/* Third character after backslash */

    while ((ch = get_char ()) != EOF)
	{
	ch &= 0xFF;
	if (ch != '\\')
	    {
	    /* Not a backslashed sequence */
	    if (ch >= 0x80)
		{
		fprintf(stderr,
		  "warning: passing through 8-bit character unchanged: 0x%x\n",
		  ch);
		}
	    (void) putchar (ch);
	    }
	else
	    {
	    /*
	     * Collect a backslashed character.  If we have to abandon
	     * our reading because we got a bad character or EOF,
	     * then we output the backslash (since it doesn't start a
	     * legal escape sequence) and push back the remaining
	     * characters for use next time.  This is so that
	     * \\x60 will become \` , for example.
	     */
	    ch_1 = get_char ();
	    switch (ch_1)
		{
		case 'x':
		case 'X':
		    /* \x.. hex sequence.  Check following character... */
		    ch_2 = get_char ();
		    if (ch_2 >= '0'  &&  ch_2 <= '9')
			backch = ch_2 - '0';
		    else if (ch_2 >= 'a'  &&  ch_2 <= 'f')
			backch = ch_2 - 'a' + 0xA;
		    else if (ch_2 >= 'A'  &&  ch_2 <= 'F')
			backch = ch_2 - 'A' + 0xA;
		    else
			{
			/* 
			 * \x not followed by valid hex digit.  Put
			 * out the backslash, and push the rest back.
			 * (We could output the x right now, but it's
			 * safer for future refinements to push it
			 * back, and the computational cost is
			 * negligible.)
			 */
			(void) putchar ('\\');
			if (ch_2 != EOF)
			    (void) push_back (ch_2);
			(void) push_back (ch_1);
			break;
			}

		    /* Third character after the backslash. */
		    ch_3 = get_char ();
		    if (ch_3 >= '0'  &&  ch_3 <= '9')
			backch = (backch << 4) | (ch_3 - '0');
		    else if (ch_3 >= 'a'  &&  ch_3 <= 'f')
			backch = (backch << 4) | (ch_3 - 'a' + 0xA);
 		    else if (ch_3 >= 'A'  &&  ch_3 <= 'F')
			backch = (backch << 4) | (ch_3 - 'A' + 0xA);
		    else
			{
			/*
			 * Not a hex digit.  The rules require \x to
			 * be followed by exactly two hex digits, so
			 * we'll reject the entire sequence.  Again,
			 * we push back everything after the backslash
			 * so that future modifications will be safer.
			 */
			(void) putchar ('\\');
			if (ch_3 != EOF)
			    (void) push_back (ch_3);
			(void) push_back (ch_2);
			(void) push_back (ch_1);
			break;
			}

		    /* 
		     * All OK.  Warn if necessary, and the output the
		     * converted character.
		     */
		    warn_not_8bit (backch);
		    (void) putchar (backch);
		    break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		    /* 
		     * We're starting a backslashed octal sequence.
		     * We just got the first character, so do the second.
		     * Octal is a bit more fun because the rules allow
		     * variable-length sequences: \7a is the same as
		     * \007a: a 7 (BEL) character followed by "a" (but
		     * note that \10a is NOT the same as \0010a: the
		     * first is a hex 8 (BS) followed by "A" while the
		     * second is hex 1 (SOH) followed by "0a").
		     */
		    backch = ch_1 - '0';
		    ch_2 = get_char ();
		    if (ch_2 >= '0'  &&  ch_2 <= '7')
			backch = (backch << 3) | (ch_2 - '0');
		    else
			{
			(void) putchar (backch);
			if (ch_2 != EOF)
			    (void) push_back (ch_2);
			break;
			}

		    /* Third character. */
		    ch_3 = get_char ();
		    if (ch_3 >= '0'  &&  ch_3 <= '7')
			backch = (backch << 3) | (ch_3 - '0');
		    else
			{
			(void) putchar (backch);
			if (ch_3 != EOF)
			    (void) push_back (ch_3);
			break;
			}

		    /* All OK. */
		    warn_not_8bit (backch);
		    (void) putchar (backch);
		    break;
    		default:
		    /*
		     * A backslash was followed by something that's
		     * not a valid hex or octal code.  Put out the
		     * backslash, and push back the following
		     * character.
		     */
		    (void) putchar ('\\');
		    if (ch_1 != EOF)
			(void) push_back (ch_1);
		}
	    }
	}
    }

/*
 * Simple character input with limited-length pushback.
 * Unfortunately, ungetc() handles only a single character of
 * pushback, and we may need several.
 */
static int get_char ()
    {
    if (num_pushed_back > 0)
	return pushed_back[--num_pushed_back];
    else
	return getchar();
    }

/*
 * Push a character onto the push-back queue.
 */
static void push_back (ch)
    int                ch;		/* Character to push back */
    {
    assert (num_pushed_back < MAX_PUSHED_BACK);

    pushed_back[num_pushed_back++] = ch;
    }

/*
 * If a character isn't 8-bit, put out a warning.
 */
static void warn_not_8bit (ch)
    int                ch;		/* Character to test */
    {
    if (ch < 0x80)
	fprintf(stderr, "warning: converted an escape sequence to "
		"a 7-bit character: 0x%x\n", ch);
    }
