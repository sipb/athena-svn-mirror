#
# Makefile for communications functions
#
.SUFFIXES: .c .o
COMMOBJS=	snmpsend.o snmprecv.o termcomm.o

all: $(COMMOBJS)
	cp $(COMMOBJS) ../obj

install: $(COMMOBJS)
	cp $(COMMOBJS) ../obj

clean:
	-/bin/rm -f  $(COMMOBJS)

.c.o:
	cc $(CFLAGS) -c $*.c

snmpsend.o: snmpsend.c

snmprecv.o: snmprecv.c

termcomm.o: termcomm.c
