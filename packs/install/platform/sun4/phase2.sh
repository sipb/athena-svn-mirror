#!/bin/sh -euk

### This is the second script file in the Athena workstation
### installation program.  It is called by the first script,
### athenainstall.

### $Header: /afs/dev.mit.edu/source/repository/packs/install/platform/sun4/phase2.sh,v 1.21 1997-04-15 22:41:50 ghudson Exp $
### $Locker:  $

echo "Set some variables"
PATH=/os/usr/bin:/srvd/bin/athena:/os/etc:/os/usr/sbin:/bin:/etc:/sbin:/usr/sbin
export PATH
umask 2

echo "Custom installation ? (will default to n after 60 seconds)[n]"

case x`/util/to 60` in
xtimeout|xn|xN|x)
     CUSTOM=N; echo "Doing standard installation"; export CUSTOM;;
*)
     CUSTOM=Y; echo "Doing custom installation"; export CUSTOM;;
esac

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

export rootdrive rrootdrive cachedrive rcachedrive usrdrive rusrdrive
export vardrive rvardrive


case $CUSTOM in
N)
  echo "standard installation"
  ln -s /afs/athena/system/sun4m_54/srvd /tmp/srvd
  ;;

Y)
   echo "custom installation"
   echo "Which rev do you want to install ? "
   read buff
   case "$buff" in
   7.7)
       echo "installing 7.7"
       ln -s /afs/athena.mit.edu/system/sun4m_53/srvd.77 /tmp/srvd
       ;;
   *)
       echo "installing 8.0"
       ln -s /afs/athena.mit.edu/system/sun4m_54/srvd /tmp/srvd
       ;;
   esac
   echo "done choosing rev"
   ;;
esac

ls -l /srvd
ls -l /tmp/srvd

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
SUN2.1G)
         echo "formatting SUN2.1G"
         cat /util/format.input.SUN2.1G | /usr/sbin/format >/dev/null 2>&1
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
