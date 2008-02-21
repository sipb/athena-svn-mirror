# Interface: define variables name, daversionappend, and function
# hack_package ().

set -e

: ${DEBATHENA_APT=/mit/debathena/apt}

munge_sections () {
    perl -0pe 's/^Section: /Section: debathena-system\//gm or die' -i debian/control
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
    perl -0pe 'open THREE, "</dev/fd/3"; $x = <THREE>; s/(^Description:.*\n( .*\n)*)/$1$x/gm or die' -i debian/control 3<&0
}

add_build_depends () {
    perl -0pe 's/^(Build-Depends:.*)$/$1, '"$1"'/m or die' -i debian/control
}

add_debathena_provides () {
    perl -0pe 's/^(Package: (.*)\n(?:(?!Provides:).+\n)*)(?:Provides: (.*)\n((?:.+\n)*))?(?=\n|\z)/$1Provides: $3, debathena-$2\n$4/mg or die; s/^Provides: , /Provides: /mg' -i debian/control
    add_changelog "Provide debathena-$name."
}

cmd_source () {
    if [ "$a" != "-A" ]; then
	echo "Nothing to do for $dist_arch." >&2
	exit 0
    fi
    echo "Building source for $name-$daversion on $dist_arch" >&2
    
    if ! [ -e "${name}_$version.dsc" ]; then
	schroot -c "$chroot" -- apt-get -d source "$name"
    fi
    
    if ! [ -e "${name}_$daversion.dsc" ]; then
	tmpdir=$(mktemp -td "debathenify.$$.XXXXXXXXXX")
	trap 'rm -rf "$tmpdir"' EXIT
	origversion=$(echo "$version" | sed 's/-[^-]*$//')
	cp -a "${name}_$origversion.orig.tar.gz" "$tmpdir/"
	#echo "! [ -e '$tmpdir/${name}_$origversion.orig.tar.gz' ] || diff -u <(xxd '${name}_$origversion.orig.tar.gz') <(xxd '$tmpdir/${name}_$origversion.orig.tar.gz')" >| /tmp/wtf
	dpkg-source -x "${name}_$version.dsc" "$tmpdir/$name-$origversion"
	
	(
	    set -x
	    cd "$tmpdir/$name-$origversion"
	    dch_done=
	    hack_package
	    [ -n "$dch_done" ]
	    schroot -c "$chroot" -u root -- sh -c "apt-get -q -y build-dep '$name' && apt-get -q -y install devscripts && su '$USER' -c 'debuild -S'"
	)
	[ $? -eq 0 ] || {
	    bash -c "diff -u <(xxd '${name}_$origversion.orig.tar.gz') <(xxd '$tmpdir/${name}_$origversion.orig.tar.gz')"
	    exit 1
	}
	
	cp -a "$tmpdir/${name}_$daversion"* .
	if [ -n "$DA_CHECK_DIFFS" ]; then
	    interdiff -z "${name}_$version.diff.gz" "${name}_$daversion.diff.gz" | \
		enscript --color --language=ansi --highlight=diffu --output=- -q | \
		less -R
	    echo -n "Press Enter to continue: " >&2
	    read dummy
	fi
    fi
}

cmd_binary () {
    sbuildhack "$dist_arch" $a "${name}_$daversion.dsc"
}

v () {
    echo "$@"
    "$@"
}

cmd_upload () {
    REPREPRO="v reprepro -Vb $DEBATHENA_APT"
    REPREPROI="$REPREPRO --ignore=wrongdistribution"
    
    case "$dist" in
	sarge) tag='~debian3.1' ;;
	etch) tag='~debian4.0' ;;
	lenny) tag='~debian4.1~0.1' ;;
	dapper) tag='~ubuntu6.06' ;;
	edgy) tag='~ubuntu6.10' ;;
	feisty) tag='~ubuntu7.04' ;;
	gutsy) tag='~ubuntu7.10' ;;
    esac
    
    if [ "$a" = "-A" ]; then
	$REPREPROI include "$dist" "${name}_${daversion}_source.changes"
    fi
    $REPREPRO include "$dist" "${name}_${daversion}${tag}_${arch}.changes"
}

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

version=$(
    schroot -q -c "$chroot" -- apt-cache showsrc "$name" | \
	sed -n 's/^Version: \(.*\)$/\1/ p' | (
	version='~~~'
	while read -r newversion; do
	    if dpkg --compare-versions "$newversion" '>' "$version"; then
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

if zcat "$DEBATHENA_APT/dists/$dist/debathena-system/binary-$arch/Packages.gz" | \
    dpkg-awk -f - "Source:^$name\$" "Version:^$daversion~" -- Architecture | \
    if [ "$a" = "-A" ]; then cat; else fgrep -vx 'Architecture: all'; fi | \
    grep -q .; then
    echo "$name $daversion already exists for $dist_arch." >&2
    exit 0
fi

for cmd; do
    "cmd_$cmd"
done
