#!/bin/sh
# $Id: do.sh,v 1.7 1996-11-21 00:32:30 ghudson Exp $

source="/source"
build="/build"
srvd="/srvd"
usage="do [-s srcdir] [-d destdir] [prepare|clean|all|check|install]"

while getopts s:b:d: opt; do
	case "$opt" in
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
case "`uname -a`" in
SunOS*5.4*sun4*)
	ATHENA_SYS=sun4m_54
	HOSTTYPE=sun4
	LD_LIBRARY_PATH=/usr/openwin/lib export LD_LIBRARY_PATH
	PATH=/usr/ccs/bin:/usr/athena/bin:/usr/bin:/usr/ucb
	PATH=${PATH}:/usr/openwin/bin
	;;
IRIX*5.3*)
	ATHENA_SYS=sgi_53
	HOSTTYPE=sgi
	PATH=/usr/athena/bin:/usr/bsd:/usr/bin:/usr/bin/X11
	;;
esac

# Extract the compiler for this platform from config.site.
compiler=`. $CONFIG_SITE; echo $CC`

if [ -r Makefile.athena ]; then
	export SRVD SOURCE COMPILER CONFIGDIR XCONFIGDIR
	SRVD="$srvd"
	SOURCE="$source"
	COMPILER="$compiler"
	CONFIGDIR="$source/packs/build/config"
	XCONFIGDIR="$source/packs/build/xconfig"
	make -f Makefile.athena "$operation"
elif [ -x ./configure ]; then
	case "$operation" in
		prepare)	rm -f config.cache; ./configure ;;
		clean)		make clean ;;
		all)		make all ;;
		check)		;;
		install)	make install "DESTDIR=$srvd" ;;
	esac
elif [ -r Imakefile ]; then
	case "$operation" in
		prepare)
			imake "-I$source/packs/build/config" -DUseInstalled \
				"-DTOPDIR=$source"
			make Makefiles
			;;
		clean)		make clean ;;
		all)		make depend all ;;
		check)		;;
		install)	make install install.man "DESTDIR=$srvd" ;;
	esac
elif [ -r Makefile ]; then
	case "$operation" in
		prepare)	;;
		clean)		make clean ;;
		all)		make all CC="$compiler" ;;
		check)		;;
		install)	make install "DESTDIR=$srvd" ;;
	esac
fi
