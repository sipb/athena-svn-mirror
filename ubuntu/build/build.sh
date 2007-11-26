#!/bin/sh

# Build a package for Ubuntu Athena.

die() { echo "$@" >&2; exit 1; }
usage() { die "build [-s srctree] [-b buildtree] package"; }
copydir() { # copydir <srcdir> <destdir> <checkfile>
  rm -rf $2
  mkdir -p $2
  (cd "$1" && tar --exclude=.svn -cf - .) | (cd "$2" && tar xpf -)
  [ -r "$2/$3" ] || die "Copy from $1 to $2 failed"
}
setbuildpath() { # setbuildpath <debiandir>; initializes $buildpath
  v=`dpkg-parsechangelog -l"$1"/changelog | sed -nr 's/Version: ([0-9]+:)?//p'`
  buildpath=$buildtree/$pkgname-$v
  unset v
}

srctree="/mit/source"
buildtree="/build"

while getopts s:b: opt; do
  case "$opt" in
  s)
    srctree="$OPTARG"
    ;;
  b)
    buildtree="$OPTARG"
    ;;
  \?)
    usage
    ;;
  esac
done

shift `expr $OPTIND - 1`
[ $# -eq 1 ] || usage
nickname=$1
pkgname=athena-$nickname

if [ -d "$srctree/ubuntu/athpkg/$nickname" ]; then
  pkgdir=$srctree/ubuntu/athpkg/$nickname

  # Case 1: Package containing Athena software.  Debian directory is
  # in $pkgdir; rest of source tree is off in athena/ or third/
  # somewhere, as determined by the Athena-Source-Dir custom header in
  # the control file.  The sources will need autotools goo copied in.

  # Locate the Athena sources.
  srcdir=`head -1 "$pkgdir/control" | sed -e 's/^# //'`
  srcpath=$srctree/$srcdir
  [ -d "$srcpath" ] || die "Can't find $srcpath"

  # Create the build directory.
  setbuildpath "$pkgdir"
  copydir "$srcpath" "$buildpath" Makefile.in
  copydir "$pkgdir" "$buildpath/debian" control

  # Copy in autotools support files and run autoconf.  Skip this for
  # packages which already contain a configure script (stuff in
  # third/).
  if [ ! -f "$buildpath/configure" ]; then
    cp /usr/share/misc/config.guess "$buildpath"
    cp /usr/share/misc/config.sub "$buildpath"
    cp /usr/share/libtool/ltmain.sh "$buildpath"
    cp $srctree/ubuntu/build/install-sh "$buildpath"
    cp $srctree/ubuntu/build/mkinstalldirs "$buildpath"
    cp $srctree/ubuntu/build/aclocal.m4 "$buildpath"
    cat /usr/share/aclocal/libtool.m4 >> $buildpath/aclocal.m4
    (cd "$buildpath" && autoconf)
    [ -r "$buildpath/configure" ] || die "Autoconfiscation failed"
  fi

  # Build the package.
  (cd "$buildpath" && dpkg-buildpackage -sn -rfakeroot) || die "Build failed"

elif [ -d "$srctree/ubuntu/pkg/$nickname" ]; then
  pkgdir=$srctree/ubuntu/pkg/$nickname

  # Case 2: Ubuntu-specific Athena package.  The whole package is in
  # $pkgdir.

  # Create the build directory.
  setbuildpath "$pkgdir/debian"
  copydir "$pkgdir" "$buildpath" debian/control

  # Build the package.
  (cd "$buildpath" && dpkg-buildpackage -sn -rfakeroot) || die "Build failed"

elif [ -d "$srctree/ubuntu/native/$nickname" ]; then
  pkgdir=$srctree/ubuntu/native/$nickname

  # Case 3: Modified Ubuntu native package.  $pkgdir/pkg contains the
  # packed source package, $pkgdir/ubuntu contains the unpacked Ubuntu
  # source tree (not used by this script), and $pkgdir/athena contains
  # the Athena-modified unpacked source tree.

  # Copy the original tarfile into the build tree for dpkg-source.
  cp $pkgdir/pkg/*.orig.tar.gz "$buildtree"

  # Create the build directory.
  setbuildpath "$pkgdir/debian"
  copydir "$pkgdir/athena" "$buildpath" debian/control

  # Build the package.
  (cd "$buildpath" && dpkg-buildpackage -rfakeroot) || die "Build failed"
fi
