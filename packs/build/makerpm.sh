#!/bin/sh
# $Id: makerpm.sh,v 1.1 2002-10-01 18:47:18 ghudson Exp $

# On Solaris, prime /build/athtools with rpm (which requires gcc,
# which requires gmake) so that we can begin the actual release build.

source=${1-/mit/source}
buildroot=${2-/build}
build=$buildroot/rpm-bootstrap
athtools=$buildroot/athtools
packages="third/gmake third/gcc third/rpm"

set -e

# Create the build directory if necessary.  Make this conditional
# because the Solaris 8 mkdir -p errors out if $build is a symlink to
# another directory.
if [ ! -d $build ]; then
  mkdir -p $build
fi

for package in $packages; do
  cd $build || exit 1
  rm -rf $package
  mkdir -p $package
  cd $package || exit 1
  (cd $source/$package && tar cf - .) | tar xfp -
  sh $source/packs/build/do.sh -s "$source" dist
  sh $source/packs/build/do.sh prepare
  sh $source/packs/build/do.sh
  sh $source/packs/build/do.sh -d "$athtools" install
done
