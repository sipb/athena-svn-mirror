CFLAGS=-Wall -O6 -fomit-frame-pointer -pipe
# my compiler doesn't need -fno-strength-reduce
LDFLAGS=-Wl,-warn-common -s

all:		mingetty
		size mingetty

install:	all
		install -s mingetty /sbin/
		install -m 644 mingetty.8 /usr/man/man8/

mingetty:	mingetty.o

clean:
		rm -f *.o mingetty

