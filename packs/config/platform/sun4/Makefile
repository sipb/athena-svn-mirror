# $Id: Makefile,v 1.8 1997-04-30 22:52:18 ghudson Exp $

ETCDIR=/etc
ATHRETCDIR=/etc/athena
ATHX11LIB=/usr/athena/lib/X11
KEYMAPS=${ATHX11LIB}/keymaps
KEYTABLES=/usr/openwin/share/etc/keytables
DEFAULT=/etc/default
INET=/etc/inet
MAIL=/etc/mail
CRONTABS=/var/spool/cron/crontabs
USRLIB=/usr/lib

OPENWINLIBS=libX.so.4 libX11.so.4 libXaw.so.4 libXaw.so.5 libXext.so \
	libXi.so.5 libXinput.so.0 libXmu.so.4 libXol.so.3 libXt.so.4 \
	libXtst.so.1
DTLIBS=libMrm.so.3 libUil.so.3 libXm.so.3

all:

check:

install:
	mkdir -p ${DESTDIR}${ETCDIR}
	mkdir -p ${DESTDIR}${ATHRETCDIR}
	mkdir -p ${DESTDIR}${ATHX11LIB}
	mkdir -p ${DESTDIR}${KEYMAPS}
	mkdir -p ${DESTDIR}${KEYTABLES}
	mkdir -p ${DESTDIR}${DEFAULT}
	mkdir -p ${DESTDIR}${INET}
	mkdir -p ${DESTDIR}${MAIL}
	mkdir -p ${DESTDIR}${CRONTABS}
	mkdir -p ${DESTDIR}${USRLIB}
	install -c -m 0644 ../../aliases ${DESTDIR}${MAIL}
	install -c -m 0644 ../../system.mwmrc ${DESTDIR}${ATHX11LIB}
	install -c -m 0644 Athena4.kt ${DESTDIR}${KEYTABLES}
	install -c -m 0644 Athena5.kt ${DESTDIR}${KEYTABLES}
	install -c -m 0644 AthenaModifiers ${DESTDIR}${KEYMAPS}
	install -c -m 0644 AthenaType5Keypad ${DESTDIR}${KEYMAPS}
	install -c -m 0644 AthenaType5Rubout ${DESTDIR}${KEYMAPS}
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
	for i in ${OPENWINLIBS}; do \
		rm -f "${DESTDIR}${USRLIB}/$$i"; \
		ln -s "../openwin/lib/$$i" "${DESTDIR}${USRLIB}/$$i"; \
	done
	for i in ${DTLIBS}; do \
		rm -f "${DESTDIR}${USRLIB}/$$i"; \
		ln -s "../dt/lib/$$i" "${DESTDIR}${USRLIB}/$$i"; \
	done

clean:

distclean:
