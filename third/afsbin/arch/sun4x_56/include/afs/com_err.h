/*
 * Header file for common error description library.
 *
 * Copyright 1988, Student Information Processing Board of the
 * Massachusetts Institute of Technology.
 *
 * For copyright and distribution info, see the documentation supplied
 * with this package.
 */

#ifndef __COM_ERR_H

#ifdef __STDC__
#ifndef __HIGHC__		/* gives us STDC but not stdarg */
#include <stdarg.h>
#else
#include <varargs.h>
#endif
/* ANSI C -- use prototypes etc */
void com_err (const char *, int32, const char *, ...);
char const *error_message (int32);
void (*com_err_hook) (const char *, int32, const char *, va_list);
void (*set_com_err_hook (void (*) (const char *, int32, const char *, va_list)))
    (const char *, int32, const char *, va_list);
void (*reset_com_err_hook ()) (const char *, int32, const char *, va_list);
#else
/* no prototypes */
void com_err ();
char *error_message ();
void (*com_err_hook) ();
void (*set_com_err_hook ()) ();
void (*reset_com_err_hook ()) ();
#endif

#define __COM_ERR_H
#endif /* ! defined(__COM_ERR_H) */
