#
# 	$Source: /afs/dev.mit.edu/source/repository/athena/bin/tarmail/Makefile,v $
#	$Author: epeisach $
#	$Locker:  $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/tarmail/Makefile,v 1.10 1989-09-16 12:54:56 epeisach Exp $
#
DESTDIR=
INCDIR=/usr/include
CFLAGS=-O -I${INCDIR}

all:	btoa atob

install:	atob btoa tarmail untarmail
	install -c -s btoa ${DESTDIR}/usr/athena
	install -c -s atob ${DESTDIR}/usr/athena
	install -c tarmail ${DESTDIR}/usr/athena
	install -c untarmail ${DESTDIR}/usr/athena
	install -c btoa.1 ${DESTDIR}/usr/man/man1
	install -c tarmail.1 ${DESTDIR}/usr/man/man1
	install -c atob.1 ${DESTDIR}/usr/man/man1

btoa:
	cc ${CFLAGS} btoa.c -o btoa

atob:
	cc ${CFLAGS} atob.c -o atob

clean:		
	rm -f atob btoa *.o

depend:
	mkdep -p ${CFLAGS} btoa.c atob.c

# DO NOT DELETE THIS LINE -- mkdep uses it.

btoa: btoa.c /usr/include/stdio.h
atob: atob.c /usr/include/stdio.h
