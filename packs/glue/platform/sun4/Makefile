# $Id: Makefile,v 1.5 1997-03-03 21:51:27 ghudson Exp $

SHELL=/bin/sh
ATHBINDIR=/usr/athena/bin
ATHMANDIR=/usr/athena/man
LIBX11=/usr/openwin/lib/X11
USRLIBDIR=/usr/lib
SUNSOFT=/afs/athena.mit.edu/software/sunsoft

all:

check:

install:
	mkdir -p ${DESTDIR}${ATHBINDIR}
	mkdir -p ${DESTDIR}${ATHMANDIR}/man1
	mkdir -p ${DESTDIR}${LIBX11}
	mkdir -p ${DESTDIR}${USRLIBDIR}
	install -c -m 555 answerbook.sh ${DESTDIR}${ATHBINDIR}/answerbook
	rm -f ${DESTDIR}${ATHBINDIR}/cc
	ln -s `athdir -c -p ${SUNSOFT} -t bin`/cc ${DESTDIR}${ATHBINDIR}/cc
	rm -f ${DESTDIR}${ATHBINDIR}/f77
	ln -s `athdir -c -p ${SUNSOFT} -t bin`/f77 ${DESTDIR}${ATHBINDIR}/f77
	rm -f ${DESTDIR}${ATHBINDIR}/mwm
	ln -s /opt/SUNWmfwm/bin/mwm ${DESTDIR}${ATHBINDIR}/mwm
	rm -f ${DESTDIR}${ATHMANDIR}/man1/mwm.1X
	ln -s /opt/SUNWmfwm/man/man1/mwm.1X ${DESTDIR}${ATHMANDIR}/man1/mwm.1X
	rm -f ${DESTDIR}${LIBX11}/system.mwmrc
	ln -s /opt/SUNWmfwm/lib/system.mwmrc ${DESTDIR}${LIBX11}/system.mwmrc
	rm -f ${DESTDIR}${ATHBINDIR}/otalk
	ln -s /usr/bin/talk ${DESTDIR}${ATHBINDIR}/otalk
	rm -f ${DESTDIR}${USRLIBDIR}/cpp
	ln -s /usr/ccs/lib/cpp ${DESTDIR}${USRLIBDIR}/cpp

clean:
	rm -f cc f77

distclean: clean
