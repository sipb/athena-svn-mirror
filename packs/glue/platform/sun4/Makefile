# $Id: Makefile,v 1.8 1997-04-02 22:22:46 ghudson Exp $

SHELL=/bin/sh
ATHBINDIR=/usr/athena/bin
USRLIBDIR=/usr/lib

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
	mkdir -p ${DESTDIR}${USRLIBDIR}
	install -c -m 555 answerbook.sh ${DESTDIR}${ATHBINDIR}/answerbook
	install -c -m 555 cc ${DESTDIR}${ATHBINDIR}
	install -c -m 555 f77 ${DESTDIR}${ATHBINDIR}
	rm -f ${DESTDIR}${ATHBINDIR}/otalk
	ln -s /usr/bin/talk ${DESTDIR}${ATHBINDIR}/otalk
	rm -f ${DESTDIR}${USRLIBDIR}/cpp
	ln -s /usr/ccs/lib/cpp ${DESTDIR}${USRLIBDIR}/cpp

clean:
	rm -f cc f77

distclean: clean
