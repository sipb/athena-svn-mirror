#!/bin/sh
# $Id: do.sh,v 1.14 1997-07-30 16:10:29 ghudson Exp $

n=""
maybe=""
source="/mit/source"
build="/build"
srvd="/srvd"
usage="do [-s srcdir] [-d destdir] [prepare|clean|all|check|install]"

while getopts ns:b:d: opt; do
	case "$opt" in
	n)
		n="-n"
		maybe="echo"
		;;
	s)
		source="$OPTARG"
		;;
	d)
		srvd="$OPTARG"
		;;
	\?)
		echo "$usage"
		exit 1
		;;
	esac
done
shift `expr $OPTIND - 1`
operation=${1-all}

# Set up the build environment.
umask 022
export ATHENA_SYS HOSTTYPE CONFIG_SITE PATH
CONFIG_SITE=$source/packs/build/config.site

# Determine proper ATHENA_SYS value.
case "`uname -a`" in
SunOS*5.5*sun4*)	ATHENA_SYS=sun4x_55	;;
SunOS*5.4*sun4*)	ATHENA_SYS=sun4m_54	;;
IRIX*5.3*)		ATHENA_SYS=sgi_53	;;
IRIX*6.2*)		ATHENA_SYS=sgi_62	;;
IRIX*6.3*)		ATHENA_SYS=sgi_63	;;
esac

# Determine platform type, appropriate path, and compiler for use with plain
# Makefiles and some third-party packages.
case "`uname -a`" in
SunOS*sun4*)
	HOSTTYPE=sun4
	LD_LIBRARY_PATH=/usr/openwin/lib export LD_LIBRARY_PATH
	PATH=/usr/ccs/bin:/usr/athena/bin:/usr/bin:/usr/ucb:/usr/openwin/bin
	compiler="/usr/gcc/bin/gcc -DSOLARIS"
	;;
IRIX*)
	HOSTTYPE=sgi
	PATH=/usr/athena/bin:/usr/bsd:/usr/bin:/usr/bin/X11
	compiler=cc
	;;
esac

if [ -r Makefile.athena ]; then
	export SRVD SOURCE COMPILER CONFIGDIR XCONFIGDIR
	SRVD="$srvd"
	SOURCE="$source"
	COMPILER="$compiler"
	CONFIGDIR="$source/packs/build/config"
	XCONFIGDIR="$source/packs/build/xconfig"
	make $n -f Makefile.athena "$operation"
elif [ -x ./configure ]; then
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
				-DUseInstalled "-DTOPDIR=$source/packs/build"
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
		clean)		make $n clean ;;
		all)		make $n all CC="$compiler" ;;
		check)		;;
		install)	make $n install "DESTDIR=$srvd" ;;
	esac
fi
