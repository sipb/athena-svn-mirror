#!/bin/sh -euk

### This is the second script file in the Athena workstation
### installation program.  It is called by the first script,
### athenainstall.

### $Header: /afs/dev.mit.edu/source/repository/packs/install/platform/sun4/phase2.sh,v 1.4 1994-01-07 14:38:22 miki Exp $
### $Locker:  $


echo "Set some variables"
PATH=/srvd/bin:/srvd/bin/athena:/srvd/etc:/srvd/usr/sbin:/bin:/etc:/sbin:/usr/sbin
export PATH
umask 2


# Define the partitions
echo "Define the partitions"
rootdrive=/dev/dsk/c0t3d0s0
rrootdrive=/dev/rdsk/c0t3d0s0

cachedrive=/dev/dsk/c0t3d0s3
rcachedrive=/dev/rdsk/c0t3d0s3

usrdrive=/dev/dsk/c0t3d0s5
rusrdrive=/dev/rdsk/c0t3d0s5

vardrive=/dev/dsk/c0t3d0s6
rvardrive=/dev/rdsk/c0t3d0s6



echo "formatting  "
if [ ${MACH}X = 4cX ]
then
        echo "formatting for 4c "
        cat /util/format.input | format >/dev/null 2>&1
else
        cat /util/format.inq | format | grep SUN0535
        if [ $? -ne 0 ] ; then
                echo "formatting SUN0424"
                cat /util/format.input.SUN0424 | /usr/sbin/format >/dev/null 2>&1
        else
                echo "formatting SUN0535"
                cat /util/format.input.SUN0535 | /usr/sbin/format >/dev/null 2>&1
        fi
fi

echo "Making the filesystems..."
echo ""
echo "Making the root file system"
echo "y" | /usr/sbin/newfs -v  $rrootdrive

echo "Making the usr file system"
echo "y" | /usr/sbin/newfs -v  $rusrdrive

echo " Making the cache file system "
echo "y" | /usr/sbin/newfs -v $rcachedrive

echo "Making the var filesystem"
echo "y" | /usr/sbin/newfs -v $rvardrive


echo "Adding AFS filesystem"
echo "Making an AFS cache available"
mkdir /var/usr
mkdir /var/usr/vice; 
mount $cachedrive  /var/usr/vice
cd /var/usr/vice
mkdir etc; mkdir cache;
chmod 0700 cache
for i in cacheinfo CellServDB SuidCells ThisCell
        do cp -p /afsin/$i etc/ ; done
cd etc
mv CellServDB CellServDB.public
ln -s CellServDB.public CellServDB
cp -p SuidCells SuidCells.public

echo "Making an /afs repository"
mkdir /tmp/afs
echo "Loading afs in the kernel"
modload /kernel/fs/afs
echo "Starting afsd "
/etc/afsd -nosettime -daemons 4

echo "Mounting hard disk's root partition..."
/etc/mount  $rootdrive /root

cd /
echo "Making dirs on root"
mkdir /root/var
mkdir /root/usr
mkdir /root/tmp
mkdir /root/proc
ln -s var/opt /root/opt

echo "Mount var, usr , var/usr/vice..."
/sbin/mount  $vardrive /root/var
/sbin/mount  $usrdrive /root/usr
chmod 1777 /root/tmp
mkdir /root/var/usr
mkdir /root/var/usr/vice
#/sbin/mount  $cachedrive /root/var/usr/vice

echo "Copying file system from installation srvd to new filesys..."
echo "Running 'track'..."
/srvd/usr/athena/etc/track -d -F /srvd -T /root -W /srvd/usr/athena/lib


echo "Create devices and dev"
mkdir /root/dev
mkdir /root/devices
find /devices -depth -print | cpio -pdm /root 2> /dev/null
find /dev -depth -print | cpio -pdm /root 2> /dev/null
chmod 755 /root/dev
chmod 755 /root/devices
cat </etc/path_to_inst >/root/etc/path_to_inst
cp /dev/null /root/reconfigure
if [ ${MACH}X = 4cX ]
then
    echo "copying 4c kernel"
    mkdir /root/kernel;
    (cd /srvd/kernel.4c; tar cf - . ) | (cd /root/kernel; tar xf - . )
    echo "copying kvm..."
    mkdir /root/usr/kvm
    (cd /srvd/usr/kvm.4c; tar cf - . ) | (cd /root/usr/kvm; tar xf - . )
    cp -r /srvd/usr/kernel /root/usr/kernel
else
    echo "copying 4m kernel"
    mkdir /root/kernel;
    (cd /srvd/kernel; tar cf - . ) | (cd /root/kernel; tar xf - . )
    echo "getting usr/kvm.4m"
    mkdir /root/usr/kvm
    (cd /srvd/usr/kvm; tar cf - . ) | (cd /root/usr/kvm; tar xf - . )
    cp -r /srvd/usr/kernel /root/usr/kernel
fi

cd /root
echo "Creating other files/directories on the pack's root..."
mkdir afs mit mnt 
ln -s /var/usr/vice usr/vice
ln -s /var/adm usr/adm
ln -s /var/spool usr/spool
ln -s /var/preserver usr/preserve
ln -s /var/tmp usr/tmp
cp -p /srvd/.c* /srvd/.l* /srvd/.p* /srvd/.r* /srvd/.t* /srvd/.x* /root/
chmod 1777 /root/tmp /root/usr/tmp


echo "Finishing etc"
cp /dev/null etc/mnttab
cp /dev/null etc/dumpdates
cp /dev/null ups_data
cp -p /srvd/etc/ftpusers etc/
cp -p /srvd/etc/inetd.conf etc/
#cp -p /srvd/etc/motd etc/
cp -p /srvd/etc/services etc/

hostname=`echo $hostname | awk -F. '{print $1}' | /usr/bin/tr "[A-Z]" "[a-z]"`
echo "Host name is $hostname"
echo "Gateway is $gateway"
echo "Address is $netaddr"
echo $hostname >etc/nodename
echo $hostname >etc/hostname.le0
echo $gateway >etc/defaultrouter
cp -p /srvd/etc/inet/hosts etc/inet/hosts
echo "$netaddr	$hostname" >>etc/inet/hosts
cd /root/etc
#ln -s inet/hosts hosts
cd /root
cp -p /srvd/etc/passwd.std etc/passwd
cp -p /srvd/etc/shadow.std etc/shadow
chmod 600 etc/shadow
cp -p /srvd/etc/group.std etc/group
ln -s ../var/adm/utmp etc/utmp
ln -s ../var/adm/utmpx etc/utmpx
ln -s ../var/adm/wtmp etc/wtmp
ln -s ../var/adm/wtmpx etc/wtmpx
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
ln -s /srvd/var/sadm sadm
mkdir tmp 2>/dev/null
chmod 1777 tmp
#cd /root/var/usr/vice
#mkdir cache
#mkdir etc

echo "Initializing var/adm and spool files "
cp /dev/null /root/var/adm/lastlog
cp /dev/null /root/var/adm/utmp
cp /dev/null /root/var/adm/utmpx
cp /dev/null /root/var/adm/wtmp
cp /dev/null /root/var/adm/wtmpx
cp /dev/null /root/var/spool/mqueue/syslog
cp -p /srvd/etc/crontab.root.add  /root/var/spool/cron/crontabs/root
echo "Installing bootblocks on root "
cp -p /ufsboot /root
/usr/sbin/installboot /srvd/lib/fs/ufs/bootblk $rrootdrive
cd /root
#ln -s usr/tmp/core.root core
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


