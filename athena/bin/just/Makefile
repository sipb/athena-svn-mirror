#
# 	$Source: /afs/dev.mit.edu/source/repository/athena/bin/just/Makefile,v $
#	$Author: treese $
#	$Locker:  $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/just/Makefile,v 1.2 1986-11-23 00:07:42 treese Exp $
#


DESTDIR=
CONFDIR= /usr/athena
INCDIR= /usr/include
CFLAGS = -O -I${INCDIR}

all: just fill

fill: just

just: just.o
	cc $(CFLAGS) -o just just.o

install: all
	install -c -s just ${DESTDIR}${CONFDIR}/just
	rm -f ${DESTDIR}${CONFDIR}/fill
	ln ${DESTDIR}${CONFDIR}/just ${DESTDIR}${CONFDIR}/fill

clean:
	rm -f just *.o core a.out *.bak *~
