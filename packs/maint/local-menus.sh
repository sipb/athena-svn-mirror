#!/bin/sh
# $Id: local-menus.sh,v 1.1 2001-06-05 19:08:47 ghudson Exp $

# Tar file containing menus (a symlink into the AFS system config area).
menutar=/usr/athena/share/gnome/athena/menus.tar

# Menu directory; used as a fallback if we can't make a local copy of
# the menus.
menudir=/usr/athena/share/gnome/athena/menus

# Local copy of the menu tar file.
localtar=/var/athena/menus.tar

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

# Nothing to do if $localtar is up to date.
if [ -s $localtar ] && [ `find $menutar -newer $localtar` != $menutar ]; then
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
