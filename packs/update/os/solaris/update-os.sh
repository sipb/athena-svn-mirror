#!/sbin/sh

CONFVARS=$UPDATE_ROOT/var/athena/update.confvars
. $CONFVARS

cd $UPDATE_ROOT
if [ "$NEWOS" = "true" ]; then
  echo "Removing the patch/pkg DB for a new OS major rev"
  pks="`cat $OLDPKGS`"
  for i in $pks; do
    rm -rf $UPDATE_ROOT/var/sadm/pkg/$i
  done
  patches="`cat $OLDPTCHS`"
  for i in $patches; do
    rm -rf $UPDATE_ROOT/var/sadm/patch/$i
  done
fi

if [ -s "$DEADFILES" ]; then
  echo "Removing outdated files"
  dead="`cat $DEADFILES`"
  for i in $dead; do
    rm -rf $UPDATE_ROOT/$i
  done
fi

if [ -s "$LOCALPACKAGES" ]; then
  echo "Installing os local packages"
  for i in `cat "$LOCALPACKAGES"`; do
    echo "$i"
    cat /util/yes-file | pkgadd -R "$UPDATE_ROOT" -d /cdrom "$i"
  done 2>/dev/null
fi

if [ -s "$LINKPACKAGES" ]; then
  echo "Installing the os link packages"
  for i in `cat "$LINKPACKAGES"`; do
    echo "$i"
    cat /util/yes-file | pkgadd -R "$UPDATE_ROOT" -d /cdrom/cdrom.link "$i"
  done 2>/dev/null
fi

if [ "$NEWOS" = "true" ]; then
  echo "Making adjustments"
  cp /cdrom/I* "$UPDATE_ROOT/var/sadm/system/admin"
  rm $UPDATE_ROOT/etc/.UNC*
  rm "$UPDATE_ROOT/etc/.sysidconfig.apps"
  cp /cdrom/.sysIDtool.state "$UPDATE_ROOT/etc/default"
fi

if [ -s "$PATCHES" ]; then
  echo "Installing OS patches"
  cat /util/yes-file | patchadd -d -R "$UPDATE_ROOT" -u \
    -M /patches/patches.link `cat $PATCHES`
fi

if [ -s "$LOCALPACKAGES" -o -s "$LINKPACKAGES" -o -s "$PATCHES" ]; then
  echo "Performing local OS changes"
  sh /srvd/usr/athena/lib/update/oschanges

  # Restore any config file that pkgadd has replaced.
  echo "Re-copying config files after OS update"
  if [ -s "$CONFCHG" ]; then
    for i in `cat $CONFCHG`; do
      if [ -f "/srvd$i" ]; then
	rm -rf "$UPDATE_ROOT$i"
	cp -p "/srvd$i" "$UPDATE_ROOT$i"
      fi
      if [ true = "$PUBLIC" ]; then
	rm -rf "$UPDATE_ROOT$i.saved"
      fi
    done
  fi
fi

echo "Finished os installation"

echo "Tracking the srvd"
track -d -F /srvd -T "$UPDATE_ROOT" -W /srvd/usr/athena/lib

echo "Copying kernel modules from /srvd/kernel"
cp -p /srvd/kernel/fs/* "$UPDATE_ROOT/kernel/fs"

rm -f "$UPDATE_ROOT/var/spool/cron/crontabs/uucp"

if [ "$NEWDEVS" = "true" ]; then
  echo "Create devices and dev"
  cd "$UPDATE_ROOT"
  drvconfig -R $UPDATE_ROOT -r devices -p "$UPDATE_ROOT/etc/path_to_inst"
  chmod 755 "$UPDATE_ROOT/dev"
  chmod 755 "$UPDATE_ROOT/devices"
  cp /dev/null "$UPDATE_ROOT/reconfigure"
fi
