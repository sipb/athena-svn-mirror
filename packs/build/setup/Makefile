# $Id: Makefile,v 1.4 1997-04-01 19:06:15 ghudson Exp $

SHELL=/bin/sh
ATHBINDIR=/usr/athena/bin
ATHETCDIR=/usr/athena/etc
ATHRBINDIR=/bin/athena
ATHRETCDIR=/etc/athena
ATHVARDIR=/var/athena
ATHCONFDIR=/usr/athena/config
ATHMANDIR=/usr/athena/man
ATHLIBDIR=/usr/athena/lib
ATHINCDIR=/usr/athena/include
ATHXAPPDIR=/usr/athena/lib/X11/app-defaults
ATHXBITDIR=/usr/athena/include/X11/bitmaps

all:
	cd platform/${HOSTTYPE} && ${MAKE} $@

check:
	cd platform/${HOSTTYPE} && ${MAKE} $@

install:
	cd platform/${HOSTTYPE} && ${MAKE} $@
	-mkdir -p ${DESTDIR}${ATHBINDIR}
	-mkdir -p ${DESTDIR}${ATHETCDIR}
	-mkdir -p ${DESTDIR}${ATHRBINDIR}
	-mkdir -p ${DESTDIR}${ATHRETCDIR}
	-mkdir -p ${DESTDIR}${ATHVARDIR}
	-mkdir -p ${DESTDIR}${ATHCONFDIR}
	-mkdir -p ${DESTDIR}${ATHLIBDIR}
	-mkdir -p ${DESTDIR}${ATHINCDIR}
	-mkdir -p ${DESTDIR}${ATHXAPPDIR}
	-mkdir -p ${DESTDIR}${ATHXBITDIR}
	-mkdir -p ${DESTDIR}${ATHMANDIR}
	-mkdir -p ${DESTDIR}${ATHMANDIR}/man1
	-mkdir -p ${DESTDIR}${ATHMANDIR}/man2
	-mkdir -p ${DESTDIR}${ATHMANDIR}/man3
	-mkdir -p ${DESTDIR}${ATHMANDIR}/man4
	-mkdir -p ${DESTDIR}${ATHMANDIR}/man5
	-mkdir -p ${DESTDIR}${ATHMANDIR}/man6
	-mkdir -p ${DESTDIR}${ATHMANDIR}/man7
	-mkdir -p ${DESTDIR}${ATHMANDIR}/man8
	-mkdir -p ${DESTDIR}${ATHMANDIR}/cat1
	-mkdir -p ${DESTDIR}${ATHMANDIR}/cat2
	-mkdir -p ${DESTDIR}${ATHMANDIR}/cat3
	-mkdir -p ${DESTDIR}${ATHMANDIR}/cat4
	-mkdir -p ${DESTDIR}${ATHMANDIR}/cat5
	-mkdir -p ${DESTDIR}${ATHMANDIR}/cat6
	-mkdir -p ${DESTDIR}${ATHMANDIR}/cat7
	-mkdir -p ${DESTDIR}${ATHMANDIR}/cat8
	rm -f ${DESTDIR}/usr/athena/emacs
	ln -s bin/emacs ${DESTDIR}/usr/athena/emacs
	rm -f ${DESTDIR}/usr/athena/zmailnotify
	ln -s bin/zmailnotify ${DESTDIR}/usr/athena/zmailnotify

clean:
	cd platform/${HOSTTYPE} && ${MAKE} $@

distclean:
	cd platform/${HOSTTYPE} && ${MAKE} $@
