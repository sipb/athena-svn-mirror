# Makefile for "track" automatic update program
#
# 	$Source: /afs/dev.mit.edu/source/repository/athena/etc/track/tar/Makefile,v $
#	$Author: probe $
#	$Locker:  $
#	$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/tar/Makefile,v 1.1 1993-10-12 04:48:57 probe Exp $
DESTDIR=
INCDIR= /usr/include
CFLAGS=	-O -I${INCDIR}
LIBS= -ll

PROGS= track # nullmail

TRACK_OBJS= track.o y.tab.o stamp.o except.o files.o misc.o update.o cksum.o

TRACK_SRCS= track.c y.tab.c stamp.c except.c files.c misc.c update.c cksum.c

TRACK_DEP= track.h track.c stamp.c except.c files.c misc.c update.c nullmail.c

all: $(PROGS)

track: $(TRACK_OBJS)
	$(CC) -o track $(TRACK_OBJS) $(LIBS)

y.tab.o : y.tab.c lex.yy.c track.h
	cc -c y.tab.c

y.tab.c : sub_gram.y 
	yacc sub_gram.y

lex.yy.c : input.l
	@echo IGNORE FIVE "Non-Portable Character Class" WARNINGS --
	lex input.l

nullmail : nullmail.c
	$(CC) $(CFLAGS) -o nullmail nullmail.c
	
install:
	for i in $(PROGS); do \
		(install -c -s $$i $(DESTDIR)/etc/athena/$$i); \
	done
	chmod 4755 $(DESTDIR)/etc/athena/track
	(cd doc; make install ${MFLAGS} DESTDIR=${DESTDIR})

clean:
	/bin/rm -f a.out core *.o *~ y.tab.c lex.yy.c $(PROGS)

lint:
	lint -uahv $(TRACK_SRCS)

depend: $(TRACK_SRCS)
	${CC} -M -I../include ${TRACK_SRCS} | \
	sed -e ':loop' \
	    -e 's/\.\.\/[^ /]*\/\.\./../' \
	    -e 't loop' | \
	awk ' { if ($$1 != prev) { print rec; rec = $$0; prev = $$1; } \
		else { if (length(rec $$2) > 78) { print rec; rec = $$0; } \
		       else rec = rec " " $$2 } } \
	      END { print rec } ' > makedep
	echo '/^# DO NOT DELETE THIS LINE/+1,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile Makefile.bak
	ex - Makefile < eddep
	rm eddep makedep
	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
	echo '# see make depend above' >> Makefile

# DO NOT DELETE THIS LINE -- make depend uses it

track.o: track.c ./mit-copyright.h ./track.h ./mit-copyright.h
track.o: /usr/include/sys/types.h /usr/include/sys/stat.h
track.o: /usr/include/sys/dir.h /usr/include/sys/param.h
track.o: /usr/include/machine/machparam.h /usr/include/signal.h
track.o: /usr/include/sys/types.h /usr/include/sys/file.h /usr/include/ctype.h
track.o: /usr/include/signal.h /usr/include/stdio.h
y.tab.o: y.tab.c ./track.h ./mit-copyright.h /usr/include/sys/types.h
y.tab.o: /usr/include/sys/stat.h /usr/include/sys/dir.h
y.tab.o: /usr/include/sys/param.h /usr/include/machine/machparam.h
y.tab.o: /usr/include/signal.h /usr/include/sys/types.h /usr/include/sys/file.h
y.tab.o: /usr/include/ctype.h /usr/include/signal.h /usr/include/stdio.h
y.tab.o: ./lex.yy.c /usr/include/stdio.h
stamp.o: stamp.c ./mit-copyright.h ./track.h ./mit-copyright.h
stamp.o: /usr/include/sys/types.h /usr/include/sys/stat.h
stamp.o: /usr/include/sys/dir.h /usr/include/sys/param.h
stamp.o: /usr/include/machine/machparam.h /usr/include/signal.h
stamp.o: /usr/include/sys/types.h /usr/include/sys/file.h /usr/include/ctype.h
stamp.o: /usr/include/signal.h /usr/include/stdio.h
except.o: except.c ./mit-copyright.h ./track.h ./mit-copyright.h
except.o: /usr/include/sys/types.h /usr/include/sys/stat.h
except.o: /usr/include/sys/dir.h /usr/include/sys/param.h
except.o: /usr/include/machine/machparam.h /usr/include/signal.h
except.o: /usr/include/sys/types.h /usr/include/sys/file.h /usr/include/ctype.h
except.o: /usr/include/signal.h /usr/include/stdio.h
files.o: files.c ./mit-copyright.h ./track.h ./mit-copyright.h
files.o: /usr/include/sys/types.h /usr/include/sys/stat.h
files.o: /usr/include/sys/dir.h /usr/include/sys/param.h
files.o: /usr/include/machine/machparam.h /usr/include/signal.h
files.o: /usr/include/sys/types.h /usr/include/sys/file.h /usr/include/ctype.h
files.o: /usr/include/signal.h /usr/include/stdio.h
misc.o: misc.c ./mit-copyright.h ./track.h ./mit-copyright.h
misc.o: /usr/include/sys/types.h /usr/include/sys/stat.h /usr/include/sys/dir.h
misc.o: /usr/include/sys/param.h /usr/include/machine/machparam.h
misc.o: /usr/include/signal.h /usr/include/sys/types.h /usr/include/sys/file.h
misc.o: /usr/include/ctype.h /usr/include/signal.h /usr/include/stdio.h
update.o: update.c ./mit-copyright.h ./track.h ./mit-copyright.h
update.o: /usr/include/sys/types.h /usr/include/sys/stat.h
update.o: /usr/include/sys/dir.h /usr/include/sys/param.h
update.o: /usr/include/machine/machparam.h /usr/include/signal.h
update.o: /usr/include/sys/types.h /usr/include/sys/file.h /usr/include/ctype.h
update.o: /usr/include/signal.h /usr/include/stdio.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
