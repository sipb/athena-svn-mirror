#ifndef lint
static char Rcs_Id[] =
    "$Id: term.c,v 1.1.1.2 2007-02-01 19:50:05 ghudson Exp $";
#endif

/*
 * term.c - deal with termcap, and unix terminal mode settings
 *
 * Pace Willisson, 1983
 *
 * Copyright 1987, 1988, 1989, 1992, 1993, 1999, 2001, 2005, Geoff Kuenning,
 * Claremont, CA.
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
 * Revision 1.54  2005/04/14 23:11:36  geoff
 * Correctly handle control-Z, including resetting the terminal.  The
 * only remaining problem is that the screen isn't automatically
 * refreshed. (Doing the latter would either require major changes to
 * make the screen-refresh code callable from the signal handler, or
 * fixing GETKEYSTROKE to fail on signals.  That's probably not hugely
 * hard but doing it portably probably is.)
 *
 * Revision 1.53  2005/04/14 14:38:23  geoff
 * Update license.  Rename move/erase to avoid library conflicts.
 *
 * Revision 1.52  2001/09/06 00:30:28  geoff
 * Many changes from Eli Zaretskii to support DJGPP compilation.
 *
 * Revision 1.51  2001/07/25 21:51:46  geoff
 * Minor license update.
 *
 * Revision 1.50  2001/07/23 20:24:04  geoff
 * Update the copyright and the license.
 *
 * Revision 1.49  1999/01/07 01:22:53  geoff
 * Update the copyright.
 *
 * Revision 1.48  1994/10/25  05:46:11  geoff
 * Fix a couple of places where ifdefs were omitted, though apparently
 * harmlessly.
 *
 * Revision 1.47  1994/09/01  06:06:32  geoff
 * Change erasechar/killchar to uerasechar/ukillchar to avoid
 * shared-library problems on HP systems.
 *
 * Revision 1.46  1994/01/25  07:12:11  geoff
 * Get rid of all old RCS log lines in preparation for the 3.1 release.
 *
 */

#include "config.h"
#include "ispell.h"
#include "proto.h"
#include "msgs.h"
#ifdef USG
#include <termio.h>
#else
#ifndef __DJGPP__
#include <sgtty.h>
#endif
#endif
#include <signal.h>

void		ierase P ((void));
void		imove P ((int row, int col));
void		inverse P ((void));
void		normal P ((void));
void		backup P ((void));
static int	iputch P ((int c));
void		terminit P ((void));
SIGNAL_TYPE	done P ((int signo));
#ifdef SIGTSTP
static SIGNAL_TYPE onstop P ((int signo));
#endif /* SIGTSTP */
void		stop P ((void));
int		shellescape P ((char * buf));
#ifdef USESH
void		shescape P ((char * buf));
#endif /* USESH */

static int	termchanged = 0;

#ifdef __DJGPP__
#include "pc/djterm.c"
#endif

void ierase ()
    {

    if (cl)
	tputs (cl, li, iputch);
    else
	{
	if (ho)
	    tputs (ho, 100, iputch);
	else if (cm)
	    tputs (tgoto (cm, 0, 0), 100, iputch);
	tputs (cd, li, iputch);
	}
    }

void imove (row, col)
    int		row;
    int		col;
    {
    tputs (tgoto (cm, col, row), 100, iputch);
    }

void inverse ()
    {
    tputs (so, 10, iputch);
    }

void normal ()
    {
    tputs (se, 10, iputch);
    }

void backup ()
    {
    if (BC)
	tputs (BC, 1, iputch);
    else
	(void) putchar ('\b');
    }

static int iputch (c)
    int			c;
    {

    return putchar (c);
    }

#ifdef USG
static struct termio	sbuf;
static struct termio	osbuf;
#else
static struct sgttyb	sbuf;
static struct sgttyb	osbuf;
#ifdef TIOCSLTC
static struct ltchars	ltc;
static struct ltchars	oltc;
#endif
#endif
static SIGNAL_TYPE	(*oldint) ();
static SIGNAL_TYPE	(*oldterm) ();
#ifdef SIGTSTP
static SIGNAL_TYPE	(*oldttin) ();
static SIGNAL_TYPE	(*oldttou) ();
static SIGNAL_TYPE	(*oldtstp) ();
#endif

void terminit ()
    {
#ifdef TIOCPGRP
    int			tpgrp;
#else
#ifdef TIOCGPGRP
    int			tpgrp;
#endif
#endif
#ifdef TIOCGWINSZ
    struct winsize	wsize;
#endif /* TIOCGWINSZ */

    tgetent (termcap, getenv ("TERM"));
    termptr = termstr;
    BC = tgetstr ("bc", &termptr);
    cd = tgetstr ("cd", &termptr);
    cl = tgetstr ("cl", &termptr);
    cm = tgetstr ("cm", &termptr);
    ho = tgetstr ("ho", &termptr);
    nd = tgetstr ("nd", &termptr);
    so = tgetstr ("so", &termptr);	/* inverse video on */
    se = tgetstr ("se", &termptr);	/* inverse video off */
    if ((sg = tgetnum ("sg")) < 0)	/* space taken by so/se */
	sg = 0;
    ti = tgetstr ("ti", &termptr);	/* terminal initialization */
    te = tgetstr ("te", &termptr);	/* terminal termination */
    co = tgetnum ("co");
    li = tgetnum ("li");
#ifdef TIOCGWINSZ
    if (ioctl (0, TIOCGWINSZ, (char *) &wsize) >= 0)
	{
	if (wsize.ws_col != 0)
	    co = wsize.ws_col;
	if (wsize.ws_row != 0)
	    li = wsize.ws_row;
	}
#endif /* TIOCGWINSZ */
    /*
     * Let the variables "LINES" and "COLUMNS" override the termcap
     * entry.  Technically, this is a terminfo-ism, but I think the
     * vast majority of users will find it pretty handy.
     */
    if (getenv ("COLUMNS") != NULL)
	co = atoi (getenv ("COLUMNS"));
    if (getenv ("LINES") != NULL)
	li = atoi (getenv ("LINES"));
#if MAX_SCREEN_SIZE > 0
    if (li > MAX_SCREEN_SIZE)
	li = MAX_SCREEN_SIZE;
#endif /* MAX_SCREEN_SIZE > 0 */
#if MAXCONTEXT == MINCONTEXT
    contextsize = MINCONTEXT;
#else /* MAXCONTEXT == MINCONTEXT */
    if (contextsize == 0)
#ifdef CONTEXTROUNDUP
	contextsize = (li * CONTEXTPCT + 99) / 100;
#else /* CONTEXTROUNDUP */
	contextsize = (li * CONTEXTPCT) / 100;
#endif /* CONTEXTROUNDUP */
    if (contextsize > MAXCONTEXT)
	contextsize = MAXCONTEXT;
    else if (contextsize < MINCONTEXT)
	contextsize = MINCONTEXT;
#endif /* MAX_CONTEXT == MIN_CONTEXT */
    /*
     * Insist on 2 lines for the screen header, 2 for blank lines
     * separating areas of the screen, 2 for word choices, and 2 for
     * the minimenu, plus however many are needed for context.  If
     * possible, make the context smaller to fit on the screen.
     */
    if (li < contextsize + 8  &&  contextsize > MINCONTEXT)
	{
	contextsize = li - 8;
	if (contextsize < MINCONTEXT)
	    contextsize = MINCONTEXT;
	}
    if (li < MINCONTEXT + 8)
	(void) fprintf (stderr, TERM_C_SMALL_SCREEN, MINCONTEXT + 8);

#ifdef SIGTSTP
#ifdef TIOCPGRP
retry:
#endif /* SIGTSTP */
#endif /* TIOCPGRP */

#ifdef USG
    if (!isatty (0))
	{
	(void) fprintf (stderr, TERM_C_NO_BATCH);
	exit (1);
	}
    (void) ioctl (0, TCGETA, (char *) &osbuf);
    termchanged = 1;

    sbuf = osbuf;
    sbuf.c_lflag &= ~(ECHO | ECHOK | ECHONL | ICANON);
    sbuf.c_oflag &= ~(OPOST);
    sbuf.c_iflag &= ~(INLCR | IGNCR | ICRNL);
    sbuf.c_cc[VMIN] = 1;
    sbuf.c_cc[VTIME] = 1;
    (void) ioctl (0, TCSETAW, (char *) &sbuf);

    uerasechar = osbuf.c_cc[VERASE];
    ukillchar = osbuf.c_cc[VKILL];

#endif

#ifdef SIGTSTP
#ifndef USG
    (void) sigsetmask (1<<(SIGTSTP-1) | 1<<(SIGTTIN-1) | 1<<(SIGTTOU-1));
#endif
#endif
#ifdef TIOCGPGRP
    if (ioctl (0, TIOCGPGRP, (char *) &tpgrp) != 0)
	{
	(void) fprintf (stderr, TERM_C_NO_BATCH);
	exit (1);
	}
#endif
#ifdef SIGTSTP
#ifdef TIOCPGRP
    if (tpgrp != getpgrp(0)) /* not in foreground */
	{
#ifndef USG
	(void) sigsetmask (1 << (SIGTSTP - 1) | 1 << (SIGTTIN - 1));
#endif
	(void) signal (SIGTTOU, SIG_DFL);
	(void) kill (0, SIGTTOU);
	/* job stops here waiting for SIGCONT */
	goto retry;
	}
#endif
#endif

#ifndef USG
    (void) ioctl (0, TIOCGETP, (char *) &osbuf);
#ifdef TIOCGLTC
    (void) ioctl (0, TIOCGLTC, (char *) &oltc);
#endif
    termchanged = 1;

    sbuf = osbuf;
    sbuf.sg_flags &= ~ECHO;
    sbuf.sg_flags |= TERM_MODE;
    (void) ioctl (0, TIOCSETP, (char *) &sbuf);

    uerasechar = sbuf.sg_erase;
    ukillchar = sbuf.sg_kill;

#ifdef TIOCSLTC
    ltc = oltc;
    ltc.t_suspc = -1;
    (void) ioctl (0, TIOCSLTC, (char *) &ltc);
#endif

#endif /* USG */

    if ((oldint = signal (SIGINT, SIG_IGN)) != SIG_IGN)
	(void) signal (SIGINT, done);
    if ((oldterm = signal (SIGTERM, SIG_IGN)) != SIG_IGN)
	(void) signal (SIGTERM, done);

#ifdef SIGTSTP
#ifndef USG
    (void) sigsetmask (0);
#endif
    if ((oldttin = signal (SIGTTIN, SIG_IGN)) != SIG_IGN)
	(void) signal (SIGTTIN, onstop);
    if ((oldttou = signal (SIGTTOU, SIG_IGN)) != SIG_IGN)
	(void) signal (SIGTTOU, onstop);
    if ((oldtstp = signal (SIGTSTP, SIG_IGN)) != SIG_IGN)
	(void) signal (SIGTSTP, onstop);
#endif
    if (ti)
	tputs (ti, 1, iputch);
    }

/* ARGSUSED */
SIGNAL_TYPE done (signo)
    int		signo;
    {
    if (tempfile[0] != '\0')
	(void) unlink (tempfile);
    if (termchanged)
	{
	if (te)
	    tputs (te, 1, iputch);
#ifdef USG
	(void) ioctl (0, TCSETAW, (char *) &osbuf);
#else
	(void) ioctl (0, TIOCSETP, (char *) &osbuf);
#ifdef TIOCSLTC
	(void) ioctl (0, TIOCSLTC, (char *) &oltc);
#endif
#endif
	}
    exit (0);
    }

#ifdef SIGTSTP
static SIGNAL_TYPE onstop (signo)
    int		signo;
    {
    if (termchanged)
	{
	imove (li - 1, 0);
	if (te)
	    tputs (te, 1, iputch);
#ifdef USG
	(void) ioctl (0, TCSETAW, (char *) &osbuf);
#else
	(void) ioctl (0, TIOCSETP, (char *) &osbuf);
#ifdef TIOCSLTC
	(void) ioctl (0, TIOCSLTC, (char *) &oltc);
#endif
#endif
	}
    (void) fflush (stdout);
    (void) signal (signo, SIG_DFL);
#ifndef USG
    (void) sigsetmask (sigblock (0) & ~(1 << (signo - 1)));
#endif
    (void) kill (0, SIGSTOP);
    /* stop here until continued */
    (void) signal (signo, onstop);
    if (termchanged)
	{
#ifdef USG
	(void) ioctl (0, TCSETAW, (char *) &sbuf);
#else
	(void) ioctl (0, TIOCSETP, (char *) &sbuf);
#ifdef TIOCSLTC
	(void) ioctl (0, TIOCSLTC, (char *) &ltc);
#endif
#endif
	if (ti)
	    tputs (ti, 1, iputch);
	}
    }
#endif

#ifndef USESH
#define NEED_SHELLESCAPE
#endif /* USESH */
#ifndef REGEX_LOOKUP
#define NEED_SHELLESCAPE
#endif /* REGEX_LOOKUP */

void stop ()
    {
#ifdef SIGTSTP
    onstop (SIGTSTP);
#else
    /* for System V and MSDOS */
    imove (li - 1, 0);
    (void) fflush (stdout);
#ifdef NEED_SHELLESCAPE
    if (getenv ("SHELL"))
	(void) shellescape (getenv ("SHELL"));
    else
	(void) shellescape ("sh");
#else
    shescape ("");
#endif /* NEED_SHELLESCAPE */
#endif /* SIGTSTP */
    }

/* Fork and exec a process.  Returns NZ if command found, regardless of
** command's return status.  Returns zero if command was not found.
** Doesn't use a shell.
*/
#ifdef NEED_SHELLESCAPE
int shellescape	(buf)
    char *	buf;
    {
    char *	argv[100];
    char *	cp = buf;
    int		i = 0;
    int		termstat;

    /* parse buf to args (destroying it in the process) */
    while (*cp != '\0')
	{
	while (*cp == ' '  ||  *cp == '\t')
	    ++cp;
	if (*cp == '\0')
	    break;
	argv[i++] = cp;
	while (*cp != ' '  &&  *cp != '\t'  &&  *cp != '\0')
	    ++cp;
	if (*cp != '\0')
	    *cp++ = '\0';
	}
    argv[i] = NULL;

#ifdef USG
    (void) ioctl (0, TCSETAW, (char *) &osbuf);
#else
    (void) ioctl (0, TIOCSETP, (char *) &osbuf);
#ifdef TIOCSLTC
    (void) ioctl (0, TIOCSLTC, (char *) &oltc);
#endif /* TIOCSLTC */
#endif
    (void) signal (SIGINT, oldint);
    (void) signal (SIGTERM, oldterm);
#ifdef SIGTSTP
    (void) signal (SIGTTIN, oldttin);
    (void) signal (SIGTTOU, oldttou);
    (void) signal (SIGTSTP, oldtstp);
#endif
    if ((i = fork ()) == 0)
	{
	(void) execvp (argv[0], (char **) argv);
	_exit (123);		/* Command not found */
	}
    else if (i > 0)
	{
	while (wait (&termstat) != i)
	    ;
	termstat = (termstat == (123 << 8)) ? 0 : -1;
	}
    else
	{
	(void) printf (TERM_C_CANT_FORK, MAYBE_CR (stderr));
	termstat = -1;		/* Couldn't fork */
	}

    if (oldint != SIG_IGN)
	(void) signal (SIGINT, done);
    if (oldterm != SIG_IGN)
	(void) signal (SIGTERM, done);

#ifdef SIGTSTP
    if (oldttin != SIG_IGN)
	(void) signal (SIGTTIN, onstop);
    if (oldttou != SIG_IGN)
	(void) signal (SIGTTOU, onstop);
    if (oldtstp != SIG_IGN)
	(void) signal (SIGTSTP, onstop);
#endif

#ifdef USG
    (void) ioctl (0, TCSETAW, (char *) &sbuf);
#else
    (void) ioctl (0, TIOCSETP, (char *) &sbuf);
#ifdef TIOCSLTC
    (void) ioctl (0, TIOCSLTC, (char *) &ltc);
#endif /* TIOCSLTC */
#endif
    if (termstat)
	{
	(void) printf (TERM_C_TYPE_SPACE);
	(void) fflush (stdout);
#ifdef COMMANDFORSPACE
	i = GETKEYSTROKE ();
	if (i != ' ' && i != '\n' && i != '\r')
	    (void) ungetc (i, stdin);
#else
	while (GETKEYSTROKE () != ' ')
	    ;
#endif
	}
    return (termstat);
    }
#endif /* NEED_SHELLESCAPE */

#ifdef	USESH
void shescape (buf)
    char *	buf;
    {
#ifdef COMMANDFORSPACE
    int		ch;
#endif
#ifdef __DJGPP__
    char	curdir[MAXPATHLEN];
#endif

#ifdef USG
    (void) ioctl (0, TCSETAW, (char *) &osbuf);
#else
    (void) ioctl (0, TIOCSETP, (char *) &osbuf);
#ifdef TIOCSLTC
    (void) ioctl (0, TIOCSLTC, (char *) &oltc);
#endif
#endif
#ifdef __DJGPP__
    /* Don't erase the screen if they want to run a single command,
     * otherwise they will be unable to see its output.
     */
    if (buf[0] == '\0')
	djgpp_restore_screen ();
    /* Change and restore the current directory, because it's a global
     * notion on MS-DOS/MS-Windows.
     */
    getcwd (curdir, MAXPATHLEN);
#endif
    (void) signal (SIGINT, oldint);
    (void) signal (SIGTERM, oldterm);
#ifdef SIGTSTP
    (void) signal (SIGTTIN, oldttin);
    (void) signal (SIGTTOU, oldttou);
    (void) signal (SIGTSTP, oldtstp);
#endif

    (void) system (buf);

    if (oldint != SIG_IGN)
	(void) signal (SIGINT, done);
    if (oldterm != SIG_IGN)
	(void) signal (SIGTERM, done);

#ifdef SIGTSTP
    if (oldttin != SIG_IGN)
	(void) signal (SIGTTIN, onstop);
    if (oldttou != SIG_IGN)
	(void) signal (SIGTTOU, onstop);
    if (oldtstp != SIG_IGN)
	(void) signal (SIGTSTP, onstop);
#endif
#ifdef __DJGPP__
    if (buf[0] == '\0')
	djgpp_ispell_screen ();
    chdir (curdir);
#endif

#ifdef USG
    (void) ioctl (0, TCSETAW, (char *) &sbuf);
#else
    (void) ioctl (0, TIOCSETP, (char *) &sbuf);
#ifdef TIOCSLTC
    (void) ioctl (0, TIOCSLTC, (char *) &ltc);
#endif
#endif
    (void) printf (TERM_C_TYPE_SPACE);
    (void) fflush (stdout);
#ifdef COMMANDFORSPACE
    ch = GETKEYSTROKE ();
    if (ch != ' '  &&  ch != '\n'  &&  ch != '\r')
	(void) ungetc (ch, stdin);
#else
    while (GETKEYSTROKE () != ' ')
	;
#endif
    }
#endif
