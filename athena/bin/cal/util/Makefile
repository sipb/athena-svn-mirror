###########################################################################
#
#				Makefile
#
#
#	Makefile for the whatsup utility object files
#
#	Copyright 1987 by the Massachusetts Institute of Technology.
#	For copying and distribution information, see the file
#	"mit-copyright.h".
#
#
#	$Source: /afs/dev.mit.edu/source/repository/athena/bin/cal/util/Makefile,v $
#	$Author: epeisach $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/util/Makefile,v 1.4 1990-02-09 09:25:51 epeisach Exp $
#
#
###########################################################################

INSTDIR=

SRCS=	 what_lib.c
OBJS = what_lib.o

CFLAGS=  ${OCFLAGS} 
LINTFLAGS =${OLINTFLAGS} 

SUBDIR= 
SCRIPT= 
STD=	
NSTD=	

.c.o:
	${CC} -c ${CFLAGS} $*.c

all: ${OBJS}

install: 

clean:
	rm -f a.out core *.s *.o *~ .*~ makedep eddep

lint:
#		util files are linted with the application source

depend:
	touch Make.depend; mkdep -fMake.depend ${CFLAGS} ${SRCS}

