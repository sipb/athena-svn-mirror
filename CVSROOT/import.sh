#!/bin/sh
# $Id: import.sh,v 1.8 2004-09-25 05:36:17 ghudson Exp $

# import - Interactive scripts to do Athena imports conveniently and correctly
#
# Usage: import [-d repdir] [-n pkgname] [-o oldver] [-v pkgver] tarfile
#
# tarfile may be a .tar.gz or .tar.bz2 file.  It must be an absolute
#  path.
# pkgname and pkgver are extracted from the name of tarfile by
#  default; this only works if the tarfile name is
#  pkgname-pkgver.tar.{gz,bz2} and pkgver is composed entirely of
#  dots and digits.  Otherwise, specify them by hand.
# repdir defaults to third/pkgname.
# oldver is deduced from the RCS files in the existing repository dir,
#  if it exists.  If the package version numbers are not in X.Y.Z
#  form, specify the old version.  "none" means no old version.
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
unset repdir pkgname oldver pkgver
usage="Usage: import [-d repdir] [-n pkgname] [-o oldver] [-v pkgver] tarfile"
while getopts d:n:o:v: opt; do
  case "$opt" in
  d)
    repdir=$OPTARG
    ;;
  n)
    pkgname=$OPTARG
    ;;
  o)
    oldver=$OPTARG
    ;;
  v)
    pkgver=$OPTARG
    ;;
  \?)
    echo "$usage" >&2
    exit 1
    ;;
  esac
done
shift `expr $OPTIND - 1 || true`
if [ $# -ne 1 ]; then
  echo "$usage" >&2
  exit 1
fi
tarfile=$1

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

# Point CVS at the Athena repository.
CVSROOT=/afs/dev.mit.edu/source/repository
export CVSROOT

# Compute package name, package version, tag, and repository directory.
: ${pkgname=`expr "$base" : '\(.*\)-[0-9\.]*$'`}
: ${pkgver=`expr "$base" : '.*-\([0-9\.]*\)$'`}
: ${repdir=third/$pkgname}

# Compute the old version if not specified.
if [ "${oldver+set}" != set ]; then
  if [ ! -d "$CVSROOT/$repdir" ]; then
    oldver=none
  else
    oldver=`find $CVSROOT/$repdir -name "*,v" -print | xargs rlog -h | 
      perl -e '
	sub vercmp {
	  @a = split(/\./, shift @_); @b = split(/\./, shift @_);
	  while (@a and @b) {
	    return $a[0] <=> $b[0] if $a[0] != $b[0];
	    shift @a; shift @b;
	  }
	  return @a <=> @b;
	}
	$pkg = "'"$pkgname"'";
	$maxver = "0";
	while (<>) {
	  if (/^\t$pkg-([\d_]+):/) {
	    ($ver = $1) =~ s/_/./g;
	    $maxver = $ver if (vercmp($ver, $maxver) == 1);
	  }
	}
	print $maxver;'`
  fi
fi

tag=${pkgname}-`echo "$pkgver" | sed -e 's/\./_/g'`
if [ none != "$oldver" ]; then
  oldtag=${pkgname}-`echo "$oldver" | sed -e 's/\./_/g'`
fi

# Figure out what this tarfile unpacks into.
tardir=`$dcmd "$tarfile" | tar -tf - | sed -e 's|^\./||' -e 's|/.*$||' | uniq`
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
