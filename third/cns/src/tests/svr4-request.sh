#!/bin/sh

ROOT=/usr/kerberos
IROOT=`echo $0 | sed 's/[^/]*$//'`
IROOT=`cd $IROOT/../root/$ROOT ; pwd`

KRBCONF=$ROOT/lib/krb.conf

if [ -r $KRBCONF ]; then
  echo "Existing configuration for realm "`sed 1q $KRBCONF`" preserved."
  echo "To reconfigure it, delete $KRBCONF and run $ROOT/install/configure."
else
  sh $IROOT/install/ask-config /tmp || exit 1
fi

