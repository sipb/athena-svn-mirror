/*
 *	Win Treese, Jeff Jimenez
 *      Student Consulting Staff
 *	MIT Project Athena
 *
 *	Lucien Van Elsen
 *	MIT Project Athena
 *
 *	Copyright (c) 1985 by the Massachusetts Institute of Technology
 *
 *      Permission to use, copy, modify, and distribute this program
 *      for any purpose and without fee is hereby granted, provided
 *      that this copyright and permission notice appear on all copies
 *      and supporting documentation, the name of M.I.T. not be used
 *      in advertising or publicity pertaining to distribution of the
 *      program without specific prior permission, and notice be given
 *      in supporting documentation that copying and distribution is
 *      by permission of M.I.T.  M.I.T. makes no representations about
 *      the suitability of this software for any purpose.  It is pro-
 *      vided "as is" without express or implied warranty.
 */

/* This file is part of the CREF finder.  It contains the signal handling
 * functions.
 *
 *	$Id: signal.c,v 1.13 1999-06-10 18:41:14 ghudson Exp $
 */


#ifndef lint
#ifndef SABER
static char *rcsid_cref_c = "$Id: signal.c,v 1.13 1999-06-10 18:41:14 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <sys/types.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <curses.h>
#include <termios.h>
#include <browser/cref.h>
#include <browser/cur_globals.h>

#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif

static RETSIGTYPE handle_resize_event P((int sig));
static RETSIGTYPE handle_interrupt_event P((int sig));
void init_signals P((void));

#undef P

void
init_signals()
{
  struct sigaction act;

  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = handle_interrupt_event;
  sigaction(SIGINT, &act, NULL);
  act.sa_handler = handle_resize_event;
  sigaction(SIGWINCH, &act, NULL);
}

static RETSIGTYPE
handle_resize_event(sig)
     int sig;
{
    struct winsize ws;
    int lines;
    int cols;
    struct sigaction act;

    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler= SIG_IGN;
    sigaction(SIGWINCH, &act, NULL);

    /*  Find out the new size.  */

    if (ioctl(fileno(stdout), TIOCGWINSZ, &ws) == -1) {
      perror("cref: finding out new screen size");
      return;
    }
    else {
      if (ws.ws_row != 0)
	lines = ws.ws_row;
      if (ws.ws_col != 0)
	cols = ws.ws_col;
    }

    /*  Check that it is large enough  */

    noraw();
    endwin();
    LINES = lines;
    COLS = cols;
    if (! initscr())
      {
	fprintf(stderr, "%s: Can't initialize display, not enough memory.\n",
		Prog_Name);
	exit(ERROR);
      }
    cbreak();
    echo();
    clear();
    
    /*  Refresh everything  */

    make_display();
    move(LINES - 3, 3);
    addstr(Prompt);
    clrtoeol();
    refresh();

    act.sa_handler= handle_resize_event;
    sigaction(SIGWINCH, &act, NULL);

    return;
}



static RETSIGTYPE
handle_interrupt_event(sig)
     int sig;
{
    struct sigaction act;

    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler= SIG_IGN;
    sigaction(SIGINT, &act, NULL);  

    quit();
}
