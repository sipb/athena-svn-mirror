# $Id: Makefile,v 1.2 1999-04-05 16:06:32 mwhitson Exp $

SHELL=/bin/sh

libexecdir=/usr/athena/libexec
libdir=/usr/athena/lib

prepare:

clean:

all:

check:

install:
	mkdir -p ${DESTDIR}${libexecdir}
	install -c -m 755 get_hesiod_pcap.sh \
		${DESTDIR}${libexecdir}/get_hesiod_pcap
	mkdir -p ${DESTDIR}${libdir}/filters
	install -c -m 755 banner.pl \
		${DESTDIR}${libdir}/filters/banner.pl
	install -c -m 644 banner.ps \
		${DESTDIR}${libdir}/filters/banner.ps
	install -c -m 644 owl.ps \
		${DESTDIR}${libdir}/filters/print.logo.ps
