#!/bin/sh
# $Id: build.sh,v 1.12 1997-01-29 23:40:02 ghudson Exp $

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
start="$1"
end="${2-$1}"

# Determine the platform type.
case "`uname -a`" in
	SunOS*sun4*)	platform=sun4 ;;
	IRIX*)		platform=sgi ;;
esac

# All output should go to the build log file.
mkdir -p $build/LOGS 2>/dev/null
logfile="$build/LOGS/washlog.`date '+%y.%m.%d.%H'`"

# Just in case the build script itself has any error output, send any
# error output to a special log file.
date >> $build/LOGS/wash-script-errors
exec >> $build/LOGS/wash-script-errors 2>&1

echo "========" >> "$logfile"
echo "Starting `date` on a $platform" >> "$logfile"

# Read in the list of packages, filtering for platform type.
packages=`( echo "$platform"; cat "$source/packs/build/packages" ) | awk '
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
for package in $packages; do
	# If arguments given, filter for start and end packages.
	if [ "$package" = "$start" ]; then
		start=""
	elif [ -n "$start" ]; then
		continue
	fi

	# Build the package.
	cd $build/$package || exit 1
	echo "**********************" >> "$logfile"
	for op in prepare clean all check install; do
		echo "***** ${package}: $op" >> "$logfile"
		( sh $source/packs/build/do.sh -s "$source" -d "$srvd" "$op" ||
			{ echo "We bombed in $package"; exit 1; } ) \
			>> "$logfile" 2>&1
	done

	if [ "$package" = "$end" ]; then
		break
	fi
done

echo "Ending `date`" >> "$logfile"
