#!/sbin/sh

method=$1
newvers=$2

# Shut down swapping.

swapdev=/dev/swap

# Note that shutting down swap should always succeed on a reboot - in that
# case we are run before S48savecore.
swap -d $swapdev
if [ $? -ne 0 ]; then
	echo "Unable to shut down swapping, so cannot populate swap partition"
	echo "with a miniroot."
	exit 2
fi

# Put a miniroot in the swap partition, and prepare to boot it.

swapmount=/var/athena/update.miniroot

echo "Preparing swap partition for boot..."

rm -rf $swapmount
mkdir $swapmount
if [ $? -ne 0 ]; then
	echo "Failure making mountpoint for swap partition."
	exit 1
fi

mkfs_efs $swapdev
if [ $? -ne 0 ]; then
	echo "Failure building filesystem on $swapdev."
	exit 1
fi

mount $swapdev $swapmount
if [ $? -ne 0 ]; then
	echo "Failure mounting $swapdev on $swapmount."
	exit 1
fi

echo "Untarring miniroot..."

(cd $swapmount; zcat /install/miniroot/miniroot.tar.Z | tar xf -)
if [ $? -ne 0 ]; then
	echo "Failed untarring miniroot."
	exit 1
fi

# Indy's have different kernels to choose from, based on the
# CPU/graphics combination.  There's also different versions
# of binaries such as /lib32/libc.so.1 between the R4000 and
# R5000 architectures.  These are split into different tar sets.
if [ `uname -m` = "IP22" ]; then
	eval `hinv | awk '\
	    /CPU: MIPS/			{ p = $3 }			\
	    /Graphics board:/		{ g = $3 }			\
	    /graphics installed/	{ g = $1 }			\
	    END		{ printf("cpu=%s; graphics=%s", p, g); }'`

	# Copy in files specific to the hardware combination (kernel)
	case ${cpu} in
	    R4600)
		hdwr=46
		;;
	    R5000)
		hdwr=50
		;;
	    *)
		echo "Unrecognized CPU type: ${cpu}"
		exit 1
		;;
	esac

	case ${graphics} in
	    GR3-XZ)
		hdwr=${hdwr}xz
		;;
	    Indy)
		hdwr=${hdwr}xl
		;;
	    *)
		echo "Unrecognized graphics subsystem: ${graphics}"
		exit 1
		;;
	esac

	echo "Untarring miniroot.${hdwr}..."
	(cd $swapmount; \
	    zcat /install/miniroot/miniroot.${hdwr}.tar.Z | tar xf -)
	if [ $? -ne 0 ]; then
	    echo "Failed untarring miniroot.${hdwr}."
	    exit 1
	fi

	# Copy in files specific to the CPU architecture
	case ${cpu} in
	    R4?00)
		cpuarch=R4000
		;;
	    R5?00)
		cpuarch=R5000
		;;
	    *)
		echo "$prog: Unrecognized CPU architecture: ${cpu}"
		exit 1
		;;
	esac

	echo "Untarring miniroot.${cpuarch}..."
	(cd $swapmount; \
	    zcat /install/miniroot/miniroot.${cpuarch}.tar.Z | tar xf -)
	if [ $? -ne 0 ]; then
	    echo "Failed untarring miniroot.${cpuarch}."
	    exit 1
	fi

fi

# The miniroot may be a different OS version from
# the currently running system.
if [ -r /install/miniroot/OSVERSION ]; then
	AIS_OSVERSION=`cat /install/miniroot/OSVERSION`
	export AIS_OSVERSION
fi

/srvd/install/netconf --swap --root $swapmount --hostname $HOST --ipaddr $ADDR

# XXX Kernel needs to be tweaked to boot with the appropriate root
# XXX partition. This should be the only thing that currently has a
# XXX partition hard-coded.

# Pass on information about where the proper root partition is.
# Note this code assumes there is just one big root partition, and
# no subpartitions like /usr. (The AFS cache partition isn't relevant.)
rootdev=`devnm / | awk '{ print $1 }'`
echo "ROOTDEVICE=$rootdev" >> $CONFVARS

mkdir -p $swapmount`dirname $CONFVARS`
cp $CONFVARS $swapmount/$CONFVARS

# Let the next boot stage know what it's supposed to do.
echo "Athena Workstation ($HOSTTYPE) Version BootSwap" \
	"$method $newvers `date`" >> "$CONFDIR/version"

# Prepare the miniroot to understand what it's supposed to do.
mkdir -p $swapmount$CONFDIR
cp $CONFDIR/version $swapmount$CONFDIR/version
cp /srvd/etc/rc2.d/S36finish-update $swapmount/etc/rc2.d
rm $swapmount/etc/rc2.d/S35afs

# Add needed Athena customizations
cp /etc/config/suppress-network-daemons $swapmount/etc/config
cp /srvd/etc/init.d/network $swapmount/etc/init.d

# Set ourselves up to boot into the swap partition.
template="scsi(%d)disk(%d)rdisk(0)partition(%d)"
if [ `uname -m` = "IP32" ]; then
	template="pci(0)$template"
fi
nvram OSLoadPartition `devnm $swapmount/var | nawk -F/ -v template=$template \
	'{ printf(template, substr($4,4,1), substr($4,6,1), substr($4,8,1)) }'`

# Clean up.
umount $swapmount
rmdir $swapmount
