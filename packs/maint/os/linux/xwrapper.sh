#!/bin/sh

# Give people a way to disable the wrapper if it's annoying.
if [ -f /etc/athena/x-no-wrapper ]; then
  exec /usr/X11R6/bin/X "$@"
fi

options=
if [ -f /var/athena/x-8-bit ]; then
  options="-bpp 8"
fi
rm -f /var/athena/x-8-bit

# Red Hat 7.3 punts their X wrapper (/usr/X11R6/bin/X is just a
# symlink to XFree86, the XFree86 4 server), so we have to use the
# symlink ourselves if it's there to maintain XFree86 3 compatibility.
if [ -x /etc/X11/X ]; then
  exec /etc/X11/X $options "$@"
else
  exec /usr/X11R6/bin/X $options "$@"
fi
