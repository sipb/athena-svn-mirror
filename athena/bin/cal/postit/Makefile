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
#	$Author: shanzer $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/postit/Makefile,v 1.2 1987-10-29 12:38:11 shanzer Exp $
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
	rm -f makedep eddep 
	for i in ${ALLSRCS}; do \
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

TAGS:
	etags *.c ../util/*.c

# DO NOT DELETE THIS LINE -- make depend uses it
# DEPENDENCIES MUST END AT END OF FILE
postmaster.o: postmaster.c
postmaster.o: ../include/mit-copyright.h
postmaster.o: /usr/include/stdio.h
postmaster.o: /usr/include/strings.h
postmaster.o: /usr/include/ctype.h
postmaster.o: /usr/include/fcntl.h
postmaster.o: /usr/include/errno.h
postmaster.o: /usr/include/gdb.h
postmaster.o: /usr/include/sys/types.h
postmaster.o: /usr/include/sys/time.h
postmaster.o: /usr/include/time.h
postmaster.o: ../include/postit.h
bulkmaster.o: bulkmaster.c
bulkmaster.o: ../include/mit-copyright.h
bulkmaster.o: /usr/include/stdio.h
bulkmaster.o: /usr/include/strings.h
bulkmaster.o: /usr/include/ctype.h
bulkmaster.o: /usr/include/fcntl.h
bulkmaster.o: /usr/include/errno.h
bulkmaster.o: /usr/include/sys/types.h
bulkmaster.o: /usr/include/sys/stat.h
bulkmaster.o: /usr/include/gdb.h
bulkmaster.o: /usr/include/sys/types.h
bulkmaster.o: /usr/include/sys/time.h
bulkmaster.o: /usr/include/time.h
bulkmaster.o: ../include/postit.h
postprint.o: postprint.c
postprint.o: ../include/mit-copyright.h
postprint.o: /usr/include/stdio.h
postprint.o: /usr/include/strings.h
postprint.o: /usr/include/ctype.h
postprint.o: /usr/include/fcntl.h
postprint.o: /usr/include/errno.h
postprint.o: /usr/include/curses.h
postprint.o: /usr/include/stdio.h
postprint.o: /usr/include/sgtty.h
postprint.o: /usr/include/sys/ioctl.h
postprint.o: /usr/include/sys/ttychars.h
postprint.o: /usr/include/sys/ttydev.h
postprint.o: /usr/include/gdb.h
postprint.o: /usr/include/sys/types.h
postprint.o: /usr/include/sys/time.h
postprint.o: /usr/include/time.h
postprint.o: ../include/postit.h
postdata.o: postdata.c
postdata.o: ../include/mit-copyright.h
postdata.o: /usr/include/stdio.h
postdata.o: /usr/include/strings.h
postdata.o: /usr/include/ctype.h
postdata.o: /usr/include/fcntl.h
postdata.o: /usr/include/errno.h
postdata.o: /usr/include/sys/wait.h
postdata.o: /usr/include/gdb.h
postdata.o: /usr/include/sys/types.h
postdata.o: /usr/include/sys/time.h
postdata.o: /usr/include/time.h
postdata.o: ../include/postit.h
postparse.o: postparse.c
postparse.o: ../include/mit-copyright.h
postparse.o: /usr/include/stdio.h
postparse.o: /usr/include/strings.h
postparse.o: /usr/include/ctype.h
postparse.o: /usr/include/fcntl.h
postparse.o: /usr/include/errno.h
postparse.o: /usr/include/gdb.h
postparse.o: /usr/include/sys/types.h
postparse.o: /usr/include/sys/time.h
postparse.o: /usr/include/time.h
postparse.o: ../include/postit.h
postprompts.o: postprompts.c
postprompts.o: ../include/mit-copyright.h
postprompts.o: /usr/include/stdio.h
postprompts.o: /usr/include/strings.h
postprompts.o: /usr/include/ctype.h
postprompts.o: /usr/include/fcntl.h
postprompts.o: /usr/include/errno.h
postprompts.o: /usr/include/gdb.h
postprompts.o: /usr/include/sys/types.h
postprompts.o: /usr/include/sys/time.h
postprompts.o: /usr/include/time.h
postprompts.o: ../include/postit.h
postmisc.o: postmisc.c
postmisc.o: ../include/mit-copyright.h
postmisc.o: /usr/include/stdio.h
postmisc.o: /usr/include/strings.h
postmisc.o: /usr/include/ctype.h
postmisc.o: /usr/include/fcntl.h
postmisc.o: /usr/include/errno.h
postmisc.o: /usr/include/curses.h
postmisc.o: /usr/include/stdio.h
postmisc.o: /usr/include/sgtty.h
postmisc.o: /usr/include/sys/ioctl.h
postmisc.o: /usr/include/sys/ttychars.h
postmisc.o: /usr/include/sys/ttydev.h
postmisc.o: /usr/include/gdb.h
postmisc.o: /usr/include/sys/types.h
postmisc.o: /usr/include/sys/time.h
postmisc.o: /usr/include/time.h
postmisc.o: ../include/postit.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
