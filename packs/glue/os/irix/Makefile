# $Id: Makefile,v 1.2 1998-12-26 15:51:45 ghudson Exp $

SHELL=/bin/sh
ATHBINDIR=/usr/athena/bin
USRBINDIR=/usr/bin
NSBINDIR=/usr/bin/X11
IAFLIBDIR=/usr/lib/iaf
ATTACHRUN=netscape
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
	mkdir -p ${DESTDIR}${NSBINDIR}
	mkdir -p ${DESTDIR}${IAFLIBDIR}
	install -c -m 555 lp.sh ${DESTDIR}${ATHBINDIR}/lp
	install -c -m 555 netscape ${DESTDIR}${NSBINDIR}
	rm -f ${DESTDIR}${USRBINDIR}/lp
	ln -s ${ATHBINDIR}/lp ${DESTDIR}${USRBINDIR}/lp
	rm -f ${DESTDIR}${ATHBINDIR}/perl
	ln -s /usr/freeware/bin/perl ${DESTDIR}${ATHBINDIR}/perl
	rm -f ${DESTDIR}${ATHBINDIR}/xlock.real
	ln -s /usr/bin/X11/xlock ${DESTDIR}${ATHBINDIR}/xlock.real
	rm -f ${DESTDIR}${ATHBINDIR}/vacation.real
	ln -s /usr/sbin/vacation ${DESTDIR}${ATHBINDIR}/vacation.real
	rm -f ${DESTDIR}${IAFLIBDIR}/scheme
	ln -s /bin/login ${DESTDIR}${IAFLIBDIR}/scheme

clean:
	rm -f ${ATTACHRUN}

distclean: clean
