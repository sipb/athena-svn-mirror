#!/bin/sh

# $Id: pkg-update.sh,v 1.1 2004-07-30 20:44:53 rbasch Exp $

# Run pkgadd to install a package from an Athena distribution directory,
# and, if this is an update of an existing package, run pkg-prune to
# remove old files from the pkg database and file system.

prog=$0
root=/
dist=
libdir=/srvd/usr/athena/lib/update

usage="Usage: $prog [-d dist] [-R root] <pkg>"
while getopts d:R: opt; do
  case "$opt" in
  d)
    dist=$OPTARG
    ;;
  R)
    root=$OPTARG
    ;;
  \?)
    echo "$usage" 1>&2
    exit 1
    ;;
  esac
done
shift `expr "$OPTIND" - 1`

# We require exactly one argument, i.e. the package name.
if [ $# -ne 1 ]; then
  echo "$usage" 1>&2
  exit 1
fi
pkg=$1

# The default distribution directory is the pkg directory for this
# machine's version number.
if [ -z "$dist" ]; then
  dist=/srvd/pkg/`awk '{ ver = $5; } END { print ver; }' /etc/athena/version`
fi

# Make sure the package is valid in the distribution directory.
if [ ! -s $dist/$pkg/pkgmap ]; then
  echo "$prog: Cannot find package $pkg in $dist" 1>&2
  exit 1
fi

# Note whether this is an update of an existing package.
if pkginfo -q -R "$root" ; then
  update=true
else
  update=false
fi

# Install the package.
pkgadd -a $libdir/admin-update -n -R "$root" -d "$dist" $pkg
status=$?
case $status in
1|3|5)
  # Fatal error -- do not prune the package.
  echo "pkgadd failed for $pkg (status $status)" 1>&2
  exit $status
  ;;
esac

# If we updated an existing version of the package, remove any old files
# that are not present in the new version.
if [ $update = true ]; then
  $libdir/pkg-prune -R "$root" -d "$dist" $pkg
fi

exit $status
