#!/bin/sh
# $Id: ksrvutil.sh,v 1.1 2004-03-14 22:37:29 zacheiss Exp $

cat >&2 <<'EOM'

ksrvutil has been replaced with the command 'k5srvutil'.  k5srvutil
supports many of the same command line options as ksrvutil, including
the 'change' command.  For more information on using k5srvutil, type
'man k5srvutil'.

In order to use k5srvutil, you must convert your srvtab into a
Kerberos 5 keytab.  To do this, run the command 'ktconvert' while
logged in as root.  This will convert /etc/athena/srvtab into a
Kerberos 5 keytab located at /etc/krb5.keytab.  /etc/athena/srvtab
will be removed if the conversion is successful.

If you need assistance, contact Athena On-Line Consulting by running
the command 'olc'.

EOM

exit 0
