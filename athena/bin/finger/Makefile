# Makefile for Hesiod/Zephyr client finger
#
#	MIT Project Athena
#
#	$Source: /afs/dev.mit.edu/source/repository/athena/bin/finger/Makefile,v $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/finger/Makefile,v 1.4 1987-08-21 18:30:58 ambar Exp $
#	$Author: ambar $
#	$Log: not supported by cvs2svn $
# Revision 1.3  87/08/21  18:28:18  ambar
# changed the name of the hesiod file.
# 
# Revision 1.2  87/08/20  16:11:21  ambar
# 
# oops: excess backslash removed.
# 
# Revision 1.1  87/08/20  16:02:51  ambar
# Initial revision
# 
#
DESTDIR=
CFLAGS = -O
CONFDIR = ${DESTDIR}/usr/athena
BINDIR = ${DESTDIR}/bin

LIBS =  -lhesiod -lzephyr -lcom_err -lkrb

SRCS =	finger.c hespwnam.c

OBJECTS = finger.o hespwnam.o

all:	finger

finger:	${OBJECTS}
	cc ${CFLAGS} -o finger ${OBJECTS} ${LIBS}

finger.o:	finger.c
	cc -c ${CFLAGS} finger.c

hespwnam.o:	hespwnam.c
	cc -c ${CFLAGS} hespwnam.c

lint:
	lint -I../../include *.c

clean:	
	rm -f *.o *~
	rm -f finger

install:	finger
	install finger ${DESTDIR}/usr/athena/finger
