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
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/signal.c,v $
 *	$Author: miki $
 *      $Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/signal.c,v 1.7 1994-04-08 13:27:44 miki Exp $
 */


#ifndef lint
#ifndef SABER
static char *rcsid_cref_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/signal.c,v 1.7 1994-04-08 13:27:44 miki Exp $";
#endif
#endif

#include <mit-copyright.h>

#include <signal.h>
#include <curses.h>
#include "cref.h"
#include "globals.h"

#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif

#ifdef VOID_SIGRET
static  void handle_resize_event P((int sig));
static  void handle_interrupt_event P((int sig));
#else
static  int handle_resize_event P((int sig));
static  int handle_interrupt_event P((int sig));
#endif
void init_signals P((void));

#undef P

void
init_signals()
{
#ifdef POSIX
  struct sigaction act;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
   act.sa_handler= (void (*)()) handle_interrupt_event;
   sigaction(SIGINT, &act, NULL);
   act.sa_handler= (void (*)()) handle_resize_event;
   sigaction(SIGWINCH, &act, NULL);
#else
  signal(SIGINT, handle_interrupt_event);
  signal(SIGWINCH, handle_resize_event);
#endif
}

#ifdef VOID_SIGRET
static void
#else
static int
#endif
handle_resize_event(sig)
     int sig;
{
    struct winsize ws;
    int lines;
    int cols;

#ifdef POSIX
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler= (void (*)()) SIG_IGN;
    sigaction(SIGWINCH, &act, NULL);
#else
    signal(SIGWINCH, SIG_IGN);
#endif

    /*  Find out the new size.  */

    if (ioctl(fileno(stdout), TIOCGWINSZ, &ws) == -1) {
      perror("cref: finding out new screen size");
#ifdef VOID_SIGRET
      return;
#else
      return(-1);
#endif
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
    crmode();
    echo();
    clear();
    
    /*  Refresh everything  */

    make_display();
    move(LINES - 3, 3);
    addstr((CREF) ? CREF_PROMPT : STOCK_PROMPT);
    clrtoeol();
    refresh();
#ifdef POSIX
    act.sa_handler= (void (*)()) handle_resize_event;
    sigaction(SIGWINCH, &act, NULL);
#else
    signal(SIGWINCH, handle_resize_event);
#endif
#ifdef VOID_SIGRET
    return;
#else
    return(0);
#endif
}



#ifdef VOID_SIGRET
static void
#else
static int
#endif
handle_interrupt_event(sig)
     int sig;
{
#ifdef POSIX
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler= (void (*)()) SIG_IGN;
    sigaction(SIGINT, &act, NULL);  
#else
    signal(SIGINT, SIG_IGN);
#endif

    quit();
}
