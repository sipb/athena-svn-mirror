# $Id: Makefile,v 1.3 1997-02-25 19:04:22 ghudson Exp $

SHELL=/bin/sh
PROTOTYPE=/usr/prototype_user
PROTOTMP=/usr/athena/lib/prototype_tmpuser
SYSTEM=/usr/athena/lib/init
ATHMANDIR=/usr/athena/man

all:
	cd platform/${HOSTTYPE} && ${MAKE} $@

check:
	cd platform/${HOSTTYPE} && ${MAKE} $@

install:
	-mkdir -p ${DESTDIR}${PROTOTYPE}
	-mkdir -p ${DESTDIR}${PROTOTMP}
	-mkdir -p ${DESTDIR}${SYSTEM}
	-mkdir -p ${DESTDIR}${ATHMANDIR}/man1
	-mkdir -p ${DESTDIR}${ATHMANDIR}/man7
	install -c -m 0644 README.mail ${DESTDIR}${PROTOTYPE}
	install -c -m 0644 README.mail ${DESTDIR}${PROTOTMP}
	install -c -m 0644 add.csh ${DESTDIR}${SYSTEM}/add
	install -c -m 0444 add.1 ${DESTDIR}${ATHMANDIR}/man1
	install -c -m 0644 dot.cshrc ${DESTDIR}${PROTOTYPE}/.cshrc
	install -c -m 0644 dot.cshrc ${DESTDIR}${PROTOTMP}/.cshrc
	install -c -m 0644 dot.login ${DESTDIR}${PROTOTYPE}/.login
	install -c -m 0644 dot.login ${DESTDIR}${PROTOTMP}/.login
	install -c -m 0644 dot.logout ${DESTDIR}${PROTOTYPE}/.logout
	install -c -m 0644 dot.mh_profile ${DESTDIR}${PROTOTYPE}/.mh_profile
	install -c -m 0644 env_remove ${DESTDIR}${SYSTEM}
	install -c -m 0644 env_setup ${DESTDIR}${SYSTEM}
	install -c -m 0444 lockers.7 ${DESTDIR}${ATHMANDIR}/man7
	install -c -m 0644 login ${DESTDIR}${SYSTEM}
	install -c -m 0444 renew.1 ${DESTDIR}${ATHMANDIR}/man1
	install -c -m 0644 temp.README ${DESTDIR}${PROTOTMP}/README
	install -c -m 0644 temp.mh_profile ${DESTDIR}${PROTOTMP}/.mh_profile
	install -c -m 0644 welcome ${DESTDIR}${PROTOTYPE}
	install -c -m 0644 welcome ${DESTDIR}${PROTOTMP}
	cd platform/${HOSTTYPE} && ${MAKE} $@

clean:
	cd platform/${HOSTTYPE} && ${MAKE} $@

distclean:
	cd platform/${HOSTTYPE} && ${MAKE} $@
