/*
 * 	signals.h -- deal with concerting signal names to numbers and back.
 */

#include <stdio.h>

/* signal_name() returns the name for a given signal, or NULL if unknown. */
char* signal_name (int signum);

/* signal_number() returns the number for a given signal, or 0 if unknown. */
int signal_number (char* name);

/* fprint_signal_name() prints out the signal name, or "signal ##". */
void fprint_signal_name (FILE *f, int signum);
