#     Copyright 1988 Massachusetts Institute of Technology.
#
#     For copying and distribution information, see the file
#     "mit-copyright.h".
#
#     $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/Makefile,v $
#     $Author: jik $
#     $Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/Makefile,v 1.7 1989-03-08 09:40:27 jik Exp $
#

DESTDIR =
TARGETS = delete undelete expunge purge lsdel
INSTALLDIR = /bin/athena
CC = cc
CFLAGS = -O
SRCS = delete.c undelete.c directories.c pattern.c util.c expunge.c \
	lsdel.c col.c

all: $(TARGETS)

install:
	for i in $(TARGETS) ;\
	do \
	install -c -s $$i $(DESTDIR)$(INSTALLDIR) ; \
	install -c man1/$$i.1 $(DESTDIR)/usr/man/man1 ; \
	done

delete: delete.o util.o
	cc $(CFLAGS) -o delete delete.o util.o

saber_delete:
	#alias s step
	#alias n next
	#load delete.c util.c

undelete: undelete.o directories.o util.o pattern.o
	cc $(CFLAGS) -o undelete undelete.o directories.o util.o pattern.o

saber_undelete:
	#alias s step
	#alias n next
	#load undelete.c directories.c util.c pattern.c

expunge: expunge.o directories.o pattern.o util.o col.o
	cc $(CFLAGS) -o expunge expunge.o directories.o pattern.o \
		util.o col.o


saber_expunge:
	#alias s step
	#alias n next
	#load expunge.c directories.c pattern.c util.c col.c

purge: expunge
	ln -s expunge purge

lsdel: lsdel.o util.o directories.o pattern.o col.o
	cc $(CFLAGS) -o lsdel lsdel.o util.o directories.o pattern.o col.o

saber_lsdel:
	#alias s step
	#alias n next
	#load lsdel.c util.c directories.c pattern.c col.c

clean:
	-rm -f *~ *.bak *.o delete undelete lsdel expunge purge

depend: delete.c undelete.c
	/usr/athena/makedepend -v $(CFLAGS) -s'# DO NOT DELETE' $(SRCS)

# DO NOT DELETE THIS LINE -- makedepend depends on it

delete.o: /usr/include/sys/types.h /usr/include/stdio.h
delete.o: /usr/include/sys/stat.h /usr/include/sys/dir.h
delete.o: /usr/include/strings.h /usr/include/sys/param.h
# /usr/include/sys/param.h includes:
#	machine/machparam.h
#	signal.h
#	sys/types.h
delete.o: /usr/include/machine/machparam.h /usr/include/sys/signal.h
delete.o: /usr/include/sys/file.h util.h delete.h
undelete.o: /usr/include/stdio.h /usr/include/sys/types.h
undelete.o: /usr/include/sys/dir.h /usr/include/sys/param.h
undelete.o: /usr/include/machine/machparam.h /usr/include/sys/signal.h
undelete.o: /usr/include/strings.h /usr/include/sys/stat.h directories.h
undelete.o: pattern.h util.h undelete.h
directories.o: /usr/include/sys/types.h /usr/include/sys/stat.h
directories.o: /usr/include/sys/param.h /usr/include/machine/machparam.h
directories.o: /usr/include/sys/signal.h /usr/include/sys/dir.h
directories.o: /usr/include/strings.h directories.h util.h
pattern.o: /usr/include/stdio.h /usr/include/sys/types.h
pattern.o: /usr/include/sys/dir.h /usr/include/sys/param.h
pattern.o: /usr/include/machine/machparam.h /usr/include/sys/signal.h
pattern.o: /usr/include/strings.h /usr/include/sys/stat.h directories.h
pattern.o: pattern.h util.h undelete.h
util.o: /usr/include/stdio.h /usr/include/sys/param.h
util.o: /usr/include/machine/machparam.h /usr/include/sys/signal.h
util.o: /usr/include/sys/types.h /usr/include/sys/stat.h
util.o: /usr/include/sys/dir.h /usr/include/strings.h /usr/include/pwd.h
util.o: directories.h util.h
expunge.o: /usr/include/stdio.h /usr/include/sys/types.h
expunge.o: /usr/include/sys/time.h
# /usr/include/sys/time.h includes:
#	time.h
expunge.o: /usr/include/sys/time.h /usr/include/sys/dir.h
expunge.o: /usr/include/sys/param.h /usr/include/machine/machparam.h
expunge.o: /usr/include/sys/signal.h /usr/include/strings.h
expunge.o: /usr/include/sys/stat.h col.h directories.h util.h pattern.h
expunge.o: expunge.h
lsdel.o: /usr/include/stdio.h /usr/include/sys/types.h /usr/include/sys/dir.h
lsdel.o: /usr/include/sys/param.h /usr/include/machine/machparam.h
lsdel.o: /usr/include/sys/signal.h /usr/include/sys/stat.h
lsdel.o: /usr/include/strings.h col.h util.h directories.h pattern.h lsdel.h
col.o: /usr/include/stdio.h /usr/include/strings.h col.h
