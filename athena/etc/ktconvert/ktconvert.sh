#!/bin/sh
# $Id: ktconvert.sh,v 1.1 2004-03-14 22:39:24 zacheiss Exp $
#
# Convert /etc/athena/srvtab to /etc/krb5.keytab, removing the srvtab
# when we're done.

usage="Usage: ktconvert [ -r ]
       -r indicates /etc/krb5.keytab should be converted back into /etc/athena/srvtab."

fromfile=/etc/athena/srvtab
tofile=/etc/krb5.keytab
readop=rst
writeop=wkt

while getopts r opt; do
  case "$opt" in
  r)
    fromfile=/etc/krb5.keytab
    tofile=/etc/athena/srvtab
    readop=rkt
    writeop=wst
    ;;
  \?)
    echo "$usage" >&2
    exit 1
    ;;
  esac
done

case `id` in
"uid=0("*)
  ;;
*)
  echo "You are not root.  This script must be run as root."
  exit 1
  ;;
esac

if [ ! -f $fromfile ]; then
  echo "$fromfile does not exist."
  exit 1
fi

if [ -f $tofile ]; then
  echo "$tofile already exists."
  exit 1
fi

# We redirect stderr below because libreadline spews warnings
# we don't care about.
/usr/athena/etc/ktutil <<EOF >/dev/null 2>&1
$readop $fromfile
$writeop $tofile
quit
EOF

if [ $? != 0 ]; then
  echo "Failed to run ktutil."
  exit 1
fi

# Ensure file we're removing is writable so saferm will remove it.
chmod 600 $fromfile
/bin/athena/saferm -z $fromfile

if [ $? != 0 ]; then
  echo "saferm failed to remove $fromfile."
  exit 1
fi

echo "Converted $fromfile to $tofile."
exit 0
