#     Copyright 1988 Massachusetts Institute of Technology.
#
#     For copying and distribution information, see the file
#     "mit-copyright.h".
#
#     $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/Makefile,v $
#     $Author: jik $
#     $Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/Makefile,v 1.9 1989-05-04 14:05:48 jik Exp $
#

DESTDIR =
TARGETS = delete undelete expunge purge lsdel
INSTALLDIR = /bin/athena
CC = cc
CFLAGS = -O
SRCS = delete.c undelete.c directories.c pattern.c util.c expunge.c \
	lsdel.c col.c
INCS = col.h delete.h directories.h expunge.h lsdel.h mit-copyright.h\
	pattern.h undelete.h util.h
MANS = man1 man1/delete.1 man1/expunge.1 man1/lsdel.1 man1/purge.1\
	man1/undelete.1
ARCHIVE = README Makefile MANIFEST PATCHLEVEL $(SRCS) $(INCS) $(MANS)

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

tar: $(ARCHIVE)
	@echo "Checking to see if everything's checked in...."
	@for i in $(ARCHIVE) ;\
	do \
	if [ -w $$i ] ; then \
		echo "$$i isn't checked in.  Check it in before making"; \
		echo "an archive."; \
		exit 1; \
	fi ; \
	exit 0; \
	done
	tar cvf - $(ARCHIVE) | compress > delete.tar.Z

shar: $(ARCHIVE)
	@echo "Checking to see if everything's checked in...."
	@for i in $(ARCHIVE) ;\
	do \
	if [ -w $$i ] ; then \
		echo "$$i isn't checked in.  Check it in before making"; \
		echo "an archive."; \
		exit 1; \
	fi ; \
	exit 0; \
	done
	makekit -oMANIFEST -h2 MANIFEST $(ARCHIVE)

patch: $(ARCHIVE)
	@echo "Checking to see if everything's checked in...."
	@for i in $(ARCHIVE) ;\
	do \
	if [ -w $$i ] ; then \
		echo "$$i isn't checked in.  Check it in before making"; \
		echo "an archive."; \
		exit 1; \
	fi ; \
	exit 0; \
	done
	makepatch $(ARCHIVE)
	mv patch delete.patch`cat PATCHLEVEL`
	shar delete.patch`cat PATCHLEVEL` > delete.patch`cat PATCHLEVEL`.shar

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
delete.o: /usr/include/sys/file.h util.h
# util.h includes:
#	mit-copyright.h
delete.o: mit-copyright.h delete.h
# delete.h includes:
#	mit-copyright.h
undelete.o: /usr/include/stdio.h /usr/include/sys/types.h
undelete.o: /usr/include/sys/dir.h /usr/include/sys/param.h
undelete.o: /usr/include/machine/machparam.h /usr/include/sys/signal.h
undelete.o: /usr/include/strings.h /usr/include/sys/stat.h directories.h
# directories.h includes:
#	mit-copyright.h
undelete.o: mit-copyright.h pattern.h
# pattern.h includes:
#	mit-copyright.h
undelete.o: util.h undelete.h
# undelete.h includes:
#	mit-copyright.h
directories.o: /usr/include/sys/types.h /usr/include/sys/stat.h
directories.o: /usr/include/sys/param.h /usr/include/machine/machparam.h
directories.o: /usr/include/sys/signal.h /usr/include/sys/dir.h
directories.o: /usr/include/strings.h directories.h mit-copyright.h util.h
pattern.o: /usr/include/stdio.h /usr/include/sys/types.h
pattern.o: /usr/include/sys/dir.h /usr/include/sys/param.h
pattern.o: /usr/include/machine/machparam.h /usr/include/sys/signal.h
pattern.o: /usr/include/strings.h /usr/include/sys/stat.h directories.h
pattern.o: mit-copyright.h pattern.h util.h undelete.h
util.o: /usr/include/stdio.h /usr/include/sys/param.h
util.o: /usr/include/machine/machparam.h /usr/include/sys/signal.h
util.o: /usr/include/sys/types.h /usr/include/sys/stat.h
util.o: /usr/include/sys/dir.h /usr/include/strings.h /usr/include/pwd.h
util.o: directories.h mit-copyright.h util.h
expunge.o: /usr/include/stdio.h /usr/include/sys/types.h
expunge.o: /usr/include/sys/time.h
# /usr/include/sys/time.h includes:
#	time.h
expunge.o: /usr/include/sys/time.h /usr/include/sys/dir.h
expunge.o: /usr/include/sys/param.h /usr/include/machine/machparam.h
expunge.o: /usr/include/sys/signal.h /usr/include/strings.h
expunge.o: /usr/include/sys/stat.h col.h
# col.h includes:
#	mit-copyright.h
expunge.o: mit-copyright.h directories.h util.h pattern.h expunge.h
# expunge.h includes:
#	mit-copyright.h
lsdel.o: /usr/include/stdio.h /usr/include/sys/types.h /usr/include/sys/dir.h
lsdel.o: /usr/include/sys/param.h /usr/include/machine/machparam.h
lsdel.o: /usr/include/sys/signal.h /usr/include/sys/stat.h
lsdel.o: /usr/include/strings.h col.h mit-copyright.h util.h directories.h
lsdel.o: pattern.h lsdel.h
# lsdel.h includes:
#	mit-copyright.h
col.o: /usr/include/stdio.h /usr/include/strings.h col.h mit-copyright.h
