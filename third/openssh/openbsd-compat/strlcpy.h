/* $Id: strlcpy.h,v 1.1.1.1 2001-11-15 19:24:15 ghudson Exp $ */

#ifndef _BSD_STRLCPY_H
#define _BSD_STRLCPY_H

#include "config.h"
#ifndef HAVE_STRLCPY
#include <sys/types.h>
size_t strlcpy(char *dst, const char *src, size_t siz);
#endif /* !HAVE_STRLCPY */

#endif /* _BSD_STRLCPY_H */
