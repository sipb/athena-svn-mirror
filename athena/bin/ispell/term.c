/*
 * term.c - deal with termcap, and unix terminal mode settings
 *
 * Pace Willisson, 1983
 */

#include <stdio.h>
#ifdef SYSV 
#define USG
#endif
#ifdef USG
#include <termio.h>
#else
#include <sgtty.h>
#endif
#include <signal.h>
#include "ispell.h"

int putch();

erase ()
{
	if (cl)
		tputs(cl, li, putch);
	else {
		if (ho)
			tputs(ho, 100, putch);
		else if (cm)
			tputs(tgoto(cm, 0, 0), 100, putch);
		tputs(cd, li, putch);
	}
}

move (row, col)
{
	tputs (tgoto (cm, col, row), 100, putch);
}

inverse ()
{
	tputs (so, 10, putch);
}

normal ()
{
	tputs (se, 10, putch);
}

backup ()
{
	if (BC)
		tputs (BC, 1, putch);
	else
		putchar ('\b');
}

putch (c)
{
	putchar (c);
}

#ifdef USG
struct termio sbuf, osbuf;
#else
struct sgttyb sbuf, osbuf;
#endif
static termchanged = 0;


terminit ()
{
	int done();
#ifdef USG
	if (!isatty(0)) {
		fprintf (stderr, "Can't deal with non interactive use yet.\n");
		exit (1);
	}
	(void) ioctl (0, TCGETA, &osbuf);
	termchanged = 1;

	sbuf = osbuf;
	sbuf.c_lflag &= ~(ECHO | ECHOK | ECHONL | ICANON);
	sbuf.c_oflag &= ~(OPOST);
	sbuf.c_iflag &= ~(INLCR | IGNCR | ICRNL);
	sbuf.c_cc[VMIN] = 1;
	sbuf.c_cc[VTIME] = 1;
	(void) ioctl (0, TCSETAW, &sbuf);

	erasechar = osbuf.c_cc[VERASE];
	killchar = osbuf.c_cc[VKILL];

	(void) signal (SIGINT, done);
#else
	int tpgrp;
	int onstop();
#ifndef _IBMR2
/* Only fakes termcap- doesn't have ospeed. */
	extern short ospeed; 
#endif
retry:
	(void) sigsetmask(1<<SIGTSTP | 1<<SIGTTIN | 1<<SIGTTOU);
	if (ioctl(0, TIOCGPGRP, &tpgrp) != 0) {
		fprintf (stderr, "Can't deal with non interactive use yet.\n");
		exit (1);
	}
	if (tpgrp != getpgrp(0)) { /* not in foreground */
		(void) sigsetmask(1<<SIGTSTP | 1<<SIGTTIN);
		(void) signal(SIGTTOU, SIG_DFL);
		(void) kill(0, SIGTTOU);
		/* job stops here waiting for SIGCONT */
		goto retry;
	}

	(void) ioctl (0, TIOCGETP, &osbuf);
	termchanged = 1;

	sbuf = osbuf;
	sbuf.sg_flags &= ~ECHO;
	sbuf.sg_flags |= RAW;
	(void) ioctl (0, TIOCSETP, &sbuf);

	erasechar = sbuf.sg_erase;
	killchar = sbuf.sg_kill;
#ifndef _IBMR2
	ospeed = sbuf.sg_ospeed;
#endif
	(void) signal (SIGINT, done);

	(void) sigsetmask(0);
	(void) signal(SIGTTIN, onstop);
	(void) signal(SIGTTOU, onstop);
	(void) signal(SIGTSTP, onstop);
#endif

	tgetent(termcap, getenv("TERM"));
	termptr = termstr;
	bs = tgetflag("bs");
	BC = tgetstr("bc", &termptr);
	UP = tgetstr("up", &termptr);
	cd = tgetstr("cd", &termptr);
	ce = tgetstr("ce", &termptr);	
	cl = tgetstr("cl", &termptr);
	cm = tgetstr("cm", &termptr);
	dc = tgetstr("dc", &termptr);
	dl = tgetstr("dl", &termptr);
	dm = tgetstr("dm", &termptr);
	ed = tgetstr("ed", &termptr);
	ei = tgetstr("ei", &termptr);
	ho = tgetstr("ho", &termptr);
	ic = tgetstr("ic", &termptr);
	il = tgetstr("al", &termptr);
	im = tgetstr("im", &termptr);
	ip = tgetstr("ip", &termptr);
	nd = tgetstr("nd", &termptr);
	vb = tgetstr("vb", &termptr);
	so = tgetstr("so", &termptr);	/* inverse video on */
	se = tgetstr("se", &termptr);	/* inverse video off */
	co = tgetnum("co");
	li = tgetnum("li");	

}

done ()
{
	(void) unlink (tempfile);
	if (termchanged)
#ifdef USG
		(void) ioctl (0, TCSETAW, &osbuf);
#else
		(void) ioctl (0, TIOCSETP, &osbuf);
#endif
	exit (0);
}

#ifndef USG
onstop(signo)
int signo;
{
	(void) ioctl (0, TIOCSETP, &osbuf);
	(void) signal(signo, SIG_DFL);
	(void) kill(0, signo);
	/* stop here until continued */
	(void) signal(signo, onstop);
	(void) ioctl (0, TIOCSETP, &sbuf);
}

stop ()
{
	onstop (SIGTSTP);
}
#endif

shellescape (buf)
char *buf;
{
#ifdef USG
#ifdef SOLARIS
  struct sigaction act;

        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;

	(void) ioctl (0, TCSETAW, &osbuf);
        act.sa_handler= (void (*)()) SIG_IGN;
        (void) sigaction(SIGINT, &act, NULL);
        (void) sigaction(SIGQUIT, &act, NULL);
#else
	(void) ioctl (0, TCSETAW, &osbuf);
	(void) signal (SIGINT, SIG_IGN);
	(void) signal (SIGQUIT, SIG_IGN);
#endif
#else
	(void) ioctl (0, TIOCSETP, &osbuf);
	(void) signal (SIGINT, 1);
	(void) signal (SIGQUIT, 1);
	(void) signal(SIGTTIN, SIG_DFL);
	(void) signal(SIGTTOU, SIG_DFL);
	(void) signal(SIGTSTP, SIG_DFL);
#endif

	(void) system (buf);

#ifndef USG
	(void) signal(SIGTTIN, onstop);
	(void) signal(SIGTTOU, onstop);
	(void) signal(SIGTSTP, onstop);
#endif
#ifdef SOLARIS
       act.sa_handler= (void (*)()) SIG_DFL;
       (void) sigaction(SIGQUIT, &act, NULL);
#else
	(void) signal (SIGINT, done);
	(void) signal (SIGQUIT, SIG_DFL);
#endif
#ifdef USG
	(void) ioctl (0, TCSETAW, &sbuf);
#else
	(void) ioctl (0, TIOCSETP, &sbuf);
#endif
	printf ("\n-- Type space to continue --");
	getchar ();
}
