#!/bin/sh
# $Id: build.sh,v 1.40 2003-01-19 19:28:19 ghudson Exp $

# This is the script for building the Athena source tree, or pieces of
# it.  It is less flexible than the do.sh script in this directory.
# See doc/maintenance in the source tree for information about
# building the tree.

source="/mit/source"
build="/build"
srvd="/.srvd"
ignore=false
nobuild=false
log=false
target=
usage="build [-s srcdir] [-b builddir] [-d destdir] [-k] [-n] [-l]"
usage="$usage [package [endpackage]]"

while getopts s:b:d:knl opt; do
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
  l)
    log=true
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
SunOS)
  os=solaris
  awk=nawk
  ;;
IRIX)
  os=irix
  awk=awk
  ;;
Linux) 
  os=linux
  awk=awk
  ;;
*)
  echo "Unrecognized system type, aborting." 1>&2
  exit 1
  ;;
esac

# Send all output from this point on to the build log file.
if [ true = "$log" ]; then
  mkdir -p "$build/logs" 2>/dev/null
  now=`date '+%y.%m.%d.%H'`
  logfile=$build/logs/washlog.$now
  rm -f "$build/logs/current"
  ln -s "washlog.$now" "$build/logs/current"
  exec >> "$logfile" 2>&1
fi

# Read in the list of packages, filtering for operating system.
packages=`$awk -f $source/packs/build/getpackages.awk \
  os="$os" start="$start" end="$end" $source/packs/build/packages` || exit 1

case $nobuild in
true)
  echo $packages
  exit
  ;;
esac

echo ========
echo Starting at `date` on $os

# Create the build directory if necessary.  Make this conditional
# because the Solaris 8 mkdir -p errors out if $build is a symlink to
# another directory.
if [ ! -d $build ]; then
  mkdir -p $build || exit 1
fi

# Build the packages.
for package in $packages; do
  cd $build || exit 1
  if [ third/krb5/src/appl != "$package" ]; then
    rm -rf $package
    mkdir -p $package
    cd $package || exit 1
    (cd $source/$package && tar cf - .) | tar xfp -
  else
    cd $package || exit 1
  fi
  PWD=$build/$package
  export PWD
  echo "**********************"
  for op in dist prepare clean all install; do
    echo "***** ${package}: $op"
    sh $source/packs/build/do.sh -s "$source" -d "$srvd" "$op"
    if [ $? -ne 0 ]; then
      echo "We bombed in $package"
      if [ false = "$ignore" ]; then
	exit 1
      fi
    fi

    # Redo the output redirection command to flush the log file.
    if [ true = "$log" ]; then
      exec >> "$logfile" 2>&1
    fi
  done
done

echo "Ending at `date`"
