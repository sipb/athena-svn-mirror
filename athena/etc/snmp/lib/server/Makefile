#
# Makefile for communications functions
#
.SUFFIXES: .c .o
SRVOBJS=	snmpservsend.o snmpdecipher.o

all: $(SRVOBJS)
	cp $(SRVOBJS) ../obj

install: $(SRVOBJS)
	cp $(SRVOBJS) ../obj

clean:
	-/bin/rm -f  $(SRVOBJS)

.c.o:
	cc $(CFLAGS) -c $*.c

snmpservsend.o: snmpservsend.c

snmpdecipher.o: snmpdecipher.c
