#	Makefile for login 
#	MIT Project Athena
#
#	$Source: /afs/dev.mit.edu/source/repository/athena/bin/login/Makefile,v $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/login/Makefile,v 1.6 1989-11-11 03:47:13 probe Exp $
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
	install -c -s -o root -m 4555 login ${DESTDIR}/bin/login 
depend:
	makedepend -s "# DO NOT REMOVE THIS LINE -- make depend uses it" \
		-o "" ${CFLAGS} login.c

# DO NOT REMOVE THIS LINE -- make depend uses it
