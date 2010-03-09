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

dnl This file provides local macros for packages which use Kerberos
dnl version 5. The public macros are:
dnl	ATHENA_KRB5
dnl		Sets KRB5_LIBS and defines HAVE_KRB5 if krb5 used.
dnl	ATHENA_KRB5_REQUIRED
dnl		Generates error if krb5 not found.
dnl
dnl All of the macros may extend CPPFLAGS and LDFLAGS to let the
dnl compiler find the requested libraries.  Put ATHENA_UTIL_COM_ERR
dnl and ATHENA_UTIL_SS before ATHENA_AFS or ATHENA_AFS_REQUIRED; there
dnl is a com_err library in the AFS libraries which requires -lutil.

dnl ----- Kerberos 5 -----

AC_DEFUN([ATHENA_KRB5_CHECK],
[AC_REQUIRE([AC_CANONICAL_TARGET])
AC_SEARCH_LIBS(gethostbyname, nsl)
AC_SEARCH_LIBS(socket, socket)
AC_SEARCH_LIBS(compile, gen)
if test "$krb5" != yes; then
	CPPFLAGS="$CPPFLAGS -I$krb5/include"
	LDFLAGS="$LDFLAGS -L$krb5/lib"
fi
AC_SEARCH_LIBS(krb5_init_context, krb5, :,
	       [AC_MSG_ERROR(Kerberos 5 libraries not found)],
	       -lk5crypto -lcom_err)])

AC_DEFUN([ATHENA_KRB5],
[AC_ARG_WITH(krb5,
	[  --with-krb5=PREFIX      Use Kerberos 5],
	[krb5="$withval"], [krb5=no])
if test "$krb5" != no; then
	ATHENA_KRB5_CHECK
	KRB5_LIBS="-lkrb5 -lk5crypto -lcom_err"
	if test "$KRB5_LIBS" != "" ; then
		case "$target_os" in
		darwin*) KRB5_LIBS="$KRB5_LIBS -framework Kerberos"
		esac
	fi
	AC_DEFINE(HAVE_KRB5)
fi
AC_SUBST(KRB5_LIBS)])

AC_DEFUN([ATHENA_KRB5_REQUIRED],
[AC_ARG_WITH(krb5,
	[  --with-krb5=PREFIX      Specify location of Kerberos 5],
	[krb5="$withval"], [krb5=yes])
if test "$krb5" != no; then
	ATHENA_KRB5_CHECK
else
	AC_MSG_ERROR(This package requires Kerberos 5.)
fi])
