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
dnl version 4. The public macros are:
dnl	ATHENA_KRB4
dnl		Sets KRB4_LIBS and defines HAVE_KRB4 if krb4 used.
dnl	ATHENA_KRB4_REQUIRED
dnl		Generates error if krb4 not found.  Sets KRB4_LIBS
dnl		otherwise.  (Special behavior because krb4 libraries
dnl		may be different if using krb4 compatibility libraries
dnl		from krb5.)
dnl
dnl All of the macros may extend CPPFLAGS and LDFLAGS to let the
dnl compiler find the requested libraries.  Put ATHENA_UTIL_COM_ERR
dnl and ATHENA_UTIL_SS before ATHENA_AFS or ATHENA_AFS_REQUIRED; there
dnl is a com_err library in the AFS libraries which requires -lutil.

dnl ----- Kerberos 4 -----

AC_DEFUN([ATHENA_KRB4_CHECK],
[AC_REQUIRE([AC_CANONICAL_TARGET])
AC_SEARCH_LIBS(gethostbyname, nsl)
AC_SEARCH_LIBS(socket, socket)
AC_SEARCH_LIBS(compile, gen)
if test "$krb4" != yes; then
	CPPFLAGS="$CPPFLAGS -I$krb4/include"
	if test -d "$krb4/include/kerberosIV"; then
		CPPFLAGS="$CPPFLAGS -I$krb4/include/kerberosIV"
	fi
	LDFLAGS="$LDFLAGS -L$krb4/lib"
fi
AC_CHECK_LIB(krb4, krb_rd_req,
	     [KRB4_LIBS="-lkrb4 -ldes425 -lkrb5 -lk5crypto -lcom_err"],
	     [AC_CHECK_LIB(krb, krb_rd_req,
			   [KRB4_LIBS="-lkrb -ldes"],
			   [AC_MSG_WARN(--with-krb4 specified but Kerberos 4 libraries not found)],
			   -ldes)],
	     -ldes425 -lkrb5 -lk5crypto -lcom_err)
if test "$KRB4_LIBS" != "" ; then
	case "$target_os" in
	darwin*) KRB4_LIBS="$KRB4_LIBS -framework Kerberos"
	esac
fi])

AC_DEFUN([ATHENA_KRB4],
[AC_ARG_WITH(krb4,
	[  --with-krb4=PREFIX      Use Kerberos 4],
	[krb4="$withval"], [krb4=no])
if test "$krb4" != no; then
	ATHENA_KRB4_CHECK
	if test "$KRB4_LIBS" != ""; then
		AC_DEFINE(HAVE_KRB4)
	fi
fi
AC_SUBST(KRB4_LIBS)])

AC_DEFUN([ATHENA_KRB4_REQUIRED],
[AC_ARG_WITH(krb4,
	[  --with-krb4=PREFIX      Specify location of Kerberos 4],
	[krb4="$withval"], [krb4=yes])
if test "$krb4" != no; then
	ATHENA_KRB4_CHECK
	if test "$KRB4_LIBS" = ""; then
		AC_MSG_ERROR(This package requires Kerberos 4.)
	fi
	AC_SUBST(KRB4_LIBS)
else
	AC_MSG_ERROR(This package requires Kerberos 4.)
fi])
