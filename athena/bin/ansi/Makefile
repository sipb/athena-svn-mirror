#
# 	$Source: /afs/dev.mit.edu/source/repository/athena/bin/ansi/Makefile,v $
#	$Author: epeisach $
#	$Locker:  $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/ansi/Makefile,v 1.3 1990-02-09 08:50:11 epeisach Exp $
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

depend:
	touch Make.depend; makedepend -fMake.depend ${CFLAGS} ansi.c unseg.c

# DO NOT DELETE THIS LINE -- make depend depends on it.

ansi.o: ansi.c /usr/include/stdio.h
unseg.o: unseg.c /usr/include/stdio.h
