CC=gcc
CFLAGS=$(RPM_OPTS) -Wall -fomit-frame-pointer -pipe -D_GNU_SOURCE
# my compiler doesn't need -fno-strength-reduce
LDFLAGS=-Wl,-warn-common -s

all:		mingetty
		size mingetty

install:	all
		mkdir -p ${DESTDIR}/etc/athena
		install mingetty ${DESTDIR}/etc/athena

mingetty:	mingetty.o

clean:
		rm -f *.o mingetty

