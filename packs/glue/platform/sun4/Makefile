# $Id: Makefile,v 1.1 1996-09-25 17:51:36 ghudson Exp $

SHELL=/bin/sh
ATHBINDIR=/usr/athena/bin

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
	install -c -m 555 answerbook.sh ${DESTDIR}${ATHBINDIR}/answerbook
	install -c -m 555 cc ${DESTDIR}${ATHBINDIR}
	install -c -m 555 f77 ${DESTDIR}${ATHBINDIR}

clean:
	rm -f cc f77

distclean: clean
