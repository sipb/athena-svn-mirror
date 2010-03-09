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

dnl This file provides local macros for packages which use com_err.
dnl The public macros are:
dnl	ATHENA_UTIL_COM_ERR
dnl		Generates error if com_err not found.
dnl
dnl All of the macros may extend CPPFLAGS and LDFLAGS to let the
dnl compiler find the requested libraries.  Put ATHENA_UTIL_COM_ERR
dnl and ATHENA_UTIL_SS before ATHENA_AFS or ATHENA_AFS_REQUIRED; there
dnl is a com_err library in the AFS libraries which requires -lutil.

dnl ----- com_err -----

AC_DEFUN([ATHENA_UTIL_COM_ERR],
[AC_ARG_WITH(com_err,
	[  --with-com_err=PREFIX   Specify location of com_err],
	[com_err="$withval"], [com_err=yes])
if test "$com_err" != no; then
	if test "$com_err" != yes; then
		CPPFLAGS="$CPPFLAGS -I$com_err/include"
		LDFLAGS="$LDFLAGS -L$com_err/lib"
	fi
	AC_SEARCH_LIBS(com_err, com_err, ,
		       [AC_MSG_ERROR(com_err library not found)])
else
	AC_MSG_ERROR(This package requires com_err.)
fi])
