/* cothreads/config-private.h.  Generated by configure.  */
/* cothreads/config-private.h.in.  Generated from configure.ac by autoheader.  */
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
**  pth_acdef.h Autoconf defines
*/

#ifndef _PTH_ACDEF_H_
#define _PTH_ACDEF_H_


/* the custom Autoconf defines */
/* #undef HAVE_SIG_ATOMIC_T */
/* #undef HAVE_PID_T */
#define HAVE_STACK_T 1
/* #undef HAVE_SIZE_T */
/* #undef HAVE_SSIZE_T */
/* #undef HAVE_SOCKLEN_T */
/* #undef HAVE_NFDS_T */
/* #undef HAVE_OFF_T */
/* #undef HAVE_GETTIMEOFDAY_ARGS1 */
/* #undef HAVE_STRUCT_TIMESPEC */
/* #undef HAVE_SYS_READ */
/* #undef HAVE_POLLIN */
#define HAVE_SS_SP 1
/* #undef HAVE_SS_BASE */
/* #undef HAVE_LONGLONG */
/* #undef HAVE_LONGDOUBLE */
/* #undef PTH_NSIG */
/* #undef PTH_DMALLOC */
#define PTH_NEED_SEPARATE_REGISTER_STACK 0


/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the <errno.h> header file. */
#define HAVE_ERRNO_H 1

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Defined if we have posix_memalign () */
#define HAVE_POSIX_MEMALIGN 1

/* Defined if libpthread has pthread_attr_setstack () */
#define HAVE_PTHREAD_ATTR_SETSTACK 1

/* Define to 1 if you have the <setjmp.h> header file. */
#define HAVE_SETJMP_H 1

/* Define to 1 if you have the <signal.h> header file. */
#define HAVE_SIGNAL_H 1

/* Define to 1 if you have the <stdarg.h> header file. */
#define HAVE_STDARG_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdio.h> header file. */
#define HAVE_STDIO_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <ucontext.h> header file. */
#define HAVE_UCONTEXT_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Name of package */
#define PACKAGE "cothreads"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME ""

/* Define to the full name and version of this package. */
#define PACKAGE_STRING ""

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME ""

/* Define to the version of this package. */
#define PACKAGE_VERSION ""

/* defined if we need a separate stack */
#define PTH_NEED_SEPARATE_REGISTER_STACK 0

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Version number of package */
#define VERSION "0.2"

#endif /* _PTH_ACDEF_H_ */