#
# 	$Source: /afs/dev.mit.edu/source/repository/athena/bin/just/Makefile,v $
#	$Author: epeisach $
#	$Locker:  $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/just/Makefile,v 1.4 1990-02-09 08:53:04 epeisach Exp $
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

depend:
	touch Make.depend; makedepend -fMake.depend ${CFLAGS} just.c

# DO NOT DELETE THIS LINE -- make depend depends on it.

just.o: just.c /usr/include/stdio.h
