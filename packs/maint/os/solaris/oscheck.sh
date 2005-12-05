#!/bin/sh

# $Id: oscheck.sh,v 1.6 2005-12-05 20:46:51 rbasch Exp $

# This script checks the integrity of the Solaris OS installation, by
# running the os-checkfiles program against the appropriate set of
# stats files for this machine architecture.

usage="Usage: $0 [-n|-y] [-l <libdir>] [-o <osroot>] [-r <root>]"

. /etc/athena/rc.conf

# The default is check-only for non-PUBLIC machines.
if [ true = "$PUBLIC" ]; then
  noop=
else
  noop="-n"
fi

rootdir=/
os=/os
libdir=/install/oscheck
checkfiles=os-checkfiles
version=`awk '{ a = $5; } END { print a; }' /etc/athena/version`
srvdstats=/srvd/pkg/$version/.stats
configfiles=/srvd/usr/athena/lib/update/configfiles
globalexceptions=$libdir/exceptions.all
localexceptions=/etc/athena/oscheck.exceptions
exceptions=/tmp/exceptions$$

while getopts l:no:r:y opt; do
  case "$opt" in
  l)
    libdir=$OPTARG
    ;;
  n)
    noop="-n"
    ;;
  o)
    os=$OPTARG
    ;;
  r)
    rootdir=$OPTARG
    ;;
  y)
    noop=
    ;;
  \?)
    echo "$usage" 1>&2
    exit 1
    ;;
  esac
done

if [ ! -f $libdir/stats.common ]; then
  echo "$libdir/stats.common does not exist; not checking OS files." >&2
  exit 1
fi

# Determine which hardware we're running on.
platform=`uname -m`

# Generate the exception list.  Start with the known global exceptions,
# if any.
rm -f $exceptions
if [ -r $globalexceptions ]; then
  cp $globalexceptions $exceptions
else
  cp /dev/null $exceptions
fi

# We don't want to bother with anything that is replaced by an MIT
# package, so extract all paths from the srvd stats file, to add them
# to the exceptions list.
awk '{ print $NF; }' $srvdstats >> $exceptions
sed -e 's|^/||' $configfiles >> $exceptions

if [ false = "$PUBLIC" ]; then
  # On a private machine, add any local exceptions to the list.
  if [ -s $localexceptions ]; then
    cat $localexceptions >> $exceptions
  fi
fi

rm -f $exceptions.sorted
sort -u $exceptions > $exceptions.sorted

# Check platform-dependent files, if there is a stats file for this
# architecture.
archstats=$libdir/stats.$platform
machroot=$libdir/mach/$platform
if [ -r $archstats ]; then
  echo "Checking files for $platform architecture..."
  $checkfiles $noop -r $rootdir -o $os -x $exceptions.sorted $archstats
  if [ -r $archstats.dup ]; then
    echo "Checking non-shared files for $platform architecture..."
    $checkfiles $noop -r $rootdir -o $machroot \
      -x $exceptions.sorted $archstats.dup
  fi
else
  echo "No stats file for $platform architecture, $archstats"
fi

# Check common files.
echo "Checking common files..."
$checkfiles $noop -r $rootdir -o $os -x $exceptions.sorted $libdir/stats.common

# Clean up
rm -f $exceptions $exceptions.sorted
