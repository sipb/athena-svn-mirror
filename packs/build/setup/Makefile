# $Id: Makefile,v 1.14 2003-05-03 15:21:34 ghudson Exp $

SHELL=/bin/sh
ATHVARDIR=/var/athena

all:
	cd os/${OS} && ${MAKE} $@

check:
	cd os/${OS} && ${MAKE} $@

install:
	cd os/${OS} && ${MAKE} $@
	mkdir -p -m 700 ${DESTDIR}${ATHVARDIR}/sessions
	mkdir -p ${DESTDIR}${ATHVARDIR}/daemon
	chown daemon ${DESTDIR}${ATHVARDIR}/daemon
	mkdir -p ${DESTDIR}${ATHVARDIR}/attachtab
	mkdir -p ${DESTDIR}${ATHVARDIR}/attachtab/locker
	mkdir -p ${DESTDIR}${ATHVARDIR}/attachtab/mountpoint
	mkdir -p ${DESTDIR}${ATHVARDIR}/attachtab/directory

clean:
	cd os/${OS} && ${MAKE} $@

distclean:
	cd os/${OS} && ${MAKE} $@
