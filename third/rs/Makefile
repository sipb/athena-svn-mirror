# $Id: Makefile,v 1.2 1996-10-10 17:16:03 ghudson Exp $

SHELL=/bin/sh
ATHBINDIR=/usr/athena/bin
ATHMANDIR=/usr/athena/man

all: rs

rs: rs.o
	${CC} ${LDFLAGS} -o rs rs.o ${LIBS}

check:

install:
	-mkdir -p ${DESTDIR}${ATHBINDIR}
	-mkdir -p ${DESTDIR}${ATHMANDIR}/man1
	install -c -m 555 rs ${DESTDIR}${ATHBINDIR}
	install -c -m 444 rs.1 ${DESTDIR}${ATHMANDIR}/man1

clean:
	rm -f rs.o rs

distclean: clean
