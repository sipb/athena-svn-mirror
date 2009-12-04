#!/bin/sh

# This is the xinit script for the kiosk user.

datadir=/usr/share/debathena-kiosk

# The rkiosk extension's unique ID, and the location of its xpi file.
rkiosk_id="{4D498D0A-05AD-4fdb-97B5-8A0AABC1FC5B}"
rkiosk_xpi=$datadir/r-kiosk.xpi

# File with preferences to set in the firefox profile.
prefs="$datadir/prefs.js"

# User chrome (UI customizations) file.
chrome="$datadir/userChrome.css"

cd || exit 1

exec > xinit.log 2>&1

XAUTHORITY=$HOME/.Xauthority
export XAUTHORITY

# Run a window manager so that firefox fullscreen mode works.
metacity --sm-disable &

# Create a new firefox profile.
rm -rf .mozilla
firefox -CreateProfile default > /dev/null || exit 1
profdir=$(find .mozilla/firefox -type d -name \*.default)

# Add the rkiosk extension.
mkdir -p "$profdir/extensions/$rkiosk_id"
(cd "$profdir/extensions/$rkiosk_id" && unzip "$rkiosk_xpi") || exit 1
cat <<EOF >> "$profdir/extensions.ini"
[ExtensionDirs]
Extension0=$profdir/extensions/$rkiosk_id
EOF

# Apply our preference settings and UI customizations.
cat "$prefs" >> "$profdir/prefs.js" || exit 1
cp "$chrome" "$profdir/chrome/userChrome.css" || exit 1

# Launch firefox.
firefox

# Clean up.
rm -rf .mozilla
