#!/bin/sh
# $Id: build.sh,v 1.19 1998-07-15 18:29:35 ghudson Exp $

# This is the script for building the Athena source tree, or pieces of
# it.  It is less flexible than the do.sh script in this directory.
# See doc/maintenance in the source tree for information about
# building the tree.

source="/mit/source"
build="/build"
srvd="/srvd"
usage="build [-s srcdir] [-b builddir] [-d destdir] [-n] [package [endpackage]]"

while getopts s:b:d:n opt; do
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
	n)
		nobuild=true
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
case "`uname -sm`" in
	SunOS*sun4*)	platform=sun4 ;;
	IRIX*)		platform=sgi ;;
esac

# Read in the list of packages, filtering for platform type.
packages=`awk '
	/^#|^$/		{ next; }
	start == $1	{ start = ""; }
	start != ""	{ next; }
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
			{ print; }
	end == $1	{ exit; }' platform="$platform" start="$start" \
		end="$end" $source/packs/build/packages`

case $nobuild in
	true)
		echo $packages
		exit
		;;
esac

# Send all output friom this point on to the build log file.
mkdir -p "$build/logs" 2>/dev/null
now=`date '+%y.%m.%d.%H'`
logfile=$build/logs/washlog.$now
rm -f "$build/logs/current"
ln -s "washlog.$now" "$build/logs/current"
exec >> "$logfile" 2>&1

echo ========
echo Starting at `date` on a $platform

# Build the packages.
for package in $packages; do
	cd $build/$package || exit 1
	echo "**********************"
	for op in prepare clean all check install; do
		echo "***** ${package}: $op"
		sh $source/packs/build/do.sh -c -s "$source" -d "$srvd" "$op" \
			|| { echo "We bombed in $package"; exit 1; }

		# Redo the output redirection command to flush the log file.
		exec >> "$logfile" 2>&1
	done
done

echo "Ending at `date`"
