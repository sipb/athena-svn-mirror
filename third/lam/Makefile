# $Id: Makefile,v 1.2 1996-10-10 17:17:40 ghudson Exp $

SHELL=/bin/sh
ATHBINDIR=/usr/athena/bin
ATHMANDIR=/usr/athena/man

all: lam

lam: lam.o
	${CC} ${LDFLAGS} -o lam lam.o ${LIBS}

check:

install:
	-mkdir -p ${DESTDIR}${ATHBINDIR}
	-mkdir -p ${DESTDIR}${ATHMANDIR}/man1
	install -c -m 555 lam ${DESTDIR}${ATHBINDIR}
	install -c -m 444 lam.1 ${DESTDIR}${ATHMANDIR}/man1

clean:
	rm -f lam.o lam

distclean: clean
