# $Id: Makefile,v 1.29 2001-04-30 20:08:01 rbasch Exp $

SHELL=/bin/sh
ETCDIR=/etc
ATHETCDIR=/usr/athena/etc
ATHRETCDIR=/etc/athena
ATHLIBDIR=/usr/athena/lib
ATHSHAREDIR=/usr/athena/share
ATHMANDIR=/usr/athena/man
CONFIG=/afs/athena.mit.edu/system/config

all:
	cd os/${OS} && ${MAKE} $@

check:
	cd os/${OS} && ${MAKE} $@

install:
	mkdir -p ${DESTDIR}${ETCDIR}
	mkdir -p ${DESTDIR}${ATHETCDIR}
	mkdir -p ${DESTDIR}${ATHRETCDIR}
	mkdir -p ${DESTDIR}${ATHLIBDIR}
	mkdir -p ${DESTDIR}${ATHSHAREDIR}
	mkdir -p ${DESTDIR}${ATHMANDIR}
	install -c -m 0644 Dash.fallback ${DESTDIR}${ATHLIBDIR}
	install -c -m 0644 athinfo.access ${DESTDIR}${ATHRETCDIR}
	install -c -m 0644 ftpusers ${DESTDIR}${ETCDIR}
	install -c -m 0644 ifhp.conf ${DESTDIR}${ETCDIR}
	install -c -m 0644 krb.conf ${DESTDIR}${ATHRETCDIR}
	install -c -m 0644 krb.realms ${DESTDIR}${ATHRETCDIR}
	install -c -m 0644 krb5.conf ${DESTDIR}${ETCDIR}
	install -c -m 0644 lpd.conf ${DESTDIR}${ETCDIR}
	install -c -m 0644 lpd.perms ${DESTDIR}${ETCDIR}
	install -c -m 0644 mailcap ${DESTDIR}${ATHETCDIR}
	install -c -m 0444 mandesc ${DESTDIR}${ATHMANDIR}
	install -c -m 0444 masks ${DESTDIR}${ATHRETCDIR}
	install -c -m 0644 mime.types ${DESTDIR}${ATHETCDIR}
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
	rm -f ${DESTDIR}${ATHSHAREDIR}/quickstations
	ln -s ${CONFIG}/quick/quickstations \
		${DESTDIR}${ATHSHAREDIR}/quickstations
	rm -f ${DESTDIR}${ATHLIBDIR}/Dash.menus
	vers=$$ATHENA_MAJOR_VERSION.$$ATHENA_MINOR_VERSION; \
		ln -s ${CONFIG}/dash/Dash.menus-$$vers \
		 ${DESTDIR}${ATHLIBDIR}/Dash.menus;
	cd os/${OS} && ${MAKE} $@

clean:
	cd os/${OS} && ${MAKE} $@

distclean:
	cd os/${OS} && ${MAKE} $@
