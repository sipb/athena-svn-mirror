#!/bin/sh
# $Id: htmlview.sh,v 1.7 2005-04-23 20:38:40 rbasch Exp $

# htmlview script adapted from the infoagents locker to take advantage
# of the local netscape, if present.

#
#   htmlview - script to view an HTML doc in an existing browser if
#	       possible, otherwise start up a new one
  usage="Usage: htmlview [ -c ] [ -f ]  URL
        -c indicates that SSL/certificate authentication is required.
        -f forces Lynx to treat displayed files as HTML"
  optstring=cf

# Parse options
cert_required=false
force_lynx_html=false
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
    -f) lynx_flags="-force_html"
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

# Quote commas in the URL; they won't work with OpenUrl().
url=`echo "$url" | sed 's/,/%2C/g'`

# If $DISPLAY is not set, just start lynx
case "$DISPLAY" in
  "")
    if [ $cert_required = true ]; then
      echo "$0: Could not find or start any WWW browser that supports " \
        "personal certificates while opening $url" >&2
      exit 1
    else
      exec /bin/athena/attachandrun infoagents lynx lynx "$url"
    fi
    ;;
esac

# Try to display the URL with the user's preferred viewer.
/usr/athena/bin/gnome-open "$url" && exit 0
if [ $cert_required = true ]; then
  echo "$0: Could not find or start any WWW browser that supports " \
    "personal certificates while opening $url" >&2
  exit 1
else
  echo "$0: Could not run gnome-open.  Will try Lynx." >&2

  # Start Lynx
  /bin/athena/attachandrun infoagents lynx lynx $lynx_flags "$url" && exit 0
fi

echo "$0: Could not find or start any WWW browser." >&2
exit 1
