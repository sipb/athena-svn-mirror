#!/bin/sh

# $Id: verify-pkgs.sh,v 1.1 2004-04-24 14:04:38 rbasch Exp $

# This script verifies the integrity of the MIT-provided packages
# in the Athena Solaris release.

usage="Usage: $0 [-n|-y] [-r <root>] [-s <srvd>]"

. /etc/athena/rc.conf

# The default is check-only for non-PUBLIC machines.
if [ true = "$PUBLIC" ]; then
  noop=
else
  noop="-n"
fi

root=/
srvd=/srvd
checkfiles=os-checkfiles

while getopts nr:s:y opt; do
  case "$opt" in
  n)
    noop="-n"
    ;;
  r)
    root=$OPTARG
    ;;
  s)
    srvd=$OPTARG
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

# Get the stats file for this version.
version=`awk '{ a = $5; } END { print a; }' /etc/athena/version`
stats="$srvd/pkg/$version/.stats"
if [ ! -s $stats ]; then
  echo "Cannot read stats file $stats" 1>&2
  exit 1
fi

# Check the file system against the stats file.
$checkfiles $noop -r "$root" -o "$srvd" "$stats"
