#!/bin/sh
# $Id: do.sh,v 1.62 2001-03-26 20:31:43 ghudson Exp $

source=/mit/source
srvd=/afs/dev.mit.edu/system/$ATHENA_SYS/srvd-current
athtoolroot=$ATHTOOLROOT
contained=false
mungepath=true
n=""
maybe=""

usage() {
	echo "Usage: do [-cnp] [-s srcdir] [-d destdir] [-t toolroot]" 1>&2
	echo "	[dist|prepare|clean|all|check|install]" 1>&2
	exit 1
}

while getopts pcd:ns:t: opt; do
	case "$opt" in
	c)
		contained=true
		;;
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
	t)
		athtoolroot=$OPTARG
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

case $contained,$athtoolroot in
true,)
	athtoolroot=$srvd
	;;
true,?*)
	echo "The -t option and -c flag are mutually exclusive." 1>&2
	exit 1
	;;
esac

# Set up the build environment.
umask 022
export ATHENA_SYS ATHENA_SYS_COMPAT HOSTTYPE OS PATH M4
M4=$athtoolroot/usr/athena/bin/m4

# Determine proper ATHENA_SYS and ATHENA_SYS_COMPAT value.
case `uname -srm` in
"SunOS 5.8 sun4"*)
	ATHENA_SYS=sun4x_58
	ATHENA_SYS_COMPAT=sun4x_57:sun4x_56:sun4x_55:sun4m_54:sun4m_53
	ATHENA_SYS_COMPAT=${ATHENA_SYS_COMPAT}:sun4m_412
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
*)
	echo "Unrecognized system type, aborting." 1>&2
	exit 1
	;;
esac

# Determine platform name.
case `uname -sm` in
"SunOS sun4"*)
	HOSTTYPE=sun4
	;;
"IRIX "*)
	HOSTTYPE=sgi
	;;
"Linux "i?86)
	HOSTTYPE=linux
	;;
esac

savepath=$PATH

# Determine operating system, appropriate path, and compiler for use
# with plain Makefiles and some third-party packages.
case `uname -s` in
SunOS)
	OS=solaris
	LD_LIBRARY_PATH=/usr/openwin/lib export LD_LIBRARY_PATH
	PATH=/usr/ccs/bin:/usr/bin:/usr/ucb:/usr/openwin/bin:/usr/gcc/bin
	CC=gcc
	CXX=g++
	WARN_CFLAGS="-Wall -Wstrict-prototypes -Wmissing-prototypes"
	ERROR_CFLAGS=-Werror
	;;
IRIX)
	OS=irix
	PATH=/usr/bsd:/usr/bin:/usr/bin/X11
	CC=cc
	CXX=CC
	WARN_CFLAGS=-fullwarn
	ERROR_CFLAGS=-w2
	;;
Linux)
	OS=linux
	PATH=/usr/bin:/bin:/usr/X11R6/bin
	CC=cc
	CXX=g++
	WARN_CFLAGS="-Wall -Wstrict-prototypes -Wmissing-prototypes"
	ERROR_CFLAGS=-Werror
	;;
esac
PATH=$athtoolroot/usr/athena/bin:$PATH

if [ false = "$mungepath" ]; then
	PATH=$savepath
fi

if [ -n "$athtoolroot" ]; then
	ext=${LD_LIBRARY_PATH+:$LD_LIBRARY_PATH}
	LD_LIBRARY_PATH=$athtoolroot/usr/athena/lib$ext
	REPLISPDIR=$athtoolroot/usr/athena/share/rep/lisp
	REPEXECDIR=$athtoolroot/usr/athena/libexec/rep
	export LD_LIBRARY_PATH REPLISPDIR REPEXECDIR
	if [ irix = "$OS" ]; then
		# libtool likes to use this variable, which overrides
		# LD_LIBRARY_PATH.  Work around that.
		LD_LIBRARYN32_PATH=$LD_LIBRARY_PATH
		export LD_LIBRARYN32_PATH
	fi
fi

# Determine the Athena version
. $source/packs/build/version
ATHENA_MAJOR_VERSION=$major
ATHENA_MINOR_VERSION=$minor
ATHENA_PATCH_VERSION=$patch
export ATHENA_MAJOR_VERSION ATHENA_MINOR_VERSION ATHENA_PATCH_VERSION

# Determine if gmake is available. (It should be, unless this is a
# full build and we haven't built it yet.)
if [ -x $athtoolroot/usr/athena/bin/gmake ]; then
	MAKE=gmake
else
	MAKE=make
fi

export WARN_CFLAGS ERROR_CFLAGS CC CXX MAKE

if [ -r Makefile.athena ]; then
	export SRVD SOURCE COMPILER ATHTOOLROOT
	SRVD=$srvd
	SOURCE=$source
	COMPILER=$CC
	ATHTOOLROOT=$athtoolroot
	if [ dist = "$operation" ]; then
		export CONFIG_SITE XCONFIGDIR
		CONFIG_SITE=$source/packs/build/config.site
		XCONFIGDIR=$source/packs/build/xconfig
	fi
	$MAKE $n -f Makefile.athena "$operation"
elif [ -f configure.in ]; then
	export ATHTOOLROOT
	ATHTOOLROOT=$athtoolroot
	if [ -x configure.athena ]; then
		configure=configure.athena
	else
		configure=configure
	fi
	case $operation in
	dist)
		# Copy in support files and run autoconf if this is a
		# directory using the Athena build system.
		if [ -f config.do -o ! -f configure ]; then
			$maybe touch config.do
			$maybe rm -f mkinstalldirs install-sh config.guess
			$maybe rm -f config.sub aclocal.m4
			$maybe cp \
				"$source/packs/build/autoconf/mkinstalldirs" .
			$maybe cp "$source/packs/build/autoconf/install-sh" .
			$maybe cp "$source/packs/build/autoconf/config.guess" .
			$maybe cp "$source/packs/build/autoconf/config.sub" .
			$maybe cp "$source/packs/build/aclocal.m4" .
			$maybe autoconf \
				-m ${ATHTOOLROOT}/usr/athena/share/autoconf \
				|| exit 1
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
		$maybe imake "-Iconfig" -DUseInstalled \
			"-DTOPDIR=`pwd`" \
			"-DTOOLROOT=$athtoolroot" \
			"-DXCONFIGDIR=`pwd`/config"
		$maybe $MAKE Makefiles
		;;
	clean)
		$MAKE $n clean
		;;
	all)
		$MAKE $n includes depend  all \
		    TOP=`pwd` MAKE="$MAKE TOP=`pwd`" ATHTOOLROOT=$ATHTOOLROOT
		;;
	check)
		;;
	install)
		$MAKE $n install install.man "DESTDIR=$srvd" TOP=`pwd` \
			MAKE="$MAKE TOP=`pwd`" ATHTOOLROOT=$ATHTOOLROOT
		;;
	esac
elif [ -r Makefile ]; then
	case $operation in
	dist|prepare)
		;;
	clean)
		$MAKE $n clean "ATHTOOLROOT=$athtoolroot"
		;;
	all)
		$MAKE $n all CC="$CC" "ATHTOOLROOT=$athtoolroot"
		;;
	check)
		$MAKE $n check
		;;
	install)
		$MAKE $n install "DESTDIR=$srvd" "ATHTOOLROOT=$athtoolroot"
		;;
	esac
else
	echo Nothing to do in `pwd` 1>&2
	exit 1
fi
