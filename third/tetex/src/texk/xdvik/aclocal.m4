dnl ### Determine integer type to use for bitmaps

AC_DEFUN(XDVI_C_BITMAP_TYPE,
[AC_MSG_CHECKING(for integer type to use in bitmaps)
AC_CACHE_VAL(xdvi_cv_bitmap_type,
[AC_TRY_RUN(
[#include <stdio.h>
main()
{
  FILE *f=fopen("conftestval", "w");
  if (!f) exit(1);
  if ((sizeof(unsigned long) == 4 || sizeof(unsigned long) == 2)
    && sizeof(unsigned long) != sizeof(unsigned int))
      fprintf(f, "BMTYPE=long BMBYTES=%d\n", sizeof(unsigned long));
  if (sizeof(unsigned int) == 4 || sizeof(unsigned int) == 2)
    fprintf(f, "BMTYPE=int BMBYTES=%d\n", sizeof(unsigned int));
  else if (sizeof(unsigned short) == 4 || sizeof(unsigned short) == 2)
    fprintf(f, "BMTYPE=short BMBYTES=%d\n", sizeof(unsigned short));
  else fprintf(f, "BMTYPE=char BMBYTES=%d\n", sizeof(unsigned char));
  exit(0);
}],
xdvi_cv_bitmap_type="`cat conftestval`",
AC_MSG_ERROR(could not determine integer type for bitmap))])
eval "$xdvi_cv_bitmap_type"
AC_DEFINE_UNQUOTED(BMTYPE, $BMTYPE)
AC_DEFINE_UNQUOTED(BMBYTES, $BMBYTES)
AC_MSG_RESULT([unsigned $BMTYPE, size = $BMBYTES])])


dnl ### Check for whether the C compiler does string concatenation

AC_DEFUN(XDVI_CC_CONCAT,
[AC_CACHE_CHECK([whether C compiler supports string concatenation], xdvi_cc_concat,
[AC_TRY_COMPILE(
[#include <stdio.h>
], [puts("Testing" " string" " concatenation");
], xdvi_cc_concat=yes, xdvi_cc_concat=no)])
if test $xdvi_cc_concat = yes; then
  AC_DEFINE(HAVE_CC_CONCAT)
fi])


dnl ### Check for at-least-pretend Streams capability

AC_DEFUN(XDVI_SYS_STREAMS,
[AC_CACHE_CHECK([for stropts.h and isastream()], xdvi_cv_sys_streams,
[AC_TRY_LINK(
[#include <stropts.h>
], [#ifndef I_SETSIG
choke me
#else
isastream(0);
#endif], xdvi_cv_sys_streams=yes, xdvi_cv_sys_streams=no)])
if test $xdvi_cv_sys_streams = yes; then
  AC_DEFINE(HAVE_STREAMS)
fi])


dnl ### Check for poll()

AC_DEFUN(XDVI_FUNC_POLL,
[AC_CACHE_CHECK([for poll.h and poll()], xdvi_cv_func_poll,
[AC_TRY_LINK(
[#include <poll.h>
], [poll((struct pollfd *) 0, 0, 0);],
xdvi_cv_func_poll=yes, xdvi_cv_func_poll=no)])
if test $xdvi_cv_func_poll = yes; then
  AC_DEFINE(HAVE_POLL)
else
  AC_CHECK_HEADERS(sys/select.h select.h)
fi])


dnl ### Check for SunOS 4

AC_DEFUN(XDVI_SYS_SUNOS_4,
[AC_CACHE_CHECK([for SunOS 4], xdvi_cv_sys_sunos_4,
[case "`(uname -sr) 2>/dev/null`" in
"SunOS 4."*)
  xdvi_cv_sys_sunos_4=yes ;;
*) xdvi_cv_sys_sunos_4=no ;;
esac])
if test $xdvi_cv_sys_sunos_4 = yes; then
  AC_DEFINE(SUNOS4)
fi])

dnl dnl ### Check for vsnprintf() added by SU 2000/03/07
dnl AC_DEFUN(AC_FUNC_VSNPRINTF,
dnl [AC_CACHE_CHECK([for vsnprintf], xdvi_cv_vsnprintf,
dnl [AC_TRY_LINK(
dnl [#include <stdio.h>
dnl ], [(void)vsnprintf((char *)NULL, 0, (char *)NULL, NULL);],
dnl xdvi_cv_vsnprintf=yes, xdvi_cv_vsnprintf=no)])
dnl if test $xdvi_cv_vsnprintf = yes; then
dnl   AC_DEFINE(HAVE_VSNPRINTF)
dnl fi])

dnl ### Check for memicmp(), which some installations have in string.h
AC_DEFUN(AC_FUNC_MEMICMP,
[AC_CACHE_CHECK([for memicmp], xdvi_cv_memicmp,
[AC_TRY_LINK(
[#include <string.h>
], [(void)memicmp((char *)NULL, (char *)NULL, 0);],
xdvi_cv_memicmp=yes, xdvi_cv_memicmp=no)])
if test $xdvi_cv_memicmp = yes; then
  AC_DEFINE(HAVE_MEMICMP)
fi])

dnl dnl ### Check for realpath() added by SU 2002/04/10
dnl AC_DEFUN(AC_FUNC_REALPATH,
dnl [AC_CACHE_CHECK([for realpath], xdvi_cv_realpath,
dnl [AC_TRY_LINK(
dnl [#include <stdlib.h>
dnl ], [(void)realpath((const char *)NULL, NULL);],
dnl xdvi_cv_realpath=yes, xdvi_cv_realpath=no)])
dnl if test $xdvi_cv_realpath = yes; then
dnl   AC_DEFINE(HAVE_REALPATH)
dnl fi])


dnl SU: the following is copied from gnome/compiler-flags.m4: turn on warnings for gcc
dnl
dnl COMPILER_WARNINGS
dnl Turn on many useful compiler warnings
dnl For now, only works on GCC
AC_DEFUN([COMPILER_WARNINGS],[
  AC_ARG_ENABLE(compile-warnings, 
    [  --enable-compile-warnings=[no/minimum/yes/maximum]
                          Turn on compiler warnings.],,enable_compile_warnings=minimum)

  AC_MSG_CHECKING(what warning flags to pass to the C compiler)
  warnCFLAGS=
  if test "x$GCC" != xyes; then
    enable_compile_warnings=no
  fi

  if test "x$enable_compile_warnings" != "xno"; then
    if test "x$GCC" = "xyes"; then
      case " $CFLAGS " in
      *[\ \	]-Wall[\ \	]*) ;;
      *) warnCFLAGS="-W -Wall -Wunused" ;;
      esac

      ## -W is not all that useful.  And it cannot be controlled
      ## with individual -Wno-xxx flags, unlike -Wall
      if test "x$enable_compile_warnings" = "xyes"; then
        warnCFLAGS="$warnCFLAGS -Wmissing-prototypes -Wmissing-declarations"
      elif test "x$enable_compile_warnings" = "xmaximum"; then
      ## just turn on about everything:
      	warnCFLAGS="-Wall -Wunused -Wmissing-prototypes -Wmissing-declarations -Wimplicit -Wparentheses -Wreturn-type -Wswitch -Wtrigraphs -Wunused -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings"
      fi
    fi
  fi
  AC_MSG_RESULT($warnCFLAGS)

  AC_ARG_ENABLE(iso-c,
    [  --enable-iso-c          Try to warn if code is not ISO C ],,
    enable_iso_c=no)

  AC_MSG_CHECKING(what language compliance flags to pass to the C compiler)
  complCFLAGS=
  if test "x$enable_iso_c" != "xno"; then
    if test "x$GCC" = "xyes"; then
      case " $CFLAGS " in
      *[\ \	]-ansi[\ \	]*) ;;
      *) complCFLAGS="$complCFLAGS -ansi" ;;
      esac

       case " $CFLAGS " in
      *[\ \	]-pedantic[\ \	]*) ;;
      *) complCFLAGS="$complCFLAGS -pedantic" ;;
      esac
    fi
  fi
  AC_MSG_RESULT($complCFLAGS)
  if test "x$cflags_set" != "xyes"; then
    CFLAGS="$CFLAGS $warnCFLAGS $complCFLAGS"
    cflags_set=yes
    AC_SUBST(cflags_set)
  fi
])

