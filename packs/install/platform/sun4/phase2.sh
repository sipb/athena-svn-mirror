#!/bin/sh -euk

### This is the second script file in the Athena workstation
### installation program.  It is called by the first script,
### athenainstall.

### $Header: /afs/dev.mit.edu/source/repository/packs/install/platform/sun4/phase2.sh,v 1.17 1996-05-10 19:25:09 ghudson Exp $
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
DISK=`/sbin/machtype -r`
export DISK
echo $DISK
case $DISK in
SUN0207)
        echo "formatting SUN0270"
        cat /util/format.input.SUN0270 | format >/dev/null 2>&1
	;;
SUN0424)
         echo "formatting SUN0424"
         cat /util/format.input.SUN0424 | /usr/sbin/format >/dev/null 2>&1
	;;
SUN0535)
         echo "formatting SUN0535"
         cat /util/format.input.SUN0535 | /usr/sbin/format >/dev/null 2>&1
	 ;;
SUN1.05)
         echo "formatting SUN1.05"
         cat /util/format.input.SUN1.05 | /usr/sbin/format >/dev/null 2>&1
	 ;;
SEAGATE*5660N)
         echo "formatting SEAGATE-ST5660N"
         cat /util/format.input.seagate.5660|/usr/sbin/format >/dev/null 2>&1
        ;;
*)
         echo "can't format the disks - type unknown"
         echo "Call an expert !"
         exit 1
         esac


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

sh /srvd/install/phase3.sh
