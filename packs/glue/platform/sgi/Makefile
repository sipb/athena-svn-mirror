# $Id: Makefile,v 1.7 1998-03-01 22:54:33 rbasch Exp $

SHELL=/bin/sh
ATHBINDIR=/usr/athena/bin
NSBINDIR=/usr/bin/X11
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
	-mkdir -p ${DESTDIR}${ATHBINDIR}
	-mkdir -p ${DESTDIR}${NSBINDIR}
	install -c -m 555 lp.sh ${DESTDIR}${ATHBINDIR}/lp
	install -c -m 555 netscape ${DESTDIR}${NSBINDIR}
	rm -f ${DESTDIR}${ATHBINDIR}/perl
	ln -s /usr/sbin/perl ${DESTDIR}${ATHBINDIR}/perl

clean:
	rm -f ${ATTACHRUN}

distclean: clean
