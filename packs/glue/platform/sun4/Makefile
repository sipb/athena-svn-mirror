# $Id: Makefile,v 1.2 1996-10-04 05:38:25 ghudson Exp $

SHELL=/bin/sh
ATHBINDIR=/usr/athena/bin
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
	mkdir -p ${DESTDIR}${LIBX11}
	install -c -m 555 answerbook.sh ${DESTDIR}${ATHBINDIR}/answerbook
	install -c -m 555 cc ${DESTDIR}${ATHBINDIR}
	install -c -m 555 f77 ${DESTDIR}${ATHBINDIR}
	rm -f ${DESTDIR}${ATHBINDIR}/mwm
	ln -s /opt/SUNWmfwm/bin/mwm ${DESTDIR}${ATHBINDIR}/mwm
	rm -f ${DESTDIR}${LIBX11}/system.mwmrc
	ln -s /opt/SUNWmfwm/lib/system.mwmrc ${DESTDIR}${LIBX11}/system.mwmrc

clean:
	rm -f cc f77

distclean: clean
