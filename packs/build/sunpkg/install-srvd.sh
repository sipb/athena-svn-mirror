#!/bin/sh

# $Id: install-srvd.sh,v 1.2 2004-04-06 23:36:53 rbasch Exp $

# This script installs packages for a new release into the srvd,
# running pkgadd with the srvd as the target root, and copying
# the new packages into the srvd package directory for the new
# version.  For a patch release, it also invokes link-pkgs to
# create the links to old packages in the new package directory.
# Finally, it creates the .order-version file in the new package
# directory.

prog=$0
maybe=
no_links=false
newver=
pkgsrc=
source=/mit/source
srvd=/.srvd
srvd_ro=/srvd

usage="Usage: $prog [-n] [-d destdir] [-N] [-p pkgsrc] [-s srcdir]"
usage="$usage [new_version [old_version]]"
while getopts d:Nnp:s: opt; do
  case "$opt" in
  d)
    srvd=$OPTARG
    ;;
  N)
    no_links=true
    ;;
  n)
    maybe=echo
    ;;
  p)
    pkgsrc=$OPTARG
    ;;
  s)
    source=$OPTARG
    ;;
  \?)
    echo "$usage" 1>&2
    exit 1
    ;;
  esac
done
shift `expr "$OPTIND" - 1`

# The new and old version numbers are optional command line arguments.
# If the old version is not given, it defaults to the version found
# in /srvd/.rvdinfo (read-only), unless the new version is given and
# has a patch number of 0.  If the new version is not given, it defaults
# to one greater than the old patch number.
newver=$1
oldver=$2
new_patch=
if [ -n "$newver" ]; then
  eval `echo $newver | nawk -F. '{ printf("major=%d; minor=%d; new_patch=%d\n",
					  $1, $2, $3); }'`
  if [ "$new_patch" = 0 -a -n "$oldver" ]; then
    echo "Patch level 0 cannot have an old version" 1>&2
    echo "$usage" 1>&2
    exit 1
  fi
fi
if  [ -z "$oldver" -a "$new_patch" != 0 ]; then
  oldver=`awk '{ ver = $5; } END { print ver; }' "$srvd_ro/.rvdinfo"`
  if [ -z "$oldver" ]; then
    echo "Cannot get old version from $srvd_ro/.rvdinfo" 1>&2
    echo "$usage" 1>&2
    exit 1
  fi
fi
if [ -z "$newver" ]; then
  newver=`echo $oldver | awk -F. '{ print $1 "." $2 "." ($3 + 1); }'`
fi

echo "The new version is $newver."
if [ -n "$oldver" ]; then
  echo "The old version is $oldver."
fi

: ${pkgsrc:="/build/PKG/$newver"}

if [ ! -d "$pkgsrc" ]; then
  echo "$pkgsrc is not a valid directory" 1>&2
  exit 1
fi

if [ -n "$oldver" -a "$no_links" = false -a ! -d "$srvd/pkg/$oldver" ]; then
  echo "$srvd/pkg/$oldver is not a valid directory" 1>&2
  exit 1
fi

pkgdest="$srvd/pkg/$newver"
pkg_order="$source/packs/update/os/solaris/pkg-order.pl"

# Create a pkgadd admin file for installing packages non-interactively.
admin=/tmp/admin$$
rm -rf "$admin"
cat > "$admin" << 'EOF'
mail=
instance=overwrite
partial=nocheck
runlevel=nocheck
idepend=nocheck
rdepend=nocheck
space=nocheck
setuid=nocheck
conflict=nocheck
action=nocheck
basedir=default
EOF

# Get the packages in the package source directory, in install order.
pkgs=`perl $pkg_order -d "$pkgsrc"`
if [ -z "$pkgs" ]; then
  echo "There are no packages in $pkgsrc" 1>&2
  exit 1
fi

# pkgadd will create $srvd/var if necessary, but does not chown it,
# causing complaints on packages with /var in the pkgmap.  Make sure
# it is owned by root.
$maybe mkdir -p "$srvd/var" || exit 1
$maybe chown root "$srvd/var" || exit 1

$maybe mkdir -p "$pkgdest" || exit 1

# Set up /bin and /usr/vice symlinks if necessary.
if [ ! -h "$srvd/bin" ]; then
  $maybe ln -s ./usr/bin "$srvd/bin"
fi
if [ ! -h "$srvd/usr/vice" ]; then
  $maybe mkdir -p "$srvd/usr"
  $maybe chown root "$srvd/usr"
  $maybe ln -s ../var/usr/vice "$srvd/usr/vice"
fi

# For each package in the list, install it in the srvd root, and copy
# it into the srvd pkg directory for the new version.
for pkg in $pkgs ; do
  if [ ! -f "$pkgsrc/$pkg/pkginfo" ]; then
    echo "$pkg is not a valid package in $pkgsrc" 1>&2
  elif pkginfo -q -c MITdontinstall -d "$pkgsrc" "$pkg" ; then
    echo "Skipping uninstallable package $pkg"
  else
    echo "$pkg"
    # Create OS directories ahead of time to avoid pkgadd complaints
    # about them being owned by an unknown (AFS) uid.
    pkgmap="$pkgsrc/$pkg/pkgmap"
    for dir in `awk '{ if ($2 == "d" && $6 == "?") print $4; }' $pkgmap` ; do
      if [ ! -d "$srvd/$dir" ]; then
	$maybe mkdir -p "$srvd/$dir"
	$maybe chown root "$srvd/$dir"
      fi
    done
    $maybe pkgadd -R "$srvd" -a "$admin" -n -d "$pkgsrc" "$pkg"
    status=$?
    case $status in
    1|3|5)
      echo "pkgadd failed for $pkg (status $status)" 1>&2
      exit 1
      ;;
    esac
    # Nuke any previous version of the package in the srvd pkg directory.
    # pkgtrans has an overwrite option, but it does not work well with
    # symlinks.
    $maybe rm -rf "$pkgdest/$pkg"
    $maybe pkgtrans "$pkgsrc" "$pkgdest" "$pkg" || exit 1
  fi
done

# Make links back to the packages in the old (current) release which
# are not being updated.
if [ -n "$oldver" -a "$no_links" = false ]; then
  echo "Making links to existing packages ..."
  $maybe perl $source/packs/build/sunpkg/link-pkgs.pl -d "$srvd/pkg" \
    "$newver" "$oldver" || exit 1
fi

# Create the .order-version and .rvdinfo files.
if [ -z "$maybe" ]; then
  tmporder=/tmp/order$$
  rm -rf $tmporder
  perl "$pkg_order" -v -d "$pkgdest" > $tmporder
  # Strip out uninstallable and srvd-only packages.
  for pkg in `pkginfo -c MITdontinstall,MITsrvd -d "$pkgdest" | \
    awk '{ print $2; }'`
  do
    grep -v "^$pkg " $tmporder > $tmporder.new
    mv $tmporder.new $tmporder
  done
  cp $tmporder "$pkgdest/.order-version"
  rm -f $tmporder
  echo "Athena RVD (sun4) Version $newver `date`" > "$srvd/.rvdinfo"
  chmod 444 "$srvd/.rvdinfo"
fi

rm -f "$admin"
