/* signames.h -- deal with concerting signal names to numbers and back.
 *
 * $Header: /afs/dev.mit.edu/source/repository/athena/etc/newsyslog/signames.h,v 1.2 1996-04-29 18:53:10 bert Exp $
 */

#ifndef SIGNAMES_H
#define SIGNAMES_H

#include <stdio.h>

/* signal_name() returns the name for a given signal, or NULL if unknown. */
char* signal_name (int signum);

/* signal_number() returns the number for a given signal, or 0 if unknown. */
int signal_number (char* name);

/* fprint_signal_name() prints out the signal name, or "signal ##". */
void fprint_signal_name (FILE *f, int signum);

#endif /* SIGNAMES_H */
