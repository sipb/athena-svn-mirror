#!/bin/sh
(
echo "Kerberos Slave Dump Log"
rm /usr/kerberos/database/slavesave.dump_ok
/usr/kerberos/etc/kdb_util slave_dump /usr/kerberos/database/slavesave
/usr/kerberos/etc/kprop /usr/kerberos/database/slavesave /usr/kerberos/database/slavelist 
) 2>&1 | /bin/mail root
