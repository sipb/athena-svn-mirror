#
# 	$Source: /afs/dev.mit.edu/source/repository/athena/bin/ansi/Makefile,v $
#	$Author: builder $
#	$Locker:  $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/ansi/Makefile,v 1.1 1985-04-12 15:28:59 builder Exp $
#


DESTDIR=
CONFDIR= /usr/athena
INCDIR= /usr/include
CFLAGS = -O -I${INCDIR}

all: ansi unseg

ansi: ansi.o
	cc $(CFLAGS) -o ansi ansi.o

unseg: unseg.o
	cc $(CFLAGS) -o unseg unseg.o

install: all
	install -c -s ansi ${DESTDIR}${CONFDIR}/ansi
	install -c -s unseg ${DESTDIR}${CONFDIR}/unseg

clean:
	rm -f ansi unseg *.o core a.out *.bak *~
