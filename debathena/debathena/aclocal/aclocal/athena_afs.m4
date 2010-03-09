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

dnl This file provides local macros for packages which use the AFS
dnl libraries.  The public macros are:
dnl	ATHENA_AFS
dnl		Sets AFS_LIBS and defines HAVE_AFS if AFS used.  Pass
dnl		in an argument giving the desired AFS libraries;
dnl		AFS_LIBS will be set to that value if AFS is found.
dnl		AFS_DIR will be set to the prefix given.
dnl	ATHENA_AFS_REQUIRED
dnl		Generates error if AFS libraries not found.  AFS_DIR
dnl		will be set to the prefix given.
dnl
dnl All of the macros may extend CPPFLAGS and LDFLAGS to let the
dnl compiler find the requested libraries.  Put ATHENA_UTIL_COM_ERR
dnl and ATHENA_UTIL_SS before ATHENA_AFS or ATHENA_AFS_REQUIRED; there
dnl is a com_err library in the AFS libraries which requires -lutil.

dnl ----- AFS -----

AC_DEFUN([ATHENA_AFS_CHECK],
[AC_SEARCH_LIBS(insque, compat)
AC_SEARCH_LIBS(gethostbyname, nsl)
AC_SEARCH_LIBS(socket, socket)
AC_SEARCH_LIBS(res_send, resolv)
if test "$afs" != yes; then
	CPPFLAGS="$CPPFLAGS -I$afs/include"
	LDFLAGS="$LDFLAGS -L$afs/lib -L$afs/lib/afs"
fi
AC_SEARCH_LIBS(pioctl, sys, :, [AC_MSG_ERROR(AFS libraries not found)],
	       -lrx -llwp -lsys -lafsutil)
AFS_DIR=$afs
AC_SUBST(AFS_DIR)])

dnl Specify desired AFS libraries as a parameter.
AC_DEFUN([ATHENA_AFS],
[AC_ARG_WITH(afs,
	[  --with-afs=PREFIX       Use AFS libraries],
	[afs="$withval"], [afs=no])
if test "$afs" != no; then
	ATHENA_AFS_CHECK
	AFS_LIBS=$1
	AC_DEFINE(HAVE_AFS)
fi
AC_SUBST(AFS_LIBS)])

AC_DEFUN([ATHENA_AFS_REQUIRED],
[AC_ARG_WITH(afs,
	[  --with-afs=PREFIX       Specify location of AFS libraries],
	[afs="$withval"], [afs=/usr/afsws])
if test "$afs" != no; then
	ATHENA_AFS_CHECK
else
	AC_MSG_ERROR(This package requires AFS libraries.)
fi])
