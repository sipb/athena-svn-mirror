# $Id: Makefile,v 1.2 1996-10-11 17:25:18 ghudson Exp $

SHELL=/bin/sh
ATHBINDIR=/usr/athena/bin
ATHMANDIR=/usr/athena/man

all: whois

whois: whois.o
	${CC} ${LDFLAGS} -o whois whois.o ${LIBS}

check:

install:
	-mkdir -p ${DESTDIR}${ATHBINDIR}
	-mkdir -p ${DESTDIR}${ATHMANDIR}/man1
	install -c -m 555 whois ${DESTDIR}${ATHBINDIR}
	install -c -m 444 whois.1 ${DESTDIR}${ATHMANDIR}/man1

clean:
	rm -f whois.o whois

distclean: clean
