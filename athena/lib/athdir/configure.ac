dnl Process this file with autoconf to produce a configure script.
AC_INIT([libathdir], [10.0.2])
AC_CONFIG_SRCDIR([athdir.c])

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_MKDIR_P

AC_DEFINE_UNQUOTED([ATHSYS], ["$ATHENA_SYS"])
AC_DEFINE_UNQUOTED([HOSTTYPE_$ATHENA_HOSTTYPE])

AC_CONFIG_FILES([Makefile athdir.pc])
AC_OUTPUT
