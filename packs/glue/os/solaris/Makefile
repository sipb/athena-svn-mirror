# $Id: Makefile,v 1.3 2004-03-14 05:02:40 zacheiss Exp $

SHELL=/bin/sh
ATHBINDIR=/usr/athena/bin
USRBINDIR=/usr/bin
USRLIBDIR=/usr/lib
PRINTDIR=${USRLIBDIR}/print
ATTACHRUN=cc f77 f90 pc CC
ATTRUNTEMPLATE=../../template.sh

all: ${ATTACHRUN}

${ATTACHRUN}: specs ${ATTRUNTEMPLATE}
	set `grep '^$@[ 	]' specs`; locker=$$2; program=$${3-$$1}; \
		bindir=`athdir -c -p /mit/$$2 -t bin`; \
		sed -e "s|@LOCKER@|$$2|g" -e "s|@BINDIR@|$$bindir|g" \
			-e "s|@PROGRAM@|$$program|g" ${ATTRUNTEMPLATE} > $@

check:

install:
	mkdir -p ${DESTDIR}${ATHBINDIR}
	mkdir -p ${DESTDIR}${USRBINDIR}
	mkdir -p ${DESTDIR}${USRLIBDIR}
	mkdir -p ${DESTDIR}${PRINTDIR}
	install -c -m 555 cc ${DESTDIR}${ATHBINDIR}
	install -c -m 555 f77 ${DESTDIR}${ATHBINDIR}
	install -c -m 555 f90 ${DESTDIR}${ATHBINDIR}
	install -c -m 555 pc ${DESTDIR}${ATHBINDIR}
	install -c -m 555 CC ${DESTDIR}${ATHBINDIR}
	install -c -m 555 lp.sh ${DESTDIR}${ATHBINDIR}/lp
	rm -f ${DESTDIR}${USRBINDIR}/lp
	ln -s ${ATHBINDIR}/lp ${DESTDIR}${USRBINDIR}/lp
	rm -f ${DESTDIR}${PRINTDIR}/printd
	ln -s /os/usr/bin/lp ${DESTDIR}${PRINTDIR}/printd
	rm -f ${DESTDIR}${ATHBINDIR}/otalk
	ln -s /usr/bin/talk ${DESTDIR}${ATHBINDIR}/otalk
	rm -f ${DESTDIR}${ATHBINDIR}/xlock.real
	ln -s /usr/openwin/bin/xlock ${DESTDIR}${ATHBINDIR}/xlock.real
	rm -f ${DESTDIR}${ATHBINDIR}/vacation.real
	ln -s /usr/bin/vacation ${DESTDIR}${ATHBINDIR}/vacation.real

clean:
	rm -f ${ATTACHRUN}

distclean: clean
