# $Id: Makefile,v 1.2 1996-10-02 07:11:20 ghudson Exp $

SHELL=/bin/sh
ATHBINDIR=/usr/athena/bin
ATHETCDIR=/usr/athena/etc
ATHRBINDIR=/bin/athena
ATHRETCDIR=/etc/athena
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
	rm -f ${DESTDIR}/usr/athena/emacs
	ln -s bin/emacs ${DESTDIR}/usr/athena/emacs
	rm -f ${DESTDIR}/usr/athena/zmailnotify
	ln -s bin/zmailnotify ${DESTDIR}/usr/athena/zmailnotify

clean:
	cd platform/${HOSTTYPE} && ${MAKE} $@

distclean:
	cd platform/${HOSTTYPE} && ${MAKE} $@
