# $Id: phase3.sh,v 1.2 1996-05-29 05:21:05 ghudson Exp $
# $Source: /afs/dev.mit.edu/source/repository/packs/install/platform/sun4/phase3.sh,v $

# This file is run out of the srvd by phase2.sh after it starts AFS.
# The contents of this file used to live in phase2.sh, which is run
# from the miniroot.

ROOT=/root; export ROOT
echo "Mounting hard disk's root partition..."
/etc/mount  $rootdrive $ROOT

cd /
echo "Making dirs on root"
mkdir /root/var
mkdir /root/usr
mkdir /root/tmp
mkdir /root/proc
#ln -s var/opt /root/opt

echo "Mount var, usr , var/usr/vice..."
/sbin/mount  $vardrive /root/var
/sbin/mount  $usrdrive /root/usr
chmod 1777 /root/tmp
mkdir /root/var/usr
mkdir /root/var/usr/vice


echo "Copying file system from installation srvd to new filesys..."
echo "Running 'track'..."
/srvd/usr/athena/etc/track -d -F /os -T /root  -W /srvd/usr/athena/lib -s stats/os_rvd slists/os_rvd
echo "tracking the kernel"
mkdir /root/kernel
/srvd/usr/athena/etc/track -d -F /os/kernel -T /root/kernel  -W /srvd/usr/athena/lib -s stats/kernel_rvd slists/kernel_rvd
echo "tracking the usr kernel"
mkdir /root/usr/kernel
/srvd/usr/athena/etc/track -d -F /os/usr/kernel -T /root/usr/kernel  -W /srvd/usr/athena/lib -s stats/usr_kernel_rvd slists/usr_kernel_rvd
echo "tracking the srvd"
/srvd/usr/athena/etc/track -d -F /srvd -T /root -W /srvd/usr/athena/lib
echo "copying afs from /srvd/kernel/fs/afs"
cp -p /srvd/kernel/fs/* /root/kernel/fs/

echo "Create devices and dev"
mkdir /root/dev
mkdir /root/devices
find /devices -depth -print | cpio -pdm /root 2> /dev/null
find /dev -depth -print | cpio -pdm /root 2> /dev/null
chmod 755 /root/dev
chmod 755 /root/devices
cat </etc/path_to_inst >/root/etc/path_to_inst
cp /dev/null /root/reconfigure


cd /root
echo "Creating other files/directories on the pack's root..."
mkdir afs mit mnt 
ln -s /var/usr/vice usr/vice
#ln -s /var/adm usr/adm
#ln -s /var/spool usr/spool
#ln -s /var/preserve usr/preserve
cp -p /srvd/.c* /srvd/.l* /srvd/.p* /srvd/.r* /srvd/.x* /root/
chmod 1777 /root/tmp 


echo "Finishing etc"
cp /dev/null etc/mnttab
cp /dev/null etc/dumpdates
#cp /dev/null ups_data
cp -p /srvd/etc/ftpusers etc/ftpusers
cp -p /srvd/etc/inet/inetd.conf etc/inet/inetd.conf
cp -p /srvd/etc/inet/services etc/inet/services
cp -p /srvd/etc/athena/inetd.conf etc/athena/inetd.conf
cp -p /srvd/etc/minor_perm etc/minor_perm
cp -p /srvd/etc/system etc/system
cp -p /srvd/etc/shells etc/shells
cp -p /srvd/etc/syslog.conf etc/syslog.conf
cp -p /srvd/etc/athena/attach.conf etc/athena/attach.conf
cp -p /srvd/etc/athena/newsyslog.conf etc/athena/newsyslog.conf
cp -p /srvd/etc/mail/sendmail.cf etc/mail/sendmail.cf
cp -p /os/etc/name_to_major etc/name_to_major
cp -p /os/etc/driver_aliases etc/driver_aliases
cp -p /os/etc/device.tab etc/device.tab
cp -p /os/etc/dgroup.tab etc/dgroup.tab
chmod 644 etc/system

hostname=`echo $hostname | awk -F. '{print $1}' | /usr/bin/tr "[A-Z]" "[a-z]"`
echo "Host name is $hostname"
echo "Gateway is $gateway"
echo "Address is $netaddr"
echo $hostname >etc/nodename
echo $hostname >etc/hostname.le0
echo $gateway >etc/defaultrouter
cp -p /os/etc/inet/hosts etc/inet/hosts
cp -p /srvd/etc/inet/netmasks etc/inet/netmasks
cp -p /os/etc/dfs/dfstab etc/dfs/dfstab
#cp -p /srvd/etc/inet/hosts etc/inet/hosts
echo "$netaddr	${hostname}.MIT.EDU $hostname" >>etc/inet/hosts
cd /root/etc
cd /root
if [ -r /srvd/etc/passwd ]; then
    cp -p /srvd/etc/passwd etc/passwd
    chmod 644 etc/passwd
    chown root etc/passwd
else
    cp -p /srvd/etc/passwd.fallback etc/passwd
fi
if [ -r /srvd/etc/shadow ]; then
    cp -p /srvd/etc/shadow etc/shadow
    chown root etc/shadow
else
    cp -p /srvd/etc/shadow.fallback etc/shadow
fi
chmod 600 etc/shadow
cp -p /srvd/etc/group etc/group
#ln -s ../var/adm/utmp etc/utmp
#ln -s ../var/adm/utmpx etc/utmpx
#ln -s ../var/adm/wtmp etc/wtmp
#ln -s ../var/adm/wtmpx etc/wtmpx
cp -p /srvd/etc/athena/*.conf etc/athena/
echo "Updating dm config"
cp -p /srvd/etc/athena/login/config etc/athena/login/config
echo "Editing rc.conf and version"
sed -e 	"s/^HOST=MITHOST.MIT.EDU/HOST=$hostname/
	s/^ADDR=MITADDR/ADDR=$netaddr/" \
	< /srvd/etc/athena/rc.conf > /root/etc/athena/rc.conf
rm -f /root/.rvdinfo
echo installed on `date` > /root/etc/athena/version
sed  -e "s/RVD/Workstation/g" < /srvd/.rvdinfo >> /root/etc/athena/version

echo "Updating vfstab"
cp -p /srvd/etc/vfstab.std etc/vfstab
cp /dev/null etc/named.local



echo "Updating var"
cd /root/var
cpio -idm </srvd/install/var.cpio
#ln -s /srvd/var/sadm sadm
mkdir sadm
(cd /os/var/sadm;  tar cf - . ) | ( cd /root/var/sadm; tar xf - . )
cd /root/var
mkdir tmp 2>/dev/null
chmod 1777 tmp

cd /var/usr/vice
for i in  CellServDB SuidCells 
        do cp -p /afs/athena/service/$i etc/ ; done
echo "Copied afs service files into var/usr/vice/etc"

echo "Initializing var/adm and spool files "
cp /dev/null /root/var/adm/lastlog
cp /dev/null /root/var/adm/utmp
cp /dev/null /root/var/adm/utmpx
cp /dev/null /root/var/adm/wtmp
cp /dev/null /root/var/adm/wtmpx
cp /dev/null /root/var/spool/mqueue/syslog
cp -p /srvd/etc/crontab.root.add  /root/var/spool/cron/crontabs/root
rm -f /root/var/spool/cron/crontabs/uucp
echo "Installing bootblocks on root "
cp -p /ufsboot /root
#/usr/sbin/installboot /srvd/lib/fs/ufs/bootblk $rrootdrive
/usr/sbin/installboot /os/lib/fs/ufs/bootblk $rrootdrive
cd /root

# Note: device scripts depend on ROOT being set properly.
auxdir=/srvd/install/aux.devs
if [ -d $auxdir ]; then
	echo "Installing extra devices..."
	for i in ${auxdir}/*; do
		if [ -x $i ]; then
			$i
		fi
	done
fi

echo "Unmounting filesystems and checking them"
cd /

umount /var/usr/vice > /dev/null 2>&1
fsck -F ufs $rcachedrive
umount /root/var > /dev/null 2>&1
fsck -F ufs $rvardrive
umount /root/usr
fsck -F ufs $rusrdrive
/sbin/umount /root > /dev/null 2>&1
/usr/sbin/fsck -F ufs $rrootdrive
sleep 5
echo "rebooting now"
reboot
