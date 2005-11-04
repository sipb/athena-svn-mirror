#!/bin/sh
# $Id: local-menus.sh,v 1.5 2005-11-04 17:27:10 ghudson Exp $

# Tar file containing menus (a symlink into the AFS system config area).
menutar=/usr/athena/share/gnome/athena/menus.tar

# Menu directory; used as a fallback if we can't make a local copy of
# the menus.
menudir=/usr/athena/share/gnome/athena/menus

# Local copy of the menu tar file.
localtar=/var/athena/menus.tar

# Record of the release version of the last menu update
versionfile=/var/athena/menus.version

# Local copy of the menu directory (made from the tar file, not from
# $menudir).
localdir=/var/athena/menus.copy

# What panel refers to; will be a symlink to either $localdir or
# $menudir.
menus=/var/athena/menus

fail() {
  rm -f $menus $localtar	# Removing $localtar ensures we will try again.
  ln -s $menudir $menus
  exit 1
}

# Fail quietly if the menu tar file doesn't exist.
if [ ! -s $menutar ]; then
  fail
fi

oldversion=none
newversion=`/bin/athena/machtype -A`
if [ -s $versionfile ]; then
  oldversion=`cat $versionfile`
fi

# Nothing to do if $localtar is up to date.  (Check the $menutar
# symlink as well as what it points to; after a full release, the
# target of the symlink might not be newer than the last time we
# updated, but the symlink itself will be newer.)
if [ "x$oldversion" = "x$newversion" ] && \
   [ -s $localtar ] && \
   [ "`find $menutar -newer $localtar`" != $menutar ] && \
   [ "`find $menutar -follow -newer $localtar`" != $menutar ]; then
  exit 0
fi

# Update the local menus area.
cp $menutar $localtar || fail
rm -rf $localdir
mkdir $localdir || fail
(cd $localdir && tar xof $localtar) || fail
chmod -R a+rX $localdir || fail
rm -rf $menus
ln -s $localdir $menus
echo "$newversion" > $versionfile
