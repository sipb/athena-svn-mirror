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
afb,24|ffb,24|ifb,24)
	# With no options, the default visual would be 8-bit.
	options="-dev /dev/fb defdepth 24"
	;;
m,24)
	# Reset the card to defaults (in case there's a new monitor).
	# Then try to configure the card to 1152x900x76 at depth 24.
	# Redirect stdin so if m64config asks for a confirmation
	# (because it can't be sure the monitor can do this) the
	# answer is "no."  Make all failures silent; if we end up in
	# 8-bit mode, that's not a tragedy.
	m64config -defaults
	m64config -res 1152x900x76 -depth 24 < /dev/null > /dev/null 2>&1
	;;
m,8)
	# Reset the card to defaults.  We may get a better resolution
	# this way then if we left the card configured to depth 24 and
	# used X options.
	m64config -defaults
	;;
esac

exec /usr/openwin/bin/Xsun $options "$@"
