#!/bin/sh
# $Id: build.sh,v 1.1 1996-09-20 05:06:55 ghudson Exp $

export ATHENA_SYS HOSTTYPE PATH CONFIG_SITE

source="/source"
build="/build"
srvd="/srvd"
usage="build [-s srcdir] [-b builddir] [-d destdir] [package [endpackage]]"

while getopts s:b:d: opt; do
	case "$opt" in
		s)
			source="$OPTARG"
			;;
		b)
			build="$OPTARG"
			;;
		d)
			srvd="$OPTARG"
			;;
		\?)
			echo "$usage"
	esac
done
shift `expr $OPTIND - 1`

configdir="$source/packs/build/config"
xconfigdir="$source/packs/build/xconfig"

umask 022

# Determine the platform type.
case "`uname -a`" in
	SunOS*5.4*sparc)
		HOSTTYPE=sun4
		ATHENA_SYS=sun4m_54
		;;
	IRIX*5.3*mips)
		HOSTTYPE=sgi
		ATHENA_SYS=sgi_53
		;;
esac

# Set up the build environment.
CONFIG_SITE=$source/packs/build/config.site
case "$HOSTTYPE" in
	sun4)
		LD_LIBRARY_PATH=/usr/openwin/lib export LD_LIBRARY_PATH
		PATH=/usr/ccs/bin:/usr/athena/bin:/usr/bin:/usr/ucb
		PATH=${PATH}:/usr/openwin/bin
		;;
	sgi)
		PATH=/usr/athena/bin:/usr/bsd:/usr/bin:/usr/bin/X11
		;;
esac

# Extract compiler value from $CONFIG_SITE, for use when building straight
# Makefiles.
compiler=`. $CONFIG_SITE; echo $CC`

# Send all output friom this point on to the build log file.
mkdir -p $build/LOGS
exec >>$build/LOGS/washlog.`date '+%y.%m.%d.%H'` 2>&1

echo ========
echo starting `date`
echo on a $HOSTTYPE

# Read in the list of packages, filtering for platform type.
packages=`( echo "$HOSTTYPE"; cat "$source/packs/build/packages" ) | awk '
	NR == 1 {
		platform = $1;
		next;
	}
	/^#|^$/ {
		next;
	}
	/[ \t]/ {
		split($2, p, ",");
		for (i in p) {
			if (p[i] == platform || p[i] == "all") {
				print $1;
				break;
			}
		}
		next;
	}
	{
		print;
	}'`

# Build the packages.
found=""
done=""
for package in $packages; do

	# If arguments given, filter for start and end packages.
	if [ -n "$done" ]; then
		break;
	fi
	if [ -n "$1" -a -z "$found" ]; then
		if [ "$package" = "$1" ]; then
			found=true
			if [ -z "$2" ]; then
				done=true
			fi
		else
			continue
		fi
	fi
	if [ "package" = "$2" ]; then
		done=true
	fi

	# Build the package.
	echo "**********************"
	echo "***** In $package"

	cd $build/$package || exit 1
	if [ -r "$build/$package/.build" ]; then
		. "$build/$package/.build"
	elif [ -x "$build/$package/configure" ]; then
		echo In ${package}: rm -f config.cache && rm -f config.cache &&
		echo In ${package}: configure && ./configure &&
		echo In ${package}: make clean && make clean &&
		echo In ${package}: make all && make all &&
		echo In ${package}: make install DESTDIR=$srvd &&
		make install DESTDIR=$srvd
	elif [ -r "$build/$package/Imakefile" ]; then
		echo In ${package}: imake -I$configdir -DUseInstalled -DSRCDIR=$source &&
		imake -I"$configdir" -DUseInstalled -DSRCDIR="$source" &&
		echo In ${package}: make Makefiles && make Makefiles &&
		echo In ${package}: make clean && make clean &&
		echo In ${package}: make depend && make depend &&
		echo In ${package}: make all && make all &&
		echo In ${package}: make install DESTDIR=$srvd &&
		make install DESTDIR=$srvd &&
		echo In ${package}: make install.man DESTDIR=$srvd &&
		make install.man DESTDIR=$srvd
	elif [ -r "$build/$package/Makefile" ]; then
		CC="$compiler" export CC
		echo In ${package}: make clean && make clean &&
		echo In ${package}: make all && make all &&
		echo In ${package}: make install DESTDIR=$srvd &&
		make install DESTDIR=$srvd
	else
		echo "Can't figure out how to build $package"
		exit 1
	fi
	if [ "$?" -ne 0 ]; then
		echo "We bombed in $package"
		exit 1
	fi
	unset CC
done

echo "Ending `date`"
