#!/bin/sh
# $Id: build.sh,v 1.24 1999-03-29 22:14:51 ghudson Exp $

# This is the script for building the Athena source tree, or pieces of
# it.  It is less flexible than the do.sh script in this directory.
# See doc/maintenance in the source tree for information about
# building the tree.

source="/mit/source"
build="/build"
srvd="/.srvd"
usage="build [-s srcdir] [-b builddir] [-d destdir] [-k] [-n]"
usage="$usage [package [endpackage]]"

while getopts s:b:d:kn opt; do
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
	k)
		ignore=true
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

# Determine the operating system.
case `uname -s` in
	SunOS)		os=solaris ;;
	IRIX)		os=irix ;;
	Linux)		os=linux ;;
esac

# Read in the list of packages, filtering for operating system.
packages=`awk '
	/^#|^$/		{ next; }
	start == $1	{ start = ""; }
	start != ""	{ next; }
	/[ \t]/ {
		split($2, p, ",");
		build = 0;
		for (i = 1; p[i]; i++) {
			if (p[i] == os || p[i] == "all")
				build = 1;
			if (p[i] == ("-" os))
				build = 0;
		}
		if (build)
			print $1;
		next;
	}
			{ print; }
	end == $1	{ exit; }' os="$os" start="$start" end="$end" \
		$source/packs/build/packages`

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
echo Starting at `date` on $os

# Build the packages.
for package in $packages; do
	cd $build/$package || exit 1
	echo "**********************"
	for op in prepare clean all check install; do
		echo "***** ${package}: $op"
		sh $source/packs/build/do.sh -c -s "$source" -d "$srvd" "$op"
		if [ $? -ne 0 ]; then
			echo "We bombed in $package"
			if [ -z "$ignore" ]; then
				exit 1
			fi
		fi

		# Redo the output redirection command to flush the log file.
		exec >> "$logfile" 2>&1
	done
done

echo "Ending at `date`"
