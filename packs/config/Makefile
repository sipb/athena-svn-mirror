# $Id: Makefile,v 1.3 1996-09-25 17:47:16 ghudson Exp $

SHELL=/bin/sh
ETCDIR=/etc
ATHRETCDIR=/etc/athena
ATHLIBDIR=/usr/athena/lib
CONFIG=/afs/athena.mit.edu/system/config

all:
	cd platform/${HOSTTYPE} && ${MAKE} $@

check:
	cd platform/${HOSTTYPE} && ${MAKE} $@

install:
	-mkdir -p ${DESTDIR}${ETCDIR}
	-mkdir -p ${DESTDIR}${ATHRETCDIR}
	-mkdir -p ${DESTDIR}${ATHLIBDIR}
	install -c -m 0644 Dash.fallback ${DESTDIR}${ATHLIBDIR}
	install -c -m 0644 ftpusers ${DESTDIR}${ETCDIR}
	install -c -m 0644 krb.conf ${DESTDIR}${ATHRETCDIR}
	install -c -m 0644 krb.conf.v5 ${DESTDIR}${ATHRETCDIR}
	install -c -m 0644 krb.realms ${DESTDIR}${ATHRETCDIR}
	install -c -m 0644 named.boot ${DESTDIR}${ETCDIR}
	install -c -m 0644 named.localhost ${DESTDIR}${ETCDIR}
	install -c -m 0644 named.localhost.rev ${DESTDIR}${ETCDIR}
	install -c -m 0644 named.root ${DESTDIR}${ETCDIR}
	install -c -m 0644 resolv.conf ${DESTDIR}${ETCDIR}
	install -c -m 0644 shells ${DESTDIR}${ETCDIR}
	rm -f ${DESTDIR}${ETCDIR}/passwd
	ln -s ${CONFIG}/passwd/${ATHENA_SYS}/passwd ${DESTDIR}${ETCDIR}/passwd
	rm -f ${DESTDIR}${ATHLIBDIR}/Dash.menus
	rm -f ${DESTDIR}${ATHLIBDIR}/Dash.menus.cmp
	. ../build/version; \
		vers=$$major.$$minor; \
		ln -s ${CONFIG}/dash/Dash.menus-$$vers \
			${DESTDIR}${ATHLIBDIR}/Dash.menus; \
		ln -s ${CONFIG}/dash/Dash.menus.cmp-$$vers \
			${DESTDIR}${ATHLIBDIR}/Dash.menus.cmp
	cd platform/${HOSTTYPE} && ${MAKE} $@

clean:
	cd platform/${HOSTTYPE} && ${MAKE} $@

distclean:
	cd platform/${HOSTTYPE} && ${MAKE} $@
