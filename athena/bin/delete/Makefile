#     Copyright 1988 Massachusetts Institute of Technology.
#
#     For copying and distribution information, see the file
#     "mit-copyright.h".
#
#     $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/Makefile,v $
#     $Author: jik $
#     $Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/Makefile,v 1.11 1989-10-23 13:46:21 jik Exp $
#

MACHINE= vax
DESTDIR =
TARGETS = delete undelete expunge purge lsdel
INSTALLDIR = /mit/jik/${MACHINE}bin
CC = cc
COMPILE_ET = compile_et
LINT = lint
LINTFLAGS =
LINTLIBS =
CFLAGS = -O
LIBS = -lcom_err
SRCS = delete.c undelete.c directories.c pattern.c util.c expunge.c \
	lsdel.c col.c shell_regexp.c delete_errs.c errors.c
INCS = col.h delete.h directories.h expunge.h lsdel.h mit-copyright.h\
	pattern.h undelete.h util.h shell_regexp.h delete_errs.h errors.h
MANS = man1/delete.1 man1/expunge.1 man1/lsdel.1 man1/purge.1\
	man1/undelete.1
ARCHIVE = README Makefile MANIFEST PATCHLEVEL $(SRCS) $(INCS) $(MANS)
ARCHIVEDIRS = man1

DELETEOBJS= delete.o util.o delete_errs.o errors.o
UNDELETEOBJS= undelete.o directories.o util.o pattern.o\
	shell_regexp.o delete_errs.o errors.o
EXPUNGEOBJS= expunge.o directories.o pattern.o util.o col.o\
	shell_regexp.o delete_errs.o errors.o
LSDELOBJS= lsdel.o util.o directories.o pattern.o col.o\
	shell_regexp.o delete_errs.o errors.o

DELETESRC= delete.c util.c delete_errs.c errors.o
UNDELETESRC= undelete.c directories.c util.c pattern.c\
	shell_regexp.c delete_errs.c errors.o
EXPUNGESRC= expunge.c directories.c pattern.c util.c col.c\
	shell_regexp.c delete_errs.c errors.o
LSDELSRC= lsdel.c util.c directories.c pattern.c col.c\
	shell_regexp.c delete_errs.c errors.o

.SUFFIXES: .c .h .et

.et.h: ; ${COMPILE_ET} $*.et
.et.c: ; ${COMPILE_ET} $*.et

all: $(TARGETS)

lint_all: lint_delete lint_undelete lint_expunge lint_lsdel

install: bin_install man_install

man_install:
	for i in $(TARGETS) ; do\
	  install -c man1/$$i.1 $(DESTDIR)/usr/man/man1 ; \
	done

bin_install: $(TARGETS)
	for i in $(TARGETS) ; do\
          if [ -f $(DESTDIR)$(INSTALLDIR)/$$i ]; then\
            mv $(DESTDIR)$(INSTALLDIR)/$$i $(DESTDIR)$(INSTALLDIR)/.#$$i ; \
          fi; \
	  install -c -s $$i $(DESTDIR)$(INSTALLDIR) ; \
        done

delete: $(DELETEOBJS)
	$(CC) $(CFLAGS) -o delete $(DELETEOBJS) $(LIBS)

saber_delete:
	#alias s step
	#alias n next
	#setopt program_name delete
	#load $(CFLAGS) $(DELETESRC) $(LIBS)

lint_delete: $(DELETESRC)
	$(LINT) $(LINTFLAGS) $(DELETESRC) $(LINTLIBS)

undelete: $(UNDELETEOBJS)
	$(CC) $(CFLAGS) -o undelete $(UNDELETEOBJS) $(LIBS)

saber_undelete:
	#alias s step
	#alias n next
	#setopt program_name undelete
	#load $(CFLAGS) $(UNDELETESRC) $(LIBS)

lint_undelete: $(UNDELETESRC)
	$(LINT) $(LINTFLAGS) $(UNDELETESRC) $(LINTLIBS)

expunge: $(EXPUNGEOBJS)
	$(CC) $(CFLAGS) -o expunge $(EXPUNGEOBJS) $(LIBS)

saber_expunge:
	#alias s step
	#alias n next
	#setopt program_name expunge
	#load $(CFLAGS) $(EXPUNGESRC) $(LIBS)

lint_expunge: $(EXPUNGESRC)
	$(LINT) $(LINTFLAGS) $(EXPUNGESRC) $(LINTLIBS)

purge: expunge
	ln -s expunge purge

lsdel: $(LSDELOBJS)
	$(CC) $(CFLAGS) -o lsdel $(LSDELOBJS) $(LIBS)

lint_lsdel: $(LSDELSRC)
	$(LINT) $(LINTFLAGS) $(LSDELSRC) $(LINTLIBS)

saber_lsdel:
	#alias s step
	#alias n next
	#setopt program_name lsdel
	#load $(CFLAGS) $(LSDELSRC) $(LIBS)

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
	makekit -oMANIFEST -h2 MANIFEST $(ARCHIVEDIRS) $(ARCHIVE)

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
	-rm -f *~ *.bak *.o delete undelete lsdel expunge purge\
		delete_errs.h delete_errs.c

depend: $(SRCS) $(INCS)
	/usr/athena/makedepend -v $(CFLAGS) -s'# DO NOT DELETE' $(SRCS)

# DO NOT DELETE THIS LINE -- makedepend depends on it
