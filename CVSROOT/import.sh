#!/bin/sh
# $Id: import.sh,v 1.6 2003-02-26 05:41:50 ghudson Exp $

# import - Interactive scripts to do Athena imports conveniently and correctly
#
# Usage: import [-d repdir] [-n pkgname] [-v pkgver] tarfile oldver
#
# tarfile may be a .tar.gz or .tar.bz2 file.  It must be an absolute
#  path.
# pkgname and pkgver are extracted from the name of tarfile by
#  default; this only works if the tarfile name is
#  pkgname-pkgver.tar.{gz,bz2} and pkgver is composed entirely of
#  dots and digits.  Otherwise, specify them by hand.
# repdir defaults to third/pkgname.
# If this is the first import, specify an empty oldver ("" on the
#  command line).
# You will get a chance to confirm the parameters before things start.

# Bomb out on any error.
set -e

echorun() {
  echo ""
  echo "$@"
  "$@"
}

confirmrun() {
  echo ""
  echo "$1 command:"
  shift
  echo " " "$@"
  echo ""
  printf "Please confirm:"
  read dummy
  "$@"
}

# Process arguments.
while getopts d:n:v: opt; do
  case "$opt" in
  d)
    repdir=$OPTARG
    ;;
  n)
    pkgname=$OPTARG
    ;;
  v)
    pkgver=$OPTARG
    ;;
  \?)
    echo "$usage"
    exit 1
    ;;
  esac
done
shift `expr $OPTIND - 1 || true`
if [ $# -ne 2 ]; then
  echo "Usage: import [-d repdir] [-n pkgname] [-v pkgver] tarfile oldver" >&2
  exit 1
fi
tarfile=$1
oldver=$2

# We handle either gzipped or bzip2-ed tarfiles; check which we got.
case $tarfile in
*.tar.gz)
  dcmd='gzip -cd'
  base=`basename "$tarfile" .tar.gz`
  ;;
*.tgz)
  dcmd='gzip -cd'
  base=`basename "$tarfile" .tgz`
  ;;
*.tar.bz2)
  dcmd='bzip2 -cd'
  base=`basename "$tarfile" .tar.bz2`
  ;;
*)
  echo "Unrecognized tarfile extension for $tarfile." >&2;
  exit 1
  ;;
esac

# Compute package name, package version, tag, and repository directory.
: ${pkgname=`expr "$base" : '\(.*\)-[0-9\.]*$'`}
: ${pkgver=`expr "$base" : '.*-\([0-9\.]*\)$'`}
: ${repdir=third/$pkgname}
tag=${pkgname}-`echo "$pkgver" | sed -e 's/\./_/g'`
if [ -n "$oldver" ]; then
  oldtag=${pkgname}-`echo "$oldver" | sed -e 's/\./_/g'`
fi

# Figure out what this tarfile unpacks into.
tardir=`$dcmd "$tarfile" | tar -tf - | sed -e 's|/.*$||' | uniq`
if [ `echo "$tardir" | wc -l` -ne 1 ]; then
  printf "%s unpacks into multiple dirs:\n%s\n" "$tarfile" "$tardir" >&2
  exit 1
fi

# Confirm parameters.
echo "Package name:         $pkgname"
echo "Package version:      $pkgver"
echo "Tarfile unpacks into: $tardir"
echo "Repository directory: $repdir"
echo "Release tag:          $tag"
echo "Old release tag:      $oldtag"
echo ""
printf "Please confirm:"
read dummy

# Point CVS at the Athena repository.
CVSROOT=/afs/dev.mit.edu/source/repository
export CVSROOT

# Create temporary area and go there.
tmpdir=/tmp/import.$$
mkdir "$tmpdir"
cd "$tmpdir"

# Extract the tarfile and massage it.
$dcmd "$tarfile" | tar -xf -
cd "$tardir"
find . -name .cvsignore -exec rm {} \;
find . -name CVS -type d -exec rm -rf {} \; -prune
perl "$CVSROOT/CVSROOT/timestamps.pl"

# Do the import.
confirmrun "Import" \
  cvs import -d -I ! -m "Import $pkgname $pkgver." "$repdir" vendor "$tag" \
  || true   # Exits with status 1 on many non-fatal errors.

# If there's no old version to merge with, that's it.
if [ -z "$oldtag" ]; then
  exit
fi

# Delete removed files on the vendor branch.
cd "$tmpdir"
echorun cvs co -r vendor "$repdir"
cd "$repdir"
echorun cvs update -j "$oldtag" -j "$tag"
echorun cvs -q update
confirmrun "Commit" cvs ci -m "Not present in $pkgname $pkgver."

# Do the merge.
cvs update -A
echorun cvs update -d -P
echorun cvs update -j "$oldtag" -j "$tag"
echorun cvs -q update || true	# Exits with status 1 if there are conflicts.
echo ""
echo "Resolve conflicts in $tmpdir/$repdir before confirming commit command."
confirmrun "Commit" cvs ci -m "Merge with $pkgname $pkgver."

# Clean up the temporary area.
cd "$HOME"
rm -rf "$tmpdir"
