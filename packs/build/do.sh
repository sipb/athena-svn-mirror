#!/bin/sh
# $Id: do.sh,v 1.25 1999-02-19 17:13:41 danw Exp $

source=/mit/source
srvd=/srvd
contained=false
n=""
maybe=""
usage="do [-cn] [-s srcdir] [-d destdir] [prepare|clean|all|check|install]"

while getopts cd:ns: opt; do
	case "$opt" in
	c)
		contained=true
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
		echo "$usage"
		exit 1
		;;
	esac
done
shift `expr "$OPTIND" - 1`
operation=${1-all}

case $contained in
true)
	athtoolroot=$srvd
	;;
false)
	athtoolroot=""
	;;
esac

# Set up the build environment.
umask 022
export ATHENA_SYS ATHENA_SYS_COMPAT HOSTTYPE OS CONFIG_SITE PATH
CONFIG_SITE=$source/packs/build/config.site

# Determine proper ATHENA_SYS and ATHENA_SYS_COMPAT value.
case `uname -srm` in
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
Linux\ 2.2.*\ i?86)
	ATHENA_SYS=i386_linux22
	ATHENA_SYS_COMPAT=i386_linux3:i386_linux2:i386_linux1
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

# Determine operating system, appropriate path, and compiler for use
# with plain Makefiles and some third-party packages.
case `uname -s` in
SunOS)
	OS=solaris
	LD_LIBRARY_PATH=/usr/openwin/lib export LD_LIBRARY_PATH
	PATH=/usr/ccs/bin:$athtoolroot/usr/athena/bin:/usr/bin:/usr/ucb
	PATH=${PATH}:/usr/openwin/bin
	compiler="/usr/gcc/bin/gcc -DSOLARIS"
	;;
IRIX)
	OS=irix
	PATH=$athtoolroot/usr/athena/bin:/usr/bsd:/usr/bin:/usr/bin/X11
	compiler=cc
	;;
Linux)
	OS=linux
	PATH=$athtoolroot/usr/athena/bin:/usr/bin:/bin:/usr/X11R6/bin
	compiler=cc
	;;
esac

if [ -r Makefile.athena ]; then
	export SRVD SOURCE COMPILER CONFIGDIR XCONFIGDIR ATHTOOLROOT
	SRVD=$srvd
	SOURCE=$source
	COMPILER=$compiler
	CONFIGDIR=$source/packs/build/config
	XCONFIGDIR=$source/packs/build/xconfig
	ATHTOOLROOT=$athtoolroot
	make $n -f Makefile.athena "$operation"
elif [ -x configure ]; then
	export ATHTOOLROOT
	ATHTOOLROOT=$athtoolroot
	if [ -x configure.athena ]; then
		configure=configure.athena
	else
		configure=configure
	fi
	case $operation in
		prepare)	$maybe rm -f config.cache
				$maybe ./$configure ;;
		clean)		make $n clean ;;
		all)		make $n all ;;
		check)		;;
		install)	make $n install "DESTDIR=$srvd" ;;
	esac
elif [ -r Imakefile ]; then
	case $operation in
		prepare)
			$maybe imake "-I$source/packs/build/config" \
				-DUseInstalled "-DTOPDIR=$source/packs/build" \
				"-DTOOLROOT=$athtoolroot"
			$maybe make Makefiles
			$maybe make depend
			;;
		clean)		make $n clean ;;
		all)		make $n all ;;
		check)		;;
		install)	make $n install install.man "DESTDIR=$srvd" ;;
	esac
elif [ -r Makefile ]; then
	case $operation in
		prepare)	;;
		clean)		make $n clean "ATHTOOLROOT=$athtoolroot";;
		all)		make $n all CC="$compiler" \
					"ATHTOOLROOT=$athtoolroot";;
		check)		;;
		install)	make $n install "DESTDIR=$srvd" \
					"ATHTOOLROOT=$athtoolroot";;
	esac
fi
