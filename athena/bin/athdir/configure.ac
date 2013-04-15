#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([athdir], [10.1], [debathena@mit.edu])
AM_INIT_AUTOMAKE([foreign -Wall])
AM_MAINTAINER_MODE([enable])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/athdir.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
# automake 1.12 seems to require this, but automake 1.11 doesn't recognize it
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
LT_INIT

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h sys/param.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([strchr])

# More stuff
PKG_CHECK_MODULES([LIBATHDIR], [athdir])
AC_SUBST([LIBATHDIR_CFLAGS])
AC_SUBST([LIBATHDIR_LIBS])

AC_CONFIG_FILES([Makefile
                 src/Makefile
		 man/Makefile
])
AC_OUTPUT
