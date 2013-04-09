#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([libathdir], [10.0.2], [debathena@mit.edu])
AM_INIT_AUTOMAKE([foreign -Wall])
AM_MAINTAINER_MODE([enable])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/athdir.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
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
AC_DEFINE_UNQUOTED([ATHSYS], ["$ATHENA_SYS"], [Define the Athena System Version])

AC_ARG_WITH([hosttype],
    [AS_HELP_STRING([--with-hosttype],
                    [machtype for compatibility])],
    [
	case $withval in
	     linux)	withval='"linux"';;
	     sun4)    	withval='"sun4"';;
	     inbsd)	withval='"inbsd"';;
	     yes)	withval='"linux"';;
	     no)	withval=NULL;;
	     *)		AC_MSG_ERROR([invalid hosttype specified]);;
	esac
	hosttype=$withval
    ],
    [hosttype='"linux"'])
AC_DEFINE_UNQUOTED([HOSTTYPE], [$hosttype], [Define the type of the Athena System])


AC_CONFIG_FILES([Makefile
                 src/Makefile
		 src/athdir.pc
])
AC_OUTPUT
