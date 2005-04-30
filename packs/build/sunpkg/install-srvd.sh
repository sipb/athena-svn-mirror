#!/bin/sh

# $Id: install-srvd.sh,v 1.7 2005-04-30 16:00:22 rbasch Exp $

# This script installs packages for a new release into the srvd,
# running pkgadd with the srvd as the target root, and copying
# the new packages into the srvd package directory for the new
# version.  For a patch release, it also invokes link-pkgs to
# create the links to old packages in the new package directory.
# Finally, it creates the .order-version file in the new package
# directory.

prog=$0
maybe=
init_dest=false
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
    init_dest=true
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

if [ -n "$oldver" -a "$init_dest" = true -a ! -d "$srvd/pkg/$oldver" ]; then
  echo "$srvd/pkg/$oldver is not a valid directory" 1>&2
  exit 1
fi

pkgdest="$srvd/pkg/$newver"
pkg_order="$source/packs/update/os/solaris/pkg-order.pl"
pkg_prune="$source/packs/update/os/solaris/pkg-prune.sh"

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

if [ "$init_dest" = true ]; then
  $maybe rm -rf "$pkgdest"
elif [ ! -d "$pkgdest" ]; then
  # If the srvd package directory does not yet exist, we want to create
  # symlinks back to the packages in the old version.
  init_dest=true
fi
$maybe mkdir -p "$pkgdest" || exit 1

# Set up the /bin symlink if necessary.
if [ ! -h "$srvd/bin" ]; then
  $maybe ln -s ./usr/bin "$srvd/bin"
fi

# Create /usr/vice if necessary, and ensure it is owned by root.
# (This directory has been excised from the openafs package, so as not
# to blow away the symlink to /var/usr/vice on machines installed
# before 9.3).
if [ ! -d "$srvd/usr/vice" ]; then
  $maybe mkdir -p "$srvd/usr/vice"
  $maybe chown root "$srvd/usr" "$srvd/usr/vice"
fi

# For each package in the list, install it in the srvd root, and copy
# it into the srvd pkg directory for the new version.
for pkg in $pkgs ; do
  if [ ! -f "$pkgsrc/$pkg/pkginfo" ]; then
    echo "$pkg is not a valid package in $pkgsrc" 1>&2
  elif pkginfo -q -c MITdontinstall -d "$pkgsrc" "$pkg" ; then
    echo "Skipping uninstallable package $pkg"
  elif [ "$init_dest" = false ] &&
      cmp -s "$pkgsrc/$pkg/pkginfo" "$pkgdest/$pkg/pkginfo" ; then
    echo "Skipping $pkg (already installed)"
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

    # Note whether this is an update of an existing package.
    if pkginfo -q -R "$srvd" ; then
      update=true
    else
      update=false
    fi

    # Install the package.
    $maybe pkgadd -R "$srvd" -a "$admin" -n -d "$pkgsrc" "$pkg"
    status=$?
    case $status in
    1|3|5)
      echo "pkgadd failed for $pkg (status $status)" 1>&2
      exit 1
      ;;
    esac

    # If we updated an existing version of the package, remove any old
    # files that are not present in the new version.
    if [ $update = true ]; then
      $maybe sh $pkg_prune -R "$srvd" -d "$pkgsrc" "$pkg"
    fi

    # Nuke any previous version of the package in the srvd pkg directory.
    # pkgtrans has an overwrite option, but it does not work well with
    # symlinks.
    $maybe rm -rf "$pkgdest/$pkg"
    $maybe pkgtrans "$pkgsrc" "$pkgdest" "$pkg" || exit 1
  fi
done

# Make links back to the packages in the old (current) release which
# are not being replaced in the new version.
if [ -n "$oldver" -a "$init_dest" = true ]; then
  echo "Making links to existing packages ..."
  $maybe perl $source/packs/build/sunpkg/link-pkgs.pl -d "$srvd/pkg" \
    "$newver" "$oldver" || exit 1
fi

# Create the .order-version, stats, and .rvdinfo files.
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

  # Generate a new stats file, and configfiles.
  echo "Generating the stats file and configfiles ..."
  tmpconfigfiles=/tmp/configfiles$$
  rm -f $tmpconfigfiles
  rm -f "$pkgdest/.stats"
  pkgs=`awk '{ print $1; }' "$pkgdest/.order-version"`
  perl $source/packs/build/sunpkg/gen-stats.pl -d "$pkgdest" \
    -e $tmpconfigfiles $pkgs \
    > "$pkgdest/.stats" || exit 1
  mkdir -p "$srvd/usr/athena/lib/update"
  rm -f "$srvd/usr/athena/lib/update/configfiles"
  # We cannot simply copy /etc/athena/rc.conf from /srvd, so exclude it
  # from configfiles.
  sed -e '\|^/etc/athena/rc.conf$|d' $tmpconfigfiles \
    > "$srvd/usr/athena/lib/update/configfiles" || exit 1
  chmod 444 "$srvd/usr/athena/lib/update/configfiles"
  rm -f $tmpconfigfiles

  # Create .rvdinfo.
  rm -f "$srvd/.rvdinfo"
  echo "Athena RVD (sun4) Version $newver `date`" > "$srvd/.rvdinfo"
  chmod 444 "$srvd/.rvdinfo"
fi

rm -f "$admin"
