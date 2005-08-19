#!/bin/sh -euk

### This is the second script file in the Athena workstation
### installation program.  It is called by the first script,
### athenainstall.

### $Id: install1.sh,v 1.33 2005-08-19 15:54:03 rbasch Exp $

echo "Set some variables"
PATH=/sbin:/usr/bin:/usr/sbin:/os/usr/bin:/usr/athena/bin
export PATH
umask 2

# bootdevice is needed if installing 9.1 (and perhaps earlier)
# versions of Athena.
bootdevice="disk"; export bootdevice

# Use format to get information about the available drives.
format < /dev/null | awk '/^[ 	]*[0-9]\./ { print; }' > /tmp/disks

ndrives=`wc -l /tmp/disks`
case `uname -m`,`uname -i` in
sun4u,SUNW,Sun-Fire-280R|sun4u,SUNW,Sun-Fire-V440)
	defaultdrive=c1t0d0;
	;;
sun4u,*)
	defaultdrive=c0t0d0;
	;;
*)
	defaultdrive=c0t3d0;
	;;
esac

if [ "$ndrives" -gt 1 ]; then
	echo ""
	echo "WARNING: This system has multiple disks.  A non-custom install"
	echo "will pick what we believe is the internal disk, $defaultdrive."
	echo "Choose a custom installation if this disk isn't right."
	echo ""
fi
echo "Custom installation (will default to 'n' after 60 seconds) [n]? \c"

case x`/util/to 60` in
xtimeout|xn|xN|x)
    CUSTOM=N
    echo "\nDoing standard installation"
    ;;
*)
    CUSTOM=Y
    echo "\nDoing custom installation"
    ;;
esac
export CUSTOM

case $CUSTOM in
Y)
    if [ "$ndrives" -gt 1 ]; then
	echo ""
	echo "This system has multiple disks.  You must select a disk"
	echo "to install onto."
	echo ""
	cat /tmp/disks
	echo ""
	echo "Please select a disk by number: \c"
	read selection
	drive=`awk "/ $selection\./ "'{ print $2; exit; }' /tmp/disks`
	if [ -z "$drive" ]; then
	    echo "Invalid selection, starting over."
	    exit 1
	fi
    else
	# Only one drive, pick it.
	drive=`awk '{ print $2; }' /tmp/disks`
    fi
    ;;
N)
    drive=$defaultdrive
    ;;
esac

# Define the partitions
echo "Define the partitions"

rootdrive=/dev/dsk/${drive}s0
rrootdrive=/dev/rdsk/${drive}s0

cachedrive=/dev/dsk/${drive}s3
rcachedrive=/dev/rdsk/${drive}s3

export drive rootdrive rrootdrive cachedrive rcachedrive

echo "Installing on ${drive}."

# Get the default Athena release for this host, by asking
# getcluster for a guaranteed low version number, and reading
# the NEW_PRODUCTION_RELEASE variable setting returned.  If
# there is no cluster information for the machine, fall back
# to the public cluster release.
# We also get the NEW_TESTING_RELEASE variable, so we recognize
# it as a valid choice below.
NEW_PRODUCTION_RELEASE=
NEW_TESTING_RELEASE=
while true; do
    eval `getcluster -b 0.0 2>/dev/null | \
	egrep 'NEW_PRODUCTION_RELEASE=|NEW_TESTING_RELEASE=`
    if [ -n "$NEW_PRODUCTION_RELEASE" -o -n "$NEW_TESTING_RELEASE" ]; then
	have_cluster=true
	break
    else
	eval `getcluster -h public-sun4 -b 0.0 | grep NEW_PRODUCTION_RELEASE=`
	have_cluster=false
	if [ -n "$NEW_PRODUCTION_RELEASE" ]; then
	    break
	fi
    fi
    echo "Cannot determine the default Athena release; will retry..."
    sleep 60
done

default_release=${NEW_PRODUCTION_RELEASE:-$NEW_TESTING_RELEASE}

major=`echo $default_release | awk -F. '{ print int($1); }'`
if [ "$major" -lt 9 ]; then
    echo "The production release for this machine is $default_release."
    echo "That release is no longer supported."
    exit 1
fi

case $CUSTOM in
N)
    REV=$default_release
    ;;

Y)
    prompt="Which Athena release do you want to install"
    prompt="$prompt [$default_release]?"
    REV=
    while [ -z "$REV" ]; do
	echo "$prompt \c"
	read rev
	case $rev in
	"")
	    REV=$default_release
	    ;;
	9.[0-4]|$NEW_PRODUCTION_RELEASE|$NEW_TESTING_RELEASE)
	    REV=$rev
	    ;;
	*.*)
	    echo "Release $rev is not supported."
	    ;;
	*)
	    echo "Invalid release"
	    ;;
	esac
    done
    ;;
esac

echo "installing $REV"

case $REV in
9.[0-2])
    rev93_or_later=N
    ;;
*)
    rev93_or_later=Y
    ;;
esac

if [ "$have_cluster" = true ]; then
    eval `getcluster -b $REV`
else
    eval `getcluster -h public-sun4 -b "$REV"`
fi

case $CUSTOM in
Y)
    echo "SYSLIB filsys name [$SYSLIB]? \c"
    read syslib
    if [ -n "$syslib" ]; then
	SYSLIB=$syslib
    fi
    ;;
esac

if [ -z "$SYSLIB" ]; then
    echo "Cannot get valid cluster information, aborting install."
    exit 1
fi

# Get the filsys records for the given name, recursing for type MUL.
get_filsys() {
    set -- `hesinfo "$1" filsys`
    case $1 in
    MUL)
	while [ -n "$2" ]; do
	    get_filsys $2
	    shift
	done
	;;
    AFS)
	case $4 in
	/srvd)
	    srvd=$2
	    ;;
	/os)
	    os=$2
	    ;;
	/install)
	    install=$2
	    ;;
	esac
    esac
}

get_filsys $SYSLIB
if [ -z "$srvd" ]; then
    echo "Cannot get valid filsys information, aborting install."
    exit 1
fi

# For a custom install, give the user an opportunity to override
# the default filesystems.
case $CUSTOM in
Y)
    echo "Enter path for /srvd [$srvd]: \c"
    read path
    if [ -n "$path" ]; then
	srvd=$path
    fi

    echo "Enter path for /os [$os]: \c"
    read path
    if [ -n "$path" ]; then
	os=$path
    fi

    echo "Enter path for /install [$install]: \c"
    read path
    if [ -n "$path" ]; then
	install=$path
    fi
    ;;
esac

# Make necessary symlinks for the various filesystems.
rm -rf /tmp/srvd /tmp/os /tmp/install /tmp/cdrom /tmp/patches
ln -s $srvd /tmp/srvd
if [ -n "$os" ]; then
    ln -s $os /tmp/os
fi
if [ -n "$install" ]; then
    case $rev93_or_later in
    Y)
	ln -s $install /tmp/install
	;;
    *)
	ln -s $install/cdrom /tmp/cdrom
	ln -s $install/patches /tmp/patches
	;;
    esac
fi

case $CUSTOM in
Y)
    echo "Automatic reboot after installation [n]? \c"
    read rebooting
    case "x$rebooting" in
    xn|xN|x)
	REBOOT=N; echo "No automatic reboot after installation"; export REBOOT
	;;
    *)
	REBOOT=Y; echo "Automatic reboot after installation"; export REBOOT
	;;
    esac
    ;;

*)     
    REBOOT=Y; echo "Automatic reboot after installation"; export REBOOT;
    ;;
esac

echo ""
ls -l /srvd
ls -l /tmp/srvd
ls -l /os
ls -l /tmp/os
case $rev93_or_later in
Y)
    ls -l /install
    ls -l /tmp/install
    ;;
*)
    ls -l /cdrom
    ls -l /tmp/cdrom
    ls -l /patches
    ls -l /tmp/patches
    ;;
esac

# Customize some rc.conf variables; only supported beginning in 9.3.
PUBLIC=true
AUTOUPDATE=true
RVDCLIENT=true
case $CUSTOM,$rev93_or_later in
Y,Y)
    echo ""
    echo "PUBLIC value for rc.conf [$PUBLIC]? \c"
    read public
    if [ -n "$public" ]; then
	PUBLIC=$public
    fi
    echo "AUTOUPDATE value for rc.conf [$AUTOUPDATE]? \c"
    read autoupdate
    if [ -n "$autoupdate" ]; then
	AUTOUPDATE=$autoupdate
    fi
    echo "RVDCLIENT value for rc.conf [$RVDCLIENT]? \c"
    read rvdclient
    if [ -n "$rvdclient" ]; then
	RVDCLIENT=$rvdclient
    fi
    echo ""
    ;;
esac
export PUBLIC AUTOUPDATE RVDCLIENT

echo "formatting  "
diskline=`format < /dev/null | grep "$drive"`
DISK=`echo "$diskline" | awk '{ print substr($3, 2, length($3) - 1); }'`
export DISK
echo $DISK

# first make sure that the disk has an external label (set it just in case)
cat /util/format.input.label | format ${drive} >/dev/null 2>&1

case $CUSTOM in
Y)
    echo "Partition the disk yourself [n]? \c"
    read partition
    case "x$partition" in
    xn|xN|x)
	PARTITION=N; echo "Doing standard partitions"; export PARTITION
	;;
    *)
	PARTITION=Y; echo "Doing custom partitions"; export PARTITION
	;;
    esac
    ;;
*)     
    PARTITION=N; echo "Doing standard partitions"
    ;;
esac

case $PARTITION in
Y)
    echo "The rest of the installation assumes that
    partition 1 is swap (at least 512MB recommended);
    partition 3 is AFS cache (256MB recommended);
    partition 0 is / (including /usr and /var) and is at least 8GB,
	but is normally the entire rest of the disk"
    sleep 10
    format $drive
    echo ""
    prtvtoc $rrootdrive
    echo ""
    echo "Enter <Return> to confirm install on $drive: \c"
    read foo
    ;;
*)
    case $DISK in
    SUN2.1G)
	echo "formatting SUN2.1G"
	cat /util/format.input.SUN2.1G | \
	    format ${drive} >/dev/null 2>&1
	;;
    SUN4.2G)
	echo "formatting SUN4.2G"
	cat /util/format.input.SUN4.2G | \
	    format ${drive} >/dev/null 2>&1
	;;
    SUN9.0G)
	echo "formatting SUN9.0G"
	cat /util/format.input.SUN9.0G | \
	    format ${drive} >/dev/null 2>&1
	;;
    SUN18G)
	echo "formatting SUN18G"
	cat /util/format.input.SUN18G | \
	    format ${drive} >/dev/null 2>&1
	;;
    SUN36G)
	echo "formatting SUN36G"
	cat /util/format.input.SUN36G | \
	    format ${drive} >/dev/null 2>&1
	;;
    ST315320A|ST315310A)
	echo "formatting ST315320A"
	cat /util/format.input.ST315320A | \
	    format ${drive} >/dev/null 2>&1
	;;
    ST320011A)
	echo "formatting ST320011A"
	cat /util/format.input.ST320011A | \
	    format ${drive} >/dev/null 2>&1
	;;
    ST320420A|ST320414A)
	echo "formatting ST320420A"
	cat /util/format.input.ST320420A | \
	    format ${drive} >/dev/null 2>&1
	;;
    ST34321A)
	echo "formatting ST34321A"
	cat /util/format.input.ST34321A | \
	    format ${drive} >/dev/null 2>&1
	;;
    ST34342A)
	echo "formatting ST34342A"
	cat /util/format.input.ST34342A | \
	    format ${drive} >/dev/null 2>&1
	;;
    ST38420A)
	echo "formatting ST38420A"
	cat /util/format.input.ST38420A | \
	    format ${drive} >/dev/null 2>&1
	;;
    ST39120A|ST39111A)
	echo "formating ST39120A"
	cat /util/format.input.ST39120A | \
	    format ${drive} >/dev/null 2>&1
	;;
    ST39140A)
	echo "formatting ST39140A"
	cat /util/format.input.ST39140A | \
	    format ${drive} >/dev/null 2>&1
	;;
    ST340824A|ST340016A)
	echo "formatting ST340824A"
	cat /util/format.input.ST340824A | \
	    format ${drive} >/dev/null 2>&1
	;;
    Seagate*)
	echo "formatting Segate Medalist"
	cat /util/format.input.Seagate.medalist | \
	    format ${drive} >/dev/null 2>&1
	;;
    *)
	mem=`machtype -M`
	fmtstring=`echo "$diskline" | awk '{
		ncyl = $(NF - 6);
		secpercyl = $(NF - 2) * substr($NF, 1, length($NF) - 1);
		swapsize = int(4 * mem / secpercyl) + 1;	# Twice memory
		cachesize = int(256 * 2048 / secpercyl) + 1;	# 256MB cache
		rootsize = ncyl - swapsize - cachesize;
		swapstart = rootsize;
		cachestart = rootsize + swapsize;
		if (rootsize * secpercyl < 8 * 1024 * 2048)	# 8GB minimum
			exit;
		printf "\npartition\n\n";
		printf "0\nroot\nwm\n0\n%dc\n\n", rootsize;
		printf "1\nswap\nwu\n%d\n%dc\n\n", swapstart, swapsize;
		printf "3\nunassigned\nwm\n%d\n%dc\n\n", cachestart, cachesize;
		printf "4\nunassigned\nwm\n0\n0c\n\n";
		printf "5\nunassigned\nwm\n0\n0c\n\n";
		printf "6\nunassigned\nwm\n0\n0c\n\n";
		printf "7\nunassigned\nwm\n0\n0c\n\n";
		printf "label\ny\nprint\nq\nq\n";
	}' mem="$mem"`
	if [ -n "$fmtstring" ]; then
		echo "Generating partition layout for $DISK"
		echo "$fmtstring" | format "$drive" > /dev/null 2>&1
	else
		echo "can't format the disks - type unknown"
		echo "Call an expert !"
		echo "Press ^C for a shell or Stop-A for the boot prompt."
		trap /bin/sh 2
		while :; do sleep 10; done
	fi
    esac
esac

# Installs prior to 9.3 test whether $partitioning is "one" or "many".
partitioning=one
export partitioning

echo "Making the filesystems..."
echo ""
echo "Making the root file system"
echo "y" | /usr/sbin/newfs -v  $rrootdrive

echo " Making the cache file system "
echo "y" | /usr/sbin/newfs -v $rcachedrive

# Set up the AFS cache partition, etc.  For pre-9.3 installs, we mounted
# /var/usr/vice instead of /usr/vice/cache.
echo "Adding AFS filesystem"
echo "Making an AFS cache available"
case $rev93_or_later in
Y)
    # Installing 9.3 or later; use the new layout.
    vice=/usr/vice
    mkdir -p $vice/cache
    mount -o nologging $cachedrive $vice/cache
    ;;
*)
    # Installing a pre-9.3 rev; use the old layout.
    vice=/var/usr/vice
    mount -o nologging $cachedrive $vice
    mkdir -p $vice/cache
    ;;
esac

chmod 0700 $vice/cache

# Create a cacheinfo file.
mkdir -p $vice/etc
df -k $cachedrive | awk '
    (NR == 2) { printf("/afs:/usr/vice/cache:%d\n", int($2 * 3 / 4)); }' \
	> $vice/etc/cacheinfo

# Create an empty CellServDB; we will start afsd with the -afsdb option,
# which will use AFSDB DNS records to find the db servers.
cp /dev/null $vice/etc/CellServDB

# Create a ThisCell file for afsd.
echo "athena.mit.edu" > $vice/etc/ThisCell
chmod 644 $vice/etc/ThisCell

echo "Making an /afs repository"
cd /
mkdir /tmp/afs
echo "Loading afs in the kernel"
modload /kernel/misc/sparcv9/nfssrv
modload /kernel/fs/sparcv9/afs
echo "Starting afsd "
/etc/afsd -afsdb -nosettime -daemons 4

# Allow setuid/setgid bits from the install cell(s), in case any such
# files are copied explicitly.
cells=`/bin/athena/fs whichcell /os /install /srvd 2>/dev/null | \
  sed -e "s/.* lives in cell '\(.*\)'/\1/" | sort -u`
for cell in $cells ; do
  echo "Allowing setuid/setgid programs from $cell"
  /bin/athena/fs setcell $cell -suid
done

type=install; export type
date >/tmp/install.log

case $REV in
9.[0-2])
    # For these revs, do not redirect descriptor 2, as that causes the
    # shell (possibly invoked at the end of install2.sh) not to issue a
    # prompt.  (Descriptor 2 is mostly redirected by the script
    # anyway).
    sh /srvd/install/install2.sh | tee -a /tmp/install.log
    ;;
*)
    sh /srvd/install/install2.sh 2>&1 | tee -a /tmp/install.log
    ;;
esac

echo "Some unexpected error occured"
echo "Please contact Athena Hotline at x3-1410."
echo "Thank you. -Athena Operations"
/sbin/sh
halt
