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
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/whatsup/Makefile,v 1.3 1989-09-16 13:15:23 epeisach Exp $
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
	mkdep ${CFLAGS} -I${INCLIB} ${SRCS}


# DO NOT DELETE THIS LINE -- mkdep uses it.

whatsup.o: whatsup.c ../include/mit-copyright.h /usr/include/curses.h
whatsup.o: /usr/include/stdio.h /usr/include/sgtty.h /usr/include/sys/ioctl.h
whatsup.o: /usr/include/sys/ttychars.h /usr/include/sys/ttydev.h
whatsup.o: /usr/include/signal.h /usr/include/strings.h ../include/whatsup.h
whatsup.o: /usr/include/gdb.h /usr/include/sys/types.h
whatsup.o: /usr/include/sys/time.h /usr/include/sys/time.h
what_ifld.o: what_ifld.c ../include/mit-copyright.h /usr/include/curses.h
what_ifld.o: /usr/include/stdio.h /usr/include/sgtty.h
what_ifld.o: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
what_ifld.o: /usr/include/sys/ttydev.h /usr/include/ctype.h
what_ifld.o: ../include/whatsup.h /usr/include/gdb.h /usr/include/sys/types.h
what_ifld.o: /usr/include/sys/time.h /usr/include/sys/time.h
what_itm.o: what_itm.c ../include/mit-copyright.h /usr/include/curses.h
what_itm.o: /usr/include/stdio.h /usr/include/sgtty.h
what_itm.o: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
what_itm.o: /usr/include/sys/ttydev.h /usr/include/strings.h
what_itm.o: ../include/whatsup.h /usr/include/gdb.h /usr/include/sys/types.h
what_itm.o: /usr/include/sys/time.h /usr/include/sys/time.h
what_sel.o: what_sel.c ../include/mit-copyright.h /usr/include/curses.h
what_sel.o: /usr/include/stdio.h /usr/include/sgtty.h
what_sel.o: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
what_sel.o: /usr/include/sys/ttydev.h /usr/include/strings.h
what_sel.o: ../include/whatsup.h /usr/include/gdb.h /usr/include/sys/types.h
what_sel.o: /usr/include/sys/time.h /usr/include/sys/time.h
what_in.o: what_in.c ../include/mit-copyright.h /usr/include/curses.h
what_in.o: /usr/include/stdio.h /usr/include/sgtty.h /usr/include/sys/ioctl.h
what_in.o: /usr/include/sys/ttychars.h /usr/include/sys/ttydev.h
what_in.o: ../include/whatsup.h /usr/include/gdb.h /usr/include/sys/types.h
what_in.o: /usr/include/sys/time.h /usr/include/sys/time.h
what_sc.o: what_sc.c ../include/mit-copyright.h /usr/include/curses.h
what_sc.o: /usr/include/stdio.h /usr/include/sgtty.h /usr/include/sys/ioctl.h
what_sc.o: /usr/include/sys/ttychars.h /usr/include/sys/ttydev.h
what_sc.o: ../include/whatsup.h /usr/include/gdb.h /usr/include/sys/types.h
what_sc.o: /usr/include/sys/time.h /usr/include/sys/time.h
what_time.o: what_time.c ../include/mit-copyright.h /usr/include/curses.h
what_time.o: /usr/include/stdio.h /usr/include/sgtty.h
what_time.o: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
what_time.o: /usr/include/sys/ttydev.h /usr/include/signal.h
what_time.o: ../include/whatsup.h /usr/include/gdb.h /usr/include/sys/types.h
what_time.o: /usr/include/sys/time.h /usr/include/sys/time.h
