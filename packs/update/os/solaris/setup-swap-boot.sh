#!/sbin/sh

# Exit with an error message.
die() {
  echo "*** setup-swap-boot failed ***"
  exit 1
}

PATH=/srvd/install:$PATH
export PATH

method=$1
newvers=$2

# Find the swap device and try to shut it down.

swapdev=`swap -l | awk '/\/dev\/dsk/ {print $1; exit;}'`
rswapdev=`echo "$swapdev" | sed -e 's,^/dev/dsk,/dev/rdsk,'`
swap -d "$swapdev" || {
  echo "Unable to shut down swapping, so cannot populate swap partition"
  echo "with a miniroot."
  exit 2
}

# Put a miniroot in the swap partition, and prepare to boot it.

swapmount=/var/athena/update.miniroot

echo "Preparing swap partition for boot..."

rm -rf "$swapmount"
mkdir "$swapmount" || die
newfs -v "$swapdev" < /dev/null || die
mount "$swapdev" "$swapmount" || die

echo "Untarring miniroot..."
(cd "$swapmount" && zcat /install/miniroot/miniroot.tar | tar xf -) || die

echo "Miniroot customization: hostname and address, gateway, netmasks.."
cp -p /etc/name_to_major "$swapmount/etc" || die
cp -p /etc/name_to_sysnum "$swapmount/etc" || die
cp -p /etc/driver_* "$swapmount/etc" || die
cp -p /etc/device.tab "$swapmount/etc" || die
cp -p /etc/devlink.tab "$swapmount/etc" || die
cp -p /etc/dgroup.tab "$swapmount/etc" || die
cp -p /etc/hosts "$swapmount/etc" || die
cp -p /etc/nodename "$swapmount/etc" || die
cp -p /etc/netmasks "$swapmount/etc" || die
cp -p /etc/defaultrouter "$swapmount/etc" || die
cp -p /etc/networks "$swapmount/etc" || die
cp -p "/etc/hostname.$NETDEV" "$swapmount/etc" || die

echo "Making devices..."
(cd $swapmount && drvconfig -r devices "-p$swapmount/etc/path_to_inst") || die
devlinks -r "$swapmount" || die
disks -r "$swapmount" || die

# Let the next boot stage know what it's supposed to do.
echo "Athena Workstation ($HOSTTYPE) Version BootSwap" \
	"$method $newvers `date`" >> $CONFDIR/version
cp "$CONFDIR/version" "$swapmount$CONFDIR/version"

echo "Copying update configuration variables onto the miniroot..."
mkdir -p $swapmount`dirname $CONFVARS`
cp $CONFVARS $swapmount/$CONFVARS

echo "Copying finish-update onto the miniroot..."
cp /srvd/etc/rc2.d/S70finish-update $swapmount/etc/rc2.d/S70finish-update

echo "Making the new vfstab..."
awk '
	$4 == "ufs"	{ if ($3 == "/") $3 = "";
			  printf "%s\t%s\t/root%s\t%s\t%s\tyes\t%s\n", \
			  $1, $2, $3, $4, $5, $7; }
	$4 == "proc"	{ print }
	$4 == "fd"	{ print }
	' /etc/vfstab > $swapmount/etc/vfstab
echo "$swapdev	$rswapdev	/	ufs	1	no	-" \
	>> $swapmount/etc/vfstab

echo "Installing the boot blocks..."
installboot "/os/usr/platform/$SUNPLATFORM/lib/fs/ufs/bootblk" "$rswapdev"

echo "Saving and changing the prom boot device..."
oldboot=`/os/usr/platform/$SUNPLATFORM/sbin/eeprom boot-device`
if [ sun4u = "$SUNPLATFORM" ]; then
  newboot=`ls -l "$swapdev" | sed -e 's,^.*-> ../../devices,,' -e 's/sd/disk/' -e 's/dad/disk/'`
else
  newboot=`ls -l "$swapdev" | sed -e 's,^.*-> ../../devices,,'`
fi
echo "$oldboot" > $swapmount/etc/boot-device
/os/usr/platform/$SUNPLATFORM/sbin/eeprom "boot-device=$newboot"

echo "Finishing up..."
cd /
umount "$swapmount"
fsck -y -F ufs "$rswapdev"
rmdir $swapmount
