#!/bin/sh

# Give people a way to disable the wrapper if it's annoying.
if [ -f /etc/athena/x-no-wrapper ]; then
	exec /usr/bin/X11/X "$@"
fi

# If the user has explicitly asked for 8-bit mode, or if this machine
# is incapable of 24-bit mode, use default visual depth 8.
# We assume any graphics board other than Indy 8-bit can deal with
# depth 24.  At worst, the X server should fall back to 8-bit mode.
if [ -f /var/athena/x-8-bit \
     -o -n "`hinv -c graphics | awk '/ 8-bit/'`" ]; then
	options="-depth 8"
else
	options="-class TrueColor -depth 24"
fi
rm -f /var/athena/x-8-bit

exec /usr/bin/X11/X $options "$@"
