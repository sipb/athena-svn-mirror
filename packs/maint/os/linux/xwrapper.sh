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

exec /usr/X11R6/bin/X $options "$@"
