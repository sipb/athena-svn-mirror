#!/bin/sh
# suitable for running from init with a line like:
# null    "/usr/kerberos/etc/run-kpropd"  unknown         on
# in /etc/ttys (or ttytab, on suns.)
PATH=/usr/kerberos/etc /usr/kerberos/etc/kpropd /usr/kerberos/database/slavedb 2>&1 > /dev/console
