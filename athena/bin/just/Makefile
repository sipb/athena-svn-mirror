#
# 	$Source: /afs/dev.mit.edu/source/repository/athena/bin/just/Makefile,v $
#	$Author: builder $
#	$Locker:  $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/just/Makefile,v 1.1 1985-04-23 20:40:50 builder Exp $
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
	ln ${DESTDIR}${CONFDIR}/just ${DESTDIR}${CONFDIR}/fill

clean:
	rm -f just *.o core a.out *.bak *~
