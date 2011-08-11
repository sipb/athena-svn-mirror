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

choice=$(zenity --title="What would you like to do?" --text="Select the task y\
ou wish to perform, then click \"OK\"" --list  --column="task" --column="Activ\
ity" --hide-column=1 "reg" "Register for Account" "web" "Browse the Web")
[ $? -eq 0 ] || exit 0
if [ "$choice" = "reg" ]; then
    errtext="An error occurred while trying to launch the registration applet.  Please try another workstation."
    classzip="/afs/athena.mit.edu/system/register/java/regclasses.zip"
    if [ ! -r "$classzip" ]; then
	zenity --error --text="${errtext}\n\n(could not unzip class file)"
	exit 0
    fi
    unzip $classzip
    java -Dhost="moira.mit.edu" regapplet.Regapplet || zenity --error --text="${errtext}\n\n(could not launch java)"
    exit 0
fi

# Warn the user that printing is wrong.
zenity --warning --title="No Printing" --text="You will not be able to print from this browser session.  Submitted print jobs will be discarded and cannot be released from Pharos printers.\n\nIf you need to print, please log in to a normal Athena session.  If you are a visitor looking to print a Campus Map, please visit the Information Center in Room 7-121."

# Create a new firefox profile.
rm -rf .mozilla
firefox -CreateProfile default > /dev/null || exit 1
profdir=$(find .mozilla/firefox -type d -name \*.default)

# Add the rkiosk extension.
rkiosk_dir="$profdir/extensions/$rkiosk_id"
mkdir -p "$rkiosk_dir"
(cd "$rkiosk_dir" && unzip "$rkiosk_xpi") || exit 1
cat <<EOF >> "$profdir/extensions.ini"
[ExtensionDirs]
Extension0=$rkiosk_dir
EOF

# Firefox 3.6+ requires a chrome.manifest for the extension.
if [ ! -e "$rkiosk_dir/chrome.manifest" ]; then
  cp "$datadir/chrome.manifest.r-kiosk" "$rkiosk_dir/chrome.manifest" || exit 1
fi

# Apply our preference settings and UI customizations.
cat "$prefs" >> "$profdir/prefs.js" || exit 1
cp "$chrome" "$profdir/chrome/userChrome.css" || exit 1

# Launch firefox.
firefox

# Clean up.
rm -rf .mozilla
