# $Id: Makefile,v 1.1 1996-09-20 05:01:36 ghudson Exp $

SHELL=/bin/sh
PROTOTYPE=/usr/athena/lib/prototype_user
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
	install -c -m 0644 README.mail ${DESTDIR}${PROTOTYPE}
	install -c -m 0644 dot.cshrc ${DESTDIR}${PROTOTYPE}/.cshrc
	install -c -m 0644 dot.login ${DESTDIR}${PROTOTYPE}/.login
	install -c -m 0644 dot.logout ${DESTDIR}${PROTOTYPE}/.logout
	install -c -m 0644 dot.mh_profile ${DESTDIR}${PROTOTYPE}/.mh_profile
	install -c -m 0644 env_remove ${DESTDIR}${SYSTEM}
	install -c -m 0644 env_setup ${DESTDIR}${SYSTEM}
	install -c -m 0644 login ${DESTDIR}${SYSTEM}
	install -c -m 0444 renew.1 ${DESTDIR}${ATHMANDIR}/man1
	install -c -m 0644 temp.README ${DESTDIR}${PROTOTMP}/README
	install -c -m 0644 temp.mh_profile ${DESTDIR}${PROTOTMP}/.mh_profile
	install -c -m 0644 welcome ${DESTDIR}${PROTOTYPE}
	cd platform/${HOSTTYPE} && ${MAKE} $@

clean:
	cd platform/${HOSTTYPE} && ${MAKE} $@

distclean:
	cd platform/${HOSTTYPE} && ${MAKE} $@
