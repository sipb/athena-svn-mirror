# $Id: Makefile,v 1.4 1997-02-27 06:58:44 ghudson Exp $

SHELL=/bin/sh
ATHBINDIR=/usr/athena/bin
ATHMANDIR=/usr/athena/man
LIBX11=/usr/openwin/lib/X11

all: cc f77

cc:
	path=`athdir -c -p /mit/sunsoft -t bin`/cc; \
		sed -e 's|@LOCKER@|sunsoft|g' -e "s|@PATH@|$$path|g" \
		-e 's|@PROGRAM@|cc|g' ../../template.sh > $@

f77:
	path=`athdir -c -p /mit/sunsoft -t bin`/f77; \
		sed -e 's|@LOCKER@|sunsoft|g' -e "s|@PATH@|$$path|g" \
		-e 's|@PROGRAM@|f77|g' ../../template.sh > $@

check:

install:
	mkdir -p ${DESTDIR}${ATHBINDIR}
	mkdir -p ${DESTDIR}${ATHMANDIR}/man1
	mkdir -p ${DESTDIR}${LIBX11}
	install -c -m 555 answerbook.sh ${DESTDIR}${ATHBINDIR}/answerbook
	install -c -m 555 cc ${DESTDIR}${ATHBINDIR}
	install -c -m 555 f77 ${DESTDIR}${ATHBINDIR}
	rm -f ${DESTDIR}${ATHBINDIR}/mwm
	ln -s /opt/SUNWmfwm/bin/mwm ${DESTDIR}${ATHBINDIR}/mwm
	rm -f ${DESTDIR}${ATHMANDIR}/man1/mwm.1X
	ln -s /opt/SUNWmfwm/man/man1/mwm.1X ${DESTDIR}${ATHMANDIR}/man1/mwm.1X
	rm -f ${DESTDIR}${LIBX11}/system.mwmrc
	ln -s /opt/SUNWmfwm/lib/system.mwmrc ${DESTDIR}${LIBX11}/system.mwmrc
	rm -f ${DESTDIR}${ATHBINDIR}/otalk
	ln -s /usr/bin/talk ${DESTDIR}${ATHBINDIR}/otalk

clean:
	rm -f cc f77

distclean: clean
