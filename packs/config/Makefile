# $Id: Makefile,v 1.1 1996-09-20 05:00:01 ghudson Exp $

SHELL=/bin/sh
ETCDIR=/etc
ATHRETCDIR=/etc/athena

all:
	cd platform/${HOSTTYPE} && ${MAKE} $@

check:
	cd platform/${HOSTTYPE} && ${MAKE} $@

install:
	-mkdir -p ${DESTDIR}${ETCDIR}
	-mkdir -p ${DESTDIR}${ATHRETCDIR}
	install -c -m 0644 ftpusers ${DESTDIR}${ETCDIR}
	install -c -m 0644 krb.conf ${DESTDIR}${ATHRETCDIR}
	install -c -m 0644 krb.conf.v5 ${DESTDIR}${ATHRETCDIR}
	install -c -m 0644 krb.realms ${DESTDIR}${ATHRETCDIR}
	install -c -m 0644 named.boot ${DESTDIR}${ETCDIR}
	install -c -m 0644 named.mit ${DESTDIR}${ETCDIR}
	install -c -m 0644 named.root ${DESTDIR}${ETCDIR}
	install -c -m 0644 resolv.conf ${DESTDIR}${ETCDIR}
	install -c -m 0644 shells ${DESTDIR}${ETCDIR}
	cd platform/${HOSTTYPE} && ${MAKE} $@
	rm -f ${DESTDIR}${ETCDIR}/passwd
	ln -s /afs/athena.mit.edu/system/config/passwd/${ATHENA_SYS}/passwd \
		${DESTDIR}${ETCDIR}/passwd

clean:
	cd platform/${HOSTTYPE} && ${MAKE} $@

distclean:
	cd platform/${HOSTTYPE} && ${MAKE} $@
