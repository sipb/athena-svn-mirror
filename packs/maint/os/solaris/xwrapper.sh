#!/bin/sh

# Give people a way to disable the wrapper if it's annoying.
if [ -f /etc/athena/x-no-wrapper ]; then
  exec /usr/openwin/bin/Xsun "$@"
fi

framebuf=`/bin/athena/machtype -d`
options=

# Determine if user has selected "restart in 8-bit mode".
if [ -f /var/athena/x-8-bit ]; then
  mode=8
else
  mode=24
fi
rm -f /var/athena/x-8-bit

# Based on the frame buffer and mode, run commands or specify extra
# options.
case $framebuf,$mode in
afb,24|ffb,24|ifb,24|jfb,24)
  # With no options, the default visual would be 8-bit.
  options="-dev /dev/fb defdepth 24"
  ;;
m,24)
  # Reset the card to defaults (in case there's a new monitor).
  # Then try to configure the card to 1152x900x76 at depth 24.
  # Redirect stdin so if m64config asks for a confirmation
  # (because it can't be sure the monitor supports this resolution)
  # the answer is "no."  In that case, if the card supports depth 24,
  # try to configure it with that depth at the default resolution,
  # bypassing confirmation if the monitor's supported resolutions
  # are unknown.
  m64config -defaults
  m64config -res 1152x900x76 -depth 24 < /dev/null > /dev/null 2>&1 || {
    conf=noconfirm
    eval `m64config -prconf | nawk '
      /^Monitor possible resolution/ { print "conf=;"; }
      /^Current resolution setting:/ { print "defres=" $4 ";"; }
      /^Possible depths:.*24(,|$)/ { print "depth24=true;"; }
      /^Current depth:/ { print "curdepth=" $3 ";"; }'`
    if [ "$curdepth" -ne 24 -a "$depth24" = true -a \
	 -n "$defres" -a "$defres" != none ]; then
      m64config -res "$defres" $conf -depth 24 < /dev/null > /dev/null 2>&1
    fi
  }
  ;;
m,8)
  # Reset the card to defaults (in case there's a new monitor).
  # Then try to configure the card to 1152x900x76 at depth 8.
  # This appears to be necessary to get an 8 bit visual on 
  # Suns that have updated to Solaris 8.
  m64config -defaults
  m64config -res 1152x900x76 -depth 8 < /dev/null > /dev/null 2>&1
  ;;
esac

exec /usr/openwin/bin/Xsun $options "$@"
