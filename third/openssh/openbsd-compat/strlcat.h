/* $Id: strlcat.h,v 1.1.1.1 2001-11-15 19:24:15 ghudson Exp $ */

#ifndef _BSD_STRLCAT_H
#define _BSD_STRLCAT_H

#include "config.h"
#ifndef HAVE_STRLCAT
#include <sys/types.h>
size_t strlcat(char *dst, const char *src, size_t siz);
#endif /* !HAVE_STRLCAT */

#endif /* _BSD_STRLCAT_H */
