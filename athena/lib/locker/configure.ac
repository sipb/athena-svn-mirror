#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([liblocker], [10.1], [debathena@mit.edu])
AM_INIT_AUTOMAKE([foreign -Wall])
AM_MAINTAINER_MODE([enable])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/afs.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
# automake 1.12 seems to require this, but automake 1.11 doesn't recognize it
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
LT_INIT

# Checks for libraries.
RRA_LIB_KRB5
RRA_LIB_KAFS

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stddef.h stdlib.h string.h sys/mount.h sys/param.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UID_T
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_GETGROUPS
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([alarm dup2 ftruncate getcwd gethostbyaddr gethostbyname inet_ntoa memmove memset mkdir regcomp rmdir strchr strcspn strdup strerror strncasecmp strrchr strtol strtoul uname])

# More stuff
PKG_CHECK_MODULES([HESIOD], [hesiod])
AC_SUBST([HESIOD_CFLAGS])
AC_SUBST([HESIOD_LIBS])
PKG_CHECK_MODULES([ZEPHYR], [zephyr])
AC_SUBST([ZEPHYR_CFLAGS])
AC_SUBST([ZEPHYR_LIBS])

AC_CONFIG_FILES([Makefile
                 src/Makefile
		 src/liblocker.pc
		 man/Makefile
])
AC_OUTPUT
