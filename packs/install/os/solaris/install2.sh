# This file is run out of the srvd by install1.sh after it starts AFS.

umask 022

# Sun documents "uname -i" as the way to find stuff under /platform or
# /usr/platform.  We rely in the fact that the uname -i names are symlinks
# to the uname -m names, in order to save space on the root.  Be on the
# alert for changes.
platform=`uname -m`
cputype=`/sbin/machtype -c`

UPDATE_ROOT=/root; export UPDATE_ROOT
echo "Mounting hard disk's root partition..."
mount $rootdrive /root

echo "turn fs fast"
/util/fastfs /root fast

cd /
echo "Making dirs on root"
mkdir /root/var
mkdir /root/usr
mkdir /root/proc
ln -s var/rtmp /root/tmp

mkdir -p /root/usr/vice
mkdir /root/var/tmp
mkdir /root/var/rtmp
chmod 1777 /root/var/tmp
chmod 1777 /root/var/rtmp

echo "installing the os packages"
libdir=/srvd/usr/athena/lib/update
case `uname -m` in
sun4u)
    for i in `cat /install/cdrom/.order.install` ; do 
	echo $i
	pkgadd -M -a $libdir/admin-update -n -R /root -d /install/cdrom $i
    done
    ;;
*)
    echo "unsupported architecture - contact Athena administration"
    exit 1
    ;;
esac

echo "correction to pkg installation"
cp /install/cdrom/INST_RELEASE /root/var/sadm/system/admin/

rm /root/etc/.UNC*
cp /install/cdrom/.sysIDtool.state /root/etc/default/

echo "Installing Requested and Security patches for OS "
patches=`cat /install/patches/current-patches` 
yes | patchadd -d -R /root -u -M /install/patches/patches.link $patches

echo "add/remove osfiles as needed\n"
sh /srvd/install/oschanges 2>/dev/null
echo "the os part is installed"

# Install the "core" Athena packages; these are the packages needed by
# finish-install to do its work.
echo "Installing Athena core packages"
# Get the package directory for the current version.
vers=`awk '{ print $5; }' /srvd/.rvdinfo`
pkgdir=/srvd/pkg/$vers
core_pkgs=`pkginfo -c MITcore -d $pkgdir | awk '{ print $2; }'`
for pkg in `$libdir/pkg-order -d $pkgdir $core_pkgs` ; do
  echo $pkg
  pkgadd -M -a $libdir/admin-update -n -R /root -d $pkgdir $pkg
done
echo "Finished installing Athena core packages."

# Install the finish-update script; it will invoke finish-install.
cp -p /srvd/etc/init.d/finish-update /root/etc/init.d
rm -f /root/etc/rc2.d/S70finish-update
ln -s ../init.d/finish-update /root/etc/rc2.d/S70finish-update

echo "Create devices and dev"
cd /root
/usr/sbin/devfsadm -r /root -t /root/etc/devlink.tab -p /root/etc/path_to_inst

chmod 755 /root/dev
chmod 755 /root/devices
cp /dev/null /root/reconfigure

cd /root
echo "Creating other files/directories..."
mkdir -p afs mit 
chmod 1777 /root/tmp 

cp /dev/null etc/mnttab
cp /dev/null etc/.mnttab.lock
cp /dev/null etc/dumpdates

if=`ifconfig -au | awk -F: '/^[a-z]/ { if ($1 != "lo0") { print $1; exit; } }'`
if [ -z "$if" ]; then if=le0; fi

# Set the hostname to the fully-qualified name, forced to lower case.
# Also put the unqualified name in the hosts file.
fqdn=`host $netaddr | sed -n -e 's|^.*domain name pointer \(.*\)$|\1|p' | \
    sed -e 's|\.*$||'`
if [ -n "$fqdn" ]; then
    hostname=$fqdn
fi
hostname=`echo $hostname | /usr/bin/tr "[A-Z]" "[a-z]"`
case $hostname in
*.*)
    shortname=`echo $hostname | awk -F. '{ print $1; }'`
    ;;
*)
    # We do not know the fully-qualified name.  Assume the mit.edu domain.
    shortname=$hostname
    hostname=$hostname.mit.edu
    ;;
esac
echo "Host name is $hostname"
echo "Address is $netaddr"
echo $hostname >etc/nodename
echo $hostname >etc/hostname.$if
set -- `/srvd/etc/athena/netparams -f /srvd/etc/athena/masks $netaddr`
echo "$2  $1" > /root/etc/inet/netmasks
echo "$4" > etc/defaultrouter
echo "$netaddr	$hostname $shortname" >> etc/inet/hosts

rm -f /root/.rvdinfo
/srvd/etc/athena/gettime -s time.mit.edu
echo installed on `date` from `df -k / | tail -1 | awk '{print $1}'` \
	> /root/etc/athena/version
if [ $CUSTOM = Y ]; then
	if [ $PARTITION = Y ]; then
		echo custom install with custom partitioning \
			>> /root/etc/athena/version
	else
		echo custom install >> /root/etc/athena/version
	fi
fi
sed -e 's/RVD/Workstation/' -e 's/Version/Version Install/' \
	< /srvd/.rvdinfo >> /root/etc/athena/version

echo "Updating vfstab"
rm -f etc/vfstab
sed "s/@DISK@/$drive/g;/d0s5/d;/d0s6/d" /srvd/etc/vfstab.std > etc/vfstab
chmod 644 etc/vfstab
cp /dev/null etc/named.local

# Set up the /usr/vice/etc directory in the target root.
mkdir -p /root/usr/vice/etc

# Copy ThisCell and cacheinfo from the miniroot.
for i in ThisCell cacheinfo ; do
    cp /usr/vice/etc/$i /root/usr/vice/etc/$i
    chown root:root /root/usr/vice/etc/$i
    chmod a+r /root/usr/vice/etc/$i
done

# Get current copies of CellAlias, CellServDB, and SuidCells from AFS.
for i in CellAlias CellServDB SuidCells ; do
    cp -p /afs/athena/service/$i /root/usr/vice/etc/
    chown root:root /root/usr/vice/etc/$i
    chmod a+r /root/usr/vice/etc/$i
done
echo "Copied afs service files into /usr/vice/etc"

echo "Setting up initial system packs symlinks"
(cd /tmp && tar cf - srvd os) | (cd /root && tar xf -)

echo "turn fs slow"
/util/fastfs /root slow

echo "Initializing var/adm and spool files "
cp /dev/null /root/var/adm/lastlog
cp /dev/null /root/var/adm/utmpx
cp /dev/null /root/var/adm/wtmpx
cp /dev/null /root/var/adm/sulog
mkdir /root/var/spool/mqueue
chmod 750 /root/var/spool/mqueue
cp /dev/null /root/var/spool/mqueue/syslog
chmod 600 /root/var/adm/sulog /root/var/spool/mqueue/syslog
rm -f /root/var/spool/cron/crontabs/uucp
echo "Installing bootblocks on root "
installboot "/os/usr/platform/$platform/lib/fs/ufs/bootblk" "$rrootdrive"

echo "setting the boot device"
luxadm set_boot_dev -y $rootdrive

mkdir -p /root/var/athena
cp /tmp/install.log /root/var/athena/install.log

# Pass along needed variables to the next stage of the install.
echo "REBOOT=$REBOOT" >> /root/var/athena/install.vars
echo "PUBLIC=$PUBLIC" >> /root/var/athena/install.vars
echo "AUTOUPDATE=$AUTOUPDATE" >> /root/var/athena/install.vars
echo "RVDCLIENT=$RVDCLIENT" >> /root/var/athena/install.vars

echo "Unmounting filesystems and checking them"
cd /

umount /usr/vice/cache > /dev/null 2>&1
fsck -y -F ufs $rcachedrive
/sbin/umount /root > /dev/null 2>&1
/usr/sbin/fsck -y -F ufs $rrootdrive
sleep 5

echo "Rebooting into the target root to complete the installation."
reboot

