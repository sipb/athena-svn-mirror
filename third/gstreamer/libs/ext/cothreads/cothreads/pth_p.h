/*
**  GNU Pth - The GNU Portable Threads
**  Copyright (c) 1999-2001 Ralf S. Engelschall <rse@engelschall.com>
**
**  This file is part of GNU Pth, a non-preemptive thread scheduling
**  library which can be found at http://www.gnu.org/software/pth/.
**
**  This library is free software; you can redistribute it and/or
**  modify it under the terms of the GNU Lesser General Public
**  License as published by the Free Software Foundation; either
**  version 2.1 of the License, or (at your option) any later version.
**
**  This library is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**  Lesser General Public License for more details.
**
**  You should have received a copy of the GNU Lesser General Public
**  License along with this library; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
**  USA, or contact Ralf S. Engelschall <rse@engelschall.com>.
**
**  pth_p.h: Pth private API definitions
*/

#ifndef _PTH_P_H_
#define _PTH_P_H_


/* mandatory system headers */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>

/* the main cothreads package */
#include "cothreads-private.h"

#ifndef PTH_DEBUG

#define pth_debug1(a1)                     /* NOP */
#define pth_debug2(a1, a2)                 /* NOP */
#define pth_debug3(a1, a2, a3)             /* NOP */
#define pth_debug4(a1, a2, a3, a4)         /* NOP */
#define pth_debug5(a1, a2, a3, a4, a5)     /* NOP */
#define pth_debug6(a1, a2, a3, a4, a5, a6) /* NOP */

#else

#define pth_debug1(a1)                     pth_debug(__FILE__, 1, a1, __LINE__)
#define pth_debug2(a1, a2)                 pth_debug(__FILE__, 2, a1, __LINE__, a2)
#define pth_debug3(a1, a2, a3)             pth_debug(__FILE__, 3, a1, __LINE__, a2, a3)
#define pth_debug4(a1, a2, a3, a4)         pth_debug(__FILE__, 4, a1, __LINE__, a2, a3, a4)
#define pth_debug5(a1, a2, a3, a4, a5)     pth_debug(__FILE__, 5, a1, __LINE__, a2, a3, a4, a5)
#define pth_debug6(a1, a2, a3, a4, a5, a6) pth_debug(__FILE__, 6, a1, __LINE__, a2, a3, a4, a5, a6)

#endif /* PTH_DEBUG */

/* enclose errno in a block */
#define errno_shield \
        for ( pth_errno_storage = errno, \
              pth_errno_flag = TRUE; \
              pth_errno_flag; \
              errno = pth_errno_storage, \
              pth_errno_flag = FALSE )

/* return plus setting an errno value */
#if defined(PTH_DEBUG)
#define return_errno(return_val,errno_val) \
        do { errno = (errno_val); \
             pth_debug4("return 0x%lx with errno %d(\"%s\")", \
                        (unsigned long)(return_val), (errno), strerror((errno))); \
             return (return_val); } while (0)
#else
#define return_errno(return_val,errno_val) \
        do { errno = (errno_val); return (return_val); } while (0)
#endif

/* make sure the scpp source extensions are skipped */
#define cpp 0
#define intern /**/

/* move intern variables to hidden namespace */
#define pth_errno_storage __pth_errno_storage
#define pth_errno_flag __pth_errno_flag

#ifdef G_HAVE_ISO_VARARGS

#define pth_debug(file, num, str, line, ...) g_message ("%s:%d " str, file, __VA_ARGS__)

#elif defined(G_HAVE_GNUC_VARARGS)

#define pth_debug(file, num, str, line, args...) g_message ("%s:%d " str, file, line, ##args)

#endif

/* prototypes for intern variables */
extern int pth_errno_storage;
extern int pth_errno_flag;

#endif /* _PTH_P_H_ */

