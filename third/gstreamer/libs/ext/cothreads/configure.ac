dnl # configure.ac originally from GNU Pth and ngpt
dnl # hacked on for inclusion into the gstreamer project 20 Jan 2002 by Andy
dnl # Wingo <wingo@pobox.com>

dnl #                              ``Programming in Bourne-Shell
dnl #                                is a higher form of masochism.''
dnl #                                           -- Unknown

AC_INIT

AM_CONFIG_HEADER(cothreads/config-private.h)
AM_INIT_AUTOMAKE(cothreads, 0.2)

dnl Add parameters for aclocal
dnl (This must come after AM_INIT_AUTOMAKE, since it modifies ACLOCAL)
ACLOCAL="$ACLOCAL -I common/m4 -I m4 $ACLOCAL_FLAGS"

AC_MSG_PART(Build Tools)

AC_PROG_CC
AC_PROG_CPP
AC_SET_MAKE
AM_PROG_LIBTOOL

dnl decide on error flags
AS_COMPILER(COMPILER)
if test "x$COMPILER" = "xforte"; then
  GST_ERROR=
else
  if test "x$GST_CVS"="xyes"; then
    #GST_ERROR="-Wall -Werror"
    GST_ERROR="-Werror"
  else
    #GST_ERROR="-Wall"
    GST_ERROR=""
  fi
fi

PTH_CFLAGS="$PTH_CFLAGS $GST_ERROR"

dnl ##  Support for some special platform/compiler options
case "$PLATFORM:$CC" in
    *-sun-solaris2*:cc )
        #   shut-up harmless warnings caused by do-while macros on Solaris
        test ".$ac_cv_prog_gcc" = ".no" && PTH_CFLAGS="$PTH_CFLAGS -w"
        ;;
    *-*-sysv*uw[27]*:cc )
        #   shut-up harmless warnings caused by do-while macros on UnixWare
        PTH_CFLAGS="$PTH_CFLAGS -w"
        ;;
    *-hp-hpux*:cc )
        #   HPUX needs a few special options to find its ANSI C brain
        PTH_CFLAGS="$PTH_CFLAGS -Ae -O"
        ;;
    *-hp-hpux*:CC )
        #   HPUX outputs useless warnings
        PTH_CFLAGS="$PTH_CFLAGS -w"
        ;;
    *-sgi-irix6.2:cc )
        #   shut-up warnings caused by IRIX brain-dead 32/64 bit stuff
        PTH_LDFLAGS="$PTH_LDFLAGS -woff 85 -Wl,-woff,84 -Wl,-woff,85"
        ;;
    *-sgi-irix6.5.[2-9]:cc )
        #   since IRIX 6.5.2 the C compiler is smart enough,
        #   but the linker is still complaining, of course
        PTH_LDFLAGS="$PTH_LDFLAGS -woff 84,85,134"
        ;;
    *-sgi-irix6.5:cc )
        #   shut-up warnings caused by IRIX brain-dead 32/64 bit stuff
        PTH_CFLAGS="$PTH_CFLAGS -woff 1110,1048"
        PTH_LDFLAGS="$PTH_LDFLAGS -woff 84,85,134"
        ;;
    *-dec-osf4*:cc )
        #   make sure the standard compliant functions are used on Tru64/OSF1 4.x
        PTH_CFLAGS="$PTH_CFLAGS -std"
        ;;
    *-dec-osf5*:cc )
        #   make sure the standard compliant functions are used on Tru64/OSF1 5.x
        PTH_CFLAGS="$PTH_CFLAGS -std -D_XOPEN_SOURCE_EXTENDED"
        ;;
    *-*-isc*:* )
        #   ISC is brain-dead and needs a bunch of options to find its brain
        PTH_CPPFLAGS="$PTH_CPPFLAGS -D_POSIX_SOURCE";
        PTH_LIBS="$PTH_LIBS -lcposix -linet";
        ;;
    *-ibm-mvs*:cc | *-ibm-mvs*:c++ )
       mvs_extras='-W "c,float(ieee)" -W "c,langlvl(extended)" -W "c,expo,dll"'
       PTH_CFLAGS="$PTH_CFLAGS $mvs_extras"
       mvs_extras='-W "l,p,map" -W "l,map,list"'
       PTH_LDFLAGS="$PTH_LDFLAGS $mvs_extras"
       ;;
esac

AC_MSG_PART(Mandatory Platform Environment)

dnl # check for standard headers
AC_HEADER_STDC
# FIXME
AC_HAVE_HEADERS(dnl
   stdio.h stdlib.h stdarg.h string.h signal.h unistd.h setjmp.h fcntl.h dnl
   errno.h sys/types.h)
AC_CHECK_FUNCTIONS(dnl
   sigaction sigprocmask sigpending sigsuspend)
AC_BEGIN_DECISION([mandatory system headers and functions])
AC_IFALLYES(dnl
   header:stdio.h header:stdlib.h header:stdarg.h header:string.h dnl
   header:signal.h header:unistd.h header:setjmp.h header:fcntl.h header:errno.h dnl
   header:sys/types.h dnl
   func:sigaction func:sigprocmask dnl
   func:sigpending func:sigsuspend,
   AC_DECIDE(fine, [all fine]))
AC_END_DECISION

AC_MSG_PART(Optional Platform Environment)

dnl # check whether ss_base instead of ss_sp attribute exists
AC_CHECK_STRUCTATTR(ss_base, sigaltstack, sys/signal.h)
AC_CHECK_STRUCTATTR(ss_sp,   sigaltstack, sys/signal.h)

dnl ##
dnl ##  MACHINE CONTEXT IMPLEMENTATION
dnl ##

AC_MSG_PART(Machine Context Implementation)

dnl #
dnl #  1. determine possibilities
dnl #

dnl #  check for MCSC method
AC_CHECK_HEADERS(ucontext.h)
AC_CHECK_FUNCTIONS(makecontext swapcontext getcontext setcontext)
AC_CHECK_MCSC(mcsc=yes, mcsc=no)

dnl #  check for SJLJ method
AC_CHECK_HEADERS(signal.h)
AC_CHECK_FUNCTIONS(sigsetjmp siglongjmp setjmp longjmp _setjmp _longjmp)
AC_CHECK_FUNCTIONS(sigaltstack sigstack)
AC_CHECK_SJLJ(sjlj=yes, sjlj=no, sjlj_type)

dnl #
dnl #  2. make a general decision
dnl #

if test ".$mcsc" = .yes; then
    mctx_mth=mcsc
    mctx_dsp=sc
    mctx_stk=mc
elif test ".$sjlj" = .yes; then
    mctx_mth=sjlj
    mctx_dsp=$sjlj_type
    mctx_stk=none
    AC_IFALLYES(func:sigstack,    mctx_stk=ss)
    AC_IFALLYES(func:sigaltstack, mctx_stk=sas)
    case $mctx_dsp in
         sjljlx|sjljisc|sjljw32 ) mctx_stk=none
    esac
else
    AC_ERROR([no appropriate mctx method found])
fi

dnl #
dnl #  3. allow decision to be overridden by user
dnl #

AC_ARG_WITH(mctx-mth,dnl
[  --with-mctx-mth=ID      force mctx method      (mcsc,sjlj)],[
case $withval in
    mcsc|sjlj ) mctx_mth=$withval ;;
    * ) AC_ERROR([invalid mctx method -- allowed: mcsc,sjlj]) ;;
esac
])dnl
AC_ARG_WITH(mctx-dsp,dnl
[  --with-mctx-dsp=ID      force mctx dispatching (sc,ssjlj,sjlj,usjlj,sjlje,...)],[
case $withval in
    sc|ssjlj|sjlj|usjlj|sjlje|sjljlx|sjljisc|sjljw32 ) mctx_dsp=$withval ;;
    * ) AC_ERROR([invalid mctx dispatching -- allowed: sc,ssjlj,sjlj,usjlj,sjlje,sjljlx,sjljisc,sjljw32]) ;;
esac
])dnl
AC_ARG_WITH(mctx-stk,dnl
[  --with-mctx-stk=ID      force mctx stack setup (mc,ss,sas,...)],[
case $withval in
    mc|ss|sas|none ) mctx_stk=$withval ;;
    * ) AC_ERROR([invalid mctx stack setup -- allowed: mc,ss,sas,none]) ;;
esac
])dnl

dnl #
dnl #  4. determine a few additional details
dnl #

dnl #  whether sigaltstack has to use stack_t instead of struct sigaltstack
AC_CHECK_TYPEDEF(stack_t, signal.h)

dnl #  ibm - test whether we need a separate register stack (IA64).
AC_CHECK_NEED_FOR_SEPARATE_STACK(PTH_NEED_SEPARATE_REGISTER_STACK)
dnl #  ibm - end

dnl #  direction of stack grow
AC_CHECK_STACKGROWTH(COTHREADS_STACKGROWTH)
if test ".$ac_cv_check_stackgrowth" = ".down"; then
    PTH_STACK_GROWTH="down"
else
    PTH_STACK_GROWTH="up"
fi
AC_SUBST(PTH_STACK_GROWTH)

dnl #  how to specify stacks for the various functions
AC_CHECK_STACKSETUP(makecontext, pth_skaddr_makecontext, pth_sksize_makecontext)
AC_CHECK_STACKSETUP(sigaltstack, pth_skaddr_sigaltstack, pth_sksize_sigaltstack)
AC_CHECK_STACKSETUP(sigstack,    pth_skaddr_sigstack,    pth_sksize_sigstack)

dnl #  how to implement POSIX compliant sig{set,long}jmp()
case $mctx_dsp in [
    ssjlj )
        pth_sigjmpbuf='sigjmp_buf'
        pth_sigsetjmp='sigsetjmp(buf,1)'
        pth_siglongjmp='siglongjmp(buf,val)'
        ;;
    sjlj )
        pth_sigjmpbuf='jmp_buf'
        pth_sigsetjmp='setjmp(buf)'
        pth_siglongjmp='longjmp(buf,val)'
        ;;
    usjlj )
        pth_sigjmpbuf='jmp_buf'
        pth_sigsetjmp='_setjmp(buf)'
        pth_siglongjmp='_longjmp(buf,val)'
        ;;
    sjlje )
        pth_sigjmpbuf='jmp_buf'
        pth_sigsetjmp='setjmp(buf)'
        pth_siglongjmp='longjmp(buf,val)'
        ;;
    sjljlx|sjljisc|sjljw32 )
        pth_sigjmpbuf='sigjmp_buf'
        pth_sigsetjmp='sigsetjmp(buf,1)'
        pth_siglongjmp='siglongjmp(buf,val)'
        ;;
] esac
pth_sigjmpbuf="#define cothreads_sigjmpbuf $pth_sigjmpbuf"
pth_sigsetjmp="#define cothreads_sigsetjmp(buf) $pth_sigsetjmp"
pth_siglongjmp="#define cothreads_siglongjmp(buf,val) $pth_siglongjmp"
AC_SUBST(pth_sigjmpbuf)
AC_SUBST(pth_sigsetjmp)
AC_SUBST(pth_siglongjmp)

dnl #
dnl #  5. export the results
dnl #

COTHREADS_MCTX_MTH_use="#define COTHREADS_MCTX_MTH_use COTHREADS_MCTX_MTH_$mctx_mth"
COTHREADS_MCTX_DSP_use="#define COTHREADS_MCTX_DSP_use COTHREADS_MCTX_DSP_$mctx_dsp"
COTHREADS_MCTX_STK_use="#define COTHREADS_MCTX_STK_use COTHREADS_MCTX_STK_$mctx_stk"
AC_SUBST(COTHREADS_MCTX_MTH_use)
AC_SUBST(COTHREADS_MCTX_DSP_use)
AC_SUBST(COTHREADS_MCTX_STK_use)

PTH_MCTX_ID="$mctx_mth/$mctx_dsp/$mctx_stk"
AC_MSG_RESULT([decision on mctx implementation... ${TB}${PTH_MCTX_ID}${TN}])
AC_SUBST(PTH_MCTX_ID)

AC_MSG_VERBOSE([decided mctx method: $mctx_mth])
AC_MSG_VERBOSE([decided mctx dispatching: $mctx_dsp])
AC_MSG_VERBOSE([decided mctx stack setup: $mctx_stk])


####################################################################################
# End pth configure code, begin cothreads configure code
####################################################################################

AC_MSG_PART(Cothreads Package Configuration)

dnl test if we have pthread_attr_setstack; if not use the older calls
AC_CHECK_LIB(pthread, pthread_attr_setstack,
   AC_DEFINE(HAVE_PTHREAD_ATTR_SETSTACK, 1,
             [Defined if libpthread has pthread_attr_setstack ()]))

dnl test if we have posix_memalign; FreeBSD doesn't
AC_CHECK_FUNC(posix_memalign,
   AC_DEFINE(HAVE_POSIX_MEMALIGN, 1,
             [Defined if we have posix_memalign ()]))

# -O2 makes it slower, oddly enough...
COTHREADS_CFLAGS=

dnl Check for glib2
PKG_CHECK_MODULES(GLIB, glib-2.0,
  HAVE_GLIB=yes,HAVE_GLIB=no)
AC_SUBST(GLIB_LIBS)
AC_SUBST(GLIB_CFLAGS)
if test "x$HAVE_GLIB" = "xno"; then
  AC_MSG_ERROR([no glib2 found1])
fi

dnl GThread-compatible stack allocation
PKG_CHECK_MODULES(GTHREAD, glib-2.0 gthread-2.0,
  HAVE_GTHREADS=yes,HAVE_GTHREADS=no)
USE_GTHREADS=$HAVE_GTHREADS
AM_CONDITIONAL(USE_GTHREADS, test "x$USE_GTHREADS" = "xyes")
GTHREAD_CFLAGS="$GTHREAD_CFLAGS -DUSE_GTHREADS"
AC_SUBST(GTHREAD_LIBS)
AC_SUBST(GTHREAD_CFLAGS)

AC_ARG_ENABLE(cothreads-debug,
AC_HELP_STRING([--enable-cothreads-debug][enable debugging code]),
[case "${enableval}" in
  yes) COTHREADS_DEBUG="#define COTHREADS_DEBUG_ENABLED" ;;
    *) COTHREADS_DEBUG="" ;;
esac], 
[COTHREADS_DEBUG=""]) dnl Default value
AC_SUBST(COTHREADS_DEBUG)


COTHREADS_CFLAGS="$COTHREADS_CFLAGS -I\$(top_srcdir) $PTH_CFLAGS"
COTHREADS_LDFLAGS="$PTH_LDFLAGS"
AC_SUBST(COTHREADS_CFLAGS)
AC_SUBST(COTHREADS_LDFLAGS)

AC_MSG_PART(Output Substitution)

AC_OUTPUT(
Makefile
tests/Makefile
cothreads/Makefile
cothreads/config-public.h
)
