#!/bin/sh
# $Id: do.sh,v 1.18 1998-03-03 21:04:42 ghudson Exp $

source="/mit/source"
srvd="/srvd"
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
		srvd="$OPTARG"
		;;
	n)
		n="-n"
		maybe="echo"
		;;
	s)
		source="$OPTARG"
		;;
	\?)
		echo "$usage"
		exit 1
		;;
	esac
done
shift `expr $OPTIND - 1`
operation=${1-all}

case "$contained" in
true)
	athtoolroot="$srvd"
	;;
false)
	athtoolroot=""
	;;
esac

# Set up the build environment.
umask 022
export ATHENA_SYS HOSTTYPE CONFIG_SITE PATH
CONFIG_SITE=$source/packs/build/config.site

# Determine proper ATHENA_SYS value.
case "`uname -srm`" in
SunOS*5.6*sun4*)	ATHENA_SYS=sun4x_56	;;
SunOS*5.5*sun4*)	ATHENA_SYS=sun4x_55	;;
SunOS*5.4*sun4*)	ATHENA_SYS=sun4m_54	;;
IRIX*5.3*)		ATHENA_SYS=sgi_53	;;
IRIX*6.2*)		ATHENA_SYS=sgi_62	;;
IRIX*6.3*)		ATHENA_SYS=sgi_63	;;
esac

# Determine platform type, appropriate path, and compiler for use with plain
# Makefiles and some third-party packages.
case "`uname -sm`" in
SunOS*sun4*)
	HOSTTYPE=sun4
	LD_LIBRARY_PATH=/usr/openwin/lib export LD_LIBRARY_PATH
	PATH=/usr/ccs/bin:$athtoolroot/usr/athena/bin:/usr/bin:/usr/ucb
	PATH=${PATH}:/usr/openwin/bin
	compiler="/usr/gcc/bin/gcc -DSOLARIS"
	;;
IRIX*)
	HOSTTYPE=sgi
	PATH=$athtoolroot/usr/athena/bin:/usr/bsd:/usr/bin:/usr/bin/X11
	compiler=cc
	;;
esac

if [ -r Makefile.athena ]; then
	export SRVD SOURCE COMPILER CONFIGDIR XCONFIGDIR ATHTOOLROOT
	SRVD="$srvd"
	SOURCE="$source"
	COMPILER="$compiler"
	CONFIGDIR="$source/packs/build/config"
	XCONFIGDIR="$source/packs/build/xconfig"
	ATHTOOLROOT="$athtoolroot"
	make $n -f Makefile.athena "$operation"
elif [ -x ./configure ]; then
	export ATHTOOLROOT
	ATHTOOLROOT="$athtoolroot"
	case "$operation" in
		prepare)	$maybe rm -f config.cache
				$maybe ./configure ;;
		clean)		make $n clean ;;
		all)		make $n all ;;
		check)		;;
		install)	make $n install "DESTDIR=$srvd" ;;
	esac
elif [ -r Imakefile ]; then
	case "$operation" in
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
	case "$operation" in
		prepare)	;;
		clean)		make $n clean "ATHTOOLROOT=$athtoolroot";;
		all)		make $n all CC="$compiler" \
					"ATHTOOLROOT=$athtoolroot";;
		check)		;;
		install)	make $n install "DESTDIR=$srvd" \
					"ATHTOOLROOT=$athtoolroot";;
	esac
fi
