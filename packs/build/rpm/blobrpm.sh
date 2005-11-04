#!/bin/sh
# $Id: blobrpm.sh,v 1.1 2005-11-04 17:25:02 ghudson Exp $

# Convert the contents of the current directory tree into an RPM.
# This script is not used by the Athena build proper, but is instead
# used to convert binary releases into RPMs which can be installed
# using the Athena install and update process.

usage="blobrpm [-d destdir] [-s srcdir] pkgname"

destdir=..
srcdir=/mit/source
while getopts d:s: opt; do
  case "$opt" in
  d)
    destdir="$OPTARG"
    ;;
  s)
    srcdir="$OPTARG"
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

pkgname=$1
spectemplate=$srcdir/packs/build/rpm/specs/$pkgname

# Verify that the spec file template is readable.
if [ ! -r "$spectemplate" ]; then
  echo "Cannot read spec file template: $spectemplate" >&2
  exit 1
fi

# Extract the package name and version from the template.
version=`sed -n -e 's/^Version: //p' "$spectemplate"`

# Create a temporary working area.
if ! tmp=`mktemp -d -t blobrpm.XXXXXXXXXX`; then
  echo "Cannot create temporary area." >&2
  exit 1
fi

mkdir -p $tmp/BUILD $tmp/SOURCES $tmp/INSTALL $tmp/SPECS $tmp/SRPMS
mkdir -p $tmp/RPMS/i386

cat > $tmp/rpmmacros << EOF
%_topdir	$tmp
EOF

cat > $tmp/rpmrc << EOF
macrofiles:	/usr/lib/rpm/macros:/usr/lib/rpm/%{_target}/macros:/etc/rpm/macros:/etc/rpm/%{_target}/macros:$tmp/rpmmacros
EOF

# Create a source tarball containing a tarball of the current tree.
srcdir="${pkgname}-$version"
mkdir -p "$tmp/$srcdir"
tar cf "$tmp/$srcdir/blob.tar" .
tar -C "$tmp" -czf "$tmp/SOURCES/source.tar.gz" "$srcdir"
rm -rf "$tmp/$srcdir"

# Create the spec file from the template.
specfile=$tmp/SPECS/$pkgname
cat > $specfile << EOF
Distribution: Athena
Vendor: MIT Athena
Source: $tmp/SOURCES/source.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-root
EOF

cat "$spectemplate" >> $specfile

cat >> $specfile << 'EOF'

%prep
%setup
%build

%install
rm -rf "$RPM_BUILD_ROOT"
mkdir "$RPM_BUILD_ROOT"
tar -C "$RPM_BUILD_ROOT" -xf blob.tar
find "$RPM_BUILD_ROOT" -type f -or -type l |
  sed -e "s|$RPM_BUILD_ROOT||" | sort > rpm.filelist

%files -f rpm.filelist
EOF

# Make the package.
rpmbuild -ba --rcfile=$tmp/rpmrc --target=i386-athena-linux \
  --buildroot=$tmp/INSTALL $tmp/SPECS/$pkgname || exit 1

# Copy the package to the output directory and clean up.
cp "$tmp"/RPMS/i386/* "$destdir"
rm -rf "$tmp"
