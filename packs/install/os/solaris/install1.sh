#!/bin/sh -euk

### This is the second script file in the Athena workstation
### installation program.  It is called by the first script,
### athenainstall.

### $Id: install1.sh,v 1.13 2001-06-01 17:13:40 miki Exp $

echo "Set some variables"
PATH=/sbin:/usr/bin:/usr/sbin:/os/usr/bin
export PATH
umask 2

bootdevice="disk"; export bootdevice
# Use format to get information about the available drives.
format < /dev/null | awk '/^[ 	]*[0-9]\./ { print; }' > /tmp/disks

ndrives=`wc -l /tmp/disks`
case `uname -m` in
sun4u)
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
echo "Custom installation ? (will default to n after 60 seconds)[n]"

case x`/util/to 60` in
xtimeout|xn|xN|x)
     CUSTOM=N; echo "Doing standard installation"; export CUSTOM;;
*)
     CUSTOM=Y; echo "Doing custom installation"; export CUSTOM;;
esac

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

usrdrive=/dev/dsk/${drive}s5
rusrdrive=/dev/rdsk/${drive}s5

vardrive=/dev/dsk/${drive}s6
rvardrive=/dev/rdsk/${drive}s6

export drive rootdrive rrootdrive cachedrive rcachedrive usrdrive rusrdrive
export vardrive rvardrive

echo "Installing on ${drive}."

case $CUSTOM in
N)
  echo "standard installation - 9.0"
  REV=9.0
  ln -s /afs/dev.mit.edu/system/sun4x_58/srvd-9.0 /tmp/srvd
  ln -s /afs/dev.mit.edu/system/sun4x_58/os /tmp/os
  ln -s /afs/dev.mit.edu/system/sun4x_58/install/cdrom /tmp/cdrom
  ln -s /afs/dev.mit.edu/system/sun4x_58/install/patches /tmp/patches
  ;;

Y)
   echo "custom installation"
   echo "Which Athena release do you want to install ? "
   read buff
   case "$buff" in
   7.7)
       echo "installing 7.7"
       REV=7.7
       ln -s /afs/athena.mit.edu/system/sun4m_53/srvd.77 /tmp/srvd
       ;;
   8.0)
       echo "installing 8.0"
       REV=8.0
       ln -s /afs/athena.mit.edu/system/sun4m_54/srvd.80 /tmp/srvd
       ln -s /afs/athena.mit.edu/system/sun4m_54/os /tmp/os 
       ;;
   8.1)
       echo "installing 8.1"
       REV=8.1
       ln -s /afs/athena.mit.edu/system/sun4x_55/srvd-8.1 /tmp/srvd
       ln -s /afs/athena.mit.edu/system/sun4x_55/os /tmp/os 
       ;;
   8.2)
       echo "installing 8.2"
       REV=8.2
       ln -s /afs/athena.mit.edu/system/sun4x_56/srvd-8.2 /tmp/srvd
       ln -s /afs/athena.mit.edu/system/sun4x_56/os /tmp/os
       ;;
   8.3)
       echo "installing 8.3"
       REV=8.3
       ln -s /afs/athena.mit.edu/system/sun4x_56/srvd-8.3 /tmp/srvd
       ln -s /afs/athena.mit.edu/system/sun4x_56/os-8.3 /tmp/os
       ;;
   8.4)
       echo "installing 8.4"
       REV=8.4
       ln -s /afs/athena.mit.edu/system/sun4x_57/srvd /tmp/srvd
       ln -s /afs/athena.mit.edu/system/sun4x_57/os /tmp/os
       ln -s /afs/athena.mit.edu/system/sun4x_57/install/cdrom /tmp/cdrom
       ln -s /afs/athena.mit.edu/system/sun4x_57/install/patches /tmp/patches
       ;;
    9.0)
       echo "standard installation - 9.0"
       REV=9.0
       ln -s /afs/dev.mit.edu/system/sun4x_58/srvd-9.0 /tmp/srvd
       ln -s /afs/dev.mit.edu/system/sun4x_58/os /tmp/os
       ln -s /afs/dev.mit.edu/system/sun4x_58/install/cdrom /tmp/cdrom
       ln -s /afs/dev.mit.edu/system/sun4x_58/install/patches /tmp/patches
       ;;
    *)
       echo "installing 8.4"
       REV=8.4
       ln -s /afs/athena.mit.edu/system/sun4x_57/srvd /tmp/srvd
       ln -s /afs/athena.mit.edu/system/sun4x_57/os /tmp/os
       ln -s /afs/athena.mit.edu/system/sun4x_57/install/cdrom /tmp/cdrom
       ln -s /afs/athena.mit.edu/system/sun4x_57/install/patches /tmp/patches
       ;;
   esac
   echo "done choosing rev"
   ;;
esac

case $CUSTOM in
Y)
   echo "Automatic reboot after installation? [n]"
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

ls -l /srvd
ls -l /tmp/srvd
ls -l /os
ls -l /tmp/os

echo "formatting  "
DISK=`format < /dev/null | \
    awk "/$drive/"'{ print substr($3, 2, length($3) - 1); }'`
export DISK
echo $DISK

# first make sure that the disk has an external label (set it just in case)
cat /util/format.input.label | format ${drive} >/dev/null 2>&1

case $CUSTOM in
Y)
   echo "Partition the disk yourself? [n]"
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
     format 
     partitioning=one
     export partitioning
     echo "boot device you want to boot from ?"
     read bootdevice
     export bootdevice
     echo "Done asking questions for custom install."
     ;;
*)
     case $DISK in
     SUN0424)
        echo "formatting SUN0424"
        partitioning=many
        cat /util/format.input.SUN0424 | \
		format ${drive} >/dev/null 2>&1
        ;;
     SUN0535)
        echo "formatting SUN0535"
        partitioning=many
        cat /util/format.input.SUN0535 | \
		format ${drive} >/dev/null 2>&1
        ;;
    SUN1.05)
       echo "formatting SUN1.05"
       partitioning=many
       cat /util/format.input.SUN1.05 | \
		format ${drive} >/dev/null 2>&1
       ;;
    SUN2.1G)
       echo "formatting SUN2.1G"
       partitioning=many
       cat /util/format.input.SUN2.1G | \
		format ${drive} >/dev/null 2>&1
       ;;
    SUN4.2G)
       echo "formatting SUN4.2G"
       partitioning=one
       cat /util/format.input.SUN4.2G | \
		format ${drive} >/dev/null 2>&1
       ;;
    SUN9.0G)
       echo "formatting SUN9.0G"
       partitioning=one
       cat /util/format.input.SUN9.0G | \
                format ${drive} >/dev/null 2>&1
       ;;
    SUN18G)
       echo "formatting SUN18G"
       partitioning=one
       cat /util/format.input.SUN18G | \
                format ${drive} >/dev/null 2>&1
       ;;
    ST315320A)
       echo "formatting ST315320A"
       partitioning=one
       cat /util/format.input.ST315320A | \
		format ${drive} >/dev/null 2>&1
	;;
    ST320420A)
       echo "formatting ST320420A"
       partitioning=one
       cat /util/format.input.ST320420A | \
		format ${drive} >/dev/null 2>&1
       ;;
    ST34321A)
       echo "formatting ST34321A"
       partitioning=one
       cat /util/format.input.ST34321A | \
		format ${drive} >/dev/null 2>&1
       ;;
    ST34342A)
       echo "formatting ST34342A"
       partitioning=one
       cat /util/format.input.ST34342A | \
		format ${drive} >/dev/null 2>&1
       ;;
    ST38420A)
       echo "formatting ST38420A"
       partitioning=one
       cat /util/format.input.ST38420A | \
		format ${drive} >/dev/null 2>&1
       ;;
    ST39120A|ST39111A)
       echo "formating ST39120A"
       partitioning=one
       cat /util/format.input.ST39120A | \
		format ${drive} >/dev/null 2>&1
       ;;
    ST39140A)
       echo "formatting ST39140A"
       partitioning=one
       cat /util/format.input.ST39140A | \
		format ${drive} >/dev/null 2>&1
       ;;
    Seagate*)
       echo "formatting Segate Medalist"
       partitioning=one
       cat /util/format.input.Seagate.medalist | \
		format ${drive} >/dev/null 2>&1
       ;;
    SEAGATE*5660N)
       echo "formatting SEAGATE-ST5660N"
       partitioning=many
       cat /util/format.input.seagate.5660 | \
		format ${drive} >/dev/null 2>&1
       ;;
    *)
       echo "can't format the disks - type unknown"
       echo "Call an expert !"
       echo "Press ^C for a shell or Stop-A for the boot prompt."
       trap /bin/sh 2
       while :; do sleep 10; done
       esac
esac
export partitioning

echo "Making the filesystems..."
echo ""
echo "Making the root file system"
echo "y" | /usr/sbin/newfs -v  $rrootdrive

echo " Making the cache file system "
echo "y" | /usr/sbin/newfs -v $rcachedrive
case $partitioning in
many)
    echo "Making the var filesystem"
    echo "y" | /usr/sbin/newfs -v $rvardrive
    echo "Making the usr file system"
    echo "y" | /usr/sbin/newfs -v  $rusrdrive
    ;;
*)
    ;;
esac



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
cp -p CellServDB CellServDB.public
cp -p SuidCells SuidCells.public
chown root:root CellServDB CellServDB.public SuidCells SuidCells.public ThisCell
chown root:root cacheinfo
chmod a+r CellServDB CellServDB.public SuidCells SuidCells.public ThisCell
chmod a+r cacheinfo
# !!
echo "Making an /afs repository"
mkdir /tmp/afs
echo "Loading afs in the kernel"
modload /kernel/misc/nfssrv
modload /kernel/fs/afs
echo "Starting afsd "
/etc/afsd -nosettime -daemons 4
type=install; export type
date >/tmp/install.log

case $REV in
 8.4)
    sh /srvd/install/install2.sh | tee -a /tmp/install.log
    ;;
 *)
    sh /srvd/install/phase3.sh 
    ;;
esac
echo "Some unexpected error occured"
echo "Please contact Athena Hotline at x3-1410."
echo "Thank you. -Athena Operations"
/sbin/sh
halt
