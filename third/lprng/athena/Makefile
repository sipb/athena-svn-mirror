# $Id: Makefile,v 1.3 1999-05-11 18:40:30 mwhitson Exp $

SHELL=/bin/sh

libexecdir=/usr/athena/libexec
datadir=/usr/athena/lib
sysconfdir=/etc/athena

prepare:

clean:

all:

check:

install:
	mkdir -p ${DESTDIR}${libexecdir}/filters
	mkdir -p ${DESTDIR}${datadir}/filters
	mkdir -p ${DESTDIR}${sysconfdir}
	install -c -m 755 get_hesiod_pcap.sh \
		${DESTDIR}${libexecdir}/get_hesiod_pcap
	install -c -m 755 banner.pl \
		${DESTDIR}${libexecdir}/filters/banner.pl
	install -c -m 644 banner.ps \
		${DESTDIR}${datadir}/filters/banner.ps
	install -c -m 644 owl.ps \
		${DESTDIR}${sysconfdir}/print.logo.ps
