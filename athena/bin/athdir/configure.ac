dnl Process this file with autoconf to produce a configure script.
AC_INIT
AC_CONFIG_SRCDIR([athdir.c])

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MKDIR_P

PKG_CHECK_MODULES([ATHDIR], [athdir])

AC_MSG_NOTICE([Adding athdir CFLAGS ${ATHDIR_CFLAGS}])
CFLAGS="${ATHDIR_CFLAGS} ${CFLAGS}"
AC_MSG_NOTICE([Adding athdir LDFLAGS ${ATHDIR_LIBS}])
LDFLAGS="${ATHDIR_LIBS} ${LDFLAGS}"

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
