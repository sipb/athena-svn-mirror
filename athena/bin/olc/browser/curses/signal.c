/*
 *	Win Treese, Jeff Jimenez
 *      Student Consulting Staff
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
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/signal.c,v $
 *	$Author: lwvanels $
 *      $Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/signal.c,v 1.4 1991-02-25 10:06:33 lwvanels Exp $
 */


#ifndef lint
static char *rcsid_cref_c = "$Header: ";
#endif	lint

#include <mit-copyright.h>

#include <signal.h>
#include <curses.h>
#include "cref.h"
#include "globals.h"

static  int handle_resize_event();
static  int handle_interrupt_event();
void init_signals();

void
init_signals()
{
  signal(SIGINT, handle_interrupt_event);
  signal(SIGWINCH, handle_resize_event);
}

static int handle_resize_event()
{
    struct winsize ws;
    int lines;
    int cols;

    signal(SIGWINCH, SIG_IGN);

    /*  Find out the new size.  */

    if (ioctl(fileno(stdout), TIOCGWINSZ, &ws) != -1)
      {
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
    crmode();
    echo();
    clear();
    
    /*  Refresh everything  */

    make_display();
    move(LINES - 3, 3);
    addstr((CREF) ? CREF_PROMPT : STOCK_PROMPT);
    clrtoeol();
    refresh();

    signal(SIGWINCH, handle_resize_event);
}



static int handle_interrupt_event( )
{
    signal(SIGINT, SIG_IGN);

    quit();
}
