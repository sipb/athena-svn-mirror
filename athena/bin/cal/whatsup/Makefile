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
#	$Author: shanzer $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/whatsup/Makefile,v 1.1 1987-10-29 12:36:00 shanzer Exp $
#
###########################################################################

INSTDIR= /usr/unsupported

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
	rm -f makedep eddep 
	for i in ${SRCS}; do \
		(${CC} ${CFLAGS} -M $$i  >> makedep); \
	 done
	echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile Makefile.bak
	ed - Makefile < eddep
	rm eddep makedep 
	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
	echo '# see make depend above' >> Makefile

# DO NOT DELETE THIS LINE -- make depend uses it
# DEPENDENCIES MUST END AT END OF FILE
whatsup.o: whatsup.c
whatsup.o: ../include/mit-copyright.h
whatsup.o: /usr/include/curses.h
whatsup.o: /usr/include/stdio.h
whatsup.o: /usr/include/sgtty.h
whatsup.o: /usr/include/sys/ioctl.h
whatsup.o: /usr/include/sys/ttychars.h
whatsup.o: /usr/include/sys/ttydev.h
whatsup.o: /usr/include/signal.h
whatsup.o: /usr/include/stdio.h
whatsup.o: /usr/include/strings.h
whatsup.o: ../include/whatsup.h
whatsup.o: /usr/include/gdb.h
whatsup.o: /usr/include/sys/types.h
whatsup.o: /usr/include/sys/time.h
whatsup.o: /usr/include/time.h
what_ifld.o: what_ifld.c
what_ifld.o: ../include/mit-copyright.h
what_ifld.o: /usr/include/curses.h
what_ifld.o: /usr/include/stdio.h
what_ifld.o: /usr/include/sgtty.h
what_ifld.o: /usr/include/sys/ioctl.h
what_ifld.o: /usr/include/sys/ttychars.h
what_ifld.o: /usr/include/sys/ttydev.h
what_ifld.o: /usr/include/stdio.h
what_ifld.o: /usr/include/ctype.h
what_ifld.o: ../include/whatsup.h
what_ifld.o: /usr/include/gdb.h
what_ifld.o: /usr/include/sys/types.h
what_ifld.o: /usr/include/sys/time.h
what_ifld.o: /usr/include/time.h
what_itm.o: what_itm.c
what_itm.o: ../include/mit-copyright.h
what_itm.o: /usr/include/curses.h
what_itm.o: /usr/include/stdio.h
what_itm.o: /usr/include/sgtty.h
what_itm.o: /usr/include/sys/ioctl.h
what_itm.o: /usr/include/sys/ttychars.h
what_itm.o: /usr/include/sys/ttydev.h
what_itm.o: /usr/include/stdio.h
what_itm.o: /usr/include/strings.h
what_itm.o: ../include/whatsup.h
what_itm.o: /usr/include/gdb.h
what_itm.o: /usr/include/sys/types.h
what_itm.o: /usr/include/sys/time.h
what_itm.o: /usr/include/time.h
what_sel.o: what_sel.c
what_sel.o: ../include/mit-copyright.h
what_sel.o: /usr/include/curses.h
what_sel.o: /usr/include/stdio.h
what_sel.o: /usr/include/sgtty.h
what_sel.o: /usr/include/sys/ioctl.h
what_sel.o: /usr/include/sys/ttychars.h
what_sel.o: /usr/include/sys/ttydev.h
what_sel.o: /usr/include/stdio.h
what_sel.o: /usr/include/strings.h
what_sel.o: ../include/whatsup.h
what_sel.o: /usr/include/gdb.h
what_sel.o: /usr/include/sys/types.h
what_sel.o: /usr/include/sys/time.h
what_sel.o: /usr/include/time.h
what_in.o: what_in.c
what_in.o: ../include/mit-copyright.h
what_in.o: /usr/include/curses.h
what_in.o: /usr/include/stdio.h
what_in.o: /usr/include/sgtty.h
what_in.o: /usr/include/sys/ioctl.h
what_in.o: /usr/include/sys/ttychars.h
what_in.o: /usr/include/sys/ttydev.h
what_in.o: /usr/include/stdio.h
what_in.o: ../include/whatsup.h
what_in.o: /usr/include/gdb.h
what_in.o: /usr/include/sys/types.h
what_in.o: /usr/include/sys/time.h
what_in.o: /usr/include/time.h
what_sc.o: what_sc.c
what_sc.o: ../include/mit-copyright.h
what_sc.o: /usr/include/curses.h
what_sc.o: /usr/include/stdio.h
what_sc.o: /usr/include/sgtty.h
what_sc.o: /usr/include/sys/ioctl.h
what_sc.o: /usr/include/sys/ttychars.h
what_sc.o: /usr/include/sys/ttydev.h
what_sc.o: /usr/include/stdio.h
what_sc.o: ../include/whatsup.h
what_sc.o: /usr/include/gdb.h
what_sc.o: /usr/include/sys/types.h
what_sc.o: /usr/include/sys/time.h
what_sc.o: /usr/include/time.h
what_time.o: what_time.c
what_time.o: ../include/mit-copyright.h
what_time.o: /usr/include/curses.h
what_time.o: /usr/include/stdio.h
what_time.o: /usr/include/sgtty.h
what_time.o: /usr/include/sys/ioctl.h
what_time.o: /usr/include/sys/ttychars.h
what_time.o: /usr/include/sys/ttydev.h
what_time.o: /usr/include/signal.h
what_time.o: /usr/include/stdio.h
what_time.o: ../include/whatsup.h
what_time.o: /usr/include/gdb.h
what_time.o: /usr/include/sys/types.h
what_time.o: /usr/include/sys/time.h
what_time.o: /usr/include/time.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
