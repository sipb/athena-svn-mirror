#!/sbin/sh

newvers=$1

CONFVARS=$UPDATE_ROOT/var/athena/update.confvars
. $CONFVARS

# drvconfig requires this.  In theory, nothing else should.
cd ${UPDATE_ROOT:-/}

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

if [ "$NEWOS" = "true" ]; then
  if [ -s "$OSPRESERVE" ]; then
    echo "Preserving config files before removing OS packages"
    ospreserve="`cat $OSPRESERVE`"
    for i in $ospreserve ; do
      if [ -f "$UPDATE_ROOT$i" ]; then
	rm -rf "$UPDATE_ROOT$i.update-preserve"
	cp -p "$UPDATE_ROOT$i" "$UPDATE_ROOT$i.update-preserve"
      fi
    done
  else
    ospreserve=
  fi

  echo "Removing existing packages and patches for a new OS rev"
  pkgs="`cat $OLDPKGS`"
  for i in $pkgs; do
    echo "$i"
    pkgrm -a "$LIBDIR/admin-update" -n -R "$UPDATE_ROOT" "$i.*"
  done 2>>$pkglog

  # We can't use patchrm on patches, because we don't back up
  # files replaced by patchadd.  So just remove the patch directory.
  patches="`cat $OLDPTCHS`"
  for i in $patches; do
    rm -rf $UPDATE_ROOT/var/sadm/patch/$i
  done

  # Restore the files we preserved above.
  if [ -n "$ospreserve" ]; then
    echo "Restoring preserved config files after removing OS packages"
    for i in $ospreserve ; do
      cp -p "$UPDATE_ROOT$i.update-preserve" "$UPDATE_ROOT$i"
      rm -f "$UPDATE_ROOT$i.update-preserve"
    done
  fi
fi

if [ -s "$DEADFILES" ]; then
  echo "Removing outdated files"
  sed -e "s|^|$UPDATE_ROOT|" "$DEADFILES" | xargs rm -rf
fi

yes="y\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny"

if [ -s "$PACKAGES" ]; then
  pkglog=$UPDATE_ROOT/var/athena/update.pkglog
  rm -f "$pkglog"
fi

if [ -s "$PACKAGES" ]; then
  # For platform-dependent packages, we assume here that the package
  # directory name is $pkg.$suffix, where the suffix is 'u' for sun4u,
  # etc.  (This could be gleaned from /install/cdrom/.packagetoc).
  suffix=`uname -m | sed -e 's|sun4||g'`

  echo "Installing the os packages"
  for i in `cat "$PACKAGES"`; do
    # If this package is platform-dependent, it will have a subdirectory
    # in the distribution directory named foo.$suffix.  If there is no
    # such directory, it must be platform-independent.
    if [ -d "/install/cdrom/$i.$suffix" ] ; then
      pkgdir="$i.$suffix"
    else
      pkgdir="$i"
    fi
    echo "$pkgdir"
    pkgadd -a $LIBDIR/admin-update -n -R "$UPDATE_ROOT" \
      -d /install/cdrom "$pkgdir"
  done 2>>$pkglog
fi

if [ "$NEWOS" = "true" ]; then
  echo "Making adjustments"
  cp /install/cdrom/INST_RELEASE "$UPDATE_ROOT/var/sadm/system/admin"
  rm -f "$UPDATE_ROOT/etc/.UNCONFIGURED"
  rm -f "$UPDATE_ROOT/etc/.sysidconfig.apps"
  touch "$UPDATE_ROOT/etc/.NFS4inst_state.domain"
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
    $LIBDIR/pkg-update -R "${UPDATE_ROOT:-/}" -d /srvd/pkg/$newvers "$i"
  done
  echo "Finished installing Athena core packages."
fi

rm -f "$UPDATE_ROOT/var/spool/cron/crontabs/uucp"

if [ "$NEWDEVS" = "true" ]; then
  echo "Create devices and dev"
  devfsadm -R "$UPDATE_ROOT"
  chmod 755 "$UPDATE_ROOT/dev"
  chmod 755 "$UPDATE_ROOT/devices"
  cp /dev/null "$UPDATE_ROOT/reconfigure"
fi
