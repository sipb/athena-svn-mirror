/*
 * conf.h
 *
 * Copyright 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Configuration info for operating system, hardware description,
 * language implementation, C library, etc.
 *
 * This file should be included in (almost) every file in the Kerberos
 * sources, and probably should *not* be needed outside of those
 * sources.  (How do we deal with /usr/include/des.h and
 * /usr/include/krb.h?)
 */

#ifndef CONF_H
#define	CONF_H

#include "mit-copyright.h"

/* appl/bsd/ *.c wants to see this, on all platforms... */
#define	KERBEROS	1

/* A few files want NDBM, though this is overridden by #undef in some
   platforms' OS configurations */
#define	NDBM	1

#include "osconf.h"

#ifdef SHORTNAMES
#include "names.h"
#endif

/*
 * Language implementation-specific definitions
 */

/* special cases */
#ifdef __HIGHC__
/* broken implementation of ANSI C */
#undef __STDC__
#endif

#ifndef __STDC__
#define const
#define volatile
#define signed
#ifndef PROTOTYPE
#define PROTOTYPE(p) ()
#endif
#else
#define PROTOTYPE(p) p
#endif

/*
 * A few checks to see that necessary definitions are included.
 */

/* byte order */

#ifndef MSBFIRST
#ifndef LSBFIRST
/* #error byte order not defined */
Error: byte order not defined.
#endif
#endif

/* machine size */
#ifndef BITS16
#ifndef BITS32
Error: how big is this machine anyways?
#endif
#endif

/* end of checks */

#endif /* CONF_H */
