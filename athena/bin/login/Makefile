#	Makefile for login 
#	MIT Project Athena
#
#	$Source: /afs/dev.mit.edu/source/repository/athena/bin/login/Makefile,v $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/login/Makefile,v 1.1 1988-09-07 18:21:09 shanzer Exp $
#
#
#

DESTDIR=
CFLAGS=	-O -DRVD -DNAMESERVER -DVFS

all: login
login:
	${CC} ${CFLAGS} -o login login.c -lknet -lkrb -ldes -lhesiod
clean:
	rm -f *.o login core *~
install: login 
	install -c -s -m 4555 login ${DESTDIR}/bin/login 

