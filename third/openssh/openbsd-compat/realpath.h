/* $Id: realpath.h,v 1.1.1.1 2001-11-15 19:24:15 ghudson Exp $ */

#ifndef _BSD_REALPATH_H
#define _BSD_REALPATH_H

#include "config.h"

#if !defined(HAVE_REALPATH) || defined(BROKEN_REALPATH)

char *realpath(const char *path, char *resolved);

#endif /* !defined(HAVE_REALPATH) || defined(BROKEN_REALPATH) */
#endif /* _BSD_REALPATH_H */
