#!/bin/sh

# This is the script for building a complete set of Athena packages.

usage="build-all [-s srcdir] [-d destdir] [-k] [-n] [-l] [-t type]"
usage="$usage [package [endpackage]]"

# Default variables

source="/mit/source"
destdir="/build"
ignore=false
nobuild=false
log=false

while getopts s:d:knlt: opt; do
  case "$opt" in
  s)
    source="$OPTARG"
    ;;
  d)
    destdir="$OPTARG"
    ;;
  k)
    ignore=true
    ;;
  n)
    nobuild=true
    ;;
  l)
    log=true
    ;;
  t)
    type="$OPTARG"
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

case `uname -s` in
Linux)
  os=linux
  : ${type=rpm}
  awk=awk
  ;;
SunOS)
  os=solaris
  : ${type=sunpkg}
  awk=nawk
  ;;
*)
  echo "Unrecognized system type, aborting." >&2
  exit 1
  ;;
esac

case $type in
rpm)
  prefix="athena-"
  ;;
sunpkg)
  if [ "$os" != solaris ]; then
    echo "Type \"sunpkg\" is only supported on Solaris, aborting." >&2
    exit 1
  fi
  prefix="MIT-"
  ;;
*)
  echo "Package type must be \"rpm\" or \"sunpkg\", aborting." >&2
  exit 1
  ;;
esac

# Send all output from this point on to the build log file.
if [ true = "$log" ]; then
  mkdir -p "$destdir/logs" 2>/dev/null
  now=`date '+%y.%m.%d.%H'`
  logfile=$destdir/logs/washlog.$now
  rm -f "$destdir/logs/current"
  ln -s "washlog.$now" "$destdir/logs/current"
  exec >> "$logfile" 2>&1
fi

# Read in the list of packages, filtering for the operating system.
# Each line of output from getpackages.awk looks something like
#    third/afsbin afs
set -- `$awk -f $source/packs/build/getpackages.awk \
  os="$os" pkgnames=1 start="$start" end="$end" \
  $source/packs/build/packages` || exit 1

case $nobuild in
true)
  echo $*
  exit
  ;;
esac

echo ========
echo Starting at `date` on $os

# Build the packages
while [ $# -ge 2 ]; do
  echo "***** $1"
  $source/packs/build/$type/build-package -s $source -d $destdir $1 $prefix$2
  if [ $? -ne 0 ]; then
    echo "We bombed in $1"
    if [ false = "$ignore" ]; then
      exit 1
    fi
  fi

  # Redo the output redirection command to flush the log file.
  if [ true = "$log" ]; then
    exec >> "$logfile" 2>&1
  fi

  shift 2
done

if [ $# -ne 0 ]; then
  echo "Extra grot at end of packages list:"
  echo "$*"
fi

echo "Ending at `date`"