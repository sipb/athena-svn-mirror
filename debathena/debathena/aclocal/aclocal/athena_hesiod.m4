dnl Copyright 1996 by the Massachusetts Institute of Technology.
dnl
dnl Permission to use, copy, modify, and distribute this
dnl software and its documentation for any purpose and without
dnl fee is hereby granted, provided that the above copyright
dnl notice appear in all copies and that both that copyright
dnl notice and this permission notice appear in supporting
dnl documentation, and that the name of M.I.T. not be used in
dnl advertising or publicity pertaining to distribution of the
dnl software without specific, written prior permission.
dnl M.I.T. makes no representations about the suitability of
dnl this software for any purpose.  It is provided "as is"
dnl without express or implied warranty.

dnl This file provides local macros for packages which use Hesiod.
dnl The public macros are:
dnl	ATHENA_HESIOD
dnl		Sets HESIOD_LIBS and defines HAVE_HESIOD if Hesiod
dnl		used.
dnl	ATHENA_HESIOD_REQUIRED
dnl		Generates error if Hesiod not found.
dnl
dnl All of the macros may extend CPPFLAGS and LDFLAGS to let the
dnl compiler find the requested libraries.  Put ATHENA_UTIL_COM_ERR
dnl and ATHENA_UTIL_SS before ATHENA_AFS or ATHENA_AFS_REQUIRED; there
dnl is a com_err library in the AFS libraries which requires -lutil.

dnl ----- Hesiod -----

AC_DEFUN([ATHENA_HESIOD_CHECK],
[AC_SEARCH_LIBS(res_send, resolv)
if test "$hesiod" != yes; then
	CPPFLAGS="$CPPFLAGS -I$hesiod/include"
	LDFLAGS="$LDFLAGS -L$hesiod/lib"
fi
AC_SEARCH_LIBS(hes_resolve, hesiod, :,
	       [AC_MSG_ERROR(Hesiod library not found)])])

AC_DEFUN([ATHENA_HESIOD],
[AC_ARG_WITH(hesiod,
	[  --with-hesiod=PREFIX    Use Hesiod],
	[hesiod="$withval"], [hesiod=no])
if test "$hesiod" != no; then
	ATHENA_HESIOD_CHECK
	HESIOD_LIBS="-lhesiod"
	AC_DEFINE(HAVE_HESIOD)
fi
AC_SUBST(HESIOD_LIBS)])

AC_DEFUN([ATHENA_HESIOD_REQUIRED],
[AC_ARG_WITH(hesiod,
	[  --with-hesiod=PREFIX    Specify location of Hesiod],
	[hesiod="$withval"], [hesiod=yes])
if test "$hesiod" != no; then
	ATHENA_HESIOD_CHECK
else
	AC_MSG_ERROR(This package requires Hesiod.)
fi])
