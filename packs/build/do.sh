#!/bin/sh
# $Id: do.sh,v 1.72 2002-03-04 14:16:11 ghudson Exp $

source=/mit/source
srvd=/afs/dev.mit.edu/system/$ATHENA_SYS/srvd-current
contained=false
mungepath=true
n=""
maybe=""

usage() {
  echo "Usage: do [-np] [-s srcdir] [-d destdir]" 1>&2
  echo "	[dist|prepare|clean|all|check|install]" 1>&2
  exit 1
}

while getopts pcd:ns:t: opt; do
  case "$opt" in
  p)
    mungepath=false
    ;;
  d)
    srvd=$OPTARG
    ;;
  n)
    n=-n
    maybe=echo
    ;;
  s)
    source=$OPTARG
    ;;
  \?)
    usage
    ;;
  esac
done
shift `expr "$OPTIND" - 1`
operation=${1-all}

case "$operation" in
dist|prepare|clean|all|check|install)
  ;;
*)
  echo Unknown operation \"$operation\" 1>&2
  usage
  ;;
esac

# Set up the build environment.
umask 022
export ATHENA_SYS ATHENA_SYS_COMPAT ATHENA_HOSTTYPE OS PATH

# Determine proper ATHENA_SYS and ATHENA_SYS_COMPAT value.
case `uname -srm` in
"SunOS 5.8 sun4"*)
  ATHENA_SYS=sun4x_58
  ATHENA_SYS_COMPAT=sun4x_57:sun4x_56:sun4x_55:sun4m_54:sun4m_53:sun4m_412
  ;;
"SunOS 5.7 sun4"*)
  ATHENA_SYS=sun4x_57
  ATHENA_SYS_COMPAT=sun4x_56:sun4x_55:sun4m_54:sun4m_53:sun4m_412
  ;;
"SunOS 5.6 sun4"*)
  ATHENA_SYS=sun4x_56
  ATHENA_SYS_COMPAT=sun4x_55:sun4m_54:sun4m_53:sun4m_412
  ;;
"SunOS 5.5 sun4"*)
  ATHENA_SYS=sun4x_55
  ATHENA_SYS_COMPAT=sun4m_54:sun4m_53:sun4m_412
  ;;
"SunOS 5.4 sun4"*)
  ATHENA_SYS=sun4m_54
  ATHENA_SYS_COMPAT=sun4m_53:sun4m_412
  ;;
"IRIX 5.3 "*)
  ATHENA_SYS=sgi_53
  ATHENA_SYS_COMPAT=sgi_52
  ;;
"IRIX 6.2 "*)
  ATHENA_SYS=sgi_62
  ATHENA_SYS_COMPAT=sgi_53:sgi_52
  ;;
"IRIX 6.3 "*)
  ATHENA_SYS=sgi_63
  ATHENA_SYS_COMPAT=sgi_62:sgi_53:sgi_52
  ;;
"IRIX 6.5 "*)
  ATHENA_SYS=sgi_65
  ATHENA_SYS_COMPAT=sgi_63:sgi_62:sgi_53:sgi_52
  ;;
Linux\ 2.2.*\ i?86)
  ATHENA_SYS=i386_linux22
  ATHENA_SYS_COMPAT=i386_linux3:i386_linux2:i386_linux1
  ;;
Linux\ 2.4.*\ i?86)
  ATHENA_SYS=i386_linux24
  ATHENA_SYS_COMPAT=i386_linux22:i386_linux3:i386_linux2:i386_linux1
  ;;
*)
  echo "Unrecognized system type, aborting." 1>&2
  exit 1
  ;;
esac

# Determine platform name.
case `uname -sm` in
"SunOS sun4"*)
  ATHENA_HOSTTYPE=sun4
  ;;
"IRIX "*)
  ATHENA_HOSTTYPE=sgi
  ;;
"Linux "i?86)
  ATHENA_HOSTTYPE=linux
  ;;
esac

savepath=$PATH

# Determine operating system, appropriate path, and compiler for use
# with plain Makefiles and some third-party packages.
case `uname -s` in
SunOS)
  OS=solaris
  LD_LIBRARY_PATH=/usr/openwin/lib export LD_LIBRARY_PATH
  LD_RUN_PATH=/usr/athena/lib:/usr/openwin/lib export LD_RUN_PATH
  PATH=/usr/ccs/bin:/usr/bin:/usr/ucb:/usr/openwin/bin:/usr/gcc/bin
  ;;
IRIX)
  OS=irix
  PATH=/usr/bsd:/usr/bin:/usr/bin/X11:/usr/gcc/bin
  ;;
Linux)
  OS=linux
  LD_RUN_PATH=/usr/athena/lib export LD_RUN_PATH
  PATH=/usr/bin:/bin:/usr/X11R6/bin
  ;;
esac
CC=gcc
CXX=g++
WARN_CFLAGS="-Wall -Wstrict-prototypes -Wmissing-prototypes"
ERROR_CFLAGS=-Werror
PATH=/usr/athena/bin:$PATH

if [ false = "$mungepath" ]; then
  PATH=$savepath
fi

# Determine the Athena version
. $source/packs/build/version
ATHENA_MAJOR_VERSION=$major
ATHENA_MINOR_VERSION=$minor
if [ -z "$ATHENA_PATCH_VERSION" ]; then
  ATHENA_PATCH_VERSION=$patch
fi
export ATHENA_MAJOR_VERSION ATHENA_MINOR_VERSION ATHENA_PATCH_VERSION

# Determine if gmake is available. (It should be, unless this is a
# full build and we haven't built it yet.)
if [ -x /usr/athena/bin/gmake ]; then
  MAKE=gmake
else
  MAKE=make
fi

export WARN_CFLAGS ERROR_CFLAGS CC CXX MAKE

if [ dist = "$operation" ]; then
  # Force all source files to the same timestamp, to prevent third-party
  # build systems from thinking some are out of date with respect to others.
  find . ! -type l -exec touch -t `date +%Y%m%d%H%M.%S` {} \;
fi

if [ -r Makefile.athena ]; then
  export SRVD SOURCE COMPILER
  SRVD=$srvd
  SOURCE=$source
  COMPILER=$CC
  if [ dist = "$operation" ]; then
    export CONFIG_SITE XCONFIGDIR
    CONFIG_SITE=$source/packs/build/config.site
    XCONFIGDIR=$source/packs/build/xconfig
  fi
  $MAKE $n -f Makefile.athena "$operation"
elif [ -f configure.in -o -f configure.ac ]; then
  if [ -f configure.athena ]; then
    configure=configure.athena
  else
    configure=configure
  fi
  case $operation in
  dist)
    # Copy in support files and run autoconf if this is a directory using the
    # Athena build system.
    if [ -f config.do -o ! -f configure ]; then
      $maybe touch config.do
      $maybe rm -f mkinstalldirs install-sh config.guess
      $maybe rm -f config.sub aclocal.m4
      $maybe cp "$source/packs/build/autoconf/mkinstalldirs" .
      $maybe cp "$source/packs/build/autoconf/install-sh" .
      $maybe cp "$source/packs/build/autoconf/config.guess" .
      $maybe cp "$source/packs/build/autoconf/config.sub" .
      $maybe cp "$source/packs/build/aclocal.m4" .
      $maybe cat "$source/packs/build/libtool/libtool.m4" >> aclocal.m4
      $maybe cp "$source/packs/build/libtool/ltmain.sh" .
      $maybe autoconf || exit 1
    fi
    $maybe cp "$source/packs/build/config.site" config.site.athena
    ;;
  prepare)
    $maybe rm -f config.cache
    CONFIG_SITE=`pwd`/config.site.athena $maybe "./$configure"
  ;;
  clean)
    $MAKE $n clean
    ;;
  all)
    $MAKE $n all
    ;;
  check)
    $MAKE -n check >/dev/null 2>&1 && $MAKE $n check || true
    ;;
  install)
    $MAKE $n install "DESTDIR=$srvd"
    ;;
  esac
elif [ -r Imakefile ]; then
  case $operation in
  dist)
    $maybe mkdir -p config
    $maybe cp $source/packs/build/xconfig/README config
    $maybe cp $source/packs/build/xconfig/mkdirhier.sh config
    $maybe cp $source/packs/build/xconfig/rtcchack.bac config
    $maybe cp $source/packs/build/xconfig/site.def config
    $maybe cp $source/packs/build/xconfig/*.cf config
    $maybe cp $source/packs/build/xconfig/*.rules config
    $maybe cp $source/packs/build/xconfig/*.tmpl config
    ;;
  prepare)
    $maybe imake "-Iconfig" -DUseInstalled "-DTOPDIR=`pwd`" \
      "-DXCONFIGDIR=`pwd`/config"
    $maybe $MAKE Makefiles
    ;;
  clean)
    $MAKE $n clean
    ;;
  all)
    $MAKE $n includes depend all TOP=`pwd` MAKE="$MAKE TOP=`pwd`"
    ;;
  check)
    ;;
  install)
    $MAKE $n install install.man "DESTDIR=$srvd" TOP=`pwd` \
      MAKE="$MAKE TOP=`pwd`"
    ;;
  esac
elif [ -r Makefile ]; then
  case $operation in
  dist|prepare)
    ;;
  clean)
    $MAKE $n clean
    ;;
  all)
    $MAKE $n all CC="$CC"
    ;;
  check)
    $MAKE $n check
    ;;
  install)
    $MAKE $n install "DESTDIR=$srvd"
    ;;
  esac
else
  echo Nothing to do in `pwd` 1>&2
  exit 1
fi
