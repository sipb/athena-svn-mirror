#
#	Makefile for quota
#
DESTDIR=
CFLAGS=	-O -DUW

PROGRAM=	quota
SRCS=		quota.c
LIBS=		-lrpcsvc

all:	${PROGRAM}

${PROGRAM}:	${SRCS}
	${CC} ${CFLAGS} -o $@ $@.c ${LIBS}

install:
	install -o root -m 4755 -s ${PROGRAM} ${DESTDIR}/usr/ucb/${PROGRAM}

tags:
	ctags -tdw *.c 

clean:
	rm -f ${PROGRAM} a.out core *.s *.o made


depend: 
	touch Make.depend; mkdep -fMake.depend -p ${INCPATH} ${CFLAGS} ${SRCS}

