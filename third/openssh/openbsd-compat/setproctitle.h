/* $Id: setproctitle.h,v 1.1.1.1 2001-11-15 19:24:15 ghudson Exp $ */

#ifndef _BSD_SETPROCTITLE_H
#define _BSD_SETPROCTITLE_H

#include "config.h"

#ifndef HAVE_SETPROCTITLE
void setproctitle(const char *fmt, ...);
#endif

#endif /* _BSD_SETPROCTITLE_H */
