#!/bin/sh -euk
# This file is run out of the srvd by update1.sh after it starts AFS.

umask 022
echo "second part of the miniroot-update"
echo "get the rootdrive, userdrive...."
rrootdrive=` cat /etc/vfstab | awk '$3 == "/root" {print $2}'`
rootdrive=` cat /etc/vfstab | awk '$3 == "/root" {print $1}'`
rvardrive=` cat /etc/vfstab | awk '$3 == "/root/var" {print $2}'`
vardrive=` cat /etc/vfstab | awk '$3 == "/root/var" {print $1}'`
ruserdrive=` cat /etc/vfstab | awk '$3 == "/root/usr" {print $2}'`
userdrive=` cat /etc/vfstab | awk '$3 == "/root/usr" {print $1}'`
rcachedrive=` cat /etc/vfstab | awk '$3 == "/root/var/usr/vice" {print $2}'`
cachedrive=` cat /etc/vfstab | awk '$3 == "/root/var/usr/vice" {print $2}'`


# Sun documents "uname -i" as the way to find stuff under /platform or
# /usr/platform.  We rely in the fact that the uname -i names are symlinks
# to the uname -m names, in order to save space on the root.  Be on the
# alert for changes.
platform=`uname -m`

ROOT=/root; export ROOT
echo "Mounting hard disk's root partition..."

cd /root
echo "remove old links to os and old pkg/patch DB"
sh /srvd/install/os-link-2.6 2>/dev/null
rm -rf /root/var/sadm/pkg/*
rm -rf /root/var/sadm/patch*/*



echo "installing the os packages"
case `uname -m` in
sun4u)
    for i in `cat /cdrom/install-local-u`
      do echo $i; cat /util/yes-file | pkgadd -R /root -d /cdrom $i ; done 2>/dev/null
    for i in `cat /cdrom/install-nolocal-u`      
      do echo $i; cat /util/yes-file | pkgadd -R /root -d /cdrom/cdrom.link $i; done   2>/dev/null;
	;;
sun4m)
    for i in `cat /cdrom/install-local-m`
      do echo $i; cat /util/yes-file | pkgadd -R /root -d /cdrom $i; done 2>/dev/null
    for i in `cat /cdrom/install-nolocal-m`      
      do echo $i; cat /util/yes-file | pkgadd -R /root -d /cdrom/cdrom.link $i; done 2>/dev/null  
 	;;
*)
    echo "unsupported architecture - contact Athena administration"
      exit 1
	;;
esac


echo "correction to pkg installation"
cp /cdrom/I* /root/var/sadm/system/admin/

echo "make it appear as  configured"
rm /root/etc/.UNC*
rm /root/etc/.sysidconfig.apps
cp /cdrom/.sysIDtool.state /root/etc/default/

echo "Installing Requested and Security patches for OS "
cat /util/yes-file | /util/patchadd -d -R /root -u -M \
      /patches/patches.link `cat /patches/current-patches`

echo "add/remove osfiles as needed\n"
sh /srvd/install/oschanges 2>/dev/null

echo "the os part is installed"

echo "tracking the srvd"
/srvd/usr/athena/etc/track -d -F /srvd -T /root -W /srvd/usr/athena/lib
echo "copying kernel modules from /srvd/kernel"
cp -p /srvd/kernel/fs/* /root/kernel/fs/


echo "Create devices and dev"
cd /root
/usr/sbin/drvconfig -R /root -r devices -p /root/etc/path_to_inst
chmod 755 /root/dev
chmod 755 /root/devices
cp /dev/null /root/reconfigure


cd /root
echo "Finishing etc"


if=`ifconfig -au | awk -F: '/^[a-z]/ { if ($1 != "lo0") { print $1; exit; } }'`
if [ -z "$if" ]; then if=le0; fi
cp /etc/hostname.$if /root/etc/
cp /etc/hosts /root/etc/
cp /etc/nodename /root/etc/
cp /etc/netmasks /root/etc/
cp /etc/defaultrouter /root/etc/
rm -f /root/.rvdinfo
method=`awk '{a=$6} END {print a}' /root/etc/athena/version`
newvers=`awk '{a=$7} END {print a}' /root/etc/athena/version`
echo "Athena Workstation (sun4) Version Reboot $method $newvers `date`" \
	>> /root/etc/athena/version
rm -f /root/var/spool/cron/crontabs/uucp
cp /dev/null etc/named.local



echo "Installing bootblocks on root "
installboot "/os/usr/platform/$platform/lib/fs/ufs/bootblk" "$rrootdrive"

echo "restore the boot-device in the prom"
boot=`cat /etc/boot-device`
/os/usr/platform/$platform/sbin/eeprom "$boot"

cd /root

cp /tmp/update.log /root/var/athena/update.miniroot.log


echo "Unmounting filesystems and checking them"
cd /
umount /root/var/usr/vice > /dev/null 2>&1
fsck -y -F ufs $rcachedrive
umount /root/var > /dev/null 2>&1
fsck -y -F ufs $rvardrive
umount /root/usr
fsck -y -F ufs $rusrdrive
/sbin/umount /root > /dev/null 2>&1
/usr/sbin/fsck -y -F ufs $rrootdrive
sleep 5

echo "Rebooting now"
reboot
