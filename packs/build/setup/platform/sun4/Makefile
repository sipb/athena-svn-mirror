# $Id: Makefile,v 1.4 1996-12-11 22:05:28 ghudson Exp $

ATHINCDIR=/usr/athena/include
ATHLIBDIR=/usr/athena/lib
BSDOBJS=bcmp.o bcopy.o bzero.o flock.o ftime.o getdtblsize.o \
	gethostid.o getpagesize.o getrusage.o getwd.o index.o killpg.o \
	mctl.o nlist.o reboot.o regex.o rindex.o scandir.o \
	setbuffer.o setpriority.o siglist.o signal.o times.o \
	ualarm.o usleep.o wait3.o wait4.o

all: libbsd.a

libbsd.a:
	ar x /usr/ucblib/libucb.a ${BSDOBJS}
	ar cr $@ ${BSDOBJS}

check:

install:
	mkdir -p ${DESTDIR}/usr/bin
	mkdir -p ${DESTDIR}${ATHINCDIR}/rpc
	mkdir -p ${DESTDIR}${ATHLIBDIR}
	install -c -m 444 auth_kerb.h ${DESTDIR}${ATHINCDIR}/rpc
	install -c -m 444 libbsd.a ${DESTDIR}${ATHLIBDIR}
	rm -f ${DESTDIR}/core
	ln -s var/tmp/core.root ${DESTDIR}/core
	rm -f ${DESTDIR}/bin
	ln -s usr/bin ${DESTDIR}/bin

clean:

distclean:
