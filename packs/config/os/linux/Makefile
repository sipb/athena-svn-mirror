# $Id: Makefile,v 1.1 1999-02-04 17:25:34 ghudson Exp $

ETCDIR=/etc
ATHRETCDIR=/etc/athena
ATHX11LIB=/usr/athena/lib/X11
APPDEFS=${ATHX11LIB}/app-defaults

all:

check:

install:
	mkdir -p ${DESTDIR}${ETCDIR}
	mkdir -p ${DESTDIR}${ATHRETCDIR}
	mkdir -p ${DESTDIR}${ATHX11LIB}
	mkdir -p ${DESTDIR}${APPDEFS}
	install -c -m 0644 ../../aliases ${DESTDIR}${ETCDIR}
	install -c -m 0644 ../../system.mwmrc ${DESTDIR}${ATHX11LIB}
	install -c -m 0644 Mwm ${DESTDIR}${APPDEFS}
	install -c -m 0644 athinfo.defs ${DESTDIR}${ATHRETCDIR}
	install -c -m 0644 attach.conf ${DESTDIR}${ATHRETCDIR}
	install -c -m 0644 crontab ${DESTDIR}${ETCDIR}
	install -c -m 0644 group ${DESTDIR}${ETCDIR}
	install -c -m 0644 inetd.conf ${DESTDIR}${ETCDIR}
	install -c -m 0644 inetd.conf.athena ${DESTDIR}${ATHRETCDIR}/inetd.conf
	install -c -m 0644 inittab ${DESTDIR}${ETCDIR}
	install -c -m 0644 newsyslog.conf ${DESTDIR}${ATHRETCDIR}
	install -c -m 0644 passwd.fallback ${DESTDIR}${ETCDIR}
	install -c -m 0644 rc.conf ${DESTDIR}${ATHRETCDIR}
	install -c -m 0644 sendmail.cf ${DESTDIR}${ETCDIR}
	install -c -m 0644 services ${DESTDIR}${ETCDIR}
	install -c -m 0644 syslog.conf ${DESTDIR}${ETCDIR}
	rm -f ${DESTDIR}${ETCDIR}/passwd
	ln -s /afs/athena.mit.edu/system/config/passwd/${ATHENA_SYS}/passwd \
		${DESTDIR}${ETCDIR}/passwd

clean:

distclean:
