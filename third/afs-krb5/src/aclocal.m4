dnl
dnl $Id: aclocal.m4,v 1.1.1.1 2003-02-13 00:14:48 zacheiss Exp $
dnl
dnl Some local macros that we need for autoconf
dnl
dnl AC_FIND_LIB(LIBRARY, FUNCTION, LIST-OF-DIRECTORIES [, ACTION-IF-FOUND
dnl		[, ACTION-IF-NOT-FOUND [, OTHER-LIBRARIES ]]])
AC_DEFUN(AC_FIND_LIB,
[AC_MSG_CHECKING([directories for -l$1])
dnl
dnl A lot of this is taken from AC_CHECK_LIB.  Note that we always check
dnl the "no directory" case first.
dnl
ac_lib_var=`echo $1['_']$2 | tr './+\055' '__p_'`
ac_save_LIBS="$LIBS"
AC_CACHE_VAL(ac_cv_lib_$ac_lib_var,
[for dir in "" $3
do
	ac_cache_save_LIBS="$LIBS"
	if test "X$dir" = "X"; then
		LIBS="$LIBS -l$1 $6"
	else
		LIBS="$LIBS -L$dir -l$1 $6"
	fi
	AC_TRY_LINK(dnl
ifelse([$2], [main], , dnl Avoid conflicting decl of main.
[/* Override any gcc2 internal prototype to avoid an error.  */
]ifelse(AC_LANG, CPLUSPLUS, [#ifdef __cplusplus
extern "C"
#endif
])dnl
[/* We use char because int might match the return type of a gcc2
    builtin and then its argument prototype would still apply.  */
char $2();
]),
	[$2()],
	if test "X$dir" = "X"; then
		eval "ac_cv_lib_$ac_lib_var=yes"
	else
		eval "ac_cv_lib_$ac_lib_var=$dir"
	fi
	break,
	eval "ac_cv_lib_$ac_lib_var=no")
	LIBS="$ac_cache_save_LIBS"
done
])dnl
LIBS="$ac_save_LIBS"
if eval "test \"`echo '$ac_cv_lib_'$ac_lib_var`\" != no"; then
  eval "dir=\"`echo '$ac_cv_lib_'$ac_lib_var`\""
  if test "$dir" = "yes"; then
	AC_MSG_RESULT([found])
  else
	AC_MSG_RESULT([found in $dir])
  fi
  ifelse([$4], ,
[
  if test "$dir" = "yes"; then
    LIBS="$LIBS -l$1"
  else 
    LIBS="$LIBS -L$dir -l$1"
  fi
], [$4])
else
  AC_MSG_RESULT(not found)
ifelse([$5], , , [$5
])dnl
fi
])
dnl
dnl Check for a tclConfig.sh
dnl
AC_DEFUN(AC_CHECK_TCLCONFIG, [
#
# Look for tclConfig.sh
#
AC_ARG_WITH(tclconfig, [  --with-tclconfig=PATH   directory containing tcl configuration (tclConfig.sh)],
	with_tclconfig=${withval})
AC_MSG_CHECKING([for Tcl configuration])
AC_CACHE_VAL(ac_cv_c_tclconfig,[
# First see if --with-tclconfig was specified
if test x"${with_tclconfig}" != x ; then
  if test -f "${with_tclconfig}/tclConfig.sh" ; then
    ac_cv_c_tclconfig=`(cd ${with_tclconfig}; pwd)`
  else
    AC_MSG_ERROR([${with_tclconfig} directory doesn't contain tclConfig.sh])
  fi
fi

if test x"${ac_cv_c_tclconfig}" = x ; then
  for dir in `ls -d ${prefix}/lib /usr/local/lib 2>/dev/null` ; do
    if test -f "$dir/tclConfig.sh" ; then
      ac_cv_c_tclconfig=`(cd $dir; pwd)`
      break
    fi
  done
fi
])
if test x"${ac_cv_c_tclconfig}" = x ; then
  AC_MSG_ERROR([Cannot find Tcl configuration definitions])
else
  TCLCONFIG=${ac_cv_c_tclconfig}/tclConfig.sh
  AC_MSG_RESULT(found $TCLCONFIG)
fi
])
dnl
dnl Load in our Tcl configuration
dnl
AC_DEFUN(AC_LOAD_TCLCONFIG, [
  . $TCLCONFIG

  CC="$TCL_CC"
  AC_SUBST(TCL_DEFS)
  AC_SUBST(TCL_SHLIB_LD)
  AC_SUBST(TCL_SHLIB_CFLAGS)
  AC_SUBST(TCL_SHLIB_SUFFIX)
])
dnl
dnl A lot of this was taken from AC_CHECK_HEADER
dnl
dnl AC_FIND_HEADER(HEADER-FILE, LIST-OF-DIRECTORIES, [ACTION-IF-FOUND
dnl		   [, ACTION-IF-NOT-FOUND]])
dnl
AC_DEFUN(AC_FIND_HEADER,
[dnl Do the transliteration at runtime so arg 1 can be a shell variable.
ac_safe=`echo "$1" | tr './\055' '___'`
AC_MSG_CHECKING([directories for $1])
ac_cache_save_CPPFLAGS="$CPPFLAGS"
AC_CACHE_VAL(ac_cv_header_$ac_safe,
[for dir in "" $2
do
	ac_cache_save_CPPFLAGS="$CPPFLAGS"
	if test x"${dir}" != x ; then
		CPPFLAGS="-I${dir} ${CPPFLAGS}"
	fi
	AC_TRY_CPP([#include <$1>], [if test x"${dir}" = x; then
	eval "ac_cv_header_$ac_safe=yes"
else
	eval "ac_cv_header_$ac_safe=$dir"
fi
break], eval "ac_cv_header_$ac_safe=no")
done
])dnl
CPPFLAGS="$ac_cache_save_CPPFLAGS"
if eval "test \"`echo '$ac_cv_header_'$ac_safe`\" != no"; then
  eval "dir=\"`echo '$ac_cv_header_'$ac_safe`\""
  if test "$dir" = "yes"; then
	AC_MSG_RESULT([found])
  else
	AC_MSG_RESULT([found in $dir])
  fi
ifelse([$3], ,
[
  if test "$dir" != "yes"; then
     CPPFLAGS="$CPPFLAGS -I$dir"
  fi
], [$3])
else
    AC_MSG_RESULT([not found])
ifelse([$4], , , [$4
])dnl
fi
])
dnl
dnl Extra macros taken from the Kerberos 5 distribution
dnl
define(CUSTOM_CONFIG,[dnl
WITH_CCOPTS dnl
WITH_CC dnl
WITH_LINKER dnl
WITH_LDOPTS dnl
WITH_CPPOPTS dnl
])dnl
dnl
dnl set $(CC) from --with-cc=value
dnl
define(WITH_CC,[
AC_ARG_WITH([cc],
[  --with-cc=COMPILER      select compiler to use])
AC_MSG_CHECKING(for C compiler)
if test "$with_cc" != ""; then
  if test "$ac_cv_prog_cc" != "" && test "$ac_cv_prog_cc" != "$with_cc"; then
    AC_MSG_ERROR(Specified compiler doesn't match cached compiler name;
	remove cache and try again.)
  else
    CC="$with_cc"
  fi
else
  AC_PROG_CC
fi
])dnl
dnl
dnl set $(LD) from --with-linker=value
dnl
define(WITH_LINKER,[
AC_ARG_WITH([linker],
[  --with-linker=LINKER    select linker to use],
AC_MSG_RESULT(LD=$withval)
LD=$withval,
if test -z "$LD" ; then LD=$CC; fi
[AC_MSG_RESULT(LD defaults to $LD)])dnl
AC_SUBST([LD])])dnl
dnl
dnl set $(CCOPTS) from --with-ccopts=value
dnl
define(WITH_CCOPTS,[
AC_ARG_WITH([ccopts],
[  --with-ccopts=CCOPTS    select compiler command line options],
AC_MSG_RESULT(CCOPTS is $withval)
CCOPTS=$withval
CFLAGS="$CFLAGS $withval",
CCOPTS=)dnl
AC_SUBST(CCOPTS)])dnl
dnl
dnl set $(LDFLAGS) from --with-ldopts=value
dnl
define(WITH_LDOPTS,[
AC_ARG_WITH([ldopts],
[  --with-ldopts=LDOPTS    select linker command line options],
AC_MSG_RESULT(LDFLAGS is $withval)
LDFLAGS=$withval,
LDFLAGS=)dnl
AC_SUBST(LDFLAGS)])dnl
dnl
dnl set $(CPPOPTS) from --with-cppopts=value
dnl
define(WITH_CPPOPTS,[
AC_ARG_WITH([cppopts],
[  --with-cppopts=CPPOPTS  select compiler preprocessor command line options],
AC_MSG_RESULT(CPPOPTS=$withval)
CPPOPTS=$withval
CPPFLAGS="$CPPFLAGS $withval",
[AC_MSG_RESULT(CPPOPTS defaults to $CPPOPTS)])dnl
AC_SUBST(CPPOPTS)])dnl
dnl
dnl Checking for POSIX setjmp/longjmp -- from Kerberos 5
dnl
define(CHECK_SETJMP,[
AC_FUNC_CHECK(sigsetjmp,
AC_MSG_CHECKING(for sigjmp_buf)
AC_CACHE_VAL(ac_cv_struct_sigjmp_buf,
[AC_TRY_COMPILE(
[#include <setjmp.h>],[sigjmp_buf x],
ac_cv_struct_sigjmp_buf=yes,ac_cv_struct_sigjmp_buf=no)])
AC_MSG_RESULT($ac_cv_struct_sigjmp_buf)
if test $ac_cv_struct_sigjmp_buf = yes; then
  AC_DEFINE(POSIX_SETJMP)
fi
)])dnl
dnl
dnl Check for POSIX signal handling -- from Kerberos 5
dnl
define(CHECK_SIGNALS,[
AC_FUNC_CHECK(sigprocmask,
AC_MSG_CHECKING(for sigset_t and POSIX_SIGNALS)
AC_CACHE_VAL(ac_cv_type_sigset_t,
[AC_TRY_COMPILE(
[#include <signal.h>],
[sigset_t x],
ac_cv_type_sigset_t=yes, ac_cv_type_sigset_t=no)])
AC_MSG_RESULT($ac_cv_type_sigset_t)
if test $ac_cv_type_sigset_t = yes; then
  AC_DEFINE(POSIX_SIGNALS)
fi
)])dnl
