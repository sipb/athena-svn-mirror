# Interface: define variables name, daversionappend, and function
# hack_package ().

set -e

: ${DEBATHENA_APT=/mit/debathena/apt}

# Process arguments.
dist_arch=$1; shift
a=
if [ "$1" = "-A" ]; then a=-A; shift; fi
chroot=$dist_arch-sbuild

if [ -z "$dist_arch" -o $# -eq 0 ]; then
    echo 'No arguments!' >&2
    exit 2
fi

dist=$(echo "$dist_arch" | sed 's/^\(.*\)-\([^-]*\)$/\1/')
arch=$(echo "$dist_arch" | sed 's/^\(.*\)-\([^-]*\)$/\2/')
: ${section=debathena-system}
: ${daname=$name}
: ${release=-proposed}
: ${maint=Debathena Project <debathena@mit.edu>}
. /mit/debathena/bin/debian-versions.sh
tag=$(gettag $dist)

if [ -e nobuild ] && fgrep -q "$dist" nobuild; then
  echo "Skipping $dist since it is listed in ./nobuild."
  exit
fi

# Create a chroot and define functions for using it.
sid=$(schroot -b -c "$chroot")
trap 'schroot -e -c "$sid"' EXIT
sch() { schroot -r -c "$sid" -- "$@"; }           # Run in the chroot
schq() { schroot -q -r -c "$sid" -- "$@"; }       # Run in the chroot quietly
schr() { schroot -r -c "$sid" -u root -- "$@"; }  # Run in the chroot as root
schr apt-get -qq -y update || exit 3

quote() {
  echo "$1" | sed 's/[^[:alnum:]]/\\&/g'
}

munge_sections () {
    perl -0pe "s/^Section: /Section: $section\\//gm or die" -i debian/control
}

add_changelog () {
    if [ -n "$dch_done" ]; then
	dch "$@"
    else
	echo | dch -v"${daversion}" -D unstable "$@"
	dch_done=1
    fi
}

append_description () {
    perl -0pe 'open THREE, "</dev/fd/3"; $x = <THREE>; s/(^Description:.*\n( .*\S.*\n)*)/$1$x/gm or die' -i debian/control 3<&0
}

add_build_depends () {
    perl -0pe 's/^(Build-Depends:.*(?:\n[ \t].*)*)$/$1, '"$1"'/m or die' -i debian/control
}

add_debathena_provides () {
    [ "$name" = "$daname" ]
    perl -0pe 's/^(Package: (.*)\n(?:(?!Provides:).+\n)*)(?:Provides: (.*)\n((?:.+\n)*))?(?=\n|\z)/$1Provides: $3, debathena-$2\n$4/mg or die; s/^Provides: , /Provides: /mg' -i debian/control
    add_changelog "Provide debathena-$name."
}

set_debathena_maintainer() {
    orig_maint="$(python -c 'from rfc822 import Message, AddressList as al; f = open("debian/control"); m=Message(f); print al(m.get("Maintainer")) + al(m.get("XSBC-Original-Maintainer"))')"
    sed -i -e '/^\(XSBC-Original-Maintainer\|Maintainer\)/d' debian/control
    MAINT="$maint" ORIG_MAINT="$orig_maint" perl -0pe 's/\n\n/\nMaintainer: $ENV{MAINT}\nXSBC-Original-Maintainer: $ENV{ORIG_MAINT}\n\n/' -i debian/control
    add_changelog "Update Maintainer to $maint."
}

rename_source () {
    perl -pe "s{^Source: $name\$}{Source: $daname}" -i debian/control
    add_changelog "Rename package to $daname."
    perl -0pe "s/^$name/$daname/" -i debian/changelog
}

cmd_source () {
    if [ "$a" != "-A" ]; then
	echo "Not building source package for $dist_arch." >&2
	return
    fi
    echo "Building source for $daname-$daversion on $dist_arch" >&2
    
    if ! [ -e "${name}_$version.dsc" ]; then
	sch apt-get -d source "$name"
    fi
    
    if ! [ -e "${daname}_$daversion.dsc" ]; then
	(
	    tmpdir=$(mktemp -td "debathenify.$$.XXXXXXXXXX")
	    trap 'rm -rf "$tmpdir"' EXIT
	    origversion=$(echo "$version" | sed 's/-[^-]*$//')
	    cp -a "${name}_$origversion.orig.tar.gz" "$tmpdir/"
	    dscdir=$(pwd)
	    cd "$tmpdir/"
	    dpkg-source -x "$dscdir/${name}_$version.dsc" "$tmpdir/$name-$origversion"
	    cd "$tmpdir/$name-$origversion"
	    dch_done=
	    schr apt-get -q -y install python
	    hack_package
            if [ "$name" != "$daname" ]; then
                rename_source
                cp -a "$tmpdir/${name}_$origversion.orig.tar.gz" "$tmpdir/${daname}_$origversion.orig.tar.gz"
                cp -a "$tmpdir/${daname}_$origversion.orig.tar.gz" "$dscdir"
            fi
	    [ -n "$dch_done" ]
	    schr apt-get -q -y install devscripts pbuilder
	    schr /usr/lib/pbuilder/pbuilder-satisfydepends
	    sch debuild -S -sa -us -uc -i -I.svn && cp -a "../${daname}_$daversion"* "$dscdir"
	)
	[ $? -eq 0 ] || exit 1
	
	if [ -n "$DA_CHECK_DIFFS" ]; then
	    interdiff -z "${name}_$version.diff.gz" "${daname}_$daversion.diff.gz" | \
		enscript --color --language=ansi --highlight=diffu --output=- -q | \
		less -R
	    echo -n "Press Enter to continue: " >&2
	    read dummy
	fi
    fi
}

cmd_binary () {
    sbuildhack "$dist_arch" $a "${daname}_$daversion.dsc"
}

v () {
    echo "$@"
    "$@"
}

cmd_upload () {
    if [ "$a" = "-A" ]; then
	v dareprepro include "${dist}${release}" "${daname}_${daversion}_source.changes"
    fi
    v dareprepro include "${dist}${release}" "${daname}_${daversion}${tag}_${arch}.changes"
}

version=$(
    sch apt-cache showsrc "$name" | \
	sed -n 's/^Version: \(.*\)$/\1/ p' | (
	version='~~~'
	while read -r newversion; do
	    if [ $(expr "$newversion" : '.*debathena') = 0 ] && \
		dpkg --compare-versions "$newversion" '>' "$version"; then
		version=$newversion
	    fi
	done
	if [ "$version" = '~~~' ]; then
	    echo "No version of $name found." >&2
	    exit 1
	fi
	echo "$version"
	)
    )
daversion=$version$daversionappend

# Look for binary packages built from the named package with the right
# version, and exit out if we find one (an architecture-specific one
# if we weren't run with the -A flag).  We need to look for either a
# Source: or a Package: header matching $name since there is no
# Source: header for a package whose name matches its source.
pkgfiles="$DEBATHENA_APT/dists/$dist/$section/binary-$arch/Packages.gz $DEBATHENA_APT/dists/${dist}-proposed/$section/binary-$arch/Packages.gz"
if { zcat $pkgfiles | \
    dpkg-awk -f - "Package:^$daname\$" "Version:^$(quote "$daversion$tag")\$" -- Architecture;
    zcat $pkgfiles | \
    dpkg-awk -f - "Source:^$daname\$" "Version:^$(quote "$daversion$tag")\$" -- Architecture; } \
    | if [ "$a" = "-A" ]; then cat; else fgrep -vx 'Architecture: all'; fi \
    | grep -q .; then
    echo "$daname $daversion already exists for $dist_arch." >&2
    exit 0
fi

for cmd; do
    "cmd_$cmd"
done
