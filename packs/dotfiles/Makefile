# $Id: Makefile,v 1.11 2002-08-23 19:40:19 mwhitson Exp $

SHELL=/bin/sh
PROTOTYPE=/usr/prototype_user
PROTOTMP=/usr/athena/lib/prototype_tmpuser
SYSTEM=/usr/athena/lib/init
ATHMANDIR=/usr/athena/man

all:
	cd os/${OS} && ${MAKE} $@

check:
	cd os/${OS} && ${MAKE} $@

install:
	mkdir -p ${DESTDIR}${PROTOTYPE}
	mkdir -p ${DESTDIR}${PROTOTMP}
	mkdir -p ${DESTDIR}${SYSTEM}
	mkdir -p ${DESTDIR}${ATHMANDIR}/man1
	mkdir -p ${DESTDIR}${ATHMANDIR}/man7
	install -c -m 0644 README.mail ${DESTDIR}${PROTOTYPE}
	install -c -m 0644 README.mail ${DESTDIR}${PROTOTMP}
	install -c -m 0644 cshrc ${DESTDIR}${SYSTEM}
	install -c -m 0644 dot.cshrc ${DESTDIR}${PROTOTYPE}/.cshrc
	install -c -m 0644 dot.cshrc ${DESTDIR}${PROTOTMP}/.cshrc
	install -c -m 0644 dot.login ${DESTDIR}${PROTOTYPE}/.login
	install -c -m 0644 dot.login ${DESTDIR}${PROTOTMP}/.login
	install -c -m 0644 dot.logout ${DESTDIR}${PROTOTYPE}/.logout
	install -c -m 0644 dot.mh_profile ${DESTDIR}${PROTOTYPE}/.mh_profile
	install -c -m 0644 dot.bash_login ${DESTDIR}${PROTOTYPE}/.bash_login
	install -c -m 0644 dot.bash_login ${DESTDIR}${PROTOTMP}/.bash_login
	install -c -m 0644 dot.bashrc ${DESTDIR}${PROTOTYPE}/.bashrc
	install -c -m 0644 dot.bashrc ${DESTDIR}${PROTOTMP}/.bashrc
	install -c -m 0644 env_remove ${DESTDIR}${SYSTEM}
	install -c -m 0644 env_setup ${DESTDIR}${SYSTEM}
	install -c -m 0444 lockers.7 ${DESTDIR}${ATHMANDIR}/man7
	install -c -m 0644 login ${DESTDIR}${SYSTEM}
	install -c -m 0644 bashrc ${DESTDIR}${SYSTEM}
	install -c -m 0644 bash_login ${DESTDIR}${SYSTEM}
	install -c -m 0444 renew.1 ${DESTDIR}${ATHMANDIR}/man1
	install -c -m 0644 temp.README ${DESTDIR}${PROTOTMP}/README
	install -c -m 0644 temp.mh_profile ${DESTDIR}${PROTOTMP}/.mh_profile
	install -c -m 0644 welcome ${DESTDIR}${PROTOTYPE}
	install -c -m 0644 welcome ${DESTDIR}${PROTOTMP}
	cd os/${OS} && ${MAKE} $@

clean:
	cd os/${OS} && ${MAKE} $@

distclean:
	cd os/${OS} && ${MAKE} $@
