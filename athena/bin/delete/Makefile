#     Copyright 1988 Massachusetts Institute of Technology.
#
#     For copying and distribution information, see the file
#     "mit-copyright.h".
#
#     $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/Makefile,v $
#     $Author: jik $
#     $Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/Makefile,v 1.19 1990-06-07 22:28:09 jik Exp $
#

DESTDIR=
TARGETS= 	delete undelete expunge purge lsdel
INSTALLDIR= 	/bin/athena
MANDIR=		/usr/man
MANSECT=	1
CC= 		cc
COMPILE_ET= 	compile_et
LINT= 		lint
DEFINES=	$(AFSDEFINES)


# These variables apply only if you want this program to recognize
# Andrew File System mount points.  If you don't want to support AFS,
# then set all the variables starting with "AFS" to nothing.
AFSBLD=		bld
AFSINC=		/afs/athena.mit.edu/astaff/project/afsdev/sandbox/$(AFSBLD)/dest/include
AFSLIB=		/afs/athena.mit.edu/astaff/project/afsdev/sandbox/$(AFSBLD)/dest/lib
AFSINCS=	-I$(AFSINC)
AFSLDFLAGS=	-L$(AFSLIB) -L$(AFSLIB)/afs
AFSLIBS=	-lsys -lrx -llwp $(AFSLIB)/afs/util.a
AFSDEFINES=	-DAFS_MOUNTPOINTS


# ETINCS is a -I flag pointing to the directory in which the et header
# files are stored. 
# ETLDFLAGS is a -L flag pointing to the directory where the et
# library is stored.
# ETLIBS lists the et libraries we want to link against
ETINCS=		-I/usr/include
ETLDFLAGS=	-L/usr/athena/lib
ETLIBS=		-lcom_err


# You probably won't have to edit anything below this line.

INCLUDES=	$(ETINCS) $(AFSINCS)
LDFLAGS=	$(ETLDFLAGS) $(AFSLDFLAGS) 
LIBS= 		$(ETLIBS) $(AFSLIBS)
CFLAGS= 	-O $(INCLUDES) $(DEFINES) $(CDEBUGFLAGS)
LINTFLAGS=	-u $(INCLUDES) $(DEFINES) $(CDEBUGFLAGS)
LINTLIBS=	

SRCS= 		delete.c undelete.c directories.c pattern.c util.c\
		expunge.c lsdel.c col.c shell_regexp.c\
		errors.c stack.c
INCS= 		col.h delete.h directories.h expunge.h lsdel.h\
		mit-copyright.h pattern.h undelete.h util.h\
		shell_regexp.h errors.h stack.h
ETS=		delete_errs.h delete_errs.c
ETSRCS=		delete_errs.et

MANS= 		man1/delete.1 man1/expunge.1 man1/lsdel.1 man1/purge.1\
		man1/undelete.1

ETLIBSRCS=	et/Makefile et/com_err.3 et/compile_et.1\
		et/com_err.texinfo.Z.uu et/error_table.y et/et_lex.lex.l\
		et/texinfo.tex.Z.uu et/*.c et/*.h et/*.et
ARCHIVE=	README Makefile PATCHLEVEL $(SRCS) $(INCS) $(ETSRCS)\
		$(MANS) 
ARCHIVEDIRS= 	man1 et et/profiled

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

# Errors are ignored on bin_install and man_install because make on
# some platforms, in combination with the shell, does really stupid
# things and detects an error where there is none.

man_install:
	-for i in $(TARGETS) ; do\
	  install -c man1/$$i.1\
		$(DESTDIR)$(MANDIR)/man$(MANSECT)/$$i.$(MANSECT);\
	done

bin_install: $(TARGETS)
	-for i in $(TARGETS) ; do\
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
	tar cvf - $(ARCHIVE) $(ETLIBSRCS) | compress > delete.tar.Z

shar: $(ARCHIVE)
	makekit -oMANIFEST -h2 MANIFEST $(ARCHIVEDIRS) $(ARCHIVE) $(ETLIBSRCS)

patch: $(ARCHIVE)
	makepatch $(ARCHIVE)
	mv patch delete.patch`cat PATCHLEVEL`
	shar delete.patch`cat PATCHLEVEL` > delete.patch`cat PATCHLEVEL`.shar

clean:
	-rm -f *~ *.bak *.o delete undelete lsdel expunge purge\
		delete_errs.h delete_errs.c

depend: $(SRCS) $(INCS) $(ETS)
	/usr/athena/makedepend -v $(CFLAGS) -s'# DO NOT DELETE' $(SRCS)

$(DELETEOBJS): delete_errs.h
$(EXPUNGEOBJS): delete_errs.h
$(UNDELETEOBJS): delete_errs.h
$(LSDELOBJS): delete_errs.h

# DO NOT DELETE THIS LINE -- makedepend depends on it
