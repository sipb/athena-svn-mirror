#!/sbin/sh

newvers=$1

CONFVARS=$UPDATE_ROOT/var/athena/update.confvars
. $CONFVARS

# drvconfig requires this.  In theory, nothing else should.
cd ${UPDATE_ROOT:-/}

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
  sed -e "s|^|$UPDATE_ROOT|" "$DEADFILES" | xargs rm -rf
fi

# Save any config files that an OS package may touch.
if [ "$OSCHANGES" = true -a -s "$OSCONFCHG" ]; then
  echo "Saving config files prior to OS update"
  for i in `cat $OSCONFCHG`; do
    if [ -f "$UPDATE_ROOT$i" ]; then
      rm -rf "$UPDATE_ROOT$i.save-update"
      cp -p "$UPDATE_ROOT$i" "$UPDATE_ROOT$i.save-update"
    fi
  done
fi

yes="y\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny"

if [ -s "$PACKAGES" ]; then
  pkglog=$UPDATE_ROOT/var/athena/update.pkglog
  rm -f "$pkglog"
fi

if [ -s "$PACKAGES" ]; then
  echo "Installing the os packages"
  for i in `cat "$PACKAGES"`; do
    echo "$i"
    pkgadd -a $LIBDIR/admin-update -n -R "$UPDATE_ROOT" \
      -d /install/cdrom "$i"
  done 2>>$pkglog
fi

if [ "$NEWOS" = "true" ]; then
  echo "Making adjustments"
  cp /install/cdrom/INST_RELEASE "$UPDATE_ROOT/var/sadm/system/admin"
  cp /install/cdrom/mach/sun4u/etc/driver_aliases "$UPDATE_ROOT/etc"
  rm $UPDATE_ROOT/etc/.UNC*
  rm "$UPDATE_ROOT/etc/.sysidconfig.apps"
  cp /install/cdrom/.sysIDtool.state "$UPDATE_ROOT/etc/default"
fi

if [ -s "$PATCHES" ]; then
  echo "Installing OS patches"
  # patchadd is stupid and elides blank arguments, so we have to be careful
  # specifying the update root.
  ur="${UPDATE_ROOT:+-R $UPDATE_ROOT}"
  echo "$yes" | patchadd -d $ur -u -M /install/patches/patches.link \
    `cat $PATCHES`
fi

if [ "$OSCHANGES" = true ]; then
  echo "Performing local OS changes"
  sh /srvd/install/oschanges

  # Restore any config file that an OS package may have clobbered.
  echo "Restoring config files after OS update"
  if [ -s "$OSCONFCHG" ]; then
    for i in `cat $OSCONFCHG`; do
      if [ -f "$UPDATE_ROOT$i.save-update" ]; then
	# If the file has been changed, restore it.  On a private machine,
	# save the version as edited by Sun packages.
	cmp -s "$UPDATE_ROOT$i" "$UPDATE_ROOT$i.save-update" || {
	  if [ false = "$PUBLIC" ]; then
	    rm -rf "$UPDATE_ROOT$i.sunpkg"
	    case $i in
	    /var/spool/cron/crontabs/*)
	      # Save a crontab file in another directory.
	      user=`basename $i`
	      save_dir=/var/athena/update.save
	      save_file="$save_dir/crontab.$user.sunpkg"
	      mkdir -p "$UPDATE_ROOT$save_dir"
	      mv "$UPDATE_ROOT$i" "$UPDATE_ROOT$save_file"
	      ;;
	    *)
	      # Generic config file, save in the same directory.
	      mv "$UPDATE_ROOT$i" "$UPDATE_ROOT$i.sunpkg"
	      ;;
	    esac
	  fi
	  rm -rf "$UPDATE_ROOT$i"
	  cp -p "$UPDATE_ROOT$i.save-update" "$UPDATE_ROOT$i"
	}
      fi
      rm -rf "$UPDATE_ROOT$i.save-update"
    done
  fi
fi

# Force a device reconfigure on reboot.
touch $UPDATE_ROOT/reconfigure

echo "Finished os installation"

# Install any "core" Athena packages which need updating now; these
# are the packages needed by finish-update to do its work.
if [ -s "$MIT_CORE_PACKAGES" ]; then
  echo "Installing new Athena core packages"
  for i in `cat "$MIT_CORE_PACKAGES"`; do
    echo "$i"
    pkgadd -a $LIBDIR/admin-update -n -R "${UPDATE_ROOT:-/}" \
      -d /srvd/pkg/$newvers "$i"
  done
  echo "Finished installing Athena core packages."
fi

# Make sure the target also has a current finish-update script.
cmp -s /srvd/etc/init.d/finish-update \
  "$UPDATE_ROOT/etc/init.d/finish-update" || {
    cp -p /srvd/etc/init.d/finish-update "$UPDATE_ROOT/etc/init.d"
    rm -f "$UPDATE_ROOT/etc/rc2.d/S70finish-update"
    ln -s ../init.d/finish-update "$UPDATE_ROOT/etc/rc2.d/S70finish-update"
}

rm -f "$UPDATE_ROOT/var/spool/cron/crontabs/uucp"

if [ "$NEWDEVS" = "true" ]; then
  echo "Create devices and dev"
  drvconfig -R "$UPDATE_ROOT" -r devices -p "$UPDATE_ROOT/etc/path_to_inst"
  chmod 755 "$UPDATE_ROOT/dev"
  chmod 755 "$UPDATE_ROOT/devices"
  cp /dev/null "$UPDATE_ROOT/reconfigure"
fi
