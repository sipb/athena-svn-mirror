#
# Makefile for the snmp message builder
#
.SUFFIXES: .c .o
BUILDOBJS=	snmpbld.o bldpdus.o bldlst.o bldlen.o bldprim.o msgbld.o

all: $(BUILDOBJS)
	cp $(BUILDOBJS) ../obj

install: $(BUILDOBJS)
	cp $(BUILDOBJS) ../obj

clean:
	-/bin/rm -f  $(BUILDOBJS)

.c.o:
	cc $(CFLAGS) -c $*.c

snmpbld.o: snmpbld.c

bldlsts.o: bldlsts.c

bldlen.o: bldlen.c

bldstr.o: bldstr.c

bldint.o: bldint.c
