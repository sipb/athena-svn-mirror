# Makefile for Hesiod client finger
#
#	MIT Project Athena
#
#	$Source: /afs/dev.mit.edu/source/repository/athena/bin/finger/Makefile,v $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/finger/Makefile,v 1.2 1987-08-20 16:11:21 ambar Exp $
#	$Author: ambar $
#	$Log: not supported by cvs2svn $
# Revision 1.1  87/08/20  16:02:51  ambar
# Initial revision
# 
#
DESTDIR=
CFLAGS = -O
CONFDIR = ${DESTDIR}/usr/athena
BINDIR = ${DESTDIR}/bin

LIBS =  -lhesiod

SRCS =	finger.c hespwnamuid.c

OBJECTS = finger.o hespwnamuid.o

all:	finger

finger:	${OBJECTS}
	cc ${CFLAGS} -o finger ${OBJECTS} ${LIBS}

finger.o:	finger.c
	cc -c ${CFLAGS} finger.c

hespwnamuid.o:	hespwnamuid.c
	cc -c ${CFLAGS} hespwnamuid.c

lint:
	lint -I../../include *.c

clean:	
	rm -f *.o *~
	rm -f finger

install:	finger
	install finger ${DESTDIR}/usr/athena/finger
