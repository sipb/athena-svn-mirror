#!/bin/sh
# $Id: do.sh,v 1.11 1997-02-27 18:30:53 ghudson Exp $

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

# Determine proper ATHENA_SYS value.
case "`uname -a`" in
SunOS*5.5*sun4*)	ATHENA_SYS=sun4x_55	;;
SunOS*5.4*sun4*)	ATHENA_SYS=sun4m_54	;;
IRIX*5.3*)		ATHENA_SYS=sgi_53	;;
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
				"-DTOPDIR=$source/packs/build"
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
