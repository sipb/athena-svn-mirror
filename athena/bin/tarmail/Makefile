#
# 	$Source: /afs/dev.mit.edu/source/repository/athena/bin/tarmail/Makefile,v $
#	$Author: shanzer $
#	$Locker:  $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/tarmail/Makefile,v 1.8 1988-01-21 18:06:30 shanzer Exp $
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
	ln ${DESTDIR}/usr/man/man1/btoa.1 ${DESTDIR}/usr/man/man1/atob.1
	ln ${DESTDIR}/usr/man/man1/btoa.1 ${DESTDIR}/usr/man/man1/untarmail.1

btoa:
	cc ${CFLAGS} btoa.c -o btoa

atob:
	cc ${CFLAGS} atob.c -o atob

clean:		
	rm -f atob btoa *.o

depend:
	cat </dev/null >x.c
	for i in btoa atob; do \
		(echo $$i: $$i.c >>makedep; \
		/bin/grep '^#[ 	]*include' x.c $$i.c | sed \
			-e 's,<\(.*\)>,"${INCDIR}/\1",' \
			-e 's/:[^"]*"\([^"]*\)".*/: \1/' \
			-e 's/\.c//' >>makedep); done
	echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile Makefile.bak
	ed - Makefile < eddep
	rm eddep makedep x.c
	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
	echo '# see make depend above' >> Makefile

# DO NOT DELETE THIS LINE -- make depend uses it

btoa: btoa.c
btoa: /usr/include/stdio.h
atob: atob.c
atob: /usr/include/stdio.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
