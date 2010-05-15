dnl Process this file with autoconf to produce a configure script.
AC_INIT
AC_CONFIG_SRCDIR([add.c])

test -z "$lbindir" && lbindir='${bindir}'
AC_SUBST([lbindir])
test -z "$lsbindir" && lsbindir='${sbindir}'
AC_SUBST([lsbindir])

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MKDIR_P

ATHENA_REGEXP
ATHENA_UTIL_COM_ERR
ATHENA_AFS_REQUIRED
ATHENA_KRB4
ATHENA_HESIOD_REQUIRED

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
