SHELL=/bin/sh
libdir=/usr/athena/lib
includedir=/usr/athena/include

LIBOBJS=	athdir.o stringlist.o
CFLAGS=		-DATHSYS=\"${ATHENA_SYS}\"

all:		libathdir.a

libathdir.a:	${LIBOBJS}
	ar -cr libathdir.a ${LIBOBJS}

clean:
	rm -f ${LIBOBJS} libathdir.a

distclean:
	rm -f ${LIBOBJS} libathdir.a

install:
	install -m 644 athdir.h ${DESTDIR}${includedir}
	install -m 644 libathdir.a ${DESTDIR}${libdir}

check:
