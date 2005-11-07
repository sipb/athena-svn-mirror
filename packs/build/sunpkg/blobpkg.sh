#!/bin/sh
# $Id: blobpkg.sh,v 1.1 2005-11-07 14:46:06 rbasch Exp $

# Convert the contents of the current directory tree into a Solaris
# package.  This script is not used by the Athena build proper, but
# is instead used to convert binary releases into packages which can
# be installed using the Athena install and update process.

usage="blobpkg [-d destdir] [-s srcdir] pkgname"

destdir=..
sourcedir=/mit/source

while getopts d:s: opt; do
  case "$opt" in
  d)
    destdir="$OPTARG"
    ;;
  s)
    sourcedir="$OPTARG"
    ;;
  \?)
    echo "$usage" >&2
    exit 1
    ;;
  esac
done
shift `expr $OPTIND - 1`

if [ $# -ne 1 ]; then
  echo "$usage" >&2
  exit 1
fi

# Sun package names cannot contain underscore, so we change those to
# hyphen.
package_name=`echo $1 | tr '_' '-'`

# Check that we are where we ought to be, and set up the environment.
case `uname -s` in
SunOS)
  PATH=/usr/xpg4/bin:/usr/ccs/bin:/usr/bin:/usr/ucb:/usr/openwin/bin
  arch=sparc
  ;;
*)
  echo "blobpkg must be run on a Solaris machine" >&2
  exit 1
  ;;
esac
PATH=/usr/athena/bin:$PATH
export PATH

# Make sure we have a valid pkginfo stub in the source tree.
pkginfo_source=$sourcedir/packs/build/sunpkg/pkginfo/$package_name
if [ ! -s $pkginfo_source ]; then
  echo "Invalid pkginfo stub file $pkginfo_source." >&2
  exit 1
fi

# Make sure the destination is valid, and get it as an absolute path.
mkdir -p $destdir || exit 1
destdir=`cd $destdir && pwd` || exit 1

# Create a temporary working area.
tmpdir=`mktemp -d -t blobpkg.XXXXXXXXXX`
if [ -z "$tmpdir" ]; then
  echo "Cannot create temporary directory." >&2
  exit 1
fi

# Create an install directory in the working area, and copy the
# contents there.
installdir=$tmpdir/INSTALL
mkdir -p $installdir || exit 1
tar cf - . | (cd $installdir; tar xfp -) || exit 1

# Generate the prototype file.  The awk hack changes the mode, owner,
# and group fields for all directories to indicate that they should be
# left unchanged when the package is installed.
prototype=`mktemp -t prototype.XXXXXXXXXX`
if [ -z "$prototype" ]; then
  echo "Cannot create temporary prototype." >&2
  exit 1
fi
pkgproto . | nawk '
  {
    if ($1 == "d")
      print $1, $2, $3, "?", "?", "?";
    else
      print $0;
  }' > $prototype || exit 1

# If this package has a filter for editing the prototype, run it.
if [ -f $sourcedir/packs/build/sunpkg/prototype/$package_name ]; then
  sh -e $sourcedir/packs/build/sunpkg/prototype/$package_name < $prototype \
    > $prototype.new
  if [ -s $prototype.new ]; then
    if cmp -s $prototype $prototype.new ; then
      echo "Warning: prototype filter for $package_name made no changes" >&2
    else
      mv $prototype.new $prototype
    fi
  else
    echo "prototype filter failed for $package_name" >&2
    exit 1
  fi
fi

# Prepend the pkginfo stub from the source tree.
# The stub must contain settings for PKG, NAME, and VERSION; others
# are defaulted as follows.
cp $pkginfo_source $installdir/pkginfo

grep -q '^ARCH=' $pkginfo_source || \
  echo "ARCH=$arch" >> $installdir/pkginfo

grep -q '^CATEGORY=' $pkginfo_source || \
  echo "CATEGORY=system" >> $installdir/pkginfo

grep -q '^BASEDIR=' $pkginfo_source || \
  echo "BASEDIR=/" >> $installdir/pkginfo

# Get the CLASSES setting.  The order here defines the order in which
# the class is processed at install time.  If the stub does not set
# CLASSES, we find all classes specified in the prototype.
# "none" is the default class, and must be listed in pkginfo.  
grep -q '^CLASSES=' $pkginfo_source || {
  # Find the non-default classes in the prototype.  Note that the following
  # assumes that the prototype does not contain a "part number" field.
  classes=`awk '$1 != "i" && $2 != "none" { print $2; }' $prototype | sort -u`
  # Always list class "none" first.
  echo CLASSES=none $classes >> $installdir/pkginfo
}

# The following three settings are used in creating a compressed
# archive for the "none" class in the package, speeding install time.
# We assume that no package requires any other kind of processing for
# this default class.

# At install time, do not check for files in the package's reloc directory.
grep -q '^PKG_SRC_NOVERIFY=' $pkginfo_source || \
  echo "PKG_SRC_NOVERIFY=none" >> $installdir/pkginfo

# Do a "quick" verify after installation, i.e. skip the checksum check.
grep -q '^PKG_DST_QKVERIFY=' $pkginfo_source || \
  echo "PKG_DST_QKVERIFY=none" >> $installdir/pkginfo

# Pass package location and destination paths to the class action script,
# instead of source/destination path pairs.
grep -q '^PKG_CAS_PASSRELATIVE=' $pkginfo_source || \
  echo "PKG_CAS_PASSRELATIVE=none" >> $installdir/pkginfo

echo "i pkginfo" >> $prototype

# Add any other package files to the prototype.

# If this package has a copyright file, copy it in.  The copyright is
# apparently displayed only at install time by pkgadd.
if [ -f $sourcedir/packs/build/sunpkg/copyright/$package_name ]; then
  cp $sourcedir/packs/build/sunpkg/copyright/$package_name \
    $installdir/copyright
  echo "i copyright" >> $prototype
fi

# Copy in the package's dependencies file, if any.
if [ -f $sourcedir/packs/build/sunpkg/depend/$package_name ]; then
  cp $sourcedir/packs/build/sunpkg/depend/$package_name $installdir/depend
  echo "i depend" >> $prototype
fi

# Copy in any procedure scripts (postinstall, etc.)
for proc in checkinstall preinstall postinstall preremove postremove ; do
  if [ -f $sourcedir/packs/build/sunpkg/$proc/$package_name ]; then
    cp $sourcedir/packs/build/sunpkg/$proc/$package_name $installdir/$proc
    echo "i $proc" >> $prototype
  fi
done

# Copy in the class action scripts for classes other than "none".
# (The "none" install script is copied explicitly into the package
# install directory below; we omit it here so that it is not listed
# in the pkgmap, since pkgadd does not seem to save it in the pkg
# save directory).
classes=`sed -n -e 's|^CLASSES=||p' $installdir/pkginfo`
for class in $classes ; do
  if [ $class != none ]; then
    for type in i r ; do
      if [ -f $sourcedir/packs/build/sunpkg/class/$type.$class ]; then
	cp $sourcedir/packs/build/sunpkg/class/$type.$class \
	  $installdir
	echo "i $type.$class" >> $prototype
      fi
    done
  fi
done

# We have completed the prototype, so copy it in.
cp $prototype $installdir/prototype

# Make the package.
rm -rf $destdir/$package_name
(cd $installdir && pkgmk -d $destdir -b $installdir) || exit 1

# Copy in the install script for the "none" class (omitted from the
# package above).
mkdir -p $destdir/$package_name/install
cp $sourcedir/packs/build/sunpkg/class/i.none \
  $destdir/$package_name/install || exit 1
chmod 644 $destdir/$package_name/install/i.none

# Build a compressed cpio archive of the package's files in class "none".
filelist=`mktemp -t filelist.XXXXXXXXXX`
if [ -z "$filelist" ]; then
  echo "Cannot create temporary file list." >&2
  exit 1
fi
mkdir -p $destdir/$package_name/archive
awk '(($2 == "f" || $2 == "v" || $2 == "e") && $3 == "none") { print $4; }' \
  $destdir/$package_name/pkgmap > $filelist
if [ -s $filelist ]; then
  (cd $destdir/$package_name/reloc && cpio -oc -C 512 < $filelist) | \
    bzip2 -c > $destdir/$package_name/archive/none.bz2 || exit 1
  if [ ! -s $destdir/$package_name/archive/none.bz2 ]; then
    echo "Failed to create compressed archive" >&2
    exit 1
  fi
  # The archived files are no longer needed in the package reloc directory.
  (cd $destdir/$package_name/reloc && cat $filelist | xargs rm -f)
fi

# Clean up.
rm -f $prototype
rm -f $filelist
rm -rf $tmpdir

exit 0
