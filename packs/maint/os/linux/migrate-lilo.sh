#!/bin/sh

umask 022

# Check if we actually need to migrate from lilo to grub.
if [ -f /boot/grub/grub.conf -o ! -f /etc/lilo.conf ]; then
  exit
fi

echo "lilo no longer works properly under RHEL 4.  Migrating to grub."
echo

# Grab the boot device and timeout from lilo.conf.
bootdev=`sed -n -e 's/boot *= *\([^ ]*\)/\1/p' /etc/lilo.conf`
timeout=`sed -n -e 's/timeout *= *\([^ ]*\)/\1/p' /etc/lilo.conf`

# lilo specifies the timeout in tenths of a second, grub in seconds.
: $[timeout /= 10]

# Install grub onto the boot device.
/sbin/grub-install "$bootdev"

# Calculate the value for the splash image.  This is annoyingly hard.
# First we have to use the grub device map file to translate the boot
# device to the equivalent grub device.  Then we have to insert a
# partition number, which is one less than the Linux partition number
# of the /boot filesystem.  Then we have to figure out whether the
# path to the splash image will contain "/boot", which depends on
# whether /boot is in its own partition.

# First calculate the grub device name for the boot partition.
grubbootdev=`awk -v d="$bootdev" '$2 == d {print $1}' /boot/grub/device.map`
bootfs=`df /boot | awk 'NR == 2 { print $1 }'`
partnum=`echo "$bootfs" | sed -e "s|${bootdev}||"`
grubpartnum=$[$partnum - 1]
grubpart=`echo "$grubbootdev" | sed -e "s/)/,${grubpartnum})/"`

# Then decide whether we need to add a "/boot" path element to that.
bootmount=`df /boot | awk 'NR == 2 { print $6 }'`
if [ /boot = "$bootmount" ]; then
  boot=""
else
  boot=/boot
fi

# Finally, put together those pieces to get the splash image value.
splashimage=$grubpart$boot/grub/splash.xpm.gz

# Create a stub grub configuration file.
cat > /boot/grub/grub.conf << EOM
# grub.conf created from lilo.conf by Linux Athena migration script.
timeout=$timeout
splashimage=$splashimage
EOM

# Iterate over the Linux kernel entries in lilo.conf.
for kernel in `sed -n -e 's/image *= *\([^ ]*\)/\1/p' /etc/lilo.conf`; do

  # Get information about this lilo kernel entry.
  args=
  initrd=
  eval `/sbin/grubby --lilo --info="$kernel"`

  if [ -n "$initrd" ]; then
    initrdflag="--initrd $initrd"
  else
    initrdflag=
  fi

  # Grab the version number from the kernel name.
  vers=`echo "$kernel" | sed -e 's|^/boot/vmlinuz-||'`

  # Add two grub entries for this lilo kernel entry, one for
  # single-user and one for regular.
  /sbin/grubby --add-kernel="$kernel" $initrdflag \
    --title="Linux-Athena ($vers) (single user mode)" \
    --args="ro root=$root $args"
  /sbin/grubby --add-kernel="$kernel" $initrdflag \
    --title="Linux-Athena ($vers)" \
    --args="ro root=$root $args"
done

# Iterate over the non-Linux kernel entries in lilo.conf.
for dev in `sed -n -e 's/other *= *\([^ ]*\)/\1/p' /etc/lilo.conf`; do
  # Get the label for this entry from lilo.conf.
  label=`awk -F' *= *' -v dev="$dev" \
    '/other *=/ { gsub(" ", "", $2); if ($2 == dev) found = 1; }
     /label *=/ { if (found) { gsub(" ", "", $2); print $2; exit; } }' \
    /etc/lilo.conf`

  # Translate the device into grub terms.
  grubdev=`awk -v dev="$dev" \
    '{ if (sub($2, "", dev) == 1)
       { sub(")", "," (dev - 1) ")", $1); print $1; exit; }
     }' /boot/grub/device.map`

  # Write a grub.conf entry.
  cat >> /boot/grub/grub.conf << EOM
title $label
	rootnoverify $grubdev
	chainloader +1
EOM
done

# Move aside lilo.conf.
mv /etc/lilo.conf /etc/lilo.conf.old
