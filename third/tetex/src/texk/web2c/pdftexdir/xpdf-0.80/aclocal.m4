dnl aclocal.m4 generated automatically by aclocal 1.3

dnl Copyright (C) 1994, 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
dnl This Makefile.in is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY, to the extent permitted by law; without
dnl even the implied warranty of MERCHANTABILITY or FITNESS FOR A
dnl PARTICULAR PURPOSE.

dnl ----- the following are from:
dnl ----- Steve Robbins <stever@jeff.cs.mcgill.ca>

dnl Convenience macros

dnl Configure-time switch with default
dnl
dnl Each switch defines an --enable-FOO and --disable-FOO option in
dnl the resulting configure script.
dnl
dnl Usage:
dnl smr_SWITCH(name, description, default, pos-def, neg-def)
dnl
dnl where:
dnl
dnl name	name of switch; generates --enable-name & --disable-name
dnl		options
dnl description	help string is set to this prefixed by "enable" or
dnl		"disable", whichever is the non-default value
dnl default	either "on" or "off"; specifies default if neither
dnl		--enable-name nor --disable-name is specified
dnl pos-def	a symbol to AC_DEFINE if switch is on (optional)
dnl neg-def	a symbol to AC_DEFINE if switch is off (optional)
dnl
AC_DEFUN(smr_SWITCH, [
    AC_MSG_CHECKING(whether to enable $2)
    AC_ARG_ENABLE(
	$1, 
	ifelse($3, on, 
	    [  --disable-[$1]    disable [$2]],
	    [  --enable-[$1]     enable [$2]]),
	[ if test "$enableval" = yes; then
	    AC_MSG_RESULT(yes)
	    ifelse($4, , , AC_DEFINE($4))
	else
	    AC_MSG_RESULT(no)
	    ifelse($5, , , AC_DEFINE($5))
	fi ],
        ifelse($3, on,
	   [ AC_MSG_RESULT(yes)
	     ifelse($4, , , AC_DEFINE($4)) ],
	   [ AC_MSG_RESULT(no) 
	    ifelse($5, , , AC_DEFINE($5))]))])


dnl Allow argument for optional libraries; wraps AC_ARG_WITH, to
dnl provide a "--with-foo-library" option in the configure script, where foo
dnl is presumed to be a library name.  The argument given by the user
dnl (i.e. "bar" in ./configure --with-foo-library=bar) may be one of three
dnl things:
dnl	* boolean (no, yes or blank): whether to use library or not
dnl	* file: assumed to be the name of the library
dnl	* directory: assumed to *contain* the library
dnl
dnl The argument is sanity-checked.  If all is well, two variables are
dnl set: "with_foo" (value is yes, no, or maybe), and "foo_LIBS" (value
dnl is either blank, a file, -lfoo, or '-L/some/dir -lfoo').  The idea
dnl is: the first tells you whether the library is to be used or not 
dnl (or the user didn't specify one way or the other) and the second
dnl to put on the command line for linking with the library.
dnl
dnl Usage:
dnl smr_ARG_WITHLIB(name, libname, description)
dnl
dnl name		name for --with argument ("foo" for libfoo)
dnl libname		(optional) actual name of library, 
dnl			if different from name
dnl description		(optional) used to construct help string
dnl
AC_DEFUN(smr_ARG_WITHLIB, [

ifelse($2, , smr_lib=[$1], smr_lib=[$2])

AC_ARG_WITH([$1]-library,
ifelse($3, , 
[  --with-$1-library[=PATH]   use $1 library],
[  --with-$1-library[=PATH]   use $1 library ($3)]),
[
    if test "$withval" = yes; then
	with_[$1]=yes
	[$1]_LIBS="-l${smr_lib}"
    elif test "$withval" = no; then
	with_[$1]=no
	[$1]_LIBS=
    else
	with_[$1]=yes
	if test -f "$withval"; then
	    [$1]_LIBS=$withval
	elif test -d "$withval"; then
	    [$1]_LIBS="-L$withval -l${smr_lib}"
	else
	    AC_MSG_ERROR(argument must be boolean, file, or directory)
	fi
    fi
], [   
    with_[$1]=maybe
    [$1]_LIBS="-l${smr_lib}"
])])


dnl Check if the include files for a library are accessible, and 
dnl define the variable "name_CFLAGS" with the proper "-I" flag for
dnl the compiler.  The user has a chance to specify the includes
dnl location, using "--with-foo-includes".
dnl
dnl This should be used *after* smr_ARG_WITHLIB *and* AC_CHECK_LIB are
dnl successful.
dnl
dnl Usage:
dnl smr_ARG_WITHINCLUDES(name, header, extra-flags)
dnl
dnl name		library name, MUST same as used with smr_ARG_WITHLIB
dnl header		a header file required for using the lib
dnl extra-flags		(optional) flags required when compiling the
dnl			header, typically more includes; for ex. X_CFLAGS
dnl
AC_DEFUN(smr_ARG_WITHINCLUDES, [

AC_ARG_WITH([$1]-includes,
[  --with-$1-includes=DIR  set directory for $1 headers],
[
    if test -d "$withval"; then
	[$1]_CFLAGS="-I${withval}"
    else
	AC_MSG_ERROR(argument must be a directory)
    fi])

dnl This bit of logic comes from autoconf's AC_PROG_CC macro.  We need
dnl to put the given include directory into CPPFLAGS temporarily, but
dnl then restore CPPFLAGS to its old value.
dnl
smr_test_CPPFLAGS="${CPPFLAGS+set}"
smr_save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS ${[$1]_CFLAGS}"

    ifelse($3, , , CPPFLAGS="$CPPFLAGS [$3]")
    AC_CHECK_HEADERS($2)

if test "$smr_test_CPPFLAGS" = set; then
    CPPFLAGS=$smr_save_CPPFLAGS
else
    CPPFLAGS=
fi
])


dnl Probe for an optional library.  This macro creates both
dnl --with-foo-library and --with-foo-includes options for the configure
dnl script.  If --with-foo-library is *not* specified, the default is to
dnl probe for the library, and use it if found.
dnl
dnl Usage:
dnl smr_CHECK_LIB(name, libname, desc, func, header, x-libs, x-flags)
dnl
dnl name	name for --with options
dnl libname	(optional) real name of library, if different from
dnl		above
dnl desc	(optional) short descr. of library, for help string
dnl func	function of library, to probe for
dnl header	(optional) header required for using library
dnl x-libs	(optional) extra libraries, if needed to link with lib
dnl x-flags	(optional) extra flags, if needed to include header files
dnl
AC_DEFUN(smr_CHECK_LIB,
[
ifelse($2, , smr_lib=[$1], smr_lib=[$2])
ifelse($5, , , smr_header=[$5])
smr_ARG_WITHLIB($1,$2,$3)
if test "$with_$1" != no; then
    AC_CHECK_LIB($smr_lib, $4, 
	smr_havelib=yes, smr_havelib=no, 
	ifelse($6, , ${$1_LIBS}, [${$1_LIBS} $6]))
    if test "$smr_havelib" = yes -a "$smr_header" != ""; then
	smr_ARG_WITHINCLUDES($1, $smr_header, $7)
	smr_safe=`echo "$smr_header" | sed 'y%./+-%__p_%'`
	if eval "test \"`echo '$ac_cv_header_'$smr_safe`\" != yes"; then
	    smr_havelib=no
	fi
    fi
    if test "$smr_havelib" = yes; then
	AC_MSG_RESULT(Using $1 library)
    else
	$1_LIBS=
	$1_CFLAGS=
	if test "$with_$1" = maybe; then
	    AC_MSG_RESULT(Not using $1 library)
	else
	    AC_MSG_WARN(Requested $1 library not found!)
	fi
    fi
fi])


# serial 1

# @defmac AC_PROG_CC_STDC
# @maindex PROG_CC_STDC
# @ovindex CC
# If the C compiler in not in ANSI C mode by default, try to add an option
# to output variable @code{CC} to make it so.  This macro tries various
# options that select ANSI C on some system or another.  It considers the
# compiler to be in ANSI C mode if it handles function prototypes correctly.
#
# If you use this macro, you should check after calling it whether the C
# compiler has been set to accept ANSI C; if not, the shell variable
# @code{am_cv_prog_cc_stdc} is set to @samp{no}.  If you wrote your source
# code in ANSI C, you can make an un-ANSIfied copy of it by using the
# program @code{ansi2knr}, which comes with Ghostscript.
# @end defmac

AC_DEFUN(AM_PROG_CC_STDC,
[AC_REQUIRE([AC_PROG_CC])
AC_BEFORE([$0], [AC_C_INLINE])
AC_BEFORE([$0], [AC_C_CONST])
dnl Force this before AC_PROG_CPP.  Some cpp's, eg on HPUX, require
dnl a magic option to avoid problems with ANSI preprocessor commands
dnl like #elif.
dnl FIXME: can't do this because then AC_AIX won't work due to a
dnl circular dependency.
dnl AC_BEFORE([$0], [AC_PROG_CPP])
AC_MSG_CHECKING(for ${CC-cc} option to accept ANSI C)
AC_CACHE_VAL(am_cv_prog_cc_stdc,
[am_cv_prog_cc_stdc=no
ac_save_CC="$CC"
# Don't try gcc -ansi; that turns off useful extensions and
# breaks some systems' header files.
# AIX			-qlanglvl=ansi
# Ultrix and OSF/1	-std1
# HP-UX			-Aa -D_HPUX_SOURCE
# SVR4			-Xc -D__EXTENSIONS__
for ac_arg in "" -qlanglvl=ansi -std1 "-Aa -D_HPUX_SOURCE" "-Xc -D__EXTENSIONS__"
do
  CC="$ac_save_CC $ac_arg"
  AC_TRY_COMPILE(
[#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
/* Most of the following tests are stolen from RCS 5.7's src/conf.sh.  */
struct buf { int x; };
FILE * (*rcsopen) (struct buf *, struct stat *, int);
static char *e (p, i)
     char **p;
     int i;
{
  return p[i];
}
static char *f (char * (*g) (char **, int), char **p, ...)
{
  char *s;
  va_list v;
  va_start (v,p);
  s = g (p, va_arg (v,int));
  va_end (v);
  return s;
}
int test (int i, double x);
struct s1 {int (*f) (int a);};
struct s2 {int (*f) (double a);};
int pairnames (int, char **, FILE *(*)(struct buf *, struct stat *, int), int, int);
int argc;
char **argv;
], [
return f (e, argv, 0) != argv[0]  ||  f (e, argv, 1) != argv[1];
],
[am_cv_prog_cc_stdc="$ac_arg"; break])
done
CC="$ac_save_CC"
])
if test -z "$am_cv_prog_cc_stdc"; then
  AC_MSG_RESULT([none needed])
else
  AC_MSG_RESULT($am_cv_prog_cc_stdc)
fi
case "x$am_cv_prog_cc_stdc" in
  x|xno) ;;
  *) CC="$CC $am_cv_prog_cc_stdc" ;;
esac
])

