DESTDIR=
INCDIR=/usr/include
CFLAGS=-O -I${INCDIR}

all: rep

rep: rep.c
	cc ${CFLAGS} rep.c -lcurses -ltermlib -o rep
install: rep
	install -c -s rep ${DESTDIR}/usr/athena/rep
	cp rep.1 ${DESTDIR}/usr/man/mann/rep.n
clean:
	rm -f core rep
