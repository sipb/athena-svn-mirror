#!/bin/sh
# $Id: makeroot.sh,v 1.16 2003-03-31 17:28:01 ghudson Exp $

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
  pwconv=/usr/sbin/pwconv

  mkdir -p "$root/dev"
  /dev/MAKEDEV -d "$root/dev" std

  cd "$sysprefix"
  list=`awk '{ x = $2; } END { print x; }' "$syscontrol"`
  rpms=`awk '/^athena/ { next; } { print $1; }' $list`

  mkdir -p "$root/var/lib/rpm" "$root/etc"
  touch "$root/etc/fstab"
  rpm --root "$root" --initdb
  rpm --root "$root" -ivh $rpms

  # Make links into destination area.
  ln -s ../build/athtools/usr/athena "$root/usr/athena"
  ln -s ../build/athtools/usr/afsws "$root/usr/afsws"

  # So packages can figure out where sendmail is.  (sendmail normally
  # comes from athena-sendmail, which we don't install; this is simpler
  # than installing either the athena-sendmail or a native sendmail RPM.)
  touch "$root/usr/sbin/sendmail"
  chmod a+x "$root/usr/sbin/sendmail"
  ;;

sun4)
  pwconv=/usr/sbin/pwconv

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
  ln -s ../.srvd/usr/afsws "$root/usr/afsws"
  ;;

esac

# It's really convenient to have a nice shell in the build root area,
# at least on Solaris.
mkdir -p "$root/bin/athena"
cp /bin/athena/tcsh "$root/bin/athena/tcsh"

# The discuss build needs the discuss user to be in the passwd file.
grep '^discuss' /etc/passwd >> $root/etc/passwd

# Create the shadow password file, so programs such as passwd get
# configured properly.
chroot $root $pwconv

# Prepare the source locker symlinks.
mkdir -p "$root/mit"
ln -s /afs/dev.mit.edu/source/src-current $root/mit/source
if [ -n "$ver" ]; then
  ln -s /afs/dev.mit.edu/source/src-$ver $root/mit/source-$ver
fi
