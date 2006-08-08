#!/bin/sh
# $Id: finish-install.sh,v 1.4 2006-08-08 21:29:54 rbasch Exp $

echo "Starting the second stage of the install at `date`."

# We get one argument, the workstation version we're installing at.
vers="$1"

. /var/athena/install.vars

echo "Setting root filesystem fast."
/srvd/install/fastfs / fast

# Install the Athena passwd/shadow/group files.  We need to do this before
# the discuss package is installed, so that pkgadd does not complain about
# files owned by the discuss user.
sys=`/bin/athena/machtype -S`
pwconfig=/afs/athena.mit.edu/system/config/passwd/$sys
if [ -r $pwconfig/passwd ]; then
  cp -p $pwconfig/passwd /etc/passwd.local
  chmod 644 /etc/passwd.local
  chown root /etc/passwd.local
else
  cp -p /srvd/etc/passwd.fallback /etc/passwd.local
fi
if [ -r $pwconfig/shadow ]; then
  cp -p $pwconfig/shadow /etc/shadow.local
  chown root /etc/shadow.local
else
  cp -p /srvd/etc/shadow.fallback /etc/shadow.local
fi
cp -p /etc/passwd.local /etc/passwd
cp -p /etc/shadow.local /etc/shadow
chmod 600 /etc/shadow
if [ -r $pwconfig/group ]; then
  cp -p $pwconfig/group /etc/group.local
  cp -p /etc/group.local /etc/group
  chmod 644 /etc/group.local /etc/group
fi

pkgdir=/srvd/pkg/$vers
libdir=/srvd/usr/athena/lib/update
echo "Installing Athena non-core packages."
for pkg in `awk '{ print $1; }' $pkgdir/.order-version` ; do
  # Skip core packages, which have already been installed.
  if pkginfo -q $pkg ; then
    :
  else
    echo "$pkg"
    pkgadd -a $libdir/admin-update -n -d $pkgdir $pkg
  fi
done
echo "Finished installing Athena non-core packages."
echo ""

# Set the PUBLIC, AUTOUPDATE, and RVDCLIENT variables in rc.conf.
# Machine-dependent variables such as HOST, ADDR, etc., should be
# properly set by package postinstall scripts.
echo "Setting rc.conf variables."
sed -e "s|^PUBLIC=[^;]*|PUBLIC=$PUBLIC|" \
    -e "s|^AUTOUPDATE=[^;]*|AUTOUPDATE=$AUTOUPDATE|" \
    -e "s|^RVDCLIENT=[^;]*|RVDCLIENT=$RVDCLIENT|" /etc/athena/rc.conf \
  > /etc/athena/rc.conf.new
mv /etc/athena/rc.conf.new /etc/athena/rc.conf

# Run catman to format Athena man pages, and create the windex
# database.  Suppress all output, since catman will complain about
# pages in lockers which cannot be read without tokens (e.g. psutils).
echo "Formatting Athena man pages."
/usr/bin/catman -M /usr/athena/man > /dev/null 2>&1

echo "Creating windex databases."
/usr/bin/catman -w -M /usr/openwin/share/man:/usr/dt/share/man:/usr/share/man:/usr/sfw/share/man

echo "Updating version"
hosttype=`/bin/athena/machtype`
echo "Athena Workstation ($hosttype) Version $vers `date`" \
  >> /etc/athena/version

echo "Setting root filesystem slow."
/srvd/install/fastfs / slow

echo "Cleaning up initial system packs symlinks"
rm -f /srvd /os

echo "Finished with install at `date`."
