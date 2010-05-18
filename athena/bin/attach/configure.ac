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

PKG_CHECK_MODULES([ATHDIR], [athdir])
AC_MSG_NOTICE([Using CFLAGS for athdir ${ATHDIR_CFLAGS}])
AC_SUBST([ATHDIR_CFLAGS])
AC_MSG_NOTICE([Using LIBS for athdir ${ATHDIR_LIBS}])
AC_SUBST([ATHDIR_LIBS])

PKG_CHECK_MODULES([COM_ERR], [com_err])
AC_MSG_NOTICE([Adding com_err CFLAGS ${COM_ERR_CFLAGS}])
CFLAGS="${COM_ERR_CFLAGS} ${CFLAGS}"
AC_MSG_NOTICE([Adding com_err LIBS ${COM_ERR_LIBS}])
LIBS="${COM_ERR_LIBS} ${LIBS}"

ATHENA_REGEXP
ATHENA_AFS_REQUIRED
ATHENA_KRB4
ATHENA_HESIOD_REQUIRED

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
