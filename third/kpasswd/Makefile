# $Id: Makefile,v 1.1 1998-05-17 03:23:46 ghudson Exp $

SHELL=/bin/sh

OBJS=kadm_cli_wrap.o kadm_net.o kadm_stream.o kpasswd.o
ALL_CFLAGS=-I/usr/athena/include -I/usr/athena/include/kerberosIV -I. ${CFLAGS}
CFLAGS=-g
LDFLAGS=-L/usr/athena/lib
LIBS=-lkrb4 -ldes425 -lkrb5 -lcrypto -lcom_err

all: kpasswd

kpasswd: ${OBJS}
	${CC} ${LDFLAGS} -o $@ ${OBJS} ${LIBS}

kadm_cli_wrap.o kadm_net.o kpasswd.o: kadm_err.h

kadm_err.h: kadm_err.et
	compile_et kadm_err.et

.c.o:
	${CC} -c ${ALL_CFLAGS} $<

check:

install:
	mkdir -p ${DESTDIR}/usr/athena/bin
	mkdir -p ${DESTDIR}/usr/athena/man/man1
	install -c -m 555 kpasswd ${DESTDIR}/usr/athena/bin
	install -c -m 444 kpasswd.1 ${DESTDIR}/usr/athena/man/man1

clean:
	rm -f ${OBJS} kpasswd kadm_err.c kadm_err.h

distclean: clean
