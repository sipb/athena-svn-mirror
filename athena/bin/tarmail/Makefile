#
# 	$Source: /afs/dev.mit.edu/source/repository/athena/bin/tarmail/Makefile,v $
#	$Author: epeisach $
#	$Locker:  $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/tarmail/Makefile,v 1.11 1990-02-09 08:58:42 epeisach Exp $
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
	touch Make.depend; mkdep -fMake.depend -p ${CFLAGS} btoa.c atob.c

# DO NOT DELETE THIS LINE -- touch Make.depend; mkdep -fMake.depend uses it.

btoa: btoa.c /usr/include/stdio.h
atob: atob.c /usr/include/stdio.h
