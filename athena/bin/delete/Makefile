#     Copyright 1988 Massachusetts Institute of Technology.
#
#     For copying and distribution information, see the file
#     "mit-copyright.h".
#
#     $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/Makefile,v $
#     $Author: jik $
#     $Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/Makefile,v 1.13 1989-11-06 21:33:08 jik Exp $
#

DESTDIR=
TARGETS= 	delete undelete expunge purge lsdel
INSTALLDIR= 	/bin/athena
CC= 		cc
COMPILE_ET= 	compile_et
LINT= 		lint
DEFINES=	-DAFS_MOUNTPOINTS
INCLUDES=	-I/afs/athena.mit.edu/astaff/project/afsdev/$(MACHINE)
CFLAGS= 	-O $(INCLUDES) $(DEFINES) $(CDEBUGFLAGS)
LDFLAGS=	-L/usr/athena/lib\
		-L/afs/athena.mit.edu/astaff/project/afsdev/build/$(MACHINE)/lib/afs
LIBS= 		-lcom_err -lsys
LINTFLAGS=	$(DEFINES) $(INCLUDES) $(CDEBUGFLAGS) -u
LINTLIBS=	
SRCS= 		delete.c undelete.c directories.c pattern.c util.c\
		expunge.c lsdel.c col.c shell_regexp.c delete_errs.c\
		errors.c stack.c
INCS= 		col.h delete.h directories.h expunge.h lsdel.h\
		mit-copyright.h pattern.h undelete.h util.h\
		shell_regexp.h delete_errs.h errors.h stack.h
MANS= 		man1/delete.1 man1/expunge.1 man1/lsdel.1 man1/purge.1\
		man1/undelete.1
ARCHIVE=	README Makefile MANIFEST PATCHLEVEL $(SRCS) $(INCS)\
		$(MANS) 
ARCHIVEDIRS= 	man1

DELETEOBJS= 	delete.o util.o delete_errs.o errors.o
UNDELETEOBJS= 	undelete.o directories.o util.o pattern.o\
		shell_regexp.o delete_errs.o errors.o stack.o
EXPUNGEOBJS= 	expunge.o directories.o pattern.o util.o col.o\
		shell_regexp.o delete_errs.o errors.o stack.o
LSDELOBJS= 	lsdel.o util.o directories.o pattern.o col.o\
		shell_regexp.o delete_errs.o errors.o stack.o

DELETESRC= 	delete.c util.c delete_errs.c errors.c
UNDELETESRC= 	undelete.c directories.c util.c pattern.c\
		shell_regexp.c delete_errs.c errors.c stack.c
EXPUNGESRC= 	expunge.c directories.c pattern.c util.c col.c\
		shell_regexp.c delete_errs.c errors.c stack.c
LSDELSRC= 	lsdel.c util.c directories.c pattern.c col.c\
		shell_regexp.c delete_errs.c errors.c stack.c

.SUFFIXES: .c .h .et

.et.h: $*.et
	${COMPILE_ET} $*.et
.et.c: $*.et
	${COMPILE_ET} $*.et

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
	$(CC) $(LDFLAGS) $(CFLAGS) -o delete $(DELETEOBJS) $(LIBS)

saber_delete:
	#setopt program_name delete
	#load $(LDFLAGS) $(CFLAGS) $(DELETESRC) $(LIBS)

lint_delete: $(DELETESRC)
	$(LINT) $(LINTFLAGS) $(DELETESRC) $(LINTLIBS)

undelete: $(UNDELETEOBJS)
	$(CC) $(LDFLAGS) $(CFLAGS) -o undelete $(UNDELETEOBJS) $(LIBS)

saber_undelete:
	#setopt program_name undelete
	#load $(LDFLAGS) $(CFLAGS) $(UNDELETESRC) $(LIBS)

lint_undelete: $(UNDELETESRC)
	$(LINT) $(LINTFLAGS) $(UNDELETESRC) $(LINTLIBS)

expunge: $(EXPUNGEOBJS)
	$(CC) $(LDFLAGS) $(CFLAGS) -o expunge $(EXPUNGEOBJS) $(LIBS)

saber_expunge:
	#setopt program_name expunge
	#load $(LDFLAGS) $(CFLAGS) $(EXPUNGESRC) $(LIBS)

lint_expunge: $(EXPUNGESRC)
	$(LINT) $(LINTFLAGS) $(EXPUNGESRC) $(LINTLIBS)

purge: expunge
	ln -s expunge purge

lsdel: $(LSDELOBJS)
	$(CC) $(LDFLAGS) $(CFLAGS) -o lsdel $(LSDELOBJS) $(LIBS)

lint_lsdel: $(LSDELSRC)
	$(LINT) $(LINTFLAGS) $(LSDELSRC) $(LINTLIBS)

saber_lsdel:
	#setopt program_name lsdel
	#load $(LDFLAGS) $(CFLAGS) $(LSDELSRC) $(LIBS)

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

$(DELETESRC): delete_errs.h
$(EXPUNGESRC): delete_errs.h
$(UNDELETESRC): delete_errs.h
$(LSDELSRC): delete_errs.h

# DO NOT DELETE THIS LINE -- makedepend depends on it
