#!/bin/sh
DESTDIR=$1
chmod 755 $DESTDIR/usr/kerberos
chmod 755 $DESTDIR/usr/kerberos/include
chmod 755 $DESTDIR/usr/kerberos/include/ss
chmod 755 $DESTDIR/usr/kerberos/etc
chmod 755 $DESTDIR/usr/kerberos/man
chmod 755 $DESTDIR/usr/kerberos/man/man1
chmod 755 $DESTDIR/usr/kerberos/man/man3
chmod 755 $DESTDIR/usr/kerberos/man/man5
chmod 755 $DESTDIR/usr/kerberos/man/man8
chmod 755 $DESTDIR/usr/kerberos/bin
chmod 755 $DESTDIR/usr/kerberos/lib
chmod 755 $DESTDIR/usr/kerberos/install
chmod 444 $DESTDIR/usr/kerberos/include/*.h
chmod 444 $DESTDIR/usr/kerberos/include/ss/*.h
chmod 555 $DESTDIR/usr/kerberos/etc/ext_srvtab
chmod 555 $DESTDIR/usr/kerberos/etc/kdb_destroy
chmod 555 $DESTDIR/usr/kerberos/etc/kdb_edit
chmod 555 $DESTDIR/usr/kerberos/etc/kdb_init
chmod 555 $DESTDIR/usr/kerberos/etc/kdb_util
chmod 555 $DESTDIR/usr/kerberos/etc/kstash
chmod 555 $DESTDIR/usr/kerberos/etc/tftpd
chmod 555 $DESTDIR/usr/kerberos/etc/kerberos
chmod 555 $DESTDIR/usr/kerberos/etc/kprop
chmod 555 $DESTDIR/usr/kerberos/etc/kpropd
chmod 555 $DESTDIR/usr/kerberos/etc/start-kpropd
chmod 555 $DESTDIR/usr/kerberos/etc/run-kpropd
chmod 555 $DESTDIR/usr/kerberos/etc/push-kprop
chmod 555 $DESTDIR/usr/kerberos/etc/in.kpropd
chmod 555 $DESTDIR/usr/kerberos/etc/kadmind
chmod 555 $DESTDIR/usr/kerberos/etc/popper
chmod 555 $DESTDIR/usr/kerberos/etc/kshd
chmod 555 $DESTDIR/usr/kerberos/etc/klogind
chmod 555 $DESTDIR/usr/kerberos/etc/login.krb
chmod 555 $DESTDIR/usr/kerberos/etc/telnetd
chmod 555 $DESTDIR/usr/kerberos/etc/ftpd
chmod 555 $DESTDIR/usr/kerberos/install/check-install
chmod 555 $DESTDIR/usr/kerberos/install/fixprot
chmod 555 $DESTDIR/usr/kerberos/install/configure
chmod 444 $DESTDIR/usr/kerberos/man/man1/*.1
chmod 444 $DESTDIR/usr/kerberos/man/man3/*.3
chmod 444 $DESTDIR/usr/kerberos/man/man5/*.5
chmod 444 $DESTDIR/usr/kerberos/man/man8/*.8
chmod 555 $DESTDIR/usr/kerberos/bin/compile_et
chmod 555 $DESTDIR/usr/kerberos/bin/mk_cmds
chmod 555 $DESTDIR/usr/kerberos/bin/rsh
chmod 555 $DESTDIR/usr/kerberos/bin/rlogin
chmod 555 $DESTDIR/usr/kerberos/bin/cygin
chmod 555 $DESTDIR/usr/kerberos/bin/tftp
chmod 555 $DESTDIR/usr/kerberos/bin/tcom
chmod 555 $DESTDIR/usr/kerberos/bin/kinit
chmod 555 $DESTDIR/usr/kerberos/bin/kdestroy
chmod 555 $DESTDIR/usr/kerberos/bin/klist
chmod 555 $DESTDIR/usr/kerberos/bin/ksrvtgt
chmod 555 $DESTDIR/usr/kerberos/bin/kpasswd
chmod 555 $DESTDIR/usr/kerberos/bin/kadmin
chmod 555 $DESTDIR/usr/kerberos/bin/ksrvutil
chmod 555 $DESTDIR/usr/kerberos/bin/build_pwfile
chmod 555 $DESTDIR/usr/kerberos/bin/movemail
chmod 555 $DESTDIR/usr/kerberos/bin/pfrom
chmod 555 $DESTDIR/usr/kerberos/bin/telnet
chmod 555 $DESTDIR/usr/kerberos/bin/ftp
if [ -f $DESTDIR/usr/kerberos/bin/krdist ]; then
  chmod 555 $DESTDIR/usr/kerberos/bin/krdist
  chmod 555 $DESTDIR/usr/kerberos/bin/rdistd
fi
chmod 444 $DESTDIR/usr/kerberos/lib/libcom_err.a
chmod 444 $DESTDIR/usr/kerberos/lib/libss.a
chmod 444 $DESTDIR/usr/kerberos/lib/libkrb.a
chmod 444 $DESTDIR/usr/kerberos/lib/libkdb.a
chmod 444 $DESTDIR/usr/kerberos/lib/libkadm.a
chmod 444 $DESTDIR/usr/kerberos/lib/libacl.a
chmod 444 $DESTDIR/usr/kerberos/lib/libkstream.a

if [ x$2 = x ]; then
  chown root $DESTDIR/usr/kerberos/bin/rcp
  chmod 4555 $DESTDIR/usr/kerberos/bin/rcp
  chown root $DESTDIR/usr/kerberos/bin/ksu 
  chmod 4555 $DESTDIR/usr/kerberos/bin/ksu
  chown root $DESTDIR/usr/kerberos/database
  chmod 700 $DESTDIR/usr/kerberos/database
else
  chmod 555 $DESTDIR/usr/kerberos/bin/rcp
  chmod 555 $DESTDIR/usr/kerberos/bin/ksu
  chmod 755 $DESTDIR/usr/kerberos/database
fi

#ifdef NOENCRYPTION
chmod 444 $DESTDIR/usr/kerberos/lib/libdes.a
#endif
