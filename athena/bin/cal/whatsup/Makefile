###########################################################################
#
#				Makefile
#
#
#	Makefile for the Whatsup Calendar Program
#
#	Copyright 1987 by the Massachusetts Institute of Technology.
#	For copying and distribution information, see the file
#	"mit-copyright.h".
#
#
#	$Source: /afs/dev.mit.edu/source/repository/athena/bin/cal/whatsup/Makefile,v $
#	$Author: epeisach $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/whatsup/Makefile,v 1.5 1990-02-09 09:28:52 epeisach Exp $
#
###########################################################################

INSTDIR= /usr/athena

SRCS=	   whatsup.c what_ifld.c what_itm.c  what_sel.c what_in.c what_sc.c \
	   what_time.c
OTHERSRC=  ../util/what_lib.c
OBJS=	   whatsup.o what_ifld.o what_itm.o  what_sel.o what_in.o what_sc.o \
	   what_time.o
OTHEROBJ=  ../util/what_lib.o
INCLIB= ../include
CFLAGS=  ${OCFLAGS} 
LINTFLAGS= ${OLINTFLAGS}

GDBLIB= -lgdb
##
## Use the following for the RT/PC where -L does not work
##
# GDBLIB= ../../gdb_distribution/lib/libgdb.a

HESLIB= -lhesiod
SCREENLIBS= -lcurses -ltermlib

.c.o:
	-${CC} -c ${CFLAGS} $*.c


all: whatsup

whatsup: ${OBJS} ${OTHEROBJ}
	cc -o whatsup ${CFLAGS} ${OBJS} ${OTHEROBJ} ${GDBLIB} ${HESLIB} ${SCREENLIBS}

#Note:  curses causes a lot of set-but-not-used errors when linting,
#       because getyx always sets both coordinates
lint:
	lint -huv ${LINTFLAGS} ${SRCS} ${OTHERSRC} -lcurses -ltermlib | egrep -v "set but not used|possible pointer alignment problem" 

clean:
	rm -f *~ *.o core makedep eddep whatsup TAGS

install: whatsup
	install -c -s -m 755 whatsup ${DESTDIR}${INSTDIR}/whatsup

TAGS:
	etags ${SRCS} ${OTHERSRC}

depend: 
	touch Make.depend; mkdep -fMake.depend ${CFLAGS} -I${INCLIB} ${SRCS}


