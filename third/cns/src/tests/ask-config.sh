#!/bin/sh

dir=$1

if [ "$dir" = "" ]; then

  echo "$0 : one argument required, directory containing config files" 1>&2
  exit 1

else

  echo "Enter name of local realm (for example, CYGNUS.COM):"
  read REALM
  echo "Enter hostname of primary kerberos server:"
  read SERVER
  cat > $dir/krb.conf << ---EOF---
$REALM
$REALM $SERVER admin server
ATHENA.MIT.EDU kerberos.mit.edu admin server
ATHENA.MIT.EDU kerberos-1.mit.edu
ATHENA.MIT.EDU kerberos-2.mit.edu
ATHENA.MIT.EDU kerberos-3.mit.edu
LCS.MIT.EDU kerberos.lcs.mit.edu. admin server
SMS_TEST.MIT.EDU dodo.mit.edu admin server
LS.MIT.EDU ls.mit.edu admin server
IFS.UMICH.EDU kerberos.ifs.umich.edu
CS.WASHINGTON.EDU hawk.cs.washington.edu
CS.WASHINGTON.EDU aspen.cs.washington.edu
CS.BERKELEY.EDU okeeffe.berkeley.edu
SOUP.MIT.EDU soup.mit.edu admin server
TELECOM.MIT.EDU bitsy.mit.edu
MEDIA.MIT.EDU kerberos.media.mit.edu
NEAR.NET kerberos.near.net
CATS.UCSC.EDU kerberos.ucsc.edu admin server
CATS.UCSC.EDU kerberos-1.ucsc.edu
WATCH.MIT.EDU kerberos.watch.mit.edu admin server
CYGNUS.COM kerberos-1.cygnus.com.
CYGNUS.COM kerberos.cygnus.com admin server
PANIX.COM kerberos.panix.com. admin server
PANIX.COM cerebus.panix.com.
---EOF---
  # do krb.realms -- only put "known" specials here.
  cat > $dir/krb.realms << ---EOF---
.MIT.EDU ATHENA.MIT.EDU
.MIT.EDU. ATHENA.MIT.EDU
MIT.EDU ATHENA.MIT.EDU
DODO.MIT.EDU SMS_TEST.MIT.EDU
.UCSC.EDU CATS.UCSC.EDU
.UCSC.EDU. CATS.UCSC.EDU
CYGNUS.COM CYGNUS.COM
.CYGNUS.COM CYGNUS.COM
---EOF---

fi

exit 0
