dnl Process this file with autoconf to produce a configure script.
AC_INIT
AC_CONFIG_SRCDIR([attachandrun.c])

test -z "$lbindir" && lbindir='${bindir}'
AC_SUBST([lbindir])

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MKDIR_P

PKG_CHECK_MODULES([ATHDIR], [athdir])

AC_MSG_NOTICE([Adding athdir CFLAGS ${ATHDIR_CFLAGS}])
CFLAGS="${ATHDIR_CFLAGS} ${CFLAGS}"
AC_MSG_NOTICE([Adding athdir LIBS ${ATHDIR_LIBS}])
LIBS="${ATHDIR_LIBS} ${LIBS}"

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
