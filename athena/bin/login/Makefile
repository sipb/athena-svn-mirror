#	Makefile for login 
#	MIT Project Athena
#
#	$Source: /afs/dev.mit.edu/source/repository/athena/bin/login/Makefile,v $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/login/Makefile,v 1.2 1988-09-09 17:21:03 shanzer Exp $
#
#
#

DESTDIR=
CFLAGS=	-O -DRVD -DNAMESERVER -DVFS

all: login
login: login.c 
	${CC} ${CFLAGS} -o login login.c -lknet -lkrb -ldes -lhesiod
clean:
	rm -f *.o login core *~
install: login 
	install -c -s -m 4555 login ${DESTDIR}/bin/login 

