#!/bin/sh
# $Id: htmlview.sh,v 1.1 2000-06-14 20:25:39 ghudson Exp $

# htmlview script adapted from the infoagents locker to take advantage
# of the local netscape, if present.

#
#   htmlview - script to view an HTML doc in an existing browser if
#	       possible, otherwise start up a new one
  usage="Usage: htmlview [ -c ] URL
	-c indicates that SSL/certificate authentication is required."
  optstring=c

# Parse options
cert_required=false
args="`getopt $optstring $*`"
if [ $? != 0 ]; then
     echo "$usage" >&2
     exit 2
fi
for arg in $args
do
  case $arg in
    -c) cert_required=true
      ;;
    --)
      ;;
    *)
      url_arg=$arg
      ;;
  esac
done

# Handle files in current directory appropriately.
case "$url_arg" in
  "")
     # No URL supplied.
     echo "$usage" >&2
     exit 2
     ;;
  http:* | ftp:* | gopher:* | /*)
     # Full URL's and absolute pathnames require no special treatment.
     url=$url_arg
     ;;
  https:*)
     # https URL's require SSL (and maybe certificate) support.
     cert_required=true
     url=$url_arg
     ;;
  *)
    # Relative path to existing file--need absolute path for running browser.
    if [ -r $url_arg ]; then
       url=`pwd`/$url_arg
    else
       url=$url_arg
    fi
    ;;
esac

# If $DISPLAY is not set, just start lynx
case "$DISPLAY" in
  "")
    exec /bin/athena/attachandrun infoagents lynx lynx "$url"
    ;;
esac

# Method of checking if a process exists
if kill -0 $$ > /dev/null 2>&1; then
    # Modern OS; easy
    process_exists="kill -0"
else
    # Old OS, e.g. Ultrix 4.2.  For this script, kill -CONT is ok.
    process_exists="kill -CONT"
fi

# Check for existing netscape
netscape_pid=`ls -l $HOME/.netscape/lock 2>/dev/null | /usr/bin/awk -F: '{ print $NF }'`
case "$netscape_pid" in
  "") ;;
  *)
    if $process_exists $netscape_pid > /dev/null 2>&1; then
        # Try to have it open the given URL; exit if successful
        /usr/athena/bin/netscape -remote "OpenUrl($url)" && exit 0
        echo "$0: Could not use netscape already running..." >&2
    else
        echo "$0: Could not find netscape process" >&2
	echo "with id $netscape_pid on this workstation." >&2
	echo "Perhaps you are running netscape on another workstation." >&2
    fi
  ;;
esac

# We are running under X, and could not use an browser already running.

# Try to run netscape; exit if successful
/usr/athena/bin/netscape "$url" && exit 0
echo "$0: Could not run Netscape.  Will try Lynx." >&2

# Start Lynx
/bin/athena/attachandrun infoagents lynx lynx "$url"

echo "$0: Could not find or start any WWW browser." >&2
exit 1
