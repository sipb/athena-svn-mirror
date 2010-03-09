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

dnl This file provides local macros for packages which use POSIX
dnl regexp support.  The public macros are:
dnl	ATHENA_REGEXP
dnl		Sets REGEX_LIBS if rx library used; ensures POSIX
dnl		regexp support.
dnl
dnl All of the macros may extend CPPFLAGS and LDFLAGS to let the
dnl compiler find the requested libraries.  Put ATHENA_UTIL_COM_ERR
dnl and ATHENA_UTIL_SS before ATHENA_AFS or ATHENA_AFS_REQUIRED; there
dnl is a com_err library in the AFS libraries which requires -lutil.

dnl ----- Regular expressions -----

AC_DEFUN([ATHENA_REGEXP],
[AC_ARG_WITH(regex,
	[  --with-regex=PREFIX     Use installed regex library],
	[regex="$withval"], [regex=no])
if test "$regex" != no; then
	if test "$regex" != yes; then
		CPPFLAGS="$CPPFLAGS -I$regex/include"
		LDFLAGS="$LDFLAGS -L$regex/lib"
	fi
	AC_SEARCH_LIBS(regcomp, regex, REGEX_LIBS=-lregex,
		       [AC_MSG_ERROR(regex library not found)])
else
	AC_CHECK_FUNC(regcomp, :,
		      [AC_MSG_ERROR(can't find POSIX regexp support)])
fi
AC_SUBST(REGEX_LIBS)])
