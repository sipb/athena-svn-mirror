#!/sbin/sh

# If an update is in progress, complete it.

# Get the SMF exit status definitions.
. /lib/svc/share/smf_include.sh

start_services() {
  if [ -f /etc/init.d/afs ]; then
    sh /etc/init.d/afs start
  fi
  if [ -f /etc/athena/config_afs ]; then
    sh /etc/athena/config_afs
  fi
}

state=`awk '{a=$5} END {print a}' /etc/athena/version`
case "$state" in
Install)
  start_services
  newvers=`awk '{a=$6} END {print a}' /etc/athena/version`
  if [ ! -r /srvd/install/finish-install-wrapper ]; then
    echo "Cannot finish partially completed install."
    echo "Please contact Athena Hotline at x3-1410.  Thank"
    echo "you. -Athena Operations"
    /bin/sh
  fi

  echo "Finishing partially completed install"
  exec sh /srvd/install/finish-install-wrapper "$newvers"
  ;;

Reboot)
  start_services
  if [ ! -r /srvd/usr/athena/lib/update/finish-update-wrapper ]; then
    echo "Cannot finish partially completed update."
    echo "Please contact Athena Hotline at x3-1410.  Thank"
    echo "you. -Athena Operations"
    /bin/sh
  fi

  echo "Finishing partially completed update"
  exec sh /srvd/usr/athena/lib/update/finish-update-wrapper
  ;;

ClearSwap)
  start_services
  method=`awk '{a=$6} END {print a}' /etc/athena/version`
  newvers=`awk '{a=$7} END {print a}' /etc/athena/version`

  UPDATE_ROOT=/
  SUNPLATFORM=`uname -m`
  export UPDATE_ROOT SUNPLATFORM

  . /srvd/usr/athena/lib/update/update-environment
  . $CONFDIR/rc.conf

  { sh /install/miniroot/setup-swap-boot \
    "$method" "$newvers" 2>&1 || {
    echo "Please contact Athena Hotline at x3-1410."
    echo "Thank you. -Athena Operations"
	    
    echo "Athena Workstation ($HOSTTYPE) Version Update" \
      "`date`" >> "$CONFDIR/version"
    exit 0
  }; } | tee -a /var/athena/update.log

  sync
  sleep 15
  exec reboot
  ;;

# The following code executes only when booted into the miniroot.
BootSwap)
  start_services
  UPDATE_ROOT=/root
  export UPDATE_ROOT

  method=`awk '{a=$6} END {print a}' /etc/athena/version`
  newvers=`awk '{a=$7} END {print a}' /etc/athena/version`

  CONFVARS="$UPDATE_ROOT/var/athena/update.confvars"
  . $CONFVARS

  PATH=/sbin:/usr/bin:/usr/sbin
  export PATH

  echo "Get the rootdrive...."
  rrootdrive=`cat /etc/vfstab | awk '$3 == "/root" {print $2}'`
  rootdrive=`cat /etc/vfstab | awk '$3 == "/root" {print $1}'`

  umask 022

  # Make symlinks from our real root to the root we're
  # installing into. For /srvd, /os, and /install, attach
  # was done into the UPDATE_ROOT before we rebooted to
  # get here, so we don't have to do the attach again. For
  # /afs, AFS is going to be mounted in UPDATE_ROOT.
  for i in /srvd /os /install; do
    rm -f "$i"
    ln -s "$UPDATE_ROOT$i" "$i"
  done

  . /srvd/usr/athena/lib/update/update-environment
  . $CONFDIR/rc.conf

  if [ "$PUBLIC" = true ]; then
    echo "Turning on delayed I/O..."
    /sbin/fastfs "$UPDATE_ROOT" fast
    /sbin/fastfs "$UPDATE_ROOT/usr" fast
    /sbin/fastfs "$UPDATE_ROOT/var" fast
  fi

  sh /srvd/usr/athena/lib/update/update-os $newvers 2>&1 \
    | tee -a $UPDATE_ROOT/var/athena/update.log

  echo "finished update-os"		    

  platform=`uname -i`
  echo "Installing bootblocks for $platform on root"
  installboot "/os/usr/platform/$platform/lib/fs/ufs/bootblk" "$rrootdrive"

  boot=`cat /etc/boot-device`
  echo "restore the prom boot-device setting to $boot"
  /os/usr/platform/$platform/sbin/eeprom "$boot"

  echo "Updating version for reboot"
  echo "Athena Workstation ($HOSTTYPE) Version Reboot" \
    "$method $newvers `date`" >> "$CONFDIR/version"

  if [ "$PUBLIC" = true ]; then
    echo "Turning off delayed I/O..."
    /sbin/fastfs "$UPDATE_ROOT" slow
    /sbin/fastfs "$UPDATE_ROOT/usr" slow
    /sbin/fastfs "$UPDATE_ROOT/var" slow
  fi
  sync
  sleep 15
  reboot
  ;;
esac

exit $SMF_EXIT_OK
