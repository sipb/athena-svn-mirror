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
#	$Author: shanzer $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/util/Makefile,v 1.1 1987-10-29 12:34:53 shanzer Exp $
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
	rm -f makedep eddep 
	for i in ${SRCS} ; do \
		(${CC} ${CFLAGS} -M $$i >> makedep); \
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
what_lib.o: what_lib.c
what_lib.o: ../include/mit-copyright.h
what_lib.o: /usr/include/stdio.h
what_lib.o: /usr/include/curses.h
what_lib.o: /usr/include/stdio.h
what_lib.o: /usr/include/sgtty.h
what_lib.o: /usr/include/sys/ioctl.h
what_lib.o: /usr/include/sys/ttychars.h
what_lib.o: /usr/include/sys/ttydev.h
what_lib.o: /usr/include/gdb.h
what_lib.o: /usr/include/sys/types.h
what_lib.o: /usr/include/sys/time.h
what_lib.o: /usr/include/time.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
