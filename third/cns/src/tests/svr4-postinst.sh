#!/bin/sh

ROOT=/usr/kerberos

$ROOT/install/fixprot /

$ROOT/install/fixsvc

if [ -f $ROOT/lib/krb.conf ]; then
    true
else
    if [ -f /tmp/krb.conf ]; then
	mv -f /tmp/krb.conf /tmp/krb.realms $ROOT/lib/
	chown root $ROOT/lib/krb.conf $ROOT/lib/krb.realms
	chmod 444 $ROOT/lib/krb.conf $ROOT/lib/krb.realms
	# Call installf here?  How do non-destructive package updates work?
    else
	echo "Can't find krb.conf config file."		1>&2
	exit 1
    fi
fi

$ROOT/install/check-install / || exit 1

$ROOT/install/check-inetd

exit 0
