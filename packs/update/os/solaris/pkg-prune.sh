#!/bin/sh

# $Id: pkg-prune.sh,v 1.1 2004-07-28 13:30:18 rbasch Exp $

# Remove old files from a package, i.e. files installed by a previous
# version of a package, but not belonging to the current version.
# Such files are removed from the package database, and, if belonging
# exclusively to the given package, from the file system.
#
# Usage is:
#
# pkg-prune [-d dist] [-n] [-R root] [-v] <pkg>
#
# The default distribution directory is /srvd/pkg/<version>, where
# <version> is the workstation's current version.
#
# If the -n option is given, then old files will be listed to standard
# output, but not removed.  If the -v option is given, files will be
# listed as they are removed from the file system.

prog=$0
root=/
dist=
noop=false
verbose=false

usage="Usage: $prog [-d dist] [-n] [-R root] [-v] <pkg>"
while getopts d:nR:v opt; do
  case "$opt" in
  d)
    dist=$OPTARG
    ;;
  n)
    noop=true
    ;;
  R)
    root=$OPTARG
    ;;
  v)
    verbose=true
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

# The package must be installed in order to be pruned.
pkginfo -q -R $root $pkg || {
  echo "Package $pkg is not installed." 1>&2
  exit 1
}

installed_list=/tmp/installed$$
pkg_list=/tmp/pkg$$

rm -rf $installed_list $pkg_list

# Get the files belonging to the package which are currently installed
# in the pkg database.
pkgchk -l -R "$root" $pkg | nawk -v root="$root" -v pkg="$pkg" '
  BEGIN {
    if (root == "/")
      path_index = 1;
    else
      path_index = length(root) + 1;
  }
  /^Pathname: / { print substr($2, path_index); }' | sort > $installed_list

# Get the files belonging to the latest version of the package, from
# the pkgmap.  This assumes that:
# 1) The package does not install any extra files via "installf".
# 2) The package basedir is "/", and the path listed in pkgmap is relative
#    to the root, i.e. does not begin with "/".
# 3) The path does not contain any variables.
nawk '
  {
    if ($2 == "s" || $2 == "l")
      print "/" substr($4, 1, index($4, "=") - 1);
    else if (index("bcdefpvx", $2) > 0)
      print "/" $4;
  }' $dist/$pkg/pkgmap | sort > $pkg_list

if [ $noop = true ]; then
  # No-op mode: just list the old files.
  comm -23 $installed_list $pkg_list
else
  oldfiles=`comm -23 $installed_list $pkg_list`
  if [ -n "$oldfiles" ]; then
    # Use removef to remove old files from the package database; it
    # outputs paths which are safe to remove, i.e. which are not
    # shared by another package.  We reverse sort that file list,
    # to ensure that, if a directory is in the list, we remove
    # any contents before the directory itself.
    echo $oldfiles | xargs removef -R $root $pkg | sort -r | \
      while read path ; do
	if [ $verbose = true ]; then
	  echo $path
	fi
	if [ -d $path -a ! -h $path ]; then
	  rmdir $path > /dev/null 2>&1
	else
	  rm -f $path
	fi
      done
    # File removal is complete.
    removef -R $root -f $pkg
  fi
fi

rm -f $installed_list $pkg_list

exit 0
