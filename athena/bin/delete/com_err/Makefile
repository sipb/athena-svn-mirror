#
# Makefile for error-table routines
#
# Copyright 1987, 1989, 1991 MIT Student Information Processing Board
# For copyright info, see mit-sipb-copyright.h.
#
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/com_err/Makefile,v 1.9 1991-02-22 07:47:16 jik Exp $
#	$Locker:  $
#

RANLIB= 	ranlib

DEFINES= 
CFLAGS=		-O $(DEFINES)
LINTFLAGS= 	-uhvpb
LINTFILES= 	error_message.c et_name.c init_et.c com_err.c
LIBOBJS= 	error_message.o et_name.o init_et.o com_err.o

BINDIR=		/usr/bin
INCDIR=		/usr/include
LIBDIR=		/usr/lib
DOCDIR=		/usr/doc
DESTDIR=

CFILES=		com_err.c compile_et.c error_message.c et_name.c\
		init_et.c strcasecmp.c test.c
HFILES=		com_err.h compiler.h error_table.h internal.h\
		mit-sipb-copyright.h 
MANS=		com_err.3 compile_et.1
YACCS=		error_table.y
LEXS=		et_lex.lex.l
ETS=		test1.et test2.et
FILES=		Makefile PATCHLEVEL README\
		com_err.texinfo.Z.uu profiled texinfo.tex.Z.uu\
		$(CFILES) $(HFILES) $(MANS) $(YACCS) $(LEXS) $(ETS)


#
# what to build...
#
all:	compile_et libcom_err.a libcom_err_p.a

lint:	llib-lcom_err.ln

archive:	et.tar

doc:	com_err.dvi

#
# rules
#
.SUFFIXES: .h .c .et .ps .x9700 .mss .dvi .texinfo

.et.c:
	./compile_et $*.et

.et.h:
	./compile_et $*.et

.texinfo.dvi:
	tex $<

.dvi.ps:
	rm -f $@.new
	dvi2ps -r $< > $@.new
	mv $@.new $@

.c.o:
	${CC} -c -pg ${CFLAGS} $*.c
	mv $*.o profiled/$*.o
	${CC} -c ${CFLAGS} $*.c

#
# real entries...
#
compile_et:	compile_et.o error_table.o strcasecmp.o
	${CC} ${CFLAGS} -o compile_et compile_et.o error_table.o\
		strcasecmp.o -ll

tags:	TAGS

com_err.ps : com_err.dvi
com_err.dvi: com_err.texinfo

error_table.o: et_lex.lex.c

install: all
	install -c -s compile_et ${DESTDIR}${BINDIR}/compile_et
	install -c -m 444 com_err.h ${DESTDIR}${INCDIR}/com_err.h
	install -c -m 444 mit-sipb-copyright.h \
				${DESTDIR}${INCDIR}/mit-sipb-copyright.h
	install -c libcom_err.a ${DESTDIR}${LIBDIR}/libcom_err.a
	${RANLIB} ${DESTDIR}${LIBDIR}/libcom_err.a
	install -c libcom_err_p.a ${DESTDIR}${LIBDIR}/libcom_err_p.a
	${RANLIB} ${DESTDIR}${LIBDIR}/libcom_err_p.a
	install -c com_err.texinfo ${DESTDIR}${DOCDIR}/com_err.texinfo
	install -c com_err.3 ${DESTDIR}/usr/man/man3/com_err.3
	install -c compile_et.1 ${DESTDIR}/usr/man/man1/compile_et.1

TAGS:	et_name.c error_message.c compile_et.c error_table.c \
		lex.yy.c perror.c init_et.c
	etags et_name.c error_message.c compile_et.c \
		error_table.c perror.c init_et.c

libcom_err.a:	$(LIBOBJS)
	ar cruv libcom_err.a $(LIBOBJS)
	${RANLIB} libcom_err.a

libcom_err_p.a:	$(LIBOBJS)
	(cd profiled; ar uv ../libcom_err_p.a $(LIBOBJS); \
		${RANLIB} ../libcom_err_p.a)

libcom_err.o:	$(LIBOBJS)
	ld -r -s -o libcom_err.o $(LIBOBJS)
	chmod -x libcom_err.o

llib-lcom_err.ln: $(LINTFILES)
	lint -Ccom_err $(LINTFLAGS) $(LINTFILES)

clean:
	rm -f *~ \#* *.bak \
		*.otl *.aux *.toc *.PS *.dvi *.x9700 *.ps \
		*.cp *.fn *.ky *.log *.pg *.tp *.vr \
		*.o profiled/*.o libcom_err.a libcom_err_p.a \
		com_err.o compile_et \
		et.ar TAGS y.tab.c lex.yy.c error_table.c \
		et_lex.lex.c \
		test1.h test1.c test2.h test2.c test \
		eddep makedep

patch: $(FILES)
	makepatch $(FILES)
	mv patch com_err.patch`cat PATCHLEVEL`
	shar com_err.patch`cat PATCHLEVEL` > com_err.patch`cat PATCHLEVEL`.shar

shar: $(FILES)
	makekit -oMANIFEST $(FILES)

tar: $(FILES)
	tar cvf - $(FILES) | compress > com_err.tar.Z

# for testing
test:	test.o test1.o test2.o libcom_err.a
	cc ${CFLAGS} -o test test.o test1.o test2.o libcom_err.a
test.o:	test1.h test2.h
test1.o : test1.c
test1.c : test1.et
test2.o : test2.c
test2.c : test2.et
# 'make depend' code
depend: ${CFILES} et_lex.lex.c
	rm -f eddep makedep
	/bin/cc -M ${CFLAGS} ${CFILES} | \
	sed 's; ./; ;' | \
	sed -e ':loop' \
	    -e 's/\.\.\/[^ /]*\/\.\./../' \
	    -e 't loop' > makedep
# add a blank line so that if there are no dependencies, ed
# won't report an error (it does on some 
	echo "$$" > eddep
	echo "a" >> eddep
	echo "" >> eddep
	echo "." >> eddep
	echo '/^# DO NOT DELETE THIS LINE/+1,$$d' >> eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile Makefile.bak
	ed - Makefile < eddep
	rm eddep makedep
#
# the last line in the makefile should be...
# DO NOT DELETE THIS LINE -- make depend depends on it.
