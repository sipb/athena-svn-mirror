# Makefile for "track" automatic update program
#
# 	$Source: /afs/dev.mit.edu/source/repository/athena/etc/track/Makefile,v $
#	$Author: rfrench $
#	$Locker:  $
#	$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/Makefile,v 1.2 1987-03-02 08:01:46 rfrench Exp $
DESTDIR=
INCDIR= /usr/include
CFLAGS=	-O -I${INCDIR}
LIBS= -ll

PROGS= track nullmail

TRACK_OBJS= track.o y.tab.o stamp.o except.o files.o misc.o update.o

TRACK_SRCS= track.c y.tab.c stamp.c except.c files.c misc.c update.c

TRACK_DEP= track.h track.c stamp.c except.c files.c misc.c update.c nullmail.c

all: $(PROGS)

track: $(TRACK_OBJS)
	$(CC) -o track $(TRACK_OBJS) $(LIBS)

y.tab.o : y.tab.c lex.yy.c track.h
	cc -c y.tab.c

y.tab.c : sub_gram.y 
	yacc sub_gram.y

lex.yy.c : input.l
	@echo IGNORE THE WARNINGS --
	lex input.l

nullmail : nullmail.c
	$(CC) $(CFLAGS) -o nullmail nullmail.c
	
install:
	for i in $(PROGS); do \
		(install -c -s $$i $(DESTDIR)/etc/athena/$$i); \
	done

clean:
	/bin/rm -f a.out core *.o *~ y.tab.c lex.yy.c $(PROGS)

lint:
	lint -uahv $(TRACK_SRCS)

depend:
	cat </dev/null >x.c
	for i in $(TRACK_DEP); do \
		(/bin/grep '^#[ 	]*include' x.c $$i | sed \
			-e '/\.\.\/h/d' \
			-e 's,<\(.*\)>,"${INCDIR}/\1",' \
			-e 's/:[^"]*"\([^"]*\)".*/: \1/' \
			-e 's/\.c//' >>makedep); done
	echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile Makefile.bak
	ed - Makefile < eddep
	rm eddep makedep x.c
	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
	echo '# see make depend above' >> Makefile

# DO NOT DELETE THIS LINE -- make depend uses it
# DEPENDENCIES MUST END AT END OF FILE
track.h: mit-copyright.h
track.h: /usr/include/sys/types.h
track.h: /usr/include/sys/stat.h
track.h: /usr/include/sys/dir.h
track.h: /usr/include/sys/param.h
track.h: /usr/include/sys/file.h
track.h: /usr/include/ctype.h
track.h: /usr/include/signal.h
track.h: /usr/include/stdio.h
track: mit-copyright.h
track: track.h
stamp: mit-copyright.h
stamp: track.h
except: mit-copyright.h
except: track.h
files: mit-copyright.h
files: track.h
misc: mit-copyright.h
misc: track.h
update: mit-copyright.h
update: track.h
nullmail: /usr/include/stdio.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
