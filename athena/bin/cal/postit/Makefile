###########################################################################
#
#				Makefile
#
#
#	Makefile for the Postit and Bulkpost Calendar Update Programs
#
#	Copyright 1987 by the Massachusetts Institute of Technology.
#	For copying and distribution information, see the file
#	"mit-copyright.h".
#
#
#	$Source: /afs/dev.mit.edu/source/repository/athena/bin/cal/postit/Makefile,v $
#	$Author: epeisach $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/postit/Makefile,v 1.4 1990-02-09 08:51:12 epeisach Exp $
#
###########################################################################

INSTDIR= /usr/athena

POSTSRC=   postmaster.c
POSTOBJ=   postmaster.o
BULKSRC=   bulkmaster.c
BULKOBJ=   bulkmaster.o
SHAREDSRCS=	postprint.c postdata.c postparse.c postprompts.c postmisc.c 
SHAREDOBJS=	postprint.o postdata.o postparse.o postprompts.o postmisc.o 
ALLSRCS=   ${POSTSRC} ${BULKSRC} ${SHAREDSRCS}
ALLOBJS=   ${POSTOBJ} ${BULKOBJ} ${SHAREDOBJS}

OTHERSRC=  ../util/what_lib.c
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

.c.o:
	-${CC} -c ${CFLAGS} $*.c


all:	postit bulkpost

postit: ${POSTOBJ} ${SHAREDOBJS} ${OTHEROBJ}
	cc -o postit ${CFLAGS} ${POSTOBJ} ${SHAREDOBJS} ${OTHEROBJ} ${GDBLIB} \
	             ${HESLIB}

bulkpost: ${BULKOBJ} ${SHAREDOBJS} ${OTHEROBJ}
	cc -o bulkpost ${CFLAGS} ${BULKOBJ} ${SHAREDOBJS} ${OTHEROBJ} \
		       ${GDBLIB} ${HESLIB}

lint:
	lint -huv ${LINTFLAGS} ${POSTSRC} ${SHAREDSRCS} ${OTHERSRC} | \
		egrep -v "possible pointer alignment problem"
	lint -huv ${LINTFLAGS} ${BULKSRC} ${SHAREDSRCS} ${OTHERSRC} | \
		egrep -v "possible pointer alignment problem"

clean:
	rm -f *~ *.o core makedep eddep bulkpost postit TAGS

install: postit bulkpost
	install -c -s -m 755 postit ${DESTDIR}${INSTDIR}/postit
	install -c -s -m 755 bulkpost ${DESTDIR}${INSTDIR}/bulkpost

depend: 
	touch Make.depend; mkdep -fMake.depend ${CFLAGS} -I${INCLIB} ${ALLSRCS}

TAGS:
	etags *.c ../util/*.c

# DO NOT DELETE THIS LINE -- touch Make.depend; mkdep -fMake.depend uses it.

postmaster.o: postmaster.c ../include/mit-copyright.h /usr/include/stdio.h
postmaster.o: /usr/include/strings.h /usr/include/ctype.h
postmaster.o: /usr/include/fcntl.h /usr/include/errno.h /usr/include/gdb.h
postmaster.o: /usr/include/sys/types.h /usr/include/sys/time.h
postmaster.o: /usr/include/sys/time.h ../include/postit.h
bulkmaster.o: bulkmaster.c ../include/mit-copyright.h /usr/include/stdio.h
bulkmaster.o: /usr/include/strings.h /usr/include/ctype.h
bulkmaster.o: /usr/include/fcntl.h /usr/include/errno.h
bulkmaster.o: /usr/include/sys/types.h /usr/include/sys/stat.h
bulkmaster.o: /usr/include/gdb.h /usr/include/sys/time.h
bulkmaster.o: /usr/include/sys/time.h ../include/postit.h
postprint.o: postprint.c ../include/mit-copyright.h /usr/include/stdio.h
postprint.o: /usr/include/strings.h /usr/include/ctype.h /usr/include/fcntl.h
postprint.o: /usr/include/errno.h /usr/include/curses.h /usr/include/sgtty.h
postprint.o: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
postprint.o: /usr/include/sys/ttydev.h /usr/include/gdb.h
postprint.o: /usr/include/sys/types.h /usr/include/sys/time.h
postprint.o: /usr/include/sys/time.h ../include/postit.h
postdata.o: postdata.c ../include/mit-copyright.h /usr/include/stdio.h
postdata.o: /usr/include/strings.h /usr/include/ctype.h /usr/include/fcntl.h
postdata.o: /usr/include/errno.h /usr/include/sys/wait.h /usr/include/gdb.h
postdata.o: /usr/include/sys/types.h /usr/include/sys/time.h
postdata.o: /usr/include/sys/time.h ../include/postit.h
postparse.o: postparse.c ../include/mit-copyright.h /usr/include/stdio.h
postparse.o: /usr/include/strings.h /usr/include/ctype.h /usr/include/fcntl.h
postparse.o: /usr/include/errno.h /usr/include/gdb.h /usr/include/sys/types.h
postparse.o: /usr/include/sys/time.h /usr/include/sys/time.h
postparse.o: ../include/postit.h
postprompts.o: postprompts.c ../include/mit-copyright.h /usr/include/stdio.h
postprompts.o: /usr/include/strings.h /usr/include/ctype.h
postprompts.o: /usr/include/fcntl.h /usr/include/errno.h /usr/include/gdb.h
postprompts.o: /usr/include/sys/types.h /usr/include/sys/time.h
postprompts.o: /usr/include/sys/time.h ../include/postit.h
postmisc.o: postmisc.c ../include/mit-copyright.h /usr/include/stdio.h
postmisc.o: /usr/include/strings.h /usr/include/ctype.h /usr/include/fcntl.h
postmisc.o: /usr/include/errno.h /usr/include/curses.h /usr/include/sgtty.h
postmisc.o: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
postmisc.o: /usr/include/sys/ttydev.h /usr/include/gdb.h
postmisc.o: /usr/include/sys/types.h /usr/include/sys/time.h
postmisc.o: /usr/include/sys/time.h ../include/postit.h
