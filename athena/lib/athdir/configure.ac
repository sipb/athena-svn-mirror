#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([libathdir], [10.0.2], [debathena@mit.edu])
AM_INIT_AUTOMAKE([-Wall])
AM_MAINTAINER_MODE([enable])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/athdir.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
# automake 1.12 seems to require this, but automake 1.11 doesn't recognize it
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
LT_INIT
AC_PROG_LIBTOOL

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h sys/param.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([strchr])

# More stuff
AS_IF([test x"$ATHENA_SYS" = x], [AC_MSG_ERROR([ATHENA_SYS must be set])])
AS_IF([test x"$ATHENA_HOSTTYPE" = x], [AC_MSG_ERROR([ATHENA_HOSTTYPE must be set])])
AC_DEFINE_UNQUOTED([ATHSYS], ["$ATHENA_SYS"], [Define the Athena System Version])
AC_DEFINE_UNQUOTED([HOSTTYPE_$ATHENA_HOSTTYPE], [Define the type of the Athena System])


AC_CONFIG_FILES([Makefile
                 src/Makefile
		 src/athdir.pc
])
AC_OUTPUT
