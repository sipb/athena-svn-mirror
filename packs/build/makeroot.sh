#!/bin/sh
# $Id: makeroot.sh,v 1.4 2002-02-28 15:51:08 ghudson Exp $

if [ $# -lt 1 ]; then
  echo "Usage: $0 rootdir [fullversion]" >&2
  exit 1
fi

root=$1
ver=$2

case `machtype` in
linux)
  sysprefix=/afs/.dev.mit.edu/system/rhlinux
  syscontrol=control/control-${ver:-current}

  mkdir -p "$root/dev"
  /dev/MAKEDEV -d "$root/dev" std

  cd "$sysprefix"
  list=`awk '{ x = $2; } END { print x; }' "$syscontrol"`
  rpms=`awk '
    /\/kernel-headers/ { print $1; }
    /^athena/ { next; }
    /\/(ee|gmc|gnorpm|gtop|libgtop|wmconfig|Xconfigurator)-/ { next; }
    /\/(iptables|kernel|pciutils|quota|shapecfg|sndconfig)-/ { next; }
    /\/(tcpdump)-/ { next; }
    { print $1; }' $list`

  mkdir -p "$root/var/lib/rpm"
  rpm --root "$root" --initdb
  rpm --root "$root" -ivh $rpms

  # Make links into destination area.
  ln -s ../usr/src/athena/athtools/usr/athena "$root/usr/athena"
  ln -s ../usr/src/athena/athtools/usr/afsws "$root/usr/afsws"
  ;;

sun4)
  # Install packages.
  for i in `cat /install/cdrom/install-local-u`; do
    echo "$i: \c"
    jot -b y 50 | pkgadd -R "$root" -d /install/cdrom "$i"
  done 2>/dev/null
  for i in `cat /install/cdrom/install-nolocal-u`; do
    echo "$i: \c"
    jot -b y 50 | pkgadd -R "$root" -d /install/cdrom/cdrom.link "$i"
  done 2>/dev/null
  cp /install/cdrom/INST_RELEASE "$root/var/sadm/system/admin"

  # Install patches.
  jot -b y 50 | patchadd -d -R "$root" -u -M /install/patches/patches.link \
    `cat /install/patches/current-patches`

  # /usr/lib/isaexec (a front end which selects between the sparcv7 and
  # sparcv9 versions of a binary) doesn't work in a chrooted environment
  # (getexecname() fails due to lack of procfs).  The only binary
  # linked to isaexec relevant to the build system is "sort".  Work
  # around the problem by copying the sparcv9 "sort" to /usr/bin,
  # since ensuring that procfs is always mounted in the chroot area is a
  # little messy.
  rm -f "$root/usr/bin/sort"
  cp "$root/usr/bin/sparcv9/sort" "$root/usr/bin/sort"

  # Make devices.
  cd "$root"
  drvconfig -r devices -p "$root/etc/path_to_inst"
  devlinks -t "$root/etc/devlink.tab" -r "$root"
  disks -r "$root"

  # third/gnome-utils wants to know where to look for log messages.
  touch "$root/var/adm/messages"

  # Copy the /os symlink.
  (cd / && tar cf - os) | (cd "$root" && tar xf -)

  # Make links into destination area.
  ln -s ../.srvd/usr/athena "$root/usr/athena"
  ln -s ../.srvd/usr/gcc "$root/usr/gcc"
  ln -s ../.srvd/usr/afsws "$root/usr/afsws"
  ;;

sgi)
  seldir=/install/selections
  dists="foundation applications dev dwb patches"
  instopts="-r $root -a -N -Vinstmode:normal -Vstartup_script:ignore"
  instopts="$instopts -Vdelay_idb_read:on -Voverlay_mode:silent"
  instopts="$instopts -Vskip_rqs:true -Vverbosity:0"

  # Copy the /os symlink.
  (cd / && tar cf - os) | (cd "$root" && tar xf -)

  # Install local packages.
  for dist in $dists ; do
    selfile=$seldir/default.$dist.local
    if [ -f $selfile ]; then
      inst $instopts -F $selfile
    fi
  done

  # Hack to skip the repeated rebuilding of the file type database.
  ftmakefile="$root/usr/lib/filetype/Makefile"
  mv "$ftmakefile" "$ftmakefile.hold"

  # Install symlink packages, skipping all man and relnotes packages.
  for dist in $dists ; do
    selfile=$seldir/default.$dist.link
    echo "di *.man.*\ndi *.*.relnotes" | cat $selfile - > /tmp/selections
    inst $instopts -F /tmp/selections -T/os
  done

  # Copy the flexlm license file, so we can run the compilers.
  (cd / && tar cf - var/flexlm/license.dat) | (cd "$root" && tar xf -)

  # Copy the compiler default options file.
  cp /etc/compiler.defaults "$root/etc"

  # The write build needs the tty group, which is not in the stock
  # IRIX group file.
  grep '^tty:' /etc/group >> "$root/etc/group"

  # Make links into destination area.
  ln -s ../.srvd/usr/athena "$root/usr/athena"
  ln -s ../.srvd/usr/gcc "$root/usr/gcc"
  ln -s ../.srvd/usr/afsws "$root/usr/afsws"
  ;;

esac

# The discuss build needs the discuss user to be in the passwd file.
grep '^discuss' /etc/passwd >> $root/etc/passwd

# Prepare the source locker symlink.
mkdir -p "$root/mit"
mitpath=$root/mit/source${ver:+-}$ver
ln -s /afs/dev.mit.edu/source/src-${ver:-current} "$mitpath"
