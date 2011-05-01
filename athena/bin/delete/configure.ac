dnl Process this file with autoconf to produce a configure script.
AC_INIT
AC_CONFIG_SRCDIR([delete.c])

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MKDIR_P

AC_CHECK_LIB([m], [log10])

ATHENA_UTIL_COM_ERR
ATHENA_AFS("-lsys -lrx -llwp -lafsutil")

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
