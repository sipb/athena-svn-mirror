# $Id: Makefile,v 1.4 2000-06-08 18:49:50 ghudson Exp $

SHELL=/bin/sh
ATHBINDIR=/usr/athena/bin
USRBINDIR=/usr/bin
NSBINDIR=/usr/bin/X11
IAFLIBDIR=/usr/lib/iaf

all:

check:

install:
	mkdir -p ${DESTDIR}${ATHBINDIR}
	mkdir -p ${DESTDIR}${USRBINDIR}
	mkdir -p ${DESTDIR}${NSBINDIR}
	mkdir -p ${DESTDIR}${IAFLIBDIR}
	install -c -m 555 lp.sh ${DESTDIR}${ATHBINDIR}/lp
	rm -f ${DESTDIR}${NSBINDIR}/netscape
	ln -s ${ATHBINDIR}/netscape ${DESTDIR}${NSBINDIR}/netscape
	rm -f ${DESTDIR}${USRBINDIR}/lp
	ln -s ${ATHBINDIR}/lp ${DESTDIR}${USRBINDIR}/lp
	rm -f ${DESTDIR}${ATHBINDIR}/xlock.real
	ln -s /usr/bin/X11/xlock ${DESTDIR}${ATHBINDIR}/xlock.real
	rm -f ${DESTDIR}${ATHBINDIR}/vacation.real
	ln -s /usr/sbin/vacation ${DESTDIR}${ATHBINDIR}/vacation.real
	rm -f ${DESTDIR}${IAFLIBDIR}/scheme
	ln -s /bin/login ${DESTDIR}${IAFLIBDIR}/scheme

clean:

distclean: clean
