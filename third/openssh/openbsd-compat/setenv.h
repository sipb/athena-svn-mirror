/* $Id: setenv.h,v 1.1.1.1 2001-11-15 19:24:15 ghudson Exp $ */

#ifndef _BSD_SETENV_H
#define _BSD_SETENV_H

#include "config.h"

#ifndef HAVE_SETENV

int setenv(register const char *name, register const char *value, int rewrite);

#endif /* !HAVE_SETENV */

#endif /* _BSD_SETENV_H */
