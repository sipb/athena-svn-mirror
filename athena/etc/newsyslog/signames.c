/*
 * 	signals.c -- deal with concerting signal names to numbers and back.
 *
 * 	$Source: /afs/dev.mit.edu/source/repository/athena/etc/newsyslog/signames.c,v $
 * 	$Author: ghudson $
 */

#ifndef lint
static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/newsyslog/signames.c,v 1.2 1996-05-07 19:55:17 ghudson Exp $";
#endif

#include <string.h>
#include <sys/signal.h>
#include "config.h"
#include "signames.h"

struct signal_map {
  char *name;
  int number;
};

/*  Note: this contains only the signals described in the POSIX.1
 *  programmer's guide, because right now I don't care about any others.
 *  If someone else decides they do, they can add them themselves. =)
 */

static const 
struct signal_map signals[] = {
#ifdef SIGHUP
  {"HUP",	SIGHUP},
#endif
#ifdef SIGINT
  {"INT",	SIGINT},
#endif
#ifdef SIGQUIT
  {"QUIT",	SIGQUIT},
#endif
#ifdef SIGILL
  {"ILL",	SIGILL},
#endif
#ifdef SIGABRT
  {"ABRT",	SIGABRT},
#endif
#ifdef SIGFPE
  {"FPE",	SIGFPE},
#endif
#ifdef SIGKILL
  {"KILL",	SIGKILL},
#endif
#ifdef SIGBUS
  {"BUS",	SIGBUS},
#endif
#ifdef SIGSEGV
  {"SEGV",	SIGSEGV},
#endif
#ifdef SIGPIPE
  {"PIPE",	SIGPIPE},
#endif
#ifdef SIGALRM
  {"ALRM",	SIGALRM},
#endif
#ifdef SIGTERM
  {"TERM",	SIGTERM},
#endif
#ifdef SIGUSR1
  {"USR1",	SIGUSR1},
#endif
#ifdef SIGUSR2
  {"USR2",	SIGUSR2},
#endif
#ifdef SIGCHLD
  {"CHLD",	SIGCHLD},
#endif
#ifdef SIGPWR
  {"PWR",	SIGPWR},    /* this is not POSIX.1, but I like it. =) */
#endif
#ifdef SIGSTOP
  {"STOP",	SIGSTOP},
#endif
#ifdef SIGTSTP
  {"TSTP",	SIGTSTP},
#endif
#ifdef SIGCONT
  {"CONT",	SIGCONT},
#endif
#ifdef SIGTTIN
  {"TTIN",	SIGTTIN},
#endif
#ifdef SIGTTOU
  {"TTOU",	SIGTTOU},
#endif
  {NULL,	0}
};

/* signal_name() returns the name for a given signal, or NULL if unknown. */
char* signal_name (int signum)
{
  int i;

  for (i=0; signals[i].name; i++) {
    if (signum == signals[i].number)
      return signals[i].name;
  }
  return NULL;
}

/* signal_number() returns the number for a given signal, or 0 if unknown. */
int signal_number (char* name)
{
  int i;

  for (i=0; signals[i].name; i++) {
    if (!strcasecmp(name, signals[i].name))
      return signals[i].number;
  }
  return 0;
}

/* fprint_signal_name() prints out the signal name, or "signal ##". */
void fprint_signal_name(FILE *f, int signum)
{
  char *name = signal_name(signum);

  if (name)
    fprintf(f, name);
  else
    fprintf(f, "signal %d", signum);
}
