#
# Makefile for misc. functions in snmp library
#
.SUFFIXES: .c .o
UTLOBJS=	auth.o commtbl.o varcvt.o error.o valprint.o oidutil.o

all: $(UTLOBJS)
	cp $(UTLOBJS) ../obj

install: $(UTLOBJS)
	cp $(UTLOBJS) ../obj

clean:
	-/bin/rm -f  $(UTLOBJS)

.c.o:
	cc $(CFLAGS) -c $*.c

auth.o: auth.c

commtbl.o: commtbl.c

varcvt.o: varcvt.c

error.o: error.c
