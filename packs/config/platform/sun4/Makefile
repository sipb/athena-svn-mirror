# $Id: Makefile,v 1.4 1997-01-27 13:41:47 ghudson Exp $

ETCDIR=/etc
ATHRETCDIR=/etc/athena
KEYTABLES=/usr/openwin/share/etc/keytables
KEYMAPS=/usr/athena/lib/X11/keymaps
APPDEFS=/usr/athena/lib/X11/app-defaults
DEFAULT=/etc/default
INET=/etc/inet
MAIL=/etc/mail
CRONTABS=/var/spool/cron/crontabs

all:

check:

install:
	mkdir -p ${DESTDIR}${ETCDIR}
	mkdir -p ${DESTDIR}${ATHRETCDIR}
	mkdir -p ${DESTDIR}${KEYTABLES}
	mkdir -p ${DESTDIR}${KEYMAPS}
	mkdir -p ${DESTDIR}${APPDEFS}
	mkdir -p ${DESTDIR}${DEFAULT}
	mkdir -p ${DESTDIR}${INET}
	mkdir -p ${DESTDIR}${MAIL}
	mkdir -p ${DESTDIR}${CRONTABS}
	install -c -m 0644 Athena4.kt ${DESTDIR}${KEYTABLES}
	install -c -m 0644 Athena5.kt ${DESTDIR}${KEYTABLES}
	install -c -m 0644 AthenaModifiers ${DESTDIR}${KEYMAPS}
	install -c -m 0644 AthenaType5Keypad ${DESTDIR}${KEYMAPS}
	install -c -m 0644 AthenaType5Rubout ${DESTDIR}${KEYMAPS}
	install -c -m 0644 Mwm ${DESTDIR}${APPDEFS}
	install -c -m 0644 SunModifiers ${DESTDIR}${KEYMAPS}
	install -c -m 0644 SunType5Keypad ${DESTDIR}${KEYMAPS}
	install -c -m 0644 SunType5Rubout ${DESTDIR}${KEYMAPS}
	install -c -m 0644 attach.conf ${DESTDIR}${ATHRETCDIR}
	install -c -m 0644 cron ${DESTDIR}${DEFAULT}
	install -c -m 0644 crontab.root ${DESTDIR}${CRONTABS}/root
	install -c -m 0644 devlink.tab ${DESTDIR}${ETCDIR}
	install -c -m 0644 group ${DESTDIR}${ETCDIR}
	install -c -m 0644 inetd.conf ${DESTDIR}${INET}
	install -c -m 0644 inetd.conf.athena ${DESTDIR}${ATHRETCDIR}/inetd.conf
	install -c -m 0644 init ${DESTDIR}${DEFAULT}
	install -c -m 0644 inittab ${DESTDIR}${ETCDIR}
	install -c -m 0644 keytable.map ${DESTDIR}${KEYTABLES}
	install -c -m 0644 logindevperm ${DESTDIR}${ETCDIR}
	install -c -m 0644 minor_perm ${DESTDIR}${ETCDIR}
	install -c -m 0644 name_to_sysnum ${DESTDIR}${ETCDIR}
	install -c -m 0644 netmasks ${DESTDIR}${INET}
	install -c -m 0644 netspy ${DESTDIR}${ETCDIR}
	install -c -m 0644 newsyslog.conf ${DESTDIR}${ATHRETCDIR}
	install -c -m 0644 nsswitch.conf ${DESTDIR}${ETCDIR}
	install -c -m 0644 passwd.fallback ${DESTDIR}${ETCDIR}
	install -c -m 0644 rc.conf ${DESTDIR}${ATHRETCDIR}
	install -c -m 0644 sendmail.cf ${DESTDIR}${MAIL}
	install -c -m 0644 services ${DESTDIR}${INET}
	install -c -m 0644 shadow.fallback ${DESTDIR}${ETCDIR}
	install -c -m 0644 su ${DESTDIR}${DEFAULT}
	install -c -m 0644 syslog.conf ${DESTDIR}${ETCDIR}
	install -c -m 0644 system ${DESTDIR}${ETCDIR}
	install -c -m 0644 vfstab.std ${DESTDIR}${ETCDIR}
	rm -f ${DESTDIR}${ETCDIR}/shadow
	ln -s /afs/athena.mit.edu/system/config/passwd/${ATHENA_SYS}/shadow \
		${DESTDIR}${ETCDIR}/shadow

clean:

distclean:
