#!/bin/sh
# $Id: build.sh,v 1.5 1996-10-12 22:02:53 ghudson Exp $

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
		exit 1
		;;
	esac
done
shift `expr $OPTIND - 1`

# Determine the platform type.
export ATHENA_SYS HOSTTYPE
case "`uname -a`" in
	SunOS*sparc)	platform=sun4 ;;
	IRIX*5.3*)	platform=sgi ;;
esac

# Send all output friom this point on to the build log file.
mkdir -p $build/LOGS 2>/dev/null
exec >> $build/LOGS/washlog.`date '+%y.%m.%d.%H'` 2>&1

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
		build = 0;
		for (i = 1; p[i]; i++) {
			if (p[i] == platform || p[i] == "all")
				build = 1;
			if (p[i] == ("-" platform))
				build = 0;
		}
		if (build)
			print $1;
		next;
	}
	{
		print;
	}'`

# Build the packages.
found=""
done=""
for package in $packages; do

	# /bin/sh isn't supposed to do path hashing, but on Solaris it does.
	# Yech.
	if [ "$HOSTTYPE" = sun4 ]; then
		hash -r
	fi

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
	if [ "$package" = "$2" ]; then
		done=true
	fi

	# Build the package.
	cd $build/$package &&
	echo "**********************" &&
	echo "***** ${package}: configure" &&
	sh $source/packs/build/do.sh -s "$source" -d "$srvd" configure &&
	echo "***** ${package}: clean" &&
	sh $source/packs/build/do.sh -s "$source" -d "$srvd" clean &&
	echo "***** ${package}: all" &&
	sh $source/packs/build/do.sh -s "$source" -d "$srvd" all &&
	echo "***** ${package}: check" &&
	sh $source/packs/build/do.sh -s "$source" -d "$srvd" check &&
	echo "***** ${package}: install" &&
	sh $source/packs/build/do.sh -s "$source" -d "$srvd" install
	if [ "$?" -ne 0 ]; then
		echo "We bombed in $package"
		exit 1
	fi
done

echo "Ending `date`"
