# $Id: Makefile,v 1.17 1998-08-18 17:30:39 ghudson Exp $

SHELL=/bin/sh
ETCDIR=/etc
ATHRETCDIR=/etc/athena
ATHLIBDIR=/usr/athena/lib
ATHMANDIR=/usr/athena/man
CONFIG=/afs/athena.mit.edu/system/config

all:
	cd platform/${HOSTTYPE} && ${MAKE} $@
	cd arch/${ATHENA_SYS} && ${MAKE} $@

check:
	cd platform/${HOSTTYPE} && ${MAKE} $@
	cd arch/${ATHENA_SYS} && ${MAKE} $@

install:
	-mkdir -p ${DESTDIR}${ETCDIR}
	-mkdir -p ${DESTDIR}${ATHRETCDIR}
	-mkdir -p ${DESTDIR}${ATHLIBDIR}
	-mkdir -p ${DESTDIR}${ATHMANDIR}
	install -c -m 0644 Dash.fallback ${DESTDIR}${ATHLIBDIR}
	install -c -m 0644 ftpusers ${DESTDIR}${ETCDIR}
	install -c -m 0644 krb.conf ${DESTDIR}${ATHRETCDIR}
	install -c -m 0644 krb.realms ${DESTDIR}${ATHRETCDIR}
	install -c -m 0644 krb5.conf ${DESTDIR}${ETCDIR}
	install -c -m 0444 mandesc ${DESTDIR}${ATHMANDIR}
	install -c -m 0444 masks ${DESTDIR}${ATHRETCDIR}
	install -c -m 0644 named.conf ${DESTDIR}${ETCDIR}
	install -c -m 0644 named.localhost ${DESTDIR}${ETCDIR}
	install -c -m 0644 named.localhost.rev ${DESTDIR}${ETCDIR}
	install -c -m 0644 named.root ${DESTDIR}${ETCDIR}
	install -c -m 0644 ntp.conf ${DESTDIR}${ETCDIR}
	install -c -m 0644 resolv.conf ${DESTDIR}${ETCDIR}
	install -c -m 0644 shells ${DESTDIR}${ETCDIR}
	install -c -m 0644 sshd_config ${DESTDIR}${ETCDIR}
	install -c -m 0644 zephyr.vars ${DESTDIR}${ATHRETCDIR}
	rm -f ${DESTDIR}${ETCDIR}/krb.conf
	ln -s athena/krb.conf ${DESTDIR}${ETCDIR}/krb.conf
	rm -f ${DESTDIR}${ETCDIR}/krb.realms
	ln -s athena/krb.realms ${DESTDIR}${ETCDIR}/krb.realms
	rm -f ${DESTDIR}${ETCDIR}/passwd
	ln -s ${CONFIG}/passwd/${ATHENA_SYS}/passwd ${DESTDIR}${ETCDIR}/passwd
	rm -f ${DESTDIR}${ATHRETCDIR}/gdss_public_key
	ln -s /afs/net.mit.edu/system/config/gdss_public_key \
		${DESTDIR}${ATHRETCDIR}/gdss_public_key
	rm -f ${DESTDIR}${ATHLIBDIR}/quickstations
	ln -s ${CONFIG}/quick/quickstations \
		${DESTDIR}${ATHLIBDIR}/quickstations
	rm -f ${DESTDIR}${ATHLIBDIR}/Dash.menus
	. ../build/version; \
		vers=$$major.$$minor; \
		ln -s ${CONFIG}/dash/Dash.menus-$$vers \
			${DESTDIR}${ATHLIBDIR}/Dash.menus;
	cd platform/${HOSTTYPE} && ${MAKE} $@
	cd arch/${ATHENA_SYS} && ${MAKE} $@

clean:
	cd platform/${HOSTTYPE} && ${MAKE} $@
	cd arch/${ATHENA_SYS} && ${MAKE} $@

distclean:
	cd platform/${HOSTTYPE} && ${MAKE} $@
	cd arch/${ATHENA_SYS} && ${MAKE} $@
