#
# Makefile for the asn parser in the NYSERNet snmp distribution
#
.SUFFIXES: .c .o
PARSEOBJS=	snmpparse.o prsepdus.o prselen.o prselst.o prseprim.o msgparse.o

all: $(PARSEOBJS)
	cp $(PARSEOBJS) ../obj

install: $(PARSEOBJS)
	cp $(PARSEOBJS) ../obj

clean:
	-/bin/rm -f  $(PARSEOBJS)

.c.o:
	cc $(CFLAGS) -c $*.c

snmpparse.o: snmpparse.c

protmsgs.o: protmsgs.c

prselen.o: prselen.c

prselst.o: prselst.c

prseint.o: prseint.c

prsestr.o: prsestr.c
