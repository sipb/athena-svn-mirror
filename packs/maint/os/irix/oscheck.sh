#!/bin/sh

# $Id: oscheck.sh,v 1.2 2000-06-21 20:51:05 ghudson Exp $

# This script checks the integrity of the IRIX OS installation, by
# running the os-checkfiles program against the appropriate set of
# stats files for this machine architecture.

usage="Usage: $0 [-n|-y] [-o <osroot>] [-r <root>]"

. /etc/athena/rc.conf

# The default is check-only for non-PUBLIC machines.
if [ "$PUBLIC" = true ]; then
    noop=
else
    noop="-n"
fi

rootdir=/
os=/os
libdir=/install/oscheck
checkfiles=os-checkfiles
srvdstats=/srvd/usr/athena/lib/stats/sys_rvd
configfiles=/srvd/usr/athena/lib/update/configfiles
globalexceptions=$libdir/exceptions.all
localexceptions=/etc/athena/oscheck.exceptions
timestampfile=/var/athena/.oscheck
exceptions=/tmp/exceptions$$
myprods=/tmp/myprods$$
osprods=/tmp/osprods$$

while getopts no:r:y opt; do
    case "$opt" in
	n)
	    noop="-n"
	    ;;
	o)
	    os=$OPTARG
	    ;;
	r)
	    rootdir=$OPTARG
	    ;;
	y)
	    noop=
	    ;;
	\?)
	    echo "$usage" 1>&2
	    exit 1
	    ;;
    esac
done

get_hdwr()
{
    eval `hinv | awk '\
	/CPU: MIPS/		{ p = $3 }				\
	/Graphics board:/	{ g = $3 }				\
	/graphics installed/	{ g = $1 }				\
	END		{ printf("cpu=%s; graphics=%s", p, g); }'`
    case ${cpu} in
	R4400)
	    hdwr="44"
	    ;;
	R4600)
	    hdwr="46"
	    ;;
	R5000)
	    hdwr="50"
	    ;;
	*)
	    echo "Unrecognized CPU type: ${cpu}" 1>&2
	    exit 1
	    ;;
    esac

    case ${graphics} in
	GR3-XZ)
	    hdwr="${hdwr}xz"
	    ;;
	Indy)
	    hdwr="${hdwr}xl"
	    ;;
	CRM)
	    hdwr="${hdwr}cr"
	    ;;
	*)
	    echo "Unrecognized graphics subsystem: ${graphics}" 1>&2
	    exit 1
	    ;;
    esac
}

if [ -f $timestampfile ]; then
    timestamp="-t $timestampfile"
fi

# Determine which hardware we're running on.
get_hdwr

# Generate the exception list.  Start with the known global exceptions,
# if any.
rm -f $exceptions
if [ -r $globalexceptions ]; then
    cp $globalexceptions $exceptions
else
    cp /dev/null $exceptions
fi

# We don't want to bother with anything tracked from the srvd, so
# extract all srvd paths from the srvd stats file, to add them to
# the exception list.
awk '{ print substr($1, 2, length($1) - 1); }' < $srvdstats >> $exceptions
sed -e 's,^/,,' $configfiles >> $exceptions

# If machine is not PUBLIC, we allow for other OS packages to be
# installed (or removed).  Any files in those packages are excepted.

if [ "$PUBLIC" = false ]; then
    rm -f $myprods $myprods.uniq $osprods $osprods.uniq

    # Get a list of products installed on this machine with their
    # version numbers.
    showprods -n -F -3 -r $rootdir \
	| awk '($1 == "I" && NF == 4) { print $2, $3; }' | sort -u > $myprods

    # Get a list of the products in a standard installation.
    if [ -r $libdir/prods.$hdwr ]; then
	cat $libdir/prods.common $libdir/prods.$hdwr | sort -u > $osprods
    else
	cp $libdir/prods.common $osprods
    fi

    # Except files from products installed only on this machine.
    comm -13 $osprods $myprods > $myprods.uniq
    if [ -s $myprods.uniq ]; then
	showfiles -s -r $rootdir `awk '{ print $1; }' $myprods.uniq` \
	    >> $exceptions
    fi

    # Except files from products removed from this machine, i.e. in
    # standard installation, but missing here.
    comm -23 $osprods $myprods > $osprods.uniq
    if [ -s $osprods.uniq ]; then
	showfiles -s -r $os `awk '{ print $1; }' $osprods.uniq` >> $exceptions
    fi

    # Add any local exceptions to the list
    if [ -s $localexceptions ]; then
	cat $localexceptions >> $exceptions
    fi
fi

rm -f $exceptions.sorted
sort -u $exceptions > $exceptions.sorted

# Check machine-dependent files, if there is a stats file for this
# architecture.
archstats=$libdir/stats.$hdwr
machroot=$libdir/mach/$hdwr
if [ -r $archstats ]; then
    echo "Checking files for $hdwr architecture..."
    $checkfiles $noop -r $rootdir -o $os $timestamp \
	-x $exceptions.sorted $archstats
    if [ -r $archstats.dup ]; then
	echo "Checking non-shared files for $hdwr architecture..."
	$checkfiles $noop -r $rootdir -o $machroot $timestamp \
	    -x $exceptions.sorted $archstats.dup
    fi
else
    echo "No stats file for $hdwr architecture, $archstats"
fi

# Check common files
echo "Checking common files..."
$checkfiles $noop -r $rootdir -o $os $timestamp -x $exceptions.sorted \
    $libdir/stats.common

# Update last-checked time
if [ -z "$noop" ]; then
    touch $timestampfile
fi

# Clean up
rm -f $exceptions $exceptions.sorted $myprods $myprods.uniq
rm -f $osprods $osprods.uniq
