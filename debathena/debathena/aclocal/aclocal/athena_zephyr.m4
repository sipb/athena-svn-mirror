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

dnl This file provides local macros for packages which use Zephyr.
dnl The public macros are:
dnl	ATHENA_ZEPHYR
dnl		Sets ZEPHYR_LIBS and defines HAVE_ZEPHYR if zephyr
dnl		used.
dnl	ATHENA_ZEPHYR_REQUIRED
dnl		Generates error if zephyr not found.
dnl
dnl All of the macros may extend CPPFLAGS and LDFLAGS to let the
dnl compiler find the requested libraries.  Put ATHENA_UTIL_COM_ERR
dnl and ATHENA_UTIL_SS before ATHENA_AFS or ATHENA_AFS_REQUIRED; there
dnl is a com_err library in the AFS libraries which requires -lutil.

dnl ----- zephyr -----

AC_DEFUN([ATHENA_ZEPHYR_CHECK],
[if test "$zephyr" != yes; then
	CPPFLAGS="$CPPFLAGS -I$zephyr/include"
	LDFLAGS="$LDFLAGS -L$zephyr/lib"
fi
AC_SEARCH_LIBS(ZFreeNotice, zephyr, :, [AC_MSG_ERROR(zephyr not found)])])

AC_DEFUN([ATHENA_ZEPHYR],
[AC_ARG_WITH(zephyr,
	[  --with-zephyr=PREFIX      Use zephyr],
	[zephyr="$withval"], [zephyr=no])
if test "$zephyr" != no; then
	ATHENA_ZEPHYR_CHECK
	ZEPHYR_LIBS="-lzephyr"
	AC_DEFINE(HAVE_ZEPHYR)
fi
AC_SUBST(ZEPHYR_LIBS)])

AC_DEFUN([ATHENA_ZEPHYR_REQUIRED],
[AC_ARG_WITH(zephyr,
	[  --with-zephyr=PREFIX      Specify location of zephyr],
	[zephyr="$withval"], [zephyr=yes])
if test "$zephyr" != no; then
	ATHENA_ZEPHYR_CHECK
else
	AC_MSG_ERROR(This package requires zephyr.)
fi])
