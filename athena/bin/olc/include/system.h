/*
 * This file is part of the OLC On-Line Consulting system.
 * It contains definitions for operating-system routines that don't
 * appear to be defined elsewhere, at least in BSD.
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: system.h,v 1.9 1999-03-06 16:48:30 ghudson Exp $
 */

#include <mit-copyright.h>

#include <sys/types.h>
#ifdef HAVE_KRB4
#include <krb.h>
#endif /* HAVE_KRB4 */

#ifdef HAVE_HESIOD
#include <hesiod.h>
#endif

/* This file used to define the prototypes for the functions listed
 * below.  Unfortunately, having prototypes breaks things if the
 * prototype is wrong, while not having them is usually just annoying.
 * Therefore, the prototypes have been removed.  On modern operating
 * systems, all system functions have prototypes in header files,
 * which should be included above.
 *
 * calloc, malloc, strcpy, strncpy, atoi, [bcopy], [bzero], close,
 * exit, free, getdtablezise, getopt, getsockopt, gethostbyname,
 * ioctl, listen, lseek, open, openlog, psignal(?), read,
 * getservbyname, setsockopt, shutdown, socket, strcmp, syslog, write
 */
